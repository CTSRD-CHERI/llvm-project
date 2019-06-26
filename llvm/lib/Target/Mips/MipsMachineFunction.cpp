//===-- MipsMachineFunctionInfo.cpp - Private data used for Mips ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MipsMachineFunction.h"
#include "MCTargetDesc/MipsABIInfo.h"
#include "MipsSubtarget.h"
#include "MipsTargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

static cl::opt<bool>
FixGlobalBaseReg("mips-fix-global-base-reg", cl::Hidden, cl::init(true),
                 cl::desc("Always use $gp as the global base register."));

MipsFunctionInfo::~MipsFunctionInfo() = default;

bool MipsFunctionInfo::globalBaseRegSet() const {
  return GlobalBaseReg;
}

static const TargetRegisterClass &getGlobalBaseRegClass(MachineFunction &MF) {
  auto &STI = static_cast<const MipsSubtarget &>(MF.getSubtarget());
  auto &TM = static_cast<const MipsTargetMachine &>(MF.getTarget());

  if (STI.inMips16Mode())
    return Mips::CPU16RegsRegClass;

  if (STI.inMicroMipsMode())
    return Mips::GPRMM16RegClass;

  if (TM.getABI().IsN64())
    return Mips::GPR64RegClass;

  return Mips::GPR32RegClass;
}

unsigned MipsFunctionInfo::getGlobalBaseRegUnchecked() const {
  assert(GlobalBaseReg);
  return GlobalBaseReg;
}

unsigned MipsFunctionInfo::getGlobalBaseReg(bool IsForTls) {
  if (IsForTls)
    UsesTlsViaGlobalReg = true;
  else
    assert(!static_cast<const MipsSubtarget &>(MF.getSubtarget()).useCheriCapTable() &&
           "$gp should only be used for TLS in cap-table mode");

  // Return if it has already been initialized.
  if (!GlobalBaseReg)
    GlobalBaseReg =
        MF.getRegInfo().createVirtualRegister(&getGlobalBaseRegClass(MF));
  return GlobalBaseReg;
}

bool MipsFunctionInfo::capGlobalBaseRegSet() const {
  return CapGlobalBaseReg;
}

unsigned MipsFunctionInfo::getCapGlobalBaseReg() {
  // Return if it has already been initialized.
  if (CapGlobalBaseReg)
    return CapGlobalBaseReg;

  const TargetRegisterClass *RC = &Mips::CheriGPRRegClass;
  return CapGlobalBaseReg = MF.getRegInfo().createVirtualRegister(RC);
}

unsigned MipsFunctionInfo::getCapGlobalBaseRegForGlobalISel() {
  assert(static_cast<const MipsSubtarget&>(MF.getSubtarget()).useCheriCapTable());
  if (!CapGlobalBaseReg) {
    getCapGlobalBaseReg();
    initCapGlobalBaseReg();
  }
  return CapGlobalBaseReg;
}

unsigned MipsFunctionInfo::getGlobalBaseRegForGlobalISel() {
  if (static_cast<const MipsSubtarget&>(MF.getSubtarget()).useCheriCapTable()) {
    if (!CapGlobalBaseReg) {
      getCapGlobalBaseReg();
      initCapGlobalBaseReg();
    }
    return CapGlobalBaseReg;
  }
  if (!GlobalBaseReg) {
    getGlobalBaseReg(false);
    initGlobalBaseReg();
  }
  return GlobalBaseReg;
}

void MipsFunctionInfo::initGlobalBaseReg() {
  if (!GlobalBaseReg)
    return;

  MachineBasicBlock &MBB = MF.front();
  MachineBasicBlock::iterator I = MBB.begin();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL;
  unsigned V0, V1;
  const TargetRegisterClass *RC;
  const MipsABIInfo &ABI =
      static_cast<const MipsTargetMachine &>(MF.getTarget()).getABI();
  RC = (ABI.IsN64()) ? &Mips::GPR64RegClass : &Mips::GPR32RegClass;

  V0 = RegInfo.createVirtualRegister(RC);
  V1 = RegInfo.createVirtualRegister(RC);

  if (ABI.IsN64()) {
    if (ABI.IsCheriPureCap()) {
      if (ABI.UsesCapabilityTable()) {
        assert(usesTlsViaGlobalReg() && "$gp should only be used for TLS");
        assert((CapGlobalBaseReg || !MF.getRegInfo().isLiveIn(Mips::C12)) &&
                    "C12 should not be used yet");
      }
      if (!MF.getRegInfo().isLiveIn(Mips::C12))
        MF.getRegInfo().addLiveIn(Mips::C12);
      MBB.addLiveIn(Mips::C12);
      BuildMI(MBB, I, DL, TII.get(Mips::CGetOffset))
        .addReg(Mips::T9_64, RegState::Define)
        .addReg(Mips::C12);
    } else {
      MF.getRegInfo().addLiveIn(Mips::T9_64);
      MBB.addLiveIn(Mips::T9_64);
    }

    // lui $v0, %hi(%neg(%gp_rel(fname)))
    // daddu $v1, $v0, $t9
    // daddiu $globalbasereg, $v1, %lo(%neg(%gp_rel(fname)))
    const GlobalValue *FName = &MF.getFunction();
    BuildMI(MBB, I, DL, TII.get(Mips::LUi64), V0)
        .addGlobalAddress(FName, 0, MipsII::MO_GPOFF_HI);
    BuildMI(MBB, I, DL, TII.get(Mips::DADDu), V1).addReg(V0)
        .addReg(Mips::T9_64);
    BuildMI(MBB, I, DL, TII.get(Mips::DADDiu), GlobalBaseReg).addReg(V1)
        .addGlobalAddress(FName, 0, MipsII::MO_GPOFF_LO);
    return;
  }

  if (!MF.getTarget().isPositionIndependent()) {
    // Set global register to __gnu_local_gp.
    //
    // lui   $v0, %hi(__gnu_local_gp)
    // addiu $globalbasereg, $v0, %lo(__gnu_local_gp)
    BuildMI(MBB, I, DL, TII.get(Mips::LUi), V0)
        .addExternalSymbol("__gnu_local_gp", MipsII::MO_ABS_HI);
    BuildMI(MBB, I, DL, TII.get(Mips::ADDiu), GlobalBaseReg).addReg(V0)
        .addExternalSymbol("__gnu_local_gp", MipsII::MO_ABS_LO);
    return;
  }

  MF.getRegInfo().addLiveIn(Mips::T9);
  MBB.addLiveIn(Mips::T9);

  if (ABI.IsN32()) {
    // lui $v0, %hi(%neg(%gp_rel(fname)))
    // addu $v1, $v0, $t9
    // addiu $globalbasereg, $v1, %lo(%neg(%gp_rel(fname)))
    const GlobalValue *FName = &MF.getFunction();
    BuildMI(MBB, I, DL, TII.get(Mips::LUi), V0)
        .addGlobalAddress(FName, 0, MipsII::MO_GPOFF_HI);
    BuildMI(MBB, I, DL, TII.get(Mips::ADDu), V1).addReg(V0).addReg(Mips::T9);
    BuildMI(MBB, I, DL, TII.get(Mips::ADDiu), GlobalBaseReg).addReg(V1)
        .addGlobalAddress(FName, 0, MipsII::MO_GPOFF_LO);
    return;
  }

  assert(ABI.IsO32());

  // For O32 ABI, the following instruction sequence is emitted to initialize
  // the global base register:
  //
  //  0. lui   $2, %hi(_gp_disp)
  //  1. addiu $2, $2, %lo(_gp_disp)
  //  2. addu  $globalbasereg, $2, $t9
  //
  // We emit only the last instruction here.
  //
  // GNU linker requires that the first two instructions appear at the beginning
  // of a function and no instructions be inserted before or between them.
  // The two instructions are emitted during lowering to MC layer in order to
  // avoid any reordering.
  //
  // Register $2 (Mips::V0) is added to the list of live-in registers to ensure
  // the value instruction 1 (addiu) defines is valid when instruction 2 (addu)
  // reads it.
  MF.getRegInfo().addLiveIn(Mips::V0);
  MBB.addLiveIn(Mips::V0);
  BuildMI(MBB, I, DL, TII.get(Mips::ADDu), GlobalBaseReg)
      .addReg(Mips::V0).addReg(Mips::T9);
}

void MipsFunctionInfo::initCapGlobalBaseReg() {
  if (!CapGlobalBaseReg)
    return;

  auto &TM = static_cast<const MipsTargetMachine &>(MF.getTarget());
  assert(TM.getABI().UsesCapabilityTable());
  // assert(MF.getTarget().isPositionIndependent() && "CHERI CODEGEN REQUIRES -fPIC");

  MachineBasicBlock &MBB = MF.front();
  MachineBasicBlock::iterator I = MBB.begin();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL;
  const MipsABIInfo &ABI = TM.getABI();

  assert(ABI.IsCheriPureCap());
  const unsigned ABIGlobalCapReg = ABI.GetGlobalCapability();

  // For the purecap ABI, $cgp is required to point to the function's/DSOs
  // capability table on function entry, so emit a single COPY
  // (which may be optimised away):
  // COPY $capglobalbasereg, $c26
  if (MCTargetOptions::cheriCapabilityTableABI() ==
      CheriCapabilityTableABI::Pcrel) {
    // However, we also support an experimental mode where we derive the $cgp
    // register from $pcc (this is closer to what MIPS does and has the
    // advantage that we don't need to reserve $cgp since we can always derive
    // it from $pcc. However, in this mode we can't restrict the bounds or
    // permissions on $pcc very much since we need it to always contain the
    // whole cap table as well as Load_Capability permission. If we decide to
    // also inline global statics into the cap-table it also needs to contain
    // write permissions which means we're back to relying only on the MMU to
    // protect the .text segment.

    // XXXAR: Mips::C12 may already be marked as live-in if we use TLS
    if (MF.getRegInfo().isLiveIn(Mips::C12)) {
      assert(usesTlsViaGlobalReg() && "$gp should only be used for TLS");
      assert(MBB.isLiveIn(Mips::C12)); // should have been added by the TLS hack
    } else {
      MF.getRegInfo().addLiveIn(Mips::C12);
      MBB.addLiveIn(Mips::C12);
    }

    // FIXME: there doesn't seem to be an easy way to get a label difference
    // For now I'll just add a new relocation or see if I can convert itm
    MachineRegisterInfo &RegInfo = MF.getRegInfo();
    const GlobalValue *FName = &MF.getFunction();
    unsigned Tmp1 = RegInfo.createVirtualRegister(&Mips::GPR64RegClass);
    unsigned Tmp2 = RegInfo.createVirtualRegister(&Mips::GPR64RegClass);
    BuildMI(MBB, I, DL, TII.get(Mips::LUi64), Tmp1)
        .addGlobalAddress(FName, 0, MipsII::MO_CAPTABLE_OFF_HI);
    BuildMI(MBB, I, DL, TII.get(Mips::DADDiu), Tmp2)
        .addReg(Tmp1)
        .addGlobalAddress(FName, 0, MipsII::MO_CAPTABLE_OFF_LO);
#if 0
    BuildMI(MBB, I, DL, TII.get(Mips::CIncOffset), CapGlobalBaseReg)
        .addReg(Mips::C12)
        .addReg(Tmp2);
    // XXXAR: the hints are being ignored (probably since $cgp is reserved?)
    RegInfo.addRegAllocationHint(CapGlobalBaseReg, ABIGlobalCapReg);
    RegInfo.setSimpleHint(CapGlobalBaseReg, ABIGlobalCapReg);
#else
    // Generate the globals in $cgp to make the llvm-objdump output more useful
    // since it will guess which global is being loaded. Also I think that if
    // we explicitly use $cgp here the register allocator will be able to use
    // one additional register (since it can optimize away the copy node and
    // just use $cgp directly for loads)
    if (MF.getTarget().getOptLevel() == CodeGenOpt::None) {
        // At -O0 ensure that the global register ends up in $c26
        BuildMI(MBB, I, DL, TII.get(Mips::CIncOffset), ABIGlobalCapReg)
            .addReg(Mips::C12)
            .addReg(Tmp2);
        BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), CapGlobalBaseReg)
            .addReg(ABIGlobalCapReg);
    } else {
        // Otherwise we will use the first available callee-save register for $cgp
        BuildMI(MBB, I, DL, TII.get(Mips::CIncOffset), CapGlobalBaseReg)
            .addReg(Mips::C12)
            .addReg(Tmp2);
    }
#endif
  } else {
    MF.getRegInfo().addLiveIn(ABIGlobalCapReg);
    MBB.addLiveIn(ABIGlobalCapReg);
    BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), CapGlobalBaseReg)
        .addReg(ABIGlobalCapReg);
  }
}


void MipsFunctionInfo::createEhDataRegsFI() {
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  for (int I = 0; I < 4; ++I) {
    const TargetRegisterClass &RC =
        static_cast<const MipsTargetMachine &>(MF.getTarget()).getABI().IsN64()
            ? Mips::GPR64RegClass
            : Mips::GPR32RegClass;

    EhDataRegFI[I] = MF.getFrameInfo().CreateStackObject(TRI.getSpillSize(RC),
        TRI.getSpillAlignment(RC), false);
  }
}

void MipsFunctionInfo::createISRRegFI() {
  // ISRs require spill slots for Status & ErrorPC Coprocessor 0 registers.
  // The current implementation only supports Mips32r2+ not Mips64rX. Status
  // is always 32 bits, ErrorPC is 32 or 64 bits dependent on architecture,
  // however Mips32r2+ is the supported architecture.
  const TargetRegisterClass &RC = Mips::GPR32RegClass;
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();

  for (int I = 0; I < 2; ++I)
    ISRDataRegFI[I] = MF.getFrameInfo().CreateStackObject(
        TRI.getSpillSize(RC), TRI.getSpillAlignment(RC), false);
}

bool MipsFunctionInfo::isEhDataRegFI(int FI) const {
  return CallsEhReturn && (FI == EhDataRegFI[0] || FI == EhDataRegFI[1]
                        || FI == EhDataRegFI[2] || FI == EhDataRegFI[3]);
}

bool MipsFunctionInfo::isISRRegFI(int FI) const {
  return IsISR && (FI == ISRDataRegFI[0] || FI == ISRDataRegFI[1]);
}
MachinePointerInfo MipsFunctionInfo::callPtrInfo(const char *ES) {
  return MachinePointerInfo(MF.getPSVManager().getExternalSymbolCallEntry(ES));
}

MachinePointerInfo MipsFunctionInfo::callPtrInfo(const GlobalValue *GV) {
  return MachinePointerInfo(MF.getPSVManager().getGlobalValueCallEntry(GV));
}

int MipsFunctionInfo::getMoveF64ViaSpillFI(const TargetRegisterClass *RC) {
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  if (MoveF64ViaSpillFI == -1) {
    MoveF64ViaSpillFI = MF.getFrameInfo().CreateStackObject(
        TRI.getSpillSize(*RC), TRI.getSpillAlignment(*RC), false);
  }
  return MoveF64ViaSpillFI;
}

void MipsFunctionInfo::anchor() {}
