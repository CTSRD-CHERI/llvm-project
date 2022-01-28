//===-- RISCVExpandAtomicPseudoInsts.cpp - Expand atomic pseudo instrs. ---===//
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

#include "RISCV.h"
#include "RISCVInstrInfo.h"
#include "RISCVTargetMachine.h"

#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define RISCV_EXPAND_ATOMIC_PSEUDO_NAME                                        \
  "RISCV atomic pseudo instruction expansion pass"

namespace {

class RISCVExpandAtomicPseudo : public MachineFunctionPass {
public:
  const RISCVInstrInfo *TII;
  static char ID;

  RISCVExpandAtomicPseudo() : MachineFunctionPass(ID) {
    initializeRISCVExpandAtomicPseudoPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return RISCV_EXPAND_ATOMIC_PSEUDO_NAME;
  }

private:
  bool expandMBB(MachineBasicBlock &MBB);
  bool expandMI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                MachineBasicBlock::iterator &NextMBBI);
  bool expandAtomicBinOp(MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MBBI, AtomicRMWInst::BinOp,
                         bool IsMasked, MVT VT, bool PtrIsCap,
                         MachineBasicBlock::iterator &NextMBBI);
  bool expandAtomicMinMaxOp(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI,
                            AtomicRMWInst::BinOp, bool IsMasked, MVT VT,
                            bool PtrIsCap,
                            MachineBasicBlock::iterator &NextMBBI);
  bool expandAtomicCmpXchg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI, bool IsMasked,
                           MVT VT, bool PtrIsCap,
                           MachineBasicBlock::iterator &NextMBBI);
};

char RISCVExpandAtomicPseudo::ID = 0;

bool RISCVExpandAtomicPseudo::runOnMachineFunction(MachineFunction &MF) {
  TII = static_cast<const RISCVInstrInfo *>(MF.getSubtarget().getInstrInfo());
  bool Modified = false;
  for (auto &MBB : MF)
    Modified |= expandMBB(MBB);
  return Modified;
}

bool RISCVExpandAtomicPseudo::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI, NMBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool RISCVExpandAtomicPseudo::expandMI(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MBBI,
                                       MachineBasicBlock::iterator &NextMBBI) {
  // RISCVInstrInfo::getInstSizeInBytes expects that the total size of the
  // expanded instructions for each pseudo is correct in the Size field of the
  // tablegen definition for the pseudo.
  const auto &Subtarget = MBB.getParent()->getSubtarget<RISCVSubtarget>();
  MVT CLenVT = Subtarget.hasCheri() ? Subtarget.typeForCapabilities() : MVT();
  switch (MBBI->getOpcode()) {
  case RISCV::PseudoAtomicLoadNand32:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Nand, false, MVT::i32,
                             false, NextMBBI);
  case RISCV::PseudoAtomicLoadNand64:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Nand, false, MVT::i64,
                             false, NextMBBI);
  case RISCV::PseudoMaskedAtomicSwap32:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Xchg, true, MVT::i32,
                             false, NextMBBI);
  case RISCV::PseudoMaskedAtomicLoadAdd32:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Add, true, MVT::i32,
                             false, NextMBBI);
  case RISCV::PseudoMaskedAtomicLoadSub32:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Sub, true, MVT::i32,
                             false, NextMBBI);
  case RISCV::PseudoMaskedAtomicLoadNand32:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Nand, true, MVT::i32,
                             false, NextMBBI);
  case RISCV::PseudoMaskedAtomicLoadMax32:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Max, true, MVT::i32,
                                false, NextMBBI);
  case RISCV::PseudoMaskedAtomicLoadMin32:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Min, true, MVT::i32,
                                false, NextMBBI);
  case RISCV::PseudoMaskedAtomicLoadUMax32:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMax, true, MVT::i32,
                                false, NextMBBI);
  case RISCV::PseudoMaskedAtomicLoadUMin32:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMin, true, MVT::i32,
                                false, NextMBBI);
  case RISCV::PseudoCmpXchg32:
    return expandAtomicCmpXchg(MBB, MBBI, false, MVT::i32, false, NextMBBI);
  case RISCV::PseudoCmpXchg64:
    return expandAtomicCmpXchg(MBB, MBBI, false, MVT::i64, false, NextMBBI);
  case RISCV::PseudoMaskedCmpXchg32:
    return expandAtomicCmpXchg(MBB, MBBI, true, MVT::i32, false, NextMBBI);
  case RISCV::PseudoAtomicLoadAddCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Add, false, CLenVT,
                             false, NextMBBI);
  case RISCV::PseudoAtomicLoadSubCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Sub, false, CLenVT,
                             false, NextMBBI);
  case RISCV::PseudoAtomicLoadAndCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::And, false, CLenVT,
                             false, NextMBBI);
  case RISCV::PseudoAtomicLoadOrCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Or, false, CLenVT,
                             false, NextMBBI);
  case RISCV::PseudoAtomicLoadXorCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Xor, false, CLenVT,
                             false, NextMBBI);
  case RISCV::PseudoAtomicLoadNandCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Nand, false, CLenVT,
                             false, NextMBBI);
  case RISCV::PseudoAtomicLoadMaxCap:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Max, false, CLenVT,
                                false, NextMBBI);
  case RISCV::PseudoAtomicLoadMinCap:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Min, false, CLenVT,
                                false, NextMBBI);
  case RISCV::PseudoAtomicLoadUMaxCap:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMax, false, CLenVT,
                                false, NextMBBI);
  case RISCV::PseudoAtomicLoadUMinCap:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMin, false, CLenVT,
                                false, NextMBBI);
  case RISCV::PseudoCmpXchgCap:
    return expandAtomicCmpXchg(MBB, MBBI, false, CLenVT, false, NextMBBI);
  case RISCV::PseudoCheriAtomicSwap8:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Xchg, false, MVT::i8,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicSwap16:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Xchg, false, MVT::i16,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadAdd8:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Add, false, MVT::i8,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadAdd16:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Add, false, MVT::i16,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadAnd8:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::And, false, MVT::i8,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadAnd16:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::And, false, MVT::i16,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadOr8:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Or, false, MVT::i8,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadOr16:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Or, false, MVT::i16,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadXor8:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Xor, false, MVT::i8,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadXor16:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Xor, false, MVT::i16,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadNand8:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Nand, false, MVT::i8,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadNand16:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Nand, false, MVT::i16,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadSub8:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Sub, false, MVT::i8,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadSub16:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Sub, false, MVT::i16,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadNand32:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Nand, false, MVT::i32,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadNand64:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Nand, false, MVT::i64,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadMax8:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Max, false, MVT::i8,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadMax16:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Max, false, MVT::i16,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadMin8:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Min, false, MVT::i8,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadMin16:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Min, false, MVT::i16,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadUMax8:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMax, false, MVT::i8,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadUMax16:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMax, false, MVT::i16,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadUMin8:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMin, false, MVT::i8,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadUMin16:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMin, false, MVT::i16,
                                true, NextMBBI);
  case RISCV::PseudoCheriCmpXchg8:
    return expandAtomicCmpXchg(MBB, MBBI, false, MVT::i8, true, NextMBBI);
  case RISCV::PseudoCheriCmpXchg16:
    return expandAtomicCmpXchg(MBB, MBBI, false, MVT::i16, true, NextMBBI);
  case RISCV::PseudoCheriCmpXchg32:
    return expandAtomicCmpXchg(MBB, MBBI, false, MVT::i32, true, NextMBBI);
  case RISCV::PseudoCheriCmpXchg64:
    return expandAtomicCmpXchg(MBB, MBBI, false, MVT::i64, true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadAddCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Add, false, CLenVT,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadSubCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Sub, false, CLenVT,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadAndCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::And, false, CLenVT,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadOrCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Or, false, CLenVT,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadXorCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Xor, false, CLenVT,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadNandCap:
    return expandAtomicBinOp(MBB, MBBI, AtomicRMWInst::Nand, false, CLenVT,
                             true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadMaxCap:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Max, false, CLenVT,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadMinCap:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::Min, false, CLenVT,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadUMaxCap:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMax, false, CLenVT,
                                true, NextMBBI);
  case RISCV::PseudoCheriAtomicLoadUMinCap:
    return expandAtomicMinMaxOp(MBB, MBBI, AtomicRMWInst::UMin, false, CLenVT,
                                true, NextMBBI);
  case RISCV::PseudoCheriCmpXchgCap:
    return expandAtomicCmpXchg(MBB, MBBI, false, CLenVT, true, NextMBBI);
  }

  return false;
}

static unsigned getLRForRMW8(bool PtrIsCap, AtomicOrdering Ordering) {
  assert(PtrIsCap);
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return RISCV::CLR_B;
  case AtomicOrdering::Acquire:
    return RISCV::CLR_B_AQ;
  case AtomicOrdering::Release:
    return RISCV::CLR_B;
  case AtomicOrdering::AcquireRelease:
    return RISCV::CLR_B_AQ;
  case AtomicOrdering::SequentiallyConsistent:
    return RISCV::CLR_B_AQ_RL;
  }
}

static unsigned getSCForRMW8(bool PtrIsCap, AtomicOrdering Ordering) {
  assert(PtrIsCap);
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return RISCV::CSC_B;
  case AtomicOrdering::Acquire:
    return RISCV::CSC_B;
  case AtomicOrdering::Release:
    return RISCV::CSC_B_RL;
  case AtomicOrdering::AcquireRelease:
    return RISCV::CSC_B_RL;
  case AtomicOrdering::SequentiallyConsistent:
    return RISCV::CSC_B_AQ_RL;
  }
}

static unsigned getLRForRMW16(bool PtrIsCap, AtomicOrdering Ordering) {
  assert(PtrIsCap);
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return RISCV::CLR_H;
  case AtomicOrdering::Acquire:
    return RISCV::CLR_H_AQ;
  case AtomicOrdering::Release:
    return RISCV::CLR_H;
  case AtomicOrdering::AcquireRelease:
    return RISCV::CLR_H_AQ;
  case AtomicOrdering::SequentiallyConsistent:
    return RISCV::CLR_H_AQ_RL;
  }
}

static unsigned getSCForRMW16(bool PtrIsCap, AtomicOrdering Ordering) {
  assert(PtrIsCap);
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return RISCV::CSC_H;
  case AtomicOrdering::Acquire:
    return RISCV::CSC_H;
  case AtomicOrdering::Release:
    return RISCV::CSC_H_RL;
  case AtomicOrdering::AcquireRelease:
    return RISCV::CSC_H_RL;
  case AtomicOrdering::SequentiallyConsistent:
    return RISCV::CSC_H_AQ_RL;
  }
}

static unsigned getLRForRMW32(bool PtrIsCap, AtomicOrdering Ordering) {
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return PtrIsCap ? RISCV::CLR_W : RISCV::LR_W;
  case AtomicOrdering::Acquire:
    return PtrIsCap ? RISCV::CLR_W_AQ : RISCV::LR_W_AQ;
  case AtomicOrdering::Release:
    return PtrIsCap ? RISCV::CLR_W : RISCV::LR_W;
  case AtomicOrdering::AcquireRelease:
    return PtrIsCap ? RISCV::CLR_W_AQ : RISCV::LR_W_AQ;
  case AtomicOrdering::SequentiallyConsistent:
    return PtrIsCap ? RISCV::CLR_W_AQ_RL : RISCV::LR_W_AQ_RL;
  }
}

static unsigned getSCForRMW32(bool PtrIsCap, AtomicOrdering Ordering) {
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return PtrIsCap ? RISCV::CSC_W : RISCV::SC_W;
  case AtomicOrdering::Acquire:
    return PtrIsCap ? RISCV::CSC_W : RISCV::SC_W;
  case AtomicOrdering::Release:
    return PtrIsCap ? RISCV::CSC_W_RL : RISCV::SC_W_RL;
  case AtomicOrdering::AcquireRelease:
    return PtrIsCap ? RISCV::CSC_W_RL : RISCV::SC_W_RL;
  case AtomicOrdering::SequentiallyConsistent:
    return PtrIsCap ? RISCV::CSC_W_AQ_RL : RISCV::SC_W_AQ_RL;
  }
}

static unsigned getLRForRMW64(bool PtrIsCap, AtomicOrdering Ordering) {
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return PtrIsCap ? RISCV::CLR_D : RISCV::LR_D;
  case AtomicOrdering::Acquire:
    return PtrIsCap ? RISCV::CLR_D_AQ : RISCV::LR_D_AQ;
  case AtomicOrdering::Release:
    return PtrIsCap ? RISCV::CLR_D : RISCV::LR_D;
  case AtomicOrdering::AcquireRelease:
    return PtrIsCap ? RISCV::CLR_D_AQ : RISCV::LR_D_AQ;
  case AtomicOrdering::SequentiallyConsistent:
    return PtrIsCap ? RISCV::CLR_D_AQ_RL : RISCV::LR_D_AQ_RL;
  }
}

static unsigned getSCForRMW64(bool PtrIsCap, AtomicOrdering Ordering) {
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return PtrIsCap ? RISCV::CSC_D : RISCV::SC_D;
  case AtomicOrdering::Acquire:
    return PtrIsCap ? RISCV::CSC_D : RISCV::SC_D;
  case AtomicOrdering::Release:
    return PtrIsCap ? RISCV::CSC_D_RL : RISCV::SC_D_RL;
  case AtomicOrdering::AcquireRelease:
    return PtrIsCap ? RISCV::CSC_D_RL : RISCV::SC_D_RL;
  case AtomicOrdering::SequentiallyConsistent:
    return PtrIsCap ? RISCV::CSC_D_AQ_RL : RISCV::SC_D_AQ_RL;
  }
}

static unsigned getLRForRMWCap64(bool PtrIsCap, AtomicOrdering Ordering) {
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return PtrIsCap ? RISCV::CLR_C_64 : RISCV::LR_C_64;
  case AtomicOrdering::Acquire:
    return PtrIsCap ? RISCV::CLR_C_AQ_64 : RISCV::LR_C_AQ_64;
  case AtomicOrdering::Release:
    return PtrIsCap ? RISCV::CLR_C_RL_64 : RISCV::LR_C_RL_64;
  case AtomicOrdering::AcquireRelease:
    return PtrIsCap ? RISCV::CLR_C_AQ_64 : RISCV::LR_C_AQ_64;
  case AtomicOrdering::SequentiallyConsistent:
    return PtrIsCap ? RISCV::CLR_C_AQ_RL_64 : RISCV::LR_C_AQ_RL_64;
  }
}

static unsigned getSCForRMWCap64(bool PtrIsCap, AtomicOrdering Ordering) {
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return PtrIsCap ? RISCV::CSC_C_64 : RISCV::SC_C_64;
  case AtomicOrdering::Acquire:
    return PtrIsCap ? RISCV::CSC_C_AQ_64 : RISCV::SC_C_AQ_64;
  case AtomicOrdering::Release:
    return PtrIsCap ? RISCV::CSC_C_64 : RISCV::SC_C_64;
  case AtomicOrdering::AcquireRelease:
    return PtrIsCap ? RISCV::CSC_C_AQ_64 : RISCV::SC_C_AQ_64;
  case AtomicOrdering::SequentiallyConsistent:
    return PtrIsCap ? RISCV::CSC_C_AQ_RL_64 : RISCV::SC_C_AQ_RL_64;
  }
}

static unsigned getLRForRMWCap128(bool PtrIsCap, AtomicOrdering Ordering) {
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return PtrIsCap ? RISCV::CLR_C_128 : RISCV::LR_C_128;
  case AtomicOrdering::Acquire:
    return PtrIsCap ? RISCV::CLR_C_AQ_128 : RISCV::LR_C_AQ_128;
  case AtomicOrdering::Release:
    return PtrIsCap ? RISCV::CLR_C_RL_128 : RISCV::LR_C_RL_128;
  case AtomicOrdering::AcquireRelease:
    return PtrIsCap ? RISCV::CLR_C_AQ_128 : RISCV::LR_C_AQ_128;
  case AtomicOrdering::SequentiallyConsistent:
    return PtrIsCap ? RISCV::CLR_C_AQ_RL_128 : RISCV::LR_C_AQ_RL_128;
  }
}

static unsigned getSCForRMWCap128(bool PtrIsCap, AtomicOrdering Ordering) {
  switch (Ordering) {
  default:
    llvm_unreachable("Unexpected AtomicOrdering");
  case AtomicOrdering::Monotonic:
    return PtrIsCap ? RISCV::CSC_C_128 : RISCV::SC_C_128;
  case AtomicOrdering::Acquire:
    return PtrIsCap ? RISCV::CSC_C_AQ_128 : RISCV::SC_C_AQ_128;
  case AtomicOrdering::Release:
    return PtrIsCap ? RISCV::CSC_C_128 : RISCV::SC_C_128;
  case AtomicOrdering::AcquireRelease:
    return PtrIsCap ? RISCV::CSC_C_AQ_128 : RISCV::SC_C_AQ_128;
  case AtomicOrdering::SequentiallyConsistent:
    return PtrIsCap ? RISCV::CSC_C_AQ_RL_128 : RISCV::SC_C_AQ_RL_128;
  }
}

static unsigned getLRForRMW(bool PtrIsCap, AtomicOrdering Ordering, MVT VT) {
  if (VT == MVT::i8)
    return getLRForRMW8(PtrIsCap, Ordering);
  if (VT == MVT::i16)
    return getLRForRMW16(PtrIsCap, Ordering);
  if (VT == MVT::i32)
    return getLRForRMW32(PtrIsCap, Ordering);
  if (VT == MVT::i64)
    return getLRForRMW64(PtrIsCap, Ordering);
  if (VT == MVT::iFATPTR64)
    return getLRForRMWCap64(PtrIsCap, Ordering);
  if (VT == MVT::iFATPTR128)
    return getLRForRMWCap128(PtrIsCap, Ordering);
  llvm_unreachable("Unexpected LR type\n");
}

static unsigned getSCForRMW(bool PtrIsCap, AtomicOrdering Ordering, MVT VT) {
  if (VT == MVT::i8)
    return getSCForRMW8(PtrIsCap, Ordering);
  if (VT == MVT::i16)
    return getSCForRMW16(PtrIsCap, Ordering);
  if (VT == MVT::i32)
    return getSCForRMW32(PtrIsCap, Ordering);
  if (VT == MVT::i64)
    return getSCForRMW64(PtrIsCap, Ordering);
  if (VT == MVT::iFATPTR64)
    return getSCForRMWCap64(PtrIsCap, Ordering);
  if (VT == MVT::iFATPTR128)
    return getSCForRMWCap128(PtrIsCap, Ordering);
  llvm_unreachable("Unexpected SC type\n");
}

static void doAtomicBinOpExpansion(const RISCVInstrInfo *TII, MachineInstr &MI,
                                   DebugLoc DL, MachineBasicBlock *ThisMBB,
                                   MachineBasicBlock *LoopMBB,
                                   MachineBasicBlock *DoneMBB,
                                   AtomicRMWInst::BinOp BinOp, MVT VT,
                                   bool PtrIsCap) {
  Register DestReg = MI.getOperand(0).getReg();
  Register ScratchReg = MI.getOperand(1).getReg();
  Register AddrReg = MI.getOperand(2).getReg();
  Register IncrReg = MI.getOperand(3).getReg();
  AtomicOrdering Ordering =
      static_cast<AtomicOrdering>(MI.getOperand(4).getImm());

  Register ScratchIntReg;
  Register DestIntReg;
  if (VT.isFatPointer()) {
    MachineFunction *MF = ThisMBB->getParent();
    const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
    IncrReg = TRI->getSubReg(IncrReg, RISCV::sub_cap_addr);
    ScratchIntReg = TRI->getSubReg(ScratchReg, RISCV::sub_cap_addr);
    DestIntReg = TRI->getSubReg(DestReg, RISCV::sub_cap_addr);
  } else {
    ScratchIntReg = ScratchReg;
    DestIntReg = DestReg;
  }

  // .loop:
  //   lr.[w|d] dest, (addr)
  //   binop scratch, dest, val
  //   sc.[w|d] scratch, scratch, (addr)
  //   bnez scratch, loop
  BuildMI(LoopMBB, DL, TII->get(getLRForRMW(PtrIsCap, Ordering, VT)), DestReg)
      .addReg(AddrReg);
  switch (BinOp) {
  default:
    llvm_unreachable("Unexpected AtomicRMW BinOp");
  case AtomicRMWInst::Xchg:
    assert(!VT.isFatPointer() && "Capabilities should use AMOSWAP.C");
    BuildMI(LoopMBB, DL, TII->get(RISCV::ADD), ScratchReg)
        .addReg(RISCV::X0)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::Add:
    if (VT.isFatPointer()) {
      BuildMI(LoopMBB, DL, TII->get(RISCV::CIncOffset), ScratchReg)
          .addReg(DestReg)
          .addReg(IncrReg);
      break;
    }
    BuildMI(LoopMBB, DL, TII->get(RISCV::ADD), ScratchReg)
        .addReg(DestReg)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::Sub:
    // If we had a second scratch register for the capability version we could
    // hoist (SUB X0, IncrReg) out of the loop and just use CIncOffset, but
    // that adds complexity, and unless there is high contention there won't be
    // much difference in the dynamic instruction count.
    BuildMI(LoopMBB, DL, TII->get(RISCV::SUB), ScratchIntReg)
        .addReg(DestIntReg)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::And:
    BuildMI(LoopMBB, DL, TII->get(RISCV::AND), ScratchIntReg)
        .addReg(DestIntReg)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::Or:
    BuildMI(LoopMBB, DL, TII->get(RISCV::OR), ScratchIntReg)
        .addReg(DestIntReg)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::Xor:
    BuildMI(LoopMBB, DL, TII->get(RISCV::XOR), ScratchIntReg)
        .addReg(DestIntReg)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::Nand:
    BuildMI(LoopMBB, DL, TII->get(RISCV::AND), ScratchIntReg)
        .addReg(DestIntReg)
        .addReg(IncrReg);
    BuildMI(LoopMBB, DL, TII->get(RISCV::XORI), ScratchIntReg)
        .addReg(ScratchIntReg)
        .addImm(-1);
    break;
  }
  if (VT.isFatPointer() && BinOp != AtomicRMWInst::Add)
    BuildMI(LoopMBB, DL, TII->get(RISCV::CSetAddr), ScratchReg)
        .addReg(DestReg)
        .addReg(ScratchIntReg);
  BuildMI(LoopMBB, DL, TII->get(getSCForRMW(PtrIsCap, Ordering, VT)),
          ScratchIntReg)
      .addReg(AddrReg)
      .addReg(ScratchReg);
  BuildMI(LoopMBB, DL, TII->get(RISCV::BNE))
      .addReg(ScratchIntReg)
      .addReg(RISCV::X0)
      .addMBB(LoopMBB);
}

static void insertMaskedMerge(const RISCVInstrInfo *TII, DebugLoc DL,
                              MachineBasicBlock *MBB, Register DestReg,
                              Register OldValReg, Register NewValReg,
                              Register MaskReg, Register ScratchReg) {
  assert(OldValReg != ScratchReg && "OldValReg and ScratchReg must be unique");
  assert(OldValReg != MaskReg && "OldValReg and MaskReg must be unique");
  assert(ScratchReg != MaskReg && "ScratchReg and MaskReg must be unique");

  // We select bits from newval and oldval using:
  // https://graphics.stanford.edu/~seander/bithacks.html#MaskedMerge
  // r = oldval ^ ((oldval ^ newval) & masktargetdata);
  BuildMI(MBB, DL, TII->get(RISCV::XOR), ScratchReg)
      .addReg(OldValReg)
      .addReg(NewValReg);
  BuildMI(MBB, DL, TII->get(RISCV::AND), ScratchReg)
      .addReg(ScratchReg)
      .addReg(MaskReg);
  BuildMI(MBB, DL, TII->get(RISCV::XOR), DestReg)
      .addReg(OldValReg)
      .addReg(ScratchReg);
}

static void doMaskedAtomicBinOpExpansion(
    const RISCVInstrInfo *TII, MachineInstr &MI, DebugLoc DL,
    MachineBasicBlock *ThisMBB, MachineBasicBlock *LoopMBB,
    MachineBasicBlock *DoneMBB, AtomicRMWInst::BinOp BinOp, MVT VT) {
  assert(VT == MVT::i32 &&
         "Should never need to expand masked 64-bit operations");
  Register DestReg = MI.getOperand(0).getReg();
  Register ScratchReg = MI.getOperand(1).getReg();
  Register AddrReg = MI.getOperand(2).getReg();
  Register IncrReg = MI.getOperand(3).getReg();
  Register MaskReg = MI.getOperand(4).getReg();
  AtomicOrdering Ordering =
      static_cast<AtomicOrdering>(MI.getOperand(5).getImm());

  // .loop:
  //   lr.w destreg, (alignedaddr)
  //   binop scratch, destreg, incr
  //   xor scratch, destreg, scratch
  //   and scratch, scratch, masktargetdata
  //   xor scratch, destreg, scratch
  //   sc.w scratch, scratch, (alignedaddr)
  //   bnez scratch, loop
  BuildMI(LoopMBB, DL, TII->get(getLRForRMW32(false, Ordering)), DestReg)
      .addReg(AddrReg);
  switch (BinOp) {
  default:
    llvm_unreachable("Unexpected AtomicRMW BinOp");
  case AtomicRMWInst::Xchg:
    BuildMI(LoopMBB, DL, TII->get(RISCV::ADDI), ScratchReg)
        .addReg(IncrReg)
        .addImm(0);
    break;
  case AtomicRMWInst::Add:
    BuildMI(LoopMBB, DL, TII->get(RISCV::ADD), ScratchReg)
        .addReg(DestReg)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::Sub:
    BuildMI(LoopMBB, DL, TII->get(RISCV::SUB), ScratchReg)
        .addReg(DestReg)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::Nand:
    BuildMI(LoopMBB, DL, TII->get(RISCV::AND), ScratchReg)
        .addReg(DestReg)
        .addReg(IncrReg);
    BuildMI(LoopMBB, DL, TII->get(RISCV::XORI), ScratchReg)
        .addReg(ScratchReg)
        .addImm(-1);
    break;
  }

  insertMaskedMerge(TII, DL, LoopMBB, ScratchReg, DestReg, ScratchReg, MaskReg,
                    ScratchReg);

  BuildMI(LoopMBB, DL, TII->get(getSCForRMW32(false, Ordering)), ScratchReg)
      .addReg(AddrReg)
      .addReg(ScratchReg);
  BuildMI(LoopMBB, DL, TII->get(RISCV::BNE))
      .addReg(ScratchReg)
      .addReg(RISCV::X0)
      .addMBB(LoopMBB);
}

bool RISCVExpandAtomicPseudo::expandAtomicBinOp(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    AtomicRMWInst::BinOp BinOp, bool IsMasked, MVT VT, bool PtrIsCap,
    MachineBasicBlock::iterator &NextMBBI) {
  assert(!(IsMasked && PtrIsCap) &&
         "Should never used masked operations with capabilities");

  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  auto LoopMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto DoneMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());

  // Insert new MBBs.
  MF->insert(++MBB.getIterator(), LoopMBB);
  MF->insert(++LoopMBB->getIterator(), DoneMBB);

  // Set up successors and transfer remaining instructions to DoneMBB.
  LoopMBB->addSuccessor(LoopMBB);
  LoopMBB->addSuccessor(DoneMBB);
  DoneMBB->splice(DoneMBB->end(), &MBB, MI, MBB.end());
  DoneMBB->transferSuccessors(&MBB);
  MBB.addSuccessor(LoopMBB);

  if (!IsMasked)
    doAtomicBinOpExpansion(TII, MI, DL, &MBB, LoopMBB, DoneMBB, BinOp, VT,
                           PtrIsCap);
  else
    doMaskedAtomicBinOpExpansion(TII, MI, DL, &MBB, LoopMBB, DoneMBB, BinOp,
                                 VT);

  NextMBBI = MBB.end();
  MI.eraseFromParent();

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *LoopMBB);
  computeAndAddLiveIns(LiveRegs, *DoneMBB);

  return true;
}

static void insertSext(const RISCVInstrInfo *TII, DebugLoc DL,
                       MachineBasicBlock *MBB, Register ValReg,
                       Register ShamtReg) {
  BuildMI(MBB, DL, TII->get(RISCV::SLL), ValReg)
      .addReg(ValReg)
      .addReg(ShamtReg);
  BuildMI(MBB, DL, TII->get(RISCV::SRA), ValReg)
      .addReg(ValReg)
      .addReg(ShamtReg);
}

bool RISCVExpandAtomicPseudo::expandAtomicMinMaxOp(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    AtomicRMWInst::BinOp BinOp, bool IsMasked, MVT VT, bool PtrIsCap,
    MachineBasicBlock::iterator &NextMBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();
  MachineFunction *MF = MBB.getParent();
  auto LoopHeadMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto LoopIfBodyMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto LoopTailMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto DoneMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());

  // Insert new MBBs.
  MF->insert(++MBB.getIterator(), LoopHeadMBB);
  MF->insert(++LoopHeadMBB->getIterator(), LoopIfBodyMBB);
  MF->insert(++LoopIfBodyMBB->getIterator(), LoopTailMBB);
  MF->insert(++LoopTailMBB->getIterator(), DoneMBB);

  // Set up successors and transfer remaining instructions to DoneMBB.
  LoopHeadMBB->addSuccessor(LoopIfBodyMBB);
  LoopHeadMBB->addSuccessor(LoopTailMBB);
  LoopIfBodyMBB->addSuccessor(LoopTailMBB);
  LoopTailMBB->addSuccessor(LoopHeadMBB);
  LoopTailMBB->addSuccessor(DoneMBB);
  DoneMBB->splice(DoneMBB->end(), &MBB, MI, MBB.end());
  DoneMBB->transferSuccessors(&MBB);
  MBB.addSuccessor(LoopHeadMBB);

  if (IsMasked) {
    assert(!PtrIsCap &&
           "Should never used masked operations with capabilities");
    assert(VT == MVT::i32 &&
           "Should never need to expand masked 64-bit operations");

    Register DestReg = MI.getOperand(0).getReg();
    Register Scratch1Reg = MI.getOperand(1).getReg();
    Register Scratch2Reg = MI.getOperand(2).getReg();
    Register AddrReg = MI.getOperand(3).getReg();
    Register IncrReg = MI.getOperand(4).getReg();
    Register MaskReg = MI.getOperand(5).getReg();
    bool IsSigned = BinOp == AtomicRMWInst::Min || BinOp == AtomicRMWInst::Max;
    AtomicOrdering Ordering =
        static_cast<AtomicOrdering>(MI.getOperand(IsSigned ? 7 : 6).getImm());

    //
    // .loophead:
    //   lr.w destreg, (alignedaddr)
    //   and scratch2, destreg, mask
    //   mv scratch1, destreg
    //   [sext scratch2 if signed min/max]
    //   ifnochangeneeded scratch2, incr, .looptail
    BuildMI(LoopHeadMBB, DL, TII->get(getLRForRMW32(PtrIsCap, Ordering)),
            DestReg)
        .addReg(AddrReg);
    BuildMI(LoopHeadMBB, DL, TII->get(RISCV::AND), Scratch2Reg)
        .addReg(DestReg)
        .addReg(MaskReg);
    BuildMI(LoopHeadMBB, DL, TII->get(RISCV::ADDI), Scratch1Reg)
        .addReg(DestReg)
        .addImm(0);

    switch (BinOp) {
    default:
      llvm_unreachable("Unexpected AtomicRMW BinOp");
    case AtomicRMWInst::Max: {
      insertSext(TII, DL, LoopHeadMBB, Scratch2Reg, MI.getOperand(6).getReg());
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BGE))
          .addReg(Scratch2Reg)
          .addReg(IncrReg)
          .addMBB(LoopTailMBB);
      break;
    }
    case AtomicRMWInst::Min: {
      insertSext(TII, DL, LoopHeadMBB, Scratch2Reg, MI.getOperand(6).getReg());
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BGE))
          .addReg(IncrReg)
          .addReg(Scratch2Reg)
          .addMBB(LoopTailMBB);
      break;
    }
    case AtomicRMWInst::UMax:
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BGEU))
          .addReg(Scratch2Reg)
          .addReg(IncrReg)
          .addMBB(LoopTailMBB);
      break;
    case AtomicRMWInst::UMin:
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BGEU))
          .addReg(IncrReg)
          .addReg(Scratch2Reg)
          .addMBB(LoopTailMBB);
      break;
    }

    // .loopifbody:
    //   xor scratch1, destreg, incr
    //   and scratch1, scratch1, mask
    //   xor scratch1, destreg, scratch1
    insertMaskedMerge(TII, DL, LoopIfBodyMBB, Scratch1Reg, DestReg, IncrReg,
                      MaskReg, Scratch1Reg);

    // .looptail:
    //   sc.w scratch1, scratch1, (addr)
    //   bnez scratch1, loop
    BuildMI(LoopTailMBB, DL, TII->get(getSCForRMW32(PtrIsCap, Ordering)), Scratch1Reg)
        .addReg(AddrReg)
        .addReg(Scratch1Reg);
    BuildMI(LoopTailMBB, DL, TII->get(RISCV::BNE))
        .addReg(Scratch1Reg)
        .addReg(RISCV::X0)
        .addMBB(LoopHeadMBB);
  } else {
    Register DestReg = MI.getOperand(0).getReg();
    Register ScratchReg = MI.getOperand(1).getReg();
    Register AddrReg = MI.getOperand(2).getReg();
    Register IncrReg = MI.getOperand(3).getReg();
    AtomicOrdering Ordering =
        static_cast<AtomicOrdering>(MI.getOperand(4).getImm());

    Register ScratchIntReg;
    Register IncrIntReg;
    if (VT.isFatPointer()) {
      const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
      ScratchIntReg = TRI->getSubReg(ScratchReg, RISCV::sub_cap_addr);
      IncrIntReg = TRI->getSubReg(IncrReg, RISCV::sub_cap_addr);
    } else {
      ScratchIntReg = ScratchReg;
      IncrIntReg = IncrReg;
    }

    //
    // .loophead:
    //   lr.[b|h] dest, (addr)
    //   mv scratch, dest
    //   ifnochangeneeded scratch, incr, .looptail
    BuildMI(LoopHeadMBB, DL, TII->get(getLRForRMW(PtrIsCap, Ordering, VT)),
            DestReg)
        .addReg(AddrReg);
    if (VT.isFatPointer())
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::CMove), ScratchReg)
          .addReg(DestReg);
    else
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::ADDI), ScratchReg)
          .addReg(DestReg)
          .addImm(0);

    switch (BinOp) {
    default:
      llvm_unreachable("Unexpected AtomicRMW BinOp");
    case AtomicRMWInst::Max: {
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BGE))
          .addReg(ScratchIntReg)
          .addReg(IncrIntReg)
          .addMBB(LoopTailMBB);
      break;
    }
    case AtomicRMWInst::Min: {
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BGE))
          .addReg(IncrIntReg)
          .addReg(ScratchIntReg)
          .addMBB(LoopTailMBB);
      break;
    }
    case AtomicRMWInst::UMax:
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BGEU))
          .addReg(ScratchIntReg)
          .addReg(IncrIntReg)
          .addMBB(LoopTailMBB);
      break;
    case AtomicRMWInst::UMin:
      BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BGEU))
          .addReg(IncrIntReg)
          .addReg(ScratchIntReg)
          .addMBB(LoopTailMBB);
      break;
    }

    // .loopifbody:
    //   mv scratch, incr
    if (VT.isFatPointer())
      BuildMI(LoopIfBodyMBB, DL, TII->get(RISCV::CMove), ScratchReg)
          .addReg(DestReg);
    else
      BuildMI(LoopIfBodyMBB, DL, TII->get(RISCV::ADDI), ScratchReg)
          .addReg(IncrReg)
          .addImm(0);

    // .looptail:
    //   sc.[b|h] scratch, scratch, (addr)
    //   bnez scratch, loop
    BuildMI(LoopTailMBB, DL, TII->get(getSCForRMW(PtrIsCap, Ordering, VT)),
            ScratchIntReg)
        .addReg(AddrReg)
        .addReg(ScratchReg);
    BuildMI(LoopTailMBB, DL, TII->get(RISCV::BNE))
        .addReg(ScratchIntReg)
        .addReg(RISCV::X0)
        .addMBB(LoopHeadMBB);
  }

  NextMBBI = MBB.end();
  MI.eraseFromParent();

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *LoopHeadMBB);
  computeAndAddLiveIns(LiveRegs, *LoopIfBodyMBB);
  computeAndAddLiveIns(LiveRegs, *LoopTailMBB);
  computeAndAddLiveIns(LiveRegs, *DoneMBB);

  return true;
}

bool RISCVExpandAtomicPseudo::expandAtomicCmpXchg(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, bool IsMasked,
    MVT VT, bool PtrIsCap, MachineBasicBlock::iterator &NextMBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();
  MachineFunction *MF = MBB.getParent();
  auto LoopHeadMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto LoopTailMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto DoneMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());

  // Insert new MBBs.
  MF->insert(++MBB.getIterator(), LoopHeadMBB);
  MF->insert(++LoopHeadMBB->getIterator(), LoopTailMBB);
  MF->insert(++LoopTailMBB->getIterator(), DoneMBB);

  // Set up successors and transfer remaining instructions to DoneMBB.
  LoopHeadMBB->addSuccessor(LoopTailMBB);
  LoopHeadMBB->addSuccessor(DoneMBB);
  LoopTailMBB->addSuccessor(DoneMBB);
  LoopTailMBB->addSuccessor(LoopHeadMBB);
  DoneMBB->splice(DoneMBB->end(), &MBB, MI, MBB.end());
  DoneMBB->transferSuccessors(&MBB);
  MBB.addSuccessor(LoopHeadMBB);

  Register DestReg = MI.getOperand(0).getReg();
  Register ScratchReg = MI.getOperand(1).getReg();
  Register AddrReg = MI.getOperand(2).getReg();
  Register CmpValReg = MI.getOperand(3).getReg();
  Register NewValReg = MI.getOperand(4).getReg();
  AtomicOrdering Ordering =
      static_cast<AtomicOrdering>(MI.getOperand(IsMasked ? 6 : 5).getImm());

  if (!IsMasked) {
    Register DestIntReg;
    Register CmpValIntReg;
    if (VT.isFatPointer()) {
      const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
      DestIntReg = TRI->getSubReg(DestReg, RISCV::sub_cap_addr);
      CmpValIntReg = TRI->getSubReg(CmpValReg, RISCV::sub_cap_addr);
    } else {
      DestIntReg = DestReg;
      CmpValIntReg = CmpValReg;
    }

    // .loophead:
    //   lr.[w|d] dest, (addr)
    //   bne dest, cmpval, done
    BuildMI(LoopHeadMBB, DL, TII->get(getLRForRMW(PtrIsCap, Ordering, VT)),
            DestReg)
        .addReg(AddrReg);
    BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BNE))
        .addReg(DestIntReg, 0)
        .addReg(CmpValIntReg, 0)
        .addMBB(DoneMBB);
    // .looptail:
    //   sc.[w|d] scratch, newval, (addr)
    //   bnez scratch, loophead
    BuildMI(LoopTailMBB, DL, TII->get(getSCForRMW(PtrIsCap, Ordering, VT)),
            ScratchReg)
        .addReg(AddrReg)
        .addReg(NewValReg);
    BuildMI(LoopTailMBB, DL, TII->get(RISCV::BNE))
        .addReg(ScratchReg)
        .addReg(RISCV::X0)
        .addMBB(LoopHeadMBB);
  } else {
    assert(!PtrIsCap &&
        "Should never used masked operations with capabilities");

    // .loophead:
    //   lr.w dest, (addr)
    //   and scratch, dest, mask
    //   bne scratch, cmpval, done
    Register MaskReg = MI.getOperand(5).getReg();
    BuildMI(LoopHeadMBB, DL, TII->get(getLRForRMW(false, Ordering, VT)),
            DestReg)
        .addReg(AddrReg);
    BuildMI(LoopHeadMBB, DL, TII->get(RISCV::AND), ScratchReg)
        .addReg(DestReg)
        .addReg(MaskReg);
    BuildMI(LoopHeadMBB, DL, TII->get(RISCV::BNE))
        .addReg(ScratchReg)
        .addReg(CmpValReg)
        .addMBB(DoneMBB);

    // .looptail:
    //   xor scratch, dest, newval
    //   and scratch, scratch, mask
    //   xor scratch, dest, scratch
    //   sc.w scratch, scratch, (adrr)
    //   bnez scratch, loophead
    insertMaskedMerge(TII, DL, LoopTailMBB, ScratchReg, DestReg, NewValReg,
                      MaskReg, ScratchReg);
    BuildMI(LoopTailMBB, DL, TII->get(getSCForRMW(false, Ordering, VT)),
            ScratchReg)
        .addReg(AddrReg)
        .addReg(ScratchReg);
    BuildMI(LoopTailMBB, DL, TII->get(RISCV::BNE))
        .addReg(ScratchReg)
        .addReg(RISCV::X0)
        .addMBB(LoopHeadMBB);
  }

  NextMBBI = MBB.end();
  MI.eraseFromParent();

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *LoopHeadMBB);
  computeAndAddLiveIns(LiveRegs, *LoopTailMBB);
  computeAndAddLiveIns(LiveRegs, *DoneMBB);

  return true;
}

} // end of anonymous namespace

INITIALIZE_PASS(RISCVExpandAtomicPseudo, "riscv-expand-atomic-pseudo",
                RISCV_EXPAND_ATOMIC_PSEUDO_NAME, false, false)

namespace llvm {

FunctionPass *createRISCVExpandAtomicPseudoPass() {
  return new RISCVExpandAtomicPseudo();
}

} // end of namespace llvm
