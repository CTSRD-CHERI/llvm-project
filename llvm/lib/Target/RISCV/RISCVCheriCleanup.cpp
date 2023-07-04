//===-- RISCVCheriCleanupOptPassInsts.cpp - Expand atomic pseudo instrs.
//---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a pass that expands atomic pseudo instructions into
// target instructions. This pass should be run at the last possible moment,
// avoiding the possibility for other passes to break the requirements for
// forward progress in the LR/SC block.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/RISCVMCTargetDesc.h"
#include "RISCV.h"
#include "RISCVInstrInfo.h"
#include "RISCVSubtarget.h"
#include "RISCVTargetMachine.h"

#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/GlobalVariable.h"

using namespace llvm;

#define RISCV_CHERI_CLEANUP_NAME "RISCV CHERIoT early bounds check elision pass"

namespace {

class RISCVCheriCleanupOpt : public MachineFunctionPass {
public:
  const RISCVInstrInfo *TII;
  inline static char ID;

  RISCVCheriCleanupOpt() : MachineFunctionPass(ID) {
    initializeRISCVCheriCleanupOptPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return RISCV_CHERI_CLEANUP_NAME; }
};

bool RISCVCheriCleanupOpt::runOnMachineFunction(MachineFunction &MF) {
  if (static_cast<const RISCVSubtarget &>(MF.getSubtarget()).getTargetABI() !=
      RISCVABI::ABI_CHERIOT)
    return false;

  auto &MRI = MF.getRegInfo();
  TII = static_cast<const RISCVInstrInfo *>(MF.getSubtarget().getInstrInfo());
  bool Modified = false;
  for (auto &MBB : MF)
    for (auto &MI : MBB)
      if (MI.getOpcode() == RISCV::PseudoCLLC) {
        // If this is not a load of a global then this is surprising.
        if (!MI.getOperand(1).isGlobal())
          continue;
        uint32_t SafeSize = 0;
        // If this is the definition of a global, then we know the size.  Allow
        // any loads in that size to be safe.
        if (auto GV = dyn_cast<GlobalVariable>(MI.getOperand(1).getGlobal()))
          if (GV->hasInitializer())
            SafeSize = MF.getDataLayout().getTypeAllocSize(GV->getValueType());
        bool UnsafeUse = false;
        for (auto &UI : MRI.use_instructions(MI.getOperand(0).getReg())) {
          size_t OpSize = 0;
          switch (UI.getOpcode()) {
          default:
            UnsafeUse = true;
            continue;
          case RISCV::CLB:
          case RISCV::CLBU:
          case RISCV::CSB:
            OpSize = 1;
            break;
          case RISCV::CLH:
          case RISCV::CLHU:
          case RISCV::CSH:
            OpSize = 2;
            break;
          case RISCV::CLW:
          case RISCV::CLWU:
          case RISCV::CSW:
            OpSize = 4;
            break;
          case RISCV::CLC_64:
          case RISCV::CSC_64:
            OpSize = 8;
            break;
          }
          size_t Offset = UI.getOperand(2).getImm();
          if (Offset == 0)
            continue;
          if (Offset + OpSize <= SafeSize)
            continue;
          UnsafeUse = true;
          break;
        }
        if (!UnsafeUse) {
          MI.setDesc(TII->get(RISCV::PseudoCLLCInbounds));
          Modified = true;
        }
      }
  return Modified;
}

} // end of anonymous namespace

INITIALIZE_PASS(RISCVCheriCleanupOpt, "riscv-cheriot-expand-cllc",
                RISCV_CHERI_CLEANUP_NAME, false, false)

namespace llvm {

FunctionPass *createRISCVCheriCleanupOptPass() {
  return new RISCVCheriCleanupOpt();
}

} // end of namespace llvm
