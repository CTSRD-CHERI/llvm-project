//===-- Mips.h - Top-level interface for Mips representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM Mips back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPS_H
#define LLVM_LIB_TARGET_MIPS_MIPS_H

#include "MCTargetDesc/MipsMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class MipsTargetMachine;
  class ModulePass;
  class FunctionPass;
  class MachineFunctionPass;
  class MipsRegisterBankInfo;
  class MipsSubtarget;
  class MipsTargetMachine;
  class InstructionSelector;
  class PassRegistry;

  ModulePass *createMipsOs16Pass();
  ModulePass *createMips16HardFloatPass();

  FunctionPass *createMipsModuleISelDagPass();
  FunctionPass *createMipsOptimizePICCallPass();
  FunctionPass *createMipsDelaySlotFillerPass();
  FunctionPass *createMipsBranchExpansion();
  FunctionPass *createMipsConstantIslandPass();
  FunctionPass *createMicroMipsSizeReducePass();
  FunctionPass *createMipsExpandPseudoPass();

  FunctionPass *createCheriInvalidatePass();
  FunctionPass *createCheriRangeChecker();
  FunctionPass *createCheriLoopPointerDecanonicalize();

  ModulePass *createCheriExpandIntrinsicsPass();
  ModulePass *createCheriPureCapABI();
  MachineFunctionPass *createCheriAddressingModeFolder();
  MachineFunctionPass *createCheri128FailHardPass();
  InstructionSelector *createMipsInstructionSelector(const MipsTargetMachine &,
                                                     MipsSubtarget &,
                                                     MipsRegisterBankInfo &);

  void initializeMipsDelaySlotFillerPass(PassRegistry &);
  void initializeMipsBranchExpansionPass(PassRegistry &);
  void initializeMicroMipsSizeReducePass(PassRegistry &);
} // end namespace llvm;

#endif
