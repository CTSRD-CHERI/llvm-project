//===-- MipsMachineFunctionInfo.cpp - Private data used for Mips ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MipsMachineFunction.h"
#include "llvm/MC/MCContext.h"
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
  if (GlobalBaseReg)
    return GlobalBaseReg;

  MipsSubtarget const &STI =
      static_cast<const MipsSubtarget &>(MF.getSubtarget());

  const TargetRegisterClass *RC =
      STI.inMips16Mode()
          ? &Mips::CPU16RegsRegClass
          : STI.inMicroMipsMode()
                ? &Mips::GPRMM16RegClass
                : static_cast<const MipsTargetMachine &>(MF.getTarget())
                          .getABI()
                          .IsN64()
                      ? &Mips::GPR64RegClass
                      : &Mips::GPR32RegClass;
  return GlobalBaseReg = MF.getRegInfo().createVirtualRegister(RC);
}

bool MipsFunctionInfo::capGlobalBaseRegSet() const {
  return CapGlobalBaseReg;
}

bool MipsFunctionInfo::capLocalBaseRegSet() const {
  return CapLocalBaseReg;
}

unsigned MipsFunctionInfo::getCapGlobalBaseReg() {
  // Return if it has already been initialized.
  if (CapGlobalBaseReg)
    return CapGlobalBaseReg;

  const TargetRegisterClass *RC = &Mips::CheriGPRRegClass;
  return CapGlobalBaseReg = MF.getRegInfo().createVirtualRegister(RC);
}


unsigned MipsFunctionInfo::getCapLocalBaseReg() {
  // Return if it has already been initialized.
  if (CapLocalBaseReg)
    return CapLocalBaseReg;

  const TargetRegisterClass *RC = &Mips::CheriGPRRegClass;
  return CapLocalBaseReg = MF.getRegInfo().createVirtualRegister(RC);
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

MCSymbol* MipsFunctionInfo::getUntrustedExternalEntrySymbol(void) {
  if(!UntrustedExternalEntrySymbol) {
    MCContext &Ctx = MF.getContext();
    UntrustedExternalEntrySymbol = Ctx.getOrCreateSymbol(Twine("__cross_domain_") + Twine(MF.getName()));

  }
  return UntrustedExternalEntrySymbol;
}
MCSymbol* MipsFunctionInfo::getTrustedExternalEntrySymbol(void) {
  if(!TrustedExternalEntrySymbol) {
    MCContext &Ctx = MF.getContext();
    TrustedExternalEntrySymbol = Ctx.getOrCreateSymbol(Twine("__cross_domain_trusted_") + Twine(MF.getName()));
  }
  return TrustedExternalEntrySymbol;
}
