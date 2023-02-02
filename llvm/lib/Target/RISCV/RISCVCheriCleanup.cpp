//===----- RISCVCheriCleanup.cpp - Optimise address calculations  ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Merge the offset of address calculation into the offset field
// of instructions in a global address lowering sequence. This pass transforms:
//   cincoffset vreg2, vreg1, Offset
//   l[bdhw] vreg3, verg2, 0
//
//   Into:
//   l[bdhw] vreg3, verg2, Offset
//
//   If the incoffset is a %lo relocation relative to %gp, this also inserts a
//   CSetBounds
//
//===----------------------------------------------------------------------===//

#include "RISCV.h"
#include "RISCVTargetMachine.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include <set>
using namespace llvm;

#define DEBUG_TYPE "riscv-cheri-fixup"
#define RISCV_CHERI_CLEANUP_NAME "RISCV CHERI Cleanup"
namespace {

/// Expand CLLC for the tiny code model and try to minimise the number of
/// instructions.
struct RISCVCheriCleanupOpt : public MachineFunctionPass {
  static char ID;
  const MachineFunction *MF;
  bool runOnMachineFunction(MachineFunction &Fn) override;

  RISCVCheriCleanupOpt() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return RISCV_CHERI_CLEANUP_NAME; }

  /// Early expand the CLLC pseudo.  For the tiny code model, mutable globals
  /// are accessed via small immediate offsets relative to $cpg.
  Optional<std::pair<MachineInstr &, Register>>
  expandCLLC(MachineBasicBlock &Mbb, MachineInstr &Cllc) {
    Register DestReg = Cllc.getOperand(0).getReg();
    const MachineOperand &Symbol = Cllc.getOperand(1);
    const GlobalValue *GV = Symbol.getGlobal();
    MachineInstr *IncOffset = nullptr;
    // Function pointers will be late expanded
    if (isa<Function>(GV))
      return {};
    DebugLoc Dl = Cllc.getDebugLoc();
    // FIXME: We should put these in PCC once we have a sensible instruction
    // for materialising the address.
    if (cast<GlobalVariable>(GV)->isConstant()) {
    }
    IncOffset =
        BuildMI(Mbb, Cllc, Dl, TII->get(RISCV::CIncOffsetImm), DestReg)
            .addReg(RISCV::C3)
            .addDisp(Symbol, 0, RISCVII::MO_CHERI_COMPARTMENT_GLOBAL);
    Cllc.eraseFromParent();
    return {{*IncOffset, DestReg}};
  }

private:
  MachineRegisterInfo *MRI;
  const RISCVInstrInfo *TII;
};
} // end anonymous namespace

char RISCVCheriCleanupOpt::ID = 0;
INITIALIZE_PASS(RISCVCheriCleanupOpt, DEBUG_TYPE, RISCV_CHERI_CLEANUP_NAME,
                false, false)

bool RISCVCheriCleanupOpt::runOnMachineFunction(MachineFunction &Fn) {
  if (Fn.getTarget().getCodeModel() != CodeModel::Tiny)
    return false;
  TII = static_cast<const RISCVInstrInfo *>(Fn.getSubtarget().getInstrInfo());
  auto &RegInfo = Fn.getRegInfo();
  bool Modified = false;
  MRI = &Fn.getRegInfo();
  SmallVector<std::pair<MachineBasicBlock &, MachineInstr &>, 8> Cllcs;
  // Collect all CLLC pseudos.
  for (MachineBasicBlock &MBB : Fn) {
    LLVM_DEBUG(dbgs() << "MBB: " << MBB.getName() << "\n");
    for (MachineInstr &I : MBB)
      if (I.getOpcode() == RISCV::PseudoCLLC)
        Cllcs.push_back({MBB, I});
  }

  for (auto [Mbb, Cllc] : Cllcs) {
    auto Expand = expandCLLC(Mbb, Cllc);
    if (!Expand)
      continue;
    Modified = true;
    auto [IncOffset, DestReg] = *Expand;
    if (!DestReg.isVirtual())
      continue;
    // If the uses are normal load / store instructions, try folding the
    // CIncOffset's immediate into the load / store's immediate.  Do this only
    // if the existing offset is 0, because that's the only use that's
    // guaranteed in-bounds.
    SmallVector<std::reference_wrapper<MachineInstr>> Uses;
    // Collect all uses that are suitable for folding, without mutating the
    // use-def chain while we're walking it.
    for (auto &Use : RegInfo.use_instructions(DestReg)) {
      auto target = Use.getOperand(1);
      switch (Use.getOpcode()) {
      default:
        continue;
      case RISCV::CLC_64:
      case RISCV::CSC_64:
        if (!target.isReg() || (target.getReg() != DestReg))
          continue;
        LLVM_FALLTHROUGH;
      case RISCV::CLB:
      case RISCV::CLH:
      case RISCV::CLBU:
      case RISCV::CLHU:
      case RISCV::CLW:
      case RISCV::CSB:
      case RISCV::CSH:
      case RISCV::CSW:
        if (Use.getOperand(2).getImm() == 0)
          Uses.push_back(Use);
      }
    }
    // Fold the CIncOffset into the load or store.
    for (auto &UseRef : Uses) {
      auto &Use = UseRef.get();
      Use.getOperand(1).setReg(RISCV::C3);
      Use.getOperand(2) = IncOffset.getOperand(2);
    }
    // If there are no more uses, delete the CSetOffset.  If there *are*
    // remaining uses, then set the bounds on this register.
    if (RegInfo.use_empty(DestReg))
      IncOffset.eraseFromParent();
    else {
      auto Reg = RegInfo.cloneVirtualRegister(DestReg);
      IncOffset.getOperand(0).setReg(Reg);

      BuildMI(Mbb, ++MachineBasicBlock::instr_iterator(IncOffset),
              IncOffset.getDebugLoc(), TII->get(RISCV::CSetBoundsImm), DestReg)
          .addReg(Reg)
          .addDisp(IncOffset.getOperand(2), 0, RISCVII::MO_CHERI_COMPARTMENT_SIZE);
    }
  }

  return Modified;
}

/// Returns an instance of the Merge Base Offset Optimization pass.
FunctionPass *llvm::createRISCVCheriCleanupOptPass() {
  return new RISCVCheriCleanupOpt();
}
