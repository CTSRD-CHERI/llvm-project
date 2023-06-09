//===-- CheriGetAddressElim.cpp ------------------------------------------ -==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass removes instructions that extract the capability address and
// replaces them with a subregister read if we determine that nothing will be
// able to observe that the metadata bits are not yet cleared. This only
// works on architectures that use a merged register file for capabilities.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#include <set>

using namespace llvm;

#define DEBUG_TYPE "cheri-getaddress-elim"
#define CHERI_GETADDRESS_ELIM_NAME "Cheri Get Address Elimination"

namespace {
class CheriGetAddressElim : public MachineFunctionPass {
public:
  static char ID;
  CheriGetAddressElim() : MachineFunctionPass(ID) {}
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnMachineFunction(MachineFunction &MF) override;
  StringRef getPassName() const override { return CHERI_GETADDRESS_ELIM_NAME; }
};
} // end anonymous namespace

char CheriGetAddressElim::ID = 0;

INITIALIZE_PASS(CheriGetAddressElim, "cheri-getaddress-elim",
                CHERI_GETADDRESS_ELIM_NAME, false, false)

void CheriGetAddressElim::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  MachineFunctionPass::getAnalysisUsage(AU);
}

FunctionPass *llvm::createCheriGetAddressElimPass() {
  return new CheriGetAddressElim();
}

static bool couldReadCapMetadata(MachineInstr *MI,
                                 MachineRegisterInfo &MRI,
                                 const TargetRegisterInfo *TRI,
                                 MVT CapTy) {
  std::vector<MachineInstr*> WorkList;
  std::set<MachineInstr*> Seen;
  WorkList.push_back(MI);
  Seen.insert(MI);
  while (!WorkList.empty()) {
    MachineInstr *Current = WorkList.back();
    WorkList.pop_back();
    Register R = Current->getOperand(0).getReg();

    if (Register::isPhysicalRegister(R))
      // We can't properly track physical register uses, so bail out here.
      // Generally if we see a physical register we're going to either do a
      // function call, inline assembly or return, on which we want to bail
      // out anyway.
      return true;

    if (TRI->isTypeLegalForClass(*MRI.getRegClass(R), CapTy))
      // We've somehow managed to copy the integer value into a capability
      // register. The uses of this register will read the capability
      // metadata.
      return true;

    // Look through all the uses that haven't yet been seen and add
    // any copy-like instruction to the worklist.
    for (auto I = MRI.use_begin(R), E = MRI.use_end(); I != E; ++I) {
      MachineInstr *Next = I->getParent();
      if (Seen.find(Next) != Seen.end())
        continue;

      // Inline assembly and calls could read the capability metadata
      // even if the associated MachineInstr says they don't.
      if (Next->isInlineAsm() || Next->isCall())
        return true;

      // If this is an instruction that could be elided add it to the
      // worklist. Other instructions don't read the metadata (otherwise
      // their description would use a capability instead of a integer
      // register).
      if (Next->isCopy() || Next->isPHI() || Next->isSubregToReg() ||
          Next->isExtractSubreg() || Next->isRegSequence()) {
        WorkList.push_back(Next);
        Seen.insert(Next);
      }
    }
  }
  return false;
}

bool CheriGetAddressElim::runOnMachineFunction(MachineFunction &MF) {
  MVT CapTy = MF.getSubtarget().getTargetLowering()->cheriCapabilityType();
  // Skip this pass if the function is compiler without CHERI support.
  if (!CapTy.isValid())
    return false;

  LLVM_DEBUG(
      dbgs() << "********** Cheri Get Address Elimination  **********\n"
             << "********** Function: " << MF.getName() << '\n');

  SmallVector<MachineInstr*, 10> Insts;
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo *TII =
      MF.getSubtarget().getInstrInfo();
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();

  bool Changed = false;
  unsigned RegIdx = TII->getCheriAddressSubregIdx(CapTy);

  // Gather all the get address instructions in the function to be processed.
  for (MachineBasicBlock &MBB : MF)
    for (MachineInstr &MII : MBB)
      if (TII->isCheriGetAddressInst(MII))
        Insts.push_back(&MII);

  for (auto &MI : Insts) {
    // If something reads the metadata bits then we can't replace the
    // get address instruction since the copy might be eliminated and the top
    // bits won't be cleared.
    if (couldReadCapMetadata(MI, MRI, TRI, CapTy))
      continue;

    // Replace the get address instruction with a sub-register copy.
    Register VReg = MI->getOperand(1).getReg();
    const TargetRegisterClass *TRC = MRI.getRegClass(MI->getOperand(0).getReg());
    Register NewReg = MRI.createVirtualRegister(TRC);
    MachineInstrBuilder CopyMI =
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(),
        TII->get(TargetOpcode::COPY), NewReg);
    if (VReg.isVirtual())
      CopyMI.addReg(VReg, 0, RegIdx);
    else
      CopyMI.addReg(TRI->getSubReg(VReg, RegIdx));
    LLVM_DEBUG(dbgs() << "Replacing " << *MI << " with a subregister read\n");
    MRI.replaceRegWith(MI->getOperand(0).getReg(), NewReg);
    MI->eraseFromParent();
    Changed = true;
  }

  return Changed;
}
