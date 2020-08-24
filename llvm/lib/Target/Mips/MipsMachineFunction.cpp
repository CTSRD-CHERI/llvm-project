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
#include "llvm/MC/MCContext.h"
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

Register MipsFunctionInfo::getGlobalBaseRegUnchecked() const {
  assert(GlobalBaseReg);
  return GlobalBaseReg;
}

Register MipsFunctionInfo::getGlobalBaseReg(MachineFunction &MF, bool IsForTls) {
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

bool MipsFunctionInfo::capLocalBaseRegSet() const { return CapLocalBaseReg; }

Register MipsFunctionInfo::getCapGlobalBaseReg(MachineFunction &MF) {
  // Return if it has already been initialized.
  if (CapGlobalBaseReg)
    return CapGlobalBaseReg;

  const TargetRegisterClass *RC = &Mips::CheriGPRRegClass;
  return CapGlobalBaseReg = MF.getRegInfo().createVirtualRegister(RC);
}

Register MipsFunctionInfo::getCapLocalBaseReg(MachineFunction &MF) {
  // Return if it has already been initialized.
  if (CapLocalBaseReg)
    return CapLocalBaseReg;

  const TargetRegisterClass *RC = &Mips::CheriGPRRegClass;
  return CapLocalBaseReg = MF.getRegInfo().createVirtualRegister(RC);
}

Register MipsFunctionInfo::getCapGlobalBaseRegForGlobalISel(MachineFunction &MF) {
  assert(static_cast<const MipsSubtarget&>(MF.getSubtarget()).useCheriCapTable());
  if (!CapGlobalBaseReg) {
    getCapGlobalBaseReg(MF);
    initCapGlobalBaseReg(MF);
  }
  return CapGlobalBaseReg;
}

Register MipsFunctionInfo::getCapLocalBaseRegForGlobalISel(MachineFunction &MF) {
  assert(
      static_cast<const MipsSubtarget &>(MF.getSubtarget()).useCheriCapTable());
  if (!CapLocalBaseReg) {
    getCapLocalBaseReg(MF);
    initCapLocalBaseReg(MF);
  }
  return CapLocalBaseReg;
}

Register MipsFunctionInfo::getCapEntryPointReg(MachineFunction &MF) {
  // Return if it has already been initialized.
  if (CapComputedEntryPoint)
    return CapComputedEntryPoint;
  // LETODO: CheriOS solves C12 being sealed with an extra entry. Might need an
  // extra case here.
  const MipsABIInfo &ABI =
      static_cast<const MipsTargetMachine &>(MF.getTarget()).getABI();
  assert(ABI.IsCheriPureCap());
  MachineBasicBlock &MBB = MF.front();
  Register Tmp = MF.getRegInfo().createVirtualRegister(&Mips::CheriGPRRegClass);
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL;
  // FIXME: This is ugly but currently required to unseal $c12 in case it is a
  // sentry. We should instead add new instructions to improve this pattern
  // cgetpcc $c1
  // daddiu $1, $zero, %pcrel_lo(.Lentry_point+4) # +4 since the getpcc is on the previous line
  // cincoffset $c2, $c1, $1
  // XXX: ugly const_cast to ensure the label is emitted
  MBB.setHasAddressTaken();
  auto BA = BlockAddress::get(const_cast<BasicBlock *>(MBB.getBasicBlock()));
  auto MBBSym = MachineOperand::CreateBA(BA, 0, 0);
  // XXX: we could do this in two instructions instead of three since we
  // know that the offset will be smaller than 16 bits
  // Since we need to ensure that these instrs are written together we need to
  // bundle them. However, this is only possible after register allocation so
  // we convert this to another pseudo instruction first.
  auto I = MBB.begin();
  Register Scratch = MF.getRegInfo().createVirtualRegister(&Mips::GPR64RegClass);
  BuildMI(MBB, I, DL, TII.get(Mips::PseudoPccRelativeAddressPostRA), Tmp)
      .add(MBBSym)
      .addReg(Scratch, RegState::Define | RegState::EarlyClobber |
          RegState::Implicit | RegState::Dead);
  CapComputedEntryPoint = Tmp;
  return Tmp;
}

Register MipsFunctionInfo::getGlobalBaseRegForGlobalISel(MachineFunction &MF) {
  if (static_cast<const MipsSubtarget&>(MF.getSubtarget()).useCheriCapTable()) {
    if (!CapGlobalBaseReg) {
      getCapGlobalBaseReg(MF);
      initCapGlobalBaseReg(MF);
    }
    return CapGlobalBaseReg;
  }
  if (!GlobalBaseReg) {
    getGlobalBaseReg(MF, false);
    initGlobalBaseReg(MF);
  }
  return GlobalBaseReg;
}

void MipsFunctionInfo::initGlobalBaseReg(MachineFunction &MF) {
  if (!GlobalBaseReg)
    return;

  MachineBasicBlock &MBB = MF.front();
  MachineBasicBlock::iterator I = MBB.begin();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL;
  const TargetRegisterClass *RC;
  const MipsABIInfo &ABI =
      static_cast<const MipsTargetMachine &>(MF.getTarget()).getABI();
  RC = (ABI.IsN64()) ? &Mips::GPR64RegClass : &Mips::GPR32RegClass;

  Register V0 = RegInfo.createVirtualRegister(RC);
  Register V1 = RegInfo.createVirtualRegister(RC);

  if (ABI.IsN64()) {
    if (ABI.IsCheriPureCap()) {
      if (ABI.IsCheriPureCap()) {
        assert(usesTlsViaGlobalReg() && "$gp should only be used for TLS");
        assert((CapGlobalBaseReg || !MF.getRegInfo().isLiveIn(Mips::C12)) &&
                    "C12 should not be used yet");
      }
      MF.getRegInfo().addLiveIn(Mips::C12);
      MBB.addLiveIn(Mips::C12);
      assert(!CapABIEntryPointReg.isValid() && "Should not be used yet");
      CapABIEntryPointReg = RegInfo.createVirtualRegister(&Mips::CheriGPRRegClass);
      BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), CapABIEntryPointReg)
          .addReg(Mips::C12);
      BuildMI(MBB, I, DL, TII.get(Mips::CGetOffset))
        .addReg(Mips::T9_64, RegState::Define)
        .addReg(CapABIEntryPointReg);
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

void MipsFunctionInfo::initCapGlobalBaseReg(MachineFunction &MF) {
  if (!CapGlobalBaseReg)
    return;

  auto &TM = static_cast<const MipsTargetMachine &>(MF.getTarget());
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
    // We also support a mode where we derive the $cgp register from $pcc.
    // This is closer to what MIPS does and has the advantage that we don't
    // need to reserve $cgp since we can always derive it from $pcc.
    // However, in this mode we can't restrict the bounds or permissions on $pcc
    // very much since we need it to always span the entire captable as well
    // as have the Load_Capability permission. If we decide to
    // also inline global statics into the cap-table it also needs to contain
    // write permissions which means we'd be back to relying only on the MMU to
    // protect the .text segment.

#if 0 // old code relative to $c12 (entry point cap), doesn't work with sentries
    // For now I'll just add a new relocation or see if I can convert itm
    MachineRegisterInfo &RegInfo = MF.getRegInfo();
    const GlobalValue *FName = &MF.getFunction();
    Register Tmp1 = RegInfo.createVirtualRegister(&Mips::GPR64RegClass);
    Register Tmp2 = RegInfo.createVirtualRegister(&Mips::GPR64RegClass);
    BuildMI(MBB, I, DL, TII.get(Mips::LUi64), Tmp1)
        .addGlobalAddress(FName, 0, MipsII::MO_CAPTABLE_OFF_HI);
    BuildMI(MBB, I, DL, TII.get(Mips::DADDiu), Tmp2)
        .addReg(Tmp1)
        .addGlobalAddress(FName, 0, MipsII::MO_CAPTABLE_OFF_LO);
    // Generate the globals in $cgp to make the llvm-objdump output more useful
    // since it will guess which global is being loaded. Also I think that if
    // we explicitly use $cgp here the register allocator will be able to use
    // one additional register (since it can optimize away the copy node and
    // just use $cgp directly for loads)
    if (MF.getTarget().getOptLevel() == CodeGenOpt::None) {
        // At -O0 ensure that the global register ends up in $c26
        BuildMI(MBB, I, DL, TII.get(Mips::CIncOffset), ABIGlobalCapReg)
            .addReg(getCapEntryPointReg())
            .addReg(Tmp2);
        BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), CapGlobalBaseReg)
            .addReg(ABIGlobalCapReg);
    } else {
        // Otherwise we will use the first available callee-save register for $cgp
        BuildMI(MBB, I, DL, TII.get(Mips::CIncOffset), CapGlobalBaseReg)
            .addReg(getCapEntryPointReg())
            .addReg(Tmp2);
    }
#else
    // Since we need to ensure that these instrs are written together we need to
    // bundle them. However, this is only possible after register allocation so
    // we convert this to another pseudo instruction first.
    Register Scratch = MF.getRegInfo().createVirtualRegister(&Mips::GPR64RegClass);
    BuildMI(MBB, I, DL, TII.get(Mips::PseudoPccRelativeAddressPostRA), CapGlobalBaseReg)
        .addExternalSymbol("_CHERI_CAPABILITY_TABLE_")
        .addReg(Scratch, RegState::Define | RegState::EarlyClobber |
            RegState::Implicit | RegState::Dead);
#endif
  } else {
    if (ABI.IsCheriOS() && ABIGlobalCapReg == 0) {
      // CheriOS can reload the global register from the local one if there
      // is no ABI defined global captable register
      getCapLocalBaseRegForGlobalISel(MF);
      BuildMI(MBB, I, DL, TII.get(Mips::LOADCAP), CapGlobalBaseReg)
          .addReg(ABI.GetNullPtr())
          .addImm((ABI.GetTABILayout()->GetThreadLocalOffset_CGP()))
          .addReg(CapLocalBaseReg);
    } else {
      MF.getRegInfo().addLiveIn(ABIGlobalCapReg);
      MBB.addLiveIn(ABIGlobalCapReg);
      BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), CapGlobalBaseReg)
          .addReg(ABIGlobalCapReg);
    }
  }
}

void MipsFunctionInfo::initCapLocalBaseReg(MachineFunction &MF) {
  if (!CapLocalBaseReg)
    return;

  auto &TM = static_cast<const MipsTargetMachine &>(MF.getTarget());
  assert(TM.getABI().IsCheriOS());

  MachineBasicBlock &MBB = MF.front();
  MachineBasicBlock::iterator I = MBB.begin();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL;
  const MipsABIInfo &ABI = TM.getABI();

  assert(ABI.IsCheriOS());
  const unsigned ABILocalCapReg = ABI.GetLocalCapability();

  if (ABILocalCapReg != 0) {
    MF.getRegInfo().addLiveIn(ABILocalCapReg);
    MBB.addLiveIn(ABILocalCapReg);
    BuildMI(MBB, I, DL, TII.get(TargetOpcode::COPY), CapLocalBaseReg)
        .addReg(ABILocalCapReg);
  }
}

void MipsFunctionInfo::createEhDataRegsFI(MachineFunction &MF) {
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  for (int I = 0; I < 4; ++I) {
    const TargetRegisterClass &RC =
        static_cast<const MipsTargetMachine &>(MF.getTarget()).getABI().IsN64()
            ? Mips::GPR64RegClass
            : Mips::GPR32RegClass;

    EhDataRegFI[I] = MF.getFrameInfo().CreateStackObject(
        TRI.getSpillSize(RC), TRI.getSpillAlign(RC), false);
  }
}

void MipsFunctionInfo::createISRRegFI(MachineFunction &MF) {
  // ISRs require spill slots for Status & ErrorPC Coprocessor 0 registers.
  // The current implementation only supports Mips32r2+ not Mips64rX. Status
  // is always 32 bits, ErrorPC is 32 or 64 bits dependent on architecture,
  // however Mips32r2+ is the supported architecture.
  const TargetRegisterClass &RC = Mips::GPR32RegClass;
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();

  for (int I = 0; I < 2; ++I)
    ISRDataRegFI[I] = MF.getFrameInfo().CreateStackObject(
        TRI.getSpillSize(RC), TRI.getSpillAlign(RC), false);
}

bool MipsFunctionInfo::isEhDataRegFI(int FI) const {
  return CallsEhReturn && (FI == EhDataRegFI[0] || FI == EhDataRegFI[1]
                        || FI == EhDataRegFI[2] || FI == EhDataRegFI[3]);
}

bool MipsFunctionInfo::isISRRegFI(int FI) const {
  return IsISR && (FI == ISRDataRegFI[0] || FI == ISRDataRegFI[1]);
}
MachinePointerInfo MipsFunctionInfo::callPtrInfo(MachineFunction &MF,
                                                 const char *ES) {
  return MachinePointerInfo(MF.getPSVManager().getExternalSymbolCallEntry(ES));
}

MachinePointerInfo MipsFunctionInfo::callPtrInfo(MachineFunction &MF,
                                                 const GlobalValue *GV) {
  return MachinePointerInfo(MF.getPSVManager().getGlobalValueCallEntry(GV));
}

int MipsFunctionInfo::getMoveF64ViaSpillFI(MachineFunction &MF,
                                           const TargetRegisterClass *RC) {
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  if (MoveF64ViaSpillFI == -1) {
    MoveF64ViaSpillFI = MF.getFrameInfo().CreateStackObject(
        TRI.getSpillSize(*RC), TRI.getSpillAlign(*RC), false);
  }
  return MoveF64ViaSpillFI;
}

void MipsFunctionInfo::anchor() {}

MCSymbol *
MipsFunctionInfo::getUntrustedExternalEntrySymbol(MachineFunction &MF) {
  if (!UntrustedExternalEntrySymbol) {
    MCContext &Ctx = MF.getContext();
    UntrustedExternalEntrySymbol =
        Ctx.getOrCreateSymbol(Twine("__cross_domain_") + Twine(MF.getName()));
  }
  return UntrustedExternalEntrySymbol;
}
MCSymbol *MipsFunctionInfo::getTrustedExternalEntrySymbol(MachineFunction &MF) {
  if (!TrustedExternalEntrySymbol) {
    MCContext &Ctx = MF.getContext();
    TrustedExternalEntrySymbol = Ctx.getOrCreateSymbol(
        Twine("__cross_domain_trusted_") + Twine(MF.getName()));
  }
  return TrustedExternalEntrySymbol;
}
