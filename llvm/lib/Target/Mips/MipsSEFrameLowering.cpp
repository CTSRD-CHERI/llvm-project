//===- MipsSEFrameLowering.cpp - Mips32/64 Frame Information --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mips32/64 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "MipsSEFrameLowering.h"
#include "MCTargetDesc/MipsABIInfo.h"
#include "MipsMachineFunction.h"
#include "MipsRegisterInfo.h"
#include "MipsSEInstrInfo.h"
#include "MipsSubtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

static llvm::cl::opt<bool>
CHERICFI(
      "cheri-cfi", llvm::cl::NotHidden,
      llvm::cl::desc("Spill return addresses as capabilities"),
      llvm::cl::init(false));


using namespace llvm;

static std::pair<unsigned, unsigned> getMFHiLoOpc(unsigned Src) {
  if (Mips::ACC64RegClass.contains(Src))
    return std::make_pair((unsigned)Mips::PseudoMFHI,
                          (unsigned)Mips::PseudoMFLO);

  if (Mips::ACC64DSPRegClass.contains(Src))
    return std::make_pair((unsigned)Mips::MFHI_DSP, (unsigned)Mips::MFLO_DSP);

  if (Mips::ACC128RegClass.contains(Src))
    return std::make_pair((unsigned)Mips::PseudoMFHI64,
                          (unsigned)Mips::PseudoMFLO64);

  return std::make_pair(0, 0);
}

namespace {

/// Helper class to expand pseudos.
class ExpandPseudo {
public:
  ExpandPseudo(MachineFunction &MF);
  bool expand();

private:
  using Iter = MachineBasicBlock::iterator;

  bool expandInstr(MachineBasicBlock &MBB, Iter I);
  void expandLoadCCond(MachineBasicBlock &MBB, Iter I);
  void expandStoreCCond(MachineBasicBlock &MBB, Iter I);
  void expandLoadACC(MachineBasicBlock &MBB, Iter I, unsigned RegSize);
  void expandStoreACC(MachineBasicBlock &MBB, Iter I, unsigned MFHiOpc,
                      unsigned MFLoOpc, unsigned RegSize);
  bool expandCopy(MachineBasicBlock &MBB, Iter I);
  bool expandCopyACC(MachineBasicBlock &MBB, Iter I, unsigned MFHiOpc,
                     unsigned MFLoOpc);
  bool expandBuildPairF64(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator I, bool FP64) const;
  bool expandExtractElementF64(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I, bool FP64) const;

  MachineFunction &MF;
  MachineRegisterInfo &MRI;
  const MipsSubtarget &Subtarget;
  const MipsSEInstrInfo &TII;
  const MipsRegisterInfo &RegInfo;
};

} // end anonymous namespace

ExpandPseudo::ExpandPseudo(MachineFunction &MF_)
    : MF(MF_), MRI(MF.getRegInfo()),
      Subtarget(static_cast<const MipsSubtarget &>(MF.getSubtarget())),
      TII(*static_cast<const MipsSEInstrInfo *>(Subtarget.getInstrInfo())),
      RegInfo(*Subtarget.getRegisterInfo()) {}

bool ExpandPseudo::expand() {
  bool Expanded = false;

  for (auto &MBB : MF) {
    for (Iter I = MBB.begin(), End = MBB.end(); I != End;)
      Expanded |= expandInstr(MBB, I++);
  }

  return Expanded;
}

bool ExpandPseudo::expandInstr(MachineBasicBlock &MBB, Iter I) {
  switch(I->getOpcode()) {
  case Mips::LOAD_CCOND_DSP:
    expandLoadCCond(MBB, I);
    break;
  case Mips::STORE_CCOND_DSP:
    expandStoreCCond(MBB, I);
    break;
  case Mips::LOAD_ACC64:
  case Mips::LOAD_ACC64DSP:
    expandLoadACC(MBB, I, 4);
    break;
  case Mips::LOAD_ACC128:
    expandLoadACC(MBB, I, 8);
    break;
  case Mips::STORE_ACC64:
    expandStoreACC(MBB, I, Mips::PseudoMFHI, Mips::PseudoMFLO, 4);
    break;
  case Mips::STORE_ACC64DSP:
    expandStoreACC(MBB, I, Mips::MFHI_DSP, Mips::MFLO_DSP, 4);
    break;
  case Mips::STORE_ACC128:
    expandStoreACC(MBB, I, Mips::PseudoMFHI64, Mips::PseudoMFLO64, 8);
    break;
  case Mips::BuildPairF64:
    if (expandBuildPairF64(MBB, I, false))
      MBB.erase(I);
    return false;
  case Mips::BuildPairF64_64:
    if (expandBuildPairF64(MBB, I, true))
      MBB.erase(I);
    return false;
  case Mips::ExtractElementF64:
    if (expandExtractElementF64(MBB, I, false))
      MBB.erase(I);
    return false;
  case Mips::ExtractElementF64_64:
    if (expandExtractElementF64(MBB, I, true))
      MBB.erase(I);
    return false;
  case TargetOpcode::COPY:
    if (!expandCopy(MBB, I))
      return false;
    break;
  default:
    return false;
  }

  MBB.erase(I);
  return true;
}

void ExpandPseudo::expandLoadCCond(MachineBasicBlock &MBB, Iter I) {
  //  load $vr, FI
  //  copy ccond, $vr

  assert(I->getOperand(0).isReg() && I->getOperand(1).isFI());

  const TargetRegisterClass *RC = RegInfo.intRegClass(4);
  unsigned VR = MRI.createVirtualRegister(RC);
  unsigned Dst = I->getOperand(0).getReg(), FI = I->getOperand(1).getIndex();

  TII.loadRegFromStack(MBB, I, VR, FI, RC, &RegInfo, 0);
  BuildMI(MBB, I, I->getDebugLoc(), TII.get(TargetOpcode::COPY), Dst)
    .addReg(VR, RegState::Kill);
}

void ExpandPseudo::expandStoreCCond(MachineBasicBlock &MBB, Iter I) {
  //  copy $vr, ccond
  //  store $vr, FI

  assert(I->getOperand(0).isReg() && I->getOperand(1).isFI());

  const TargetRegisterClass *RC = RegInfo.intRegClass(4);
  unsigned VR = MRI.createVirtualRegister(RC);
  unsigned Src = I->getOperand(0).getReg(), FI = I->getOperand(1).getIndex();

  BuildMI(MBB, I, I->getDebugLoc(), TII.get(TargetOpcode::COPY), VR)
    .addReg(Src, getKillRegState(I->getOperand(0).isKill()));
  TII.storeRegToStack(MBB, I, VR, true, FI, RC, &RegInfo, 0);
}

void ExpandPseudo::expandLoadACC(MachineBasicBlock &MBB, Iter I,
                                 unsigned RegSize) {
  //  load $vr0, FI
  //  copy lo, $vr0
  //  load $vr1, FI + 4
  //  copy hi, $vr1

  assert(I->getOperand(0).isReg() && I->getOperand(1).isFI());

  const TargetRegisterClass *RC = RegInfo.intRegClass(RegSize);
  unsigned VR0 = MRI.createVirtualRegister(RC);
  unsigned VR1 = MRI.createVirtualRegister(RC);
  unsigned Dst = I->getOperand(0).getReg(), FI = I->getOperand(1).getIndex();
  unsigned Lo = RegInfo.getSubReg(Dst, Mips::sub_lo);
  unsigned Hi = RegInfo.getSubReg(Dst, Mips::sub_hi);
  DebugLoc DL = I->getDebugLoc();
  const MCInstrDesc &Desc = TII.get(TargetOpcode::COPY);

  TII.loadRegFromStack(MBB, I, VR0, FI, RC, &RegInfo, 0);
  BuildMI(MBB, I, DL, Desc, Lo).addReg(VR0, RegState::Kill);
  TII.loadRegFromStack(MBB, I, VR1, FI, RC, &RegInfo, RegSize);
  BuildMI(MBB, I, DL, Desc, Hi).addReg(VR1, RegState::Kill);
}

void ExpandPseudo::expandStoreACC(MachineBasicBlock &MBB, Iter I,
                                  unsigned MFHiOpc, unsigned MFLoOpc,
                                  unsigned RegSize) {
  //  mflo $vr0, src
  //  store $vr0, FI
  //  mfhi $vr1, src
  //  store $vr1, FI + 4

  assert(I->getOperand(0).isReg() && I->getOperand(1).isFI());

  const TargetRegisterClass *RC = RegInfo.intRegClass(RegSize);
  unsigned VR0 = MRI.createVirtualRegister(RC);
  unsigned VR1 = MRI.createVirtualRegister(RC);
  unsigned Src = I->getOperand(0).getReg(), FI = I->getOperand(1).getIndex();
  unsigned SrcKill = getKillRegState(I->getOperand(0).isKill());
  DebugLoc DL = I->getDebugLoc();

  BuildMI(MBB, I, DL, TII.get(MFLoOpc), VR0).addReg(Src);
  TII.storeRegToStack(MBB, I, VR0, true, FI, RC, &RegInfo, 0);
  BuildMI(MBB, I, DL, TII.get(MFHiOpc), VR1).addReg(Src, SrcKill);
  TII.storeRegToStack(MBB, I, VR1, true, FI, RC, &RegInfo, RegSize);
}

bool ExpandPseudo::expandCopy(MachineBasicBlock &MBB, Iter I) {
  unsigned Src = I->getOperand(1).getReg();
  std::pair<unsigned, unsigned> Opcodes = getMFHiLoOpc(Src);

  if (!Opcodes.first)
    return false;

  return expandCopyACC(MBB, I, Opcodes.first, Opcodes.second);
}

bool ExpandPseudo::expandCopyACC(MachineBasicBlock &MBB, Iter I,
                                 unsigned MFHiOpc, unsigned MFLoOpc) {
  //  mflo $vr0, src
  //  copy dst_lo, $vr0
  //  mfhi $vr1, src
  //  copy dst_hi, $vr1

  unsigned Dst = I->getOperand(0).getReg(), Src = I->getOperand(1).getReg();
  const TargetRegisterClass *DstRC = RegInfo.getMinimalPhysRegClass(Dst);
  unsigned VRegSize = RegInfo.getRegSizeInBits(*DstRC) / 16;
  const TargetRegisterClass *RC = RegInfo.intRegClass(VRegSize);
  unsigned VR0 = MRI.createVirtualRegister(RC);
  unsigned VR1 = MRI.createVirtualRegister(RC);
  unsigned SrcKill = getKillRegState(I->getOperand(1).isKill());
  unsigned DstLo = RegInfo.getSubReg(Dst, Mips::sub_lo);
  unsigned DstHi = RegInfo.getSubReg(Dst, Mips::sub_hi);
  DebugLoc DL = I->getDebugLoc();

  BuildMI(MBB, I, DL, TII.get(MFLoOpc), VR0).addReg(Src);
  BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), DstLo)
    .addReg(VR0, RegState::Kill);
  BuildMI(MBB, I, DL, TII.get(MFHiOpc), VR1).addReg(Src, SrcKill);
  BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), DstHi)
    .addReg(VR1, RegState::Kill);
  return true;
}

/// This method expands the same instruction that MipsSEInstrInfo::
/// expandBuildPairF64 does, for the case when ABI is fpxx and mthc1 is not
/// available and the case where the ABI is FP64A. It is implemented here
/// because frame indexes are eliminated before MipsSEInstrInfo::
/// expandBuildPairF64 is called.
bool ExpandPseudo::expandBuildPairF64(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator I,
                                      bool FP64) const {
  // For fpxx and when mthc1 is not available, use:
  //   spill + reload via ldc1
  //
  // The case where dmtc1 is available doesn't need to be handled here
  // because it never creates a BuildPairF64 node.
  //
  // The FP64A ABI (fp64 with nooddspreg) must also use a spill/reload sequence
  // for odd-numbered double precision values (because the lower 32-bits is
  // transferred with mtc1 which is redirected to the upper half of the even
  // register). Unfortunately, we have to make this decision before register
  // allocation so for now we use a spill/reload sequence for all
  // double-precision values in regardless of being an odd/even register.
  if ((Subtarget.isABI_FPXX() && !Subtarget.hasMTHC1()) ||
      (FP64 && !Subtarget.useOddSPReg())) {
    unsigned DstReg = I->getOperand(0).getReg();
    unsigned LoReg = I->getOperand(1).getReg();
    unsigned HiReg = I->getOperand(2).getReg();

    // It should be impossible to have FGR64 on MIPS-II or MIPS32r1 (which are
    // the cases where mthc1 is not available). 64-bit architectures and
    // MIPS32r2 or later can use FGR64 though.
    assert(Subtarget.isGP64bit() || Subtarget.hasMTHC1() ||
           !Subtarget.isFP64bit());

    const TargetRegisterClass *RC = &Mips::GPR32RegClass;
    const TargetRegisterClass *RC2 =
        FP64 ? &Mips::FGR64RegClass : &Mips::AFGR64RegClass;

    // We re-use the same spill slot each time so that the stack frame doesn't
    // grow too much in functions with a large number of moves.
    int FI = MF.getInfo<MipsFunctionInfo>()->getMoveF64ViaSpillFI(RC2);
    if (!Subtarget.isLittle())
      std::swap(LoReg, HiReg);
    TII.storeRegToStack(MBB, I, LoReg, I->getOperand(1).isKill(), FI, RC,
                        &RegInfo, 0);
    TII.storeRegToStack(MBB, I, HiReg, I->getOperand(2).isKill(), FI, RC,
                        &RegInfo, 4);
    TII.loadRegFromStack(MBB, I, DstReg, FI, RC2, &RegInfo, 0);
    return true;
  }

  return false;
}

/// This method expands the same instruction that MipsSEInstrInfo::
/// expandExtractElementF64 does, for the case when ABI is fpxx and mfhc1 is not
/// available and the case where the ABI is FP64A. It is implemented here
/// because frame indexes are eliminated before MipsSEInstrInfo::
/// expandExtractElementF64 is called.
bool ExpandPseudo::expandExtractElementF64(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator I,
                                           bool FP64) const {
  const MachineOperand &Op1 = I->getOperand(1);
  const MachineOperand &Op2 = I->getOperand(2);

  if ((Op1.isReg() && Op1.isUndef()) || (Op2.isReg() && Op2.isUndef())) {
    unsigned DstReg = I->getOperand(0).getReg();
    BuildMI(MBB, I, I->getDebugLoc(), TII.get(Mips::IMPLICIT_DEF), DstReg);
    return true;
  }

  // For fpxx and when mfhc1 is not available, use:
  //   spill + reload via ldc1
  //
  // The case where dmfc1 is available doesn't need to be handled here
  // because it never creates a ExtractElementF64 node.
  //
  // The FP64A ABI (fp64 with nooddspreg) must also use a spill/reload sequence
  // for odd-numbered double precision values (because the lower 32-bits is
  // transferred with mfc1 which is redirected to the upper half of the even
  // register). Unfortunately, we have to make this decision before register
  // allocation so for now we use a spill/reload sequence for all
  // double-precision values in regardless of being an odd/even register.

  if ((Subtarget.isABI_FPXX() && !Subtarget.hasMTHC1()) ||
      (FP64 && !Subtarget.useOddSPReg())) {
    unsigned DstReg = I->getOperand(0).getReg();
    unsigned SrcReg = Op1.getReg();
    unsigned N = Op2.getImm();
    int64_t Offset = 4 * (Subtarget.isLittle() ? N : (1 - N));

    // It should be impossible to have FGR64 on MIPS-II or MIPS32r1 (which are
    // the cases where mfhc1 is not available). 64-bit architectures and
    // MIPS32r2 or later can use FGR64 though.
    assert(Subtarget.isGP64bit() || Subtarget.hasMTHC1() ||
           !Subtarget.isFP64bit());

    const TargetRegisterClass *RC =
        FP64 ? &Mips::FGR64RegClass : &Mips::AFGR64RegClass;
    const TargetRegisterClass *RC2 = &Mips::GPR32RegClass;

    // We re-use the same spill slot each time so that the stack frame doesn't
    // grow too much in functions with a large number of moves.
    int FI = MF.getInfo<MipsFunctionInfo>()->getMoveF64ViaSpillFI(RC);
    TII.storeRegToStack(MBB, I, SrcReg, Op1.isKill(), FI, RC, &RegInfo, 0);
    TII.loadRegFromStack(MBB, I, DstReg, FI, RC2, &RegInfo, Offset);
    return true;
  }

  return false;
}

MipsSEFrameLowering::MipsSEFrameLowering(const MipsSubtarget &STI)
    : MipsFrameLowering(STI, STI.getStackAlignment()) {}

int MipsSEFrameLowering::createUnsafeEndObject(MachineFunction &MF) const {
  MachineFrameInfo &MFI    = MF.getFrameInfo();
  MipsABIInfo ABI = STI.getABI();
  // This is the 'prev' pointer. here.
  return MFI.CreateStackObject(ABI.GetCapSize()*2, ABI.GetCapSize(), true);
}

bool MipsSEFrameLowering::partitionUnsafeObjects(void) const {
  return true;
}

void MipsSEFrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");
  MachineFrameInfo &MFI    = MF.getFrameInfo();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();

  const MipsSEInstrInfo &TII =
      *static_cast<const MipsSEInstrInfo *>(STI.getInstrInfo());
  const MipsRegisterInfo &RegInfo =
      *static_cast<const MipsRegisterInfo *>(STI.getRegisterInfo());

  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl;
  MipsABIInfo ABI = STI.getABI();
  unsigned SP = ABI.GetStackPtr();
  unsigned USP = ABI.GetUnsafeStackPtr();
  unsigned FP = ABI.GetFramePtr();
  unsigned ZERO = ABI.GetNullPtr();
  unsigned MOVE = ABI.GetSPMoveOp();
  unsigned ADDiu = ABI.GetPtrAddiuOp();
  unsigned AND = ABI.IsN64() ? Mips::AND64 : Mips::AND;
  unsigned CTLP = ABI.GetLocalCapability();
  unsigned CRD = ABI.GetReturnData();
  // C0 is always live in.  In non-CHERI MIPS, C0 is treated as a read-only
  // register that is implicitly read by loads and stores.
  MBB.addLiveIn(Mips::DDC);

  bool NeedAuxEntry = ABI.IsCheriPureCap() &&  MF.getFunction().hasExternalLinkage();

  if(ABI.IsCheriPureCap()) {
    MF.getRegInfo().addLiveIn(CRD);
    MBB.addLiveIn(CRD);
    MF.getRegInfo().addLiveIn(CTLP);
    MBB.addLiveIn(CTLP);

    if(NeedAuxEntry) {
      unsigned TA = Mips::C12;
      unsigned TMP = Mips::C14;

      // Create BBs
      MachineBasicBlock &exteralMBB = *MF.CreateMachineBasicBlock(MBB.getBasicBlock());

      MachineFunction::iterator iter = MF.begin();
      MF.insert(iter, &exteralMBB);

      exteralMBB.addSuccessorWithoutProb(&MBB);

      // Add CTLP live in
      exteralMBB.addLiveIn(CTLP);


      // Called for cross domain calls
      MachineBasicBlock::iterator exteralMBBI = exteralMBB.begin();

      //MIBundleBuilder bundleBuilder(exteralMBB, exteralMBBI);

      BuildMI(exteralMBB, exteralMBBI, dl, TII.get(TargetOpcode::AUX_FUNC_START)).addSym(
          MipsFI->getUntrustedExternalEntrySymbol());
      BuildMI(exteralMBB, exteralMBBI, dl, TII.get(Mips::LOADCAP), TMP).addReg(ZERO)
          .addImm(ABI.GetTABILayout()->GetThreadLocalOffset_CDL()).addReg(CTLP);
      // Moved here to fill delay slot

      BuildMI(exteralMBB, exteralMBBI, dl, TII.get(Mips::CJALRSlotless), TA).addReg(TMP);

      // simple_stub     <-- called only if we trust every library we link with (fastest)
      BuildMI(exteralMBB, exteralMBBI, dl, TII.get(TargetOpcode::AUX_FUNC_START)).addSym(
          MipsFI->getTrustedExternalEntrySymbol());
      BuildMI(exteralMBB, exteralMBBI, dl, TII.get(Mips::LOADCAP), SP).addReg(ZERO)
          .addImm(ABI.GetTABILayout()->GetThreadLocalOffset_CSP()).addReg(CTLP);
      BuildMI(exteralMBB, exteralMBBI, dl, TII.get(Mips::LOADCAP), USP).addReg(ZERO)
          .addImm(ABI.GetTABILayout()->GetThreadLocalOffset_CUSP()).addReg(CTLP);

      if (ABI.GetGlobalCapability() != 0) {
        // If this should be live in, make it so
        BuildMI(exteralMBB, exteralMBBI, dl, TII.get(Mips::LOADCAP), ABI.GetGlobalCapability())
            .addReg(ABI.GetNullPtr()).addImm((ABI.GetTABILayout()->GetThreadLocalOffset_CGP()))
            .addReg(CTLP);
      }

      // Function actually starts here
      MF.setHasCustomFunctionStarts(true);
      BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::FUNC_START));
    }
  }

  const TargetRegisterClass *RC = ABI.ArePtrs64bit() ?
        &Mips::GPR64RegClass : &Mips::GPR32RegClass;

  // First, compute final stack size.
  uint64_t StackSize = MFI.getStackSize();
  uint64_t UnsafeStackSize = MFI.getUnsafeStackSize();

  if(MFI.hasStaticUnsafeObjects()) {
    // We allocate and set bounds to ensure the stack is still large enough
    // It will alos fail if there is no stack. Either way get a new one
    int64_t minSize = ABI.GetTABILayout()->GetMinStack_Size();
    // FIXME lasyness made use c15, should be a new virtual register
    if (isInt<11>(minSize) && isInt<11>(-minSize)) {
      BuildMI(MBB, MBBI, dl, TII.get(Mips::CIncOffsetImm), Mips::C15)
          .addReg(USP)
          .addImm(-minSize);
      BuildMI(MBB, MBBI, dl, TII.get(Mips::CSetBoundsImm), Mips::C15)
          .addReg(Mips::C15)
          .addImm(minSize);
    } else {
        unsigned MinReg = TII.loadImmediate(-minSize, MBB, MBBI, dl, nullptr);
        BuildMI(MBB, MBBI, dl, TII.get(Mips::CIncOffset), Mips::C15)
            .addReg(USP)
            .addReg(MinReg);
        BuildMI(MBB, MBBI, dl, TII.get(Mips::DSUBu), MinReg).addReg(ZERO).addReg(MinReg);
        BuildMI(MBB, MBBI, dl, TII.get(Mips::CSetBounds), Mips::C15)
            .addReg(Mips::C15, RegState::Kill)
            .addReg(MinReg, RegState::Kill);
    }

    // Store old csp
    BuildMI(MBB, MBBI, dl, TII.get(Mips::STORECAP))
            .addReg(SP)
            .addReg(ZERO)
            .addImm(-UnsafeStackSize+ABI.GetTABILayout()->GetStackPointerOffset_Prev())
            .addReg(USP);

    // Get new csp. We fold the move with inc offset we need for allocating a frame

    if (isInt<11>(-StackSize)) {
      BuildMI(MBB, MBBI, dl, TII.get(Mips::CIncOffsetImm), SP)
          .addReg(USP).addImm(-StackSize);
    } else {
      unsigned Reg = TII.loadImmediate(-StackSize, MBB, MBBI, dl, nullptr);
      BuildMI(MBB, MBBI, dl, TII.get(Mips::CIncOffset), SP)
          .addReg(SP).addReg(Reg, RegState::Kill);
    }

    // Get new cusp
    BuildMI(MBB, MBBI, dl, TII.get(Mips::LOADCAP), USP)
            .addReg(ZERO)
            .addImm(StackSize + ABI.GetTABILayout()->GetStackPointerOffset_Next())
            .addReg(SP);
  }

  // No need to allocate space on the stack.
  if (StackSize == 0 && !MFI.adjustsStack()) return;

  MachineModuleInfo &MMI = MF.getMMI();
  const MCRegisterInfo *MRI = MMI.getContext().getRegisterInfo();

  // Adjust stack.
  if(!MFI.hasStaticUnsafeObjects()) // We will have allocated with the move
    TII.adjustStackPtr(SP, -StackSize, MBB, MBBI);

  // emit ".cfi_def_cfa_offset StackSize"
  unsigned CFIIndex = MF.addFrameInst(
      MCCFIInstruction::createDefCfaOffset(nullptr, -StackSize));
  BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(CFIIndex);

  if (MF.getFunction().hasFnAttribute("interrupt"))
    emitInterruptPrologueStub(MF, MBB);

  bool IsRASpilled = false;
  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();

  if (!CSI.empty()) {
    // Find the instruction past the last instruction that saves a callee-saved
    // register to the stack.
    for (unsigned i = 0; i < CSI.size(); ++i)
      ++MBBI;

    // Iterate over list of callee-saved registers and emit .cfi_offset
    // directives.
    for (std::vector<CalleeSavedInfo>::const_iterator I = CSI.begin(),
           E = CSI.end(); I != E; ++I) {
      int64_t Offset = MFI.getObjectOffset(I->getFrameIdx());
      unsigned Reg = I->getReg();
      if (Reg == Mips::RA_64 || Reg == Mips::RA)
        IsRASpilled = true;

      // If Reg is a double precision register, emit two cfa_offsets,
      // one for each of the paired single precision registers.
      if (Mips::AFGR64RegClass.contains(Reg)) {
        unsigned Reg0 =
            MRI->getDwarfRegNum(RegInfo.getSubReg(Reg, Mips::sub_lo), true);
        unsigned Reg1 =
            MRI->getDwarfRegNum(RegInfo.getSubReg(Reg, Mips::sub_hi), true);

        if (!STI.isLittle())
          std::swap(Reg0, Reg1);

        unsigned CFIIndex = MF.addFrameInst(
            MCCFIInstruction::createOffset(nullptr, Reg0, Offset));
        BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex);

        CFIIndex = MF.addFrameInst(
            MCCFIInstruction::createOffset(nullptr, Reg1, Offset + 4));
        BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex);
      } else if (Mips::FGR64RegClass.contains(Reg)) {
        unsigned Reg0 = MRI->getDwarfRegNum(Reg, true);
        unsigned Reg1 = MRI->getDwarfRegNum(Reg, true) + 1;

        if (!STI.isLittle())
          std::swap(Reg0, Reg1);

        unsigned CFIIndex = MF.addFrameInst(
          MCCFIInstruction::createOffset(nullptr, Reg0, Offset));
        BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex);

        CFIIndex = MF.addFrameInst(
          MCCFIInstruction::createOffset(nullptr, Reg1, Offset + 4));
        BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex);
      } else {
        // Reg is either in GPR32 or FGR32.
        unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::createOffset(
            nullptr, MRI->getDwarfRegNum(Reg, true), Offset));
        BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex);
      }
    }
  }

  if (MipsFI->callsEhReturn()) {
    // Insert instructions that spill eh data registers.
    for (int I = 0; I < 4; ++I) {
      if (!MBB.isLiveIn(ABI.GetEhDataReg(I)))
        MBB.addLiveIn(ABI.GetEhDataReg(I));
      TII.storeRegToStackSlot(MBB, MBBI, ABI.GetEhDataReg(I), false,
                              MipsFI->getEhDataRegFI(I), RC, &RegInfo);
    }

    // Emit .cfi_offset directives for eh data registers.
    for (int I = 0; I < 4; ++I) {
      int64_t Offset = MFI.getObjectOffset(MipsFI->getEhDataRegFI(I));
      unsigned Reg = MRI->getDwarfRegNum(ABI.GetEhDataReg(I), true);
      unsigned CFIIndex = MF.addFrameInst(
          MCCFIInstruction::createOffset(nullptr, Reg, Offset));
      BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
          .addCFIIndex(CFIIndex);
    }
  }

  // if framepointer enabled, set it to point to the stack pointer.
  if (hasFP(MF)) {
    // Insert instruction "move $fp, $sp" at this location.
    BuildMI(MBB, MBBI, dl, TII.get(MOVE), FP).addReg(SP).addReg(ZERO)
      .setMIFlag(MachineInstr::FrameSetup);

    // emit ".cfi_def_cfa_register $fp"
    unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::createDefCfaRegister(
        nullptr, MRI->getDwarfRegNum(FP, true)));
    BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex);

    if (RegInfo.needsStackRealignment(MF)) {
      // addiu $Reg, $zero, -MaxAlignment
      // andi $sp, $sp, $Reg
      unsigned VR = MF.getRegInfo().createVirtualRegister(RC);
      assert(isInt<16>(MFI.getMaxAlignment()) &&
             "Function's alignment size requirement is not supported.");
      int MaxAlign = -(int)MFI.getMaxAlignment();
      unsigned IntSP = SP;
      if (ABI.IsCheriPureCap()) {
        IntSP = MF.getRegInfo().createVirtualRegister(RC);
        BuildMI(MBB, MBBI, dl, TII.get(Mips::CGetOffset), IntSP).addReg(SP);
      }


      BuildMI(MBB, MBBI, dl, TII.get(ADDiu), VR).addReg(ZERO) .addImm(MaxAlign);
      BuildMI(MBB, MBBI, dl, TII.get(AND), IntSP).addReg(IntSP).addReg(VR);

      if (ABI.IsCheriPureCap())
        BuildMI(MBB, MBBI, dl, TII.get(Mips::CSetOffset), SP).addReg(SP)
          .addReg(IntSP);

      if (hasBP(MF)) {
        // move $s7, $sp
        unsigned BP = ABI.GetBasePtr();
        BuildMI(MBB, MBBI, dl, TII.get(MOVE), BP)
          .addReg(SP)
          .addReg(ZERO);
      }
    }
  }

  // If we've spilled RA and we're targeting CHERI, then we want to use
  // cjr $c16 for return, not jr $ra
  if (CHERICFI && IsRASpilled && STI.isCheri())
    for (auto &BB : MF)
      for (auto &I : BB.terminators())
        if (I.getOpcode() == Mips::RetRA) {
            BuildMI(BB, (MachineBasicBlock::iterator)I, I.getDebugLoc(),
                TII.get(Mips::CJR))
              .addReg(Mips::C16);
            I.eraseFromBundle();
            break;
        }

}

void MipsSEFrameLowering::emitInterruptPrologueStub(
    MachineFunction &MF, MachineBasicBlock &MBB) const {
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // Report an error the target doesn't support Mips32r2 or later.
  // The epilogue relies on the use of the "ehb" to clear execution
  // hazards. Pre R2 Mips relies on an implementation defined number
  // of "ssnop"s to clear the execution hazard. Support for ssnop hazard
  // clearing is not provided so reject that configuration.
  if (!STI.hasMips32r2())
    report_fatal_error(
        "\"interrupt\" attribute is not supported on pre-MIPS32R2 or "
        "MIPS16 targets.");

  // The GP register contains the "user" value, so we cannot perform
  // any gp relative loads until we restore the "kernel" or "system" gp
  // value. Until support is written we shall only accept the static
  // relocation model.
  if ((STI.getRelocationModel() != Reloc::Static))
    report_fatal_error("\"interrupt\" attribute is only supported for the "
                       "static relocation model on MIPS at the present time.");

  if (!STI.isABI_O32() || STI.hasMips64())
    report_fatal_error("\"interrupt\" attribute is only supported for the "
                       "O32 ABI on MIPS32R2+ at the present time.");

  // Perform ISR handling like GCC
  StringRef IntKind =
      MF.getFunction().getFnAttribute("interrupt").getValueAsString();
  const TargetRegisterClass *PtrRC = &Mips::GPR32RegClass;

  // EIC interrupt handling needs to read the Cause register to disable
  // interrupts.
  if (IntKind == "eic") {
    // Coprocessor registers are always live per se.
    MBB.addLiveIn(Mips::COP013);
    BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::MFC0), Mips::K0)
        .addReg(Mips::COP013)
        .addImm(0)
        .setMIFlag(MachineInstr::FrameSetup);

    BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::EXT), Mips::K0)
        .addReg(Mips::K0)
        .addImm(10)
        .addImm(6)
        .setMIFlag(MachineInstr::FrameSetup);
  }

  // Fetch and spill EPC
  MBB.addLiveIn(Mips::COP014);
  BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::MFC0), Mips::K1)
      .addReg(Mips::COP014)
      .addImm(0)
      .setMIFlag(MachineInstr::FrameSetup);

  STI.getInstrInfo()->storeRegToStack(MBB, MBBI, Mips::K1, false,
                                      MipsFI->getISRRegFI(0), PtrRC,
                                      STI.getRegisterInfo(), 0);

  // Fetch and Spill Status
  MBB.addLiveIn(Mips::COP012);
  BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::MFC0), Mips::K1)
      .addReg(Mips::COP012)
      .addImm(0)
      .setMIFlag(MachineInstr::FrameSetup);

  STI.getInstrInfo()->storeRegToStack(MBB, MBBI, Mips::K1, false,
                                      MipsFI->getISRRegFI(1), PtrRC,
                                      STI.getRegisterInfo(), 0);

  // Build the configuration for disabling lower priority interrupts. Non EIC
  // interrupts need to be masked off with zero, EIC from the Cause register.
  unsigned InsPosition = 8;
  unsigned InsSize = 0;
  unsigned SrcReg = Mips::ZERO;

  // If the interrupt we're tied to is the EIC, switch the source for the
  // masking off interrupts to the cause register.
  if (IntKind == "eic") {
    SrcReg = Mips::K0;
    InsPosition = 10;
    InsSize = 6;
  } else
    InsSize = StringSwitch<unsigned>(IntKind)
                  .Case("sw0", 1)
                  .Case("sw1", 2)
                  .Case("hw0", 3)
                  .Case("hw1", 4)
                  .Case("hw2", 5)
                  .Case("hw3", 6)
                  .Case("hw4", 7)
                  .Case("hw5", 8)
                  .Default(0);
  assert(InsSize != 0 && "Unknown interrupt type!");

  BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::INS), Mips::K1)
      .addReg(SrcReg)
      .addImm(InsPosition)
      .addImm(InsSize)
      .addReg(Mips::K1)
      .setMIFlag(MachineInstr::FrameSetup);

  // Mask off KSU, ERL, EXL
  BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::INS), Mips::K1)
      .addReg(Mips::ZERO)
      .addImm(1)
      .addImm(4)
      .addReg(Mips::K1)
      .setMIFlag(MachineInstr::FrameSetup);

  // Disable the FPU as we are not spilling those register sets.
  if (!STI.useSoftFloat())
    BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::INS), Mips::K1)
        .addReg(Mips::ZERO)
        .addImm(29)
        .addImm(1)
        .addReg(Mips::K1)
        .setMIFlag(MachineInstr::FrameSetup);

  // Set the new status
  BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::MTC0), Mips::COP012)
      .addReg(Mips::K1)
      .addImm(0)
      .setMIFlag(MachineInstr::FrameSetup);
}

void MipsSEFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo &MFI            = MF.getFrameInfo();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();

  const MipsSEInstrInfo &TII =
      *static_cast<const MipsSEInstrInfo *>(STI.getInstrInfo());
  const MipsRegisterInfo &RegInfo =
      *static_cast<const MipsRegisterInfo *>(STI.getRegisterInfo());

  DebugLoc DL = MBBI->getDebugLoc();
  MipsABIInfo ABI = STI.getABI();
  unsigned SP = ABI.GetStackPtr();
  unsigned USP = ABI.GetUnsafeStackPtr();
  unsigned FP = ABI.GetFramePtr();
  unsigned ZERO = ABI.GetNullPtr();
  unsigned MOVE = ABI.GetSPMoveOp();

  // No need to clean up if we didn't allocate space on the stack.
  if (MFI.getStackSize() == 0 && !MFI.adjustsStack()) return;

  // if framepointer enabled, restore the stack pointer.
  if (hasFP(MF)) {
    // Find the first instruction that restores a callee-saved register.
    MachineBasicBlock::iterator I = MBBI;

    for (unsigned i = 0; i < MFI.getCalleeSavedInfo().size(); ++i)
      --I;

    // Insert instruction "move $sp, $fp" at this location.
    BuildMI(MBB, I, DL, TII.get(MOVE), SP).addReg(FP).addReg(ZERO);
  }

  if (MipsFI->callsEhReturn()) {
    const TargetRegisterClass *RC =
        ABI.ArePtrs64bit() ? &Mips::GPR64RegClass : &Mips::GPR32RegClass;

    // Find first instruction that restores a callee-saved register.
    MachineBasicBlock::iterator I = MBBI;
    for (unsigned i = 0; i < MFI.getCalleeSavedInfo().size(); ++i)
      --I;

    // Insert instructions that restore eh data registers.
    for (int J = 0; J < 4; ++J) {
      TII.loadRegFromStackSlot(MBB, I, ABI.GetEhDataReg(J),
                               MipsFI->getEhDataRegFI(J), RC, &RegInfo);
    }
  }

  if (MF.getFunction().hasFnAttribute("interrupt"))
    emitInterruptEpilogueStub(MF, MBB);

  // Get the number of bytes from FrameInfo
  uint64_t StackSize = MFI.getStackSize();
  uint64_t UnsafeStackSize = MFI.getUnsafeStackSize();

  uint64_t SafeStackSize = StackSize - UnsafeStackSize;

  if (StackSize == 0)
    return;

  if(MFI.hasStaticUnsafeObjects()) {
    // Down stack folded with adjust

    // I really need this instruction
    // csetboundsback $ca, $cb, im -> cgetoffset $at, $cb; daddiu $at, $at, im; csetoffset $ca, $zero, $zero; csetbounds $ca, $ca, $at; csetoffset $ca, $ca, $at

    // Store old csp
    BuildMI(MBB, MBBI, DL, TII.get(Mips::STORECAP))
            .addReg(USP)
            .addReg(ZERO)
            .addImm(SafeStackSize + ABI.GetTABILayout()->GetStackPointerOffset_Next())
            .addReg(SP);

    // setboundsback $cusp, $csp, SAFE needs a whole bunch of instructions as this instruction does not exist
    // instead we sacrifice a little memory safety and only use incoffset


    BuildMI(MBB, MBBI, DL, TII.get(Mips::CIncOffsetImm), USP)
           .addReg(SP)
           .addImm(SafeStackSize);

    /* Implements set bounds back. Too long.
    BuildMI(MBB, MBBI, DL, TII.get(Mips::CGetOffset), (Mips::AT))
            .addReg(SP);
    BuildMI(MBB, MBBI, DL, TII.get(Mips::DADDiu), Mips::AT)
            .addReg(Mips::AT)
            .addImm(SafeStackSize);
    BuildMI(MBB, MBBI, DL, TII.get(Mips::CSetOffset), USP)
            .addReg(SP)
            .addReg(ZERO);
    BuildMI(MBB, MBBI, DL, TII.get(Mips::CSetBounds), USP)
            .addReg(USP)
            .addReg(Mips::AT);
    BuildMI(MBB, MBBI, DL, TII.get(Mips::CSetOffset), USP)
            .addReg(USP)
            .addReg(Mips::AT);
    */

    // Restore stack
    BuildMI(MBB, MBBI, DL, TII.get(Mips::LOADCAP), (SP))
            .addReg(ZERO)
            .addImm(ABI.GetTABILayout()->GetStackPointerOffset_Prev())
            .addReg(USP);

    // TODO: only if using dynamic linking
    // Save cusp as it may have changed, and this may be a cross domain return
    BuildMI(MBB, MBBI, DL, TII.get(Mips::STORECAP))
            .addReg(USP)
            .addReg(ZERO)
            .addImm(ABI.GetTABILayout()->GetThreadLocalOffset_CUSP())
            .addReg(ABI.GetLocalCapability());
  } else {
    // Adjust stack.
    TII.adjustStackPtr(SP, StackSize, MBB, MBBI);
  }


}

bool MipsSEFrameLowering::assignCalleeSavedSpillSlots(MachineFunction &MF,
          const TargetRegisterInfo *TRI, std::vector<CalleeSavedInfo> &CSI) const {
  // If we're on CHERI and we're compiling for the compatible ABI, then reserve
  // an extra spill slot for the return capability, if $ra is spilled.
  if (CHERICFI && STI.isCheri() && !STI.getABI().IsCheriPureCap())
    for (auto &I : CSI)
      if (I.getReg() == Mips::RA_64) {
        CSI.push_back(CalleeSavedInfo(Mips::C16, 0));
        break;
      }
  return false;
}

void MipsSEFrameLowering::emitInterruptEpilogueStub(
    MachineFunction &MF, MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // Perform ISR handling like GCC
  const TargetRegisterClass *PtrRC = &Mips::GPR32RegClass;

  // Disable Interrupts.
  BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::DI), Mips::ZERO);
  BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::EHB));

  // Restore EPC
  STI.getInstrInfo()->loadRegFromStackSlot(MBB, MBBI, Mips::K1,
                                           MipsFI->getISRRegFI(0), PtrRC,
                                           STI.getRegisterInfo());
  BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::MTC0), Mips::COP014)
      .addReg(Mips::K1)
      .addImm(0);

  // Restore Status
  STI.getInstrInfo()->loadRegFromStackSlot(MBB, MBBI, Mips::K1,
                                           MipsFI->getISRRegFI(1), PtrRC,
                                           STI.getRegisterInfo());
  BuildMI(MBB, MBBI, DL, STI.getInstrInfo()->get(Mips::MTC0), Mips::COP012)
      .addReg(Mips::K1)
      .addImm(0);
}

int MipsSEFrameLowering::getFrameIndexReference(const MachineFunction &MF,
                                                int FI,
                                                unsigned &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  MipsABIInfo ABI = STI.getABI();

  if (MFI.isFixedObjectIndex(FI))
    FrameReg = hasFP(MF) ? ABI.GetFramePtr() : ABI.GetStackPtr();
  else
    FrameReg = hasBP(MF) ? ABI.GetBasePtr() : ABI.GetStackPtr();

  return MFI.getObjectOffset(FI) + MFI.getStackSize() -
         getOffsetOfLocalArea() + MFI.getOffsetAdjustment();
}

bool MipsSEFrameLowering::
spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator MI,
                          const std::vector<CalleeSavedInfo> &CSI,
                          const TargetRegisterInfo *TRI) const {
  MachineFunction *MF = MBB.getParent();
  MachineBasicBlock *EntryBlock = &MF->front();
  const TargetInstrInfo &TII = *STI.getInstrInfo();

  bool IsRetAddrTaken = MF->getFrameInfo().isReturnAddressTaken();
  bool KillRAOnSpill = true;
  if (STI.isCheri() && !STI.getABI().IsCheriPureCap())
    KillRAOnSpill = false;


  for (unsigned i = 0, e = CSI.size(); i != e; ++i) {
    // Add the callee-saved register as live-in. Do not add if the register is
    // RA and return address is taken, because it has already been added in
    // method MipsTargetLowering::lowerRETURNADDR.
    // It's killed at the spill, unless the register is RA and return address
    // is taken.
    unsigned Reg = CSI[i].getReg();
    bool IsRA = (Reg == Mips::RA || Reg == Mips::RA_64);
    if (STI.getABI().IsCheriPureCap())
      IsRA = (Reg == Mips::C17);
    bool IsRAAndRetAddrIsTaken = IsRA && IsRetAddrTaken;
    if (!IsRAAndRetAddrIsTaken)
      EntryBlock->addLiveIn(Reg);
    // $c16 is not a callee-save register, so if we're being asked to spill it
    // then we're actually using it to hold the return address as a capability.
    if (Reg == Mips::C16) {
      BuildMI(MBB, &*MI, MI->getDebugLoc(), TII.get(Mips::CGetPCC), Mips::C16);
      BuildMI(MBB, &*MI, MI->getDebugLoc(), TII.get(Mips::CSetOffset), Mips::C16)
          .addReg(Mips::C16).addReg(Mips::RA_64, RegState::Kill);
      MachineFrameInfo &MFI = MBB.getParent()->getFrameInfo();
      MFI.setObjectAlignment(CSI[i].getFrameIdx(), 32);
    }

    // ISRs require HI/LO to be spilled into kernel registers to be then
    // spilled to the stack frame.
    bool IsLOHI = (Reg == Mips::LO0 || Reg == Mips::LO0_64 ||
                   Reg == Mips::HI0 || Reg == Mips::HI0_64);
    const Function &Func = MBB.getParent()->getFunction();
    if (IsLOHI && Func.hasFnAttribute("interrupt")) {
      DebugLoc DL = MI->getDebugLoc();

      unsigned Op = 0;
      if (!STI.getABI().ArePtrs64bit()) {
        Op = (Reg == Mips::HI0) ? Mips::MFHI : Mips::MFLO;
        Reg = Mips::K0;
      } else {
        Op = (Reg == Mips::HI0) ? Mips::MFHI64 : Mips::MFLO64;
        Reg = Mips::K0_64;
      }
      BuildMI(MBB, MI, DL, TII.get(Op), Mips::K0)
          .setMIFlag(MachineInstr::FrameSetup);
    }

    // Insert the spill to the stack frame.
    bool IsKill = !IsRAAndRetAddrIsTaken && KillRAOnSpill;
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.storeRegToStackSlot(*EntryBlock, MI, Reg, IsKill,
                            CSI[i].getFrameIdx(), RC, TRI);
  }

  return true;
}

bool
MipsSEFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  // Reserve call frame if the size of the maximum call frame fits into 16-bit
  // immediate field and there are no variable sized objects on the stack.
  // Make sure the second register scavenger spill slot can be accessed with one
  // instruction.
  return isInt<16>(MFI.getMaxCallFrameSize() + getStackAlignment()) &&
    !MFI.hasVarSizedObjects();
}

/// Mark \p Reg and all registers aliasing it in the bitset.
static void setAliasRegs(MachineFunction &MF, BitVector &SavedRegs,
                         unsigned Reg) {
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  for (MCRegAliasIterator AI(Reg, TRI, true); AI.isValid(); ++AI)
    SavedRegs.set(*AI);
}

void MipsSEFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                               BitVector &SavedRegs,
                                               RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();
  MipsABIInfo ABI = STI.getABI();
  unsigned FP = ABI.GetFramePtr();
  unsigned BP = ABI.GetBasePtr();

  // Mark $fp as used if function has dedicated frame pointer.
  if (hasFP(MF))
    setAliasRegs(MF, SavedRegs, FP);
  // Mark $s7 as used if function has dedicated base pointer.
  if (hasBP(MF))
    setAliasRegs(MF, SavedRegs, BP);

  // Create spill slots for eh data registers if function calls eh_return.
  if (MipsFI->callsEhReturn())
    MipsFI->createEhDataRegsFI();

  // Create spill slots for Coprocessor 0 registers if function is an ISR.
  if (MipsFI->isISR())
    MipsFI->createISRRegFI();

  // Expand pseudo instructions which load, store or copy accumulators.
  // Add an emergency spill slot if a pseudo was expanded.
  if (ExpandPseudo(MF).expand()) {
    // The spill slot should be half the size of the accumulator. If target have
    // general-purpose registers 64 bits wide, it should be 64-bit, otherwise
    // it should be 32-bit.
    const TargetRegisterClass &RC = STI.isGP64bit() ?
      Mips::GPR64RegClass : Mips::GPR32RegClass;
    int FI = MF.getFrameInfo().CreateStackObject(TRI->getSpillSize(RC),
                                                 TRI->getSpillAlignment(RC),
                                                 false);
    RS->addScavengingFrameIndex(FI);
  }

  // Set scavenging frame index if necessary.
  uint64_t MaxSPOffset = estimateStackSize(MF);

  // We will need the emergency spill slot if we have any stack offsets that
  // don't fit in an immediate.  In normal MIPS code, this means a 16-bit
  // field, for CHERI code we have an 11-bit immediate (plus a 4-bit shift) for
  // csc / cld, but only 8-bit immediates (plus a 3-bit shift) for c[ls]x
  // instructions for other spills.  The latter only matters if we're using the
  // capability-stack ABI.
  // XXXAR: no longer true if we have the big immediate CLC enabled
  if (STI.isCheri()) {
    if (STI.isABI_CheriPureCap()) {
      if (isInt<10>(MaxSPOffset))
        return;
    } else if (isInt<15>(MaxSPOffset))
      return;
  }
  // MSA has a minimum offset of 10 bits signed. If there is a variable
  // sized object on the stack, the estimation cannot account for it.
  else if (isIntN(STI.hasMSA() ? 10 : 16, MaxSPOffset) &&
      !MF.getFrameInfo().hasVarSizedObjects())
    return;

  const TargetRegisterClass &RC =
      ABI.ArePtrs64bit() ? Mips::GPR64RegClass : Mips::GPR32RegClass;
  int FI = MF.getFrameInfo().CreateStackObject(TRI->getSpillSize(RC),
                                               TRI->getSpillAlignment(RC),
                                               false);
  RS->addScavengingFrameIndex(FI);
}

const MipsFrameLowering *
llvm::createMipsSEFrameLowering(const MipsSubtarget &ST) {
  return new MipsSEFrameLowering(ST);
}
