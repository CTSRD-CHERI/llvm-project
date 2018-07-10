//===---- MipsABIInfo.cpp - Information about MIPS ABI's ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MipsABIInfo.h"
#include "MipsRegisterInfo.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCTargetOptions.h"

using namespace llvm;

namespace {
static const MCPhysReg O32IntRegs[4] = {Mips::A0, Mips::A1, Mips::A2, Mips::A3};

static const MCPhysReg Mips64IntRegs[8] = {
    Mips::A0_64, Mips::A1_64, Mips::A2_64, Mips::A3_64,
    Mips::T0_64, Mips::T1_64, Mips::T2_64, Mips::T3_64};

static const MCPhysReg CheriCapArgRegs[8] = {
    Mips::C3, Mips::C4, Mips::C5, Mips::C6,
    Mips::C7, Mips::C8, Mips::C9};

}

ArrayRef<MCPhysReg> MipsABIInfo::GetByValArgRegs() const {
  if (IsCheriPureCap())
    return makeArrayRef(CheriCapArgRegs);
  if (IsO32())
    return makeArrayRef(O32IntRegs);
  if (IsN32() || IsN64())
    return makeArrayRef(Mips64IntRegs);
  llvm_unreachable("Unhandled ABI");
}

ArrayRef<MCPhysReg> MipsABIInfo::GetVarArgRegs() const {
  if (IsCheriPureCap())
    return makeArrayRef(CheriCapArgRegs);
  if (IsO32())
    return makeArrayRef(O32IntRegs);
  if (IsN32() || IsN64())
    return makeArrayRef(Mips64IntRegs);
  llvm_unreachable("Unhandled ABI");
}

unsigned MipsABIInfo::GetCalleeAllocdArgSizeInBytes(CallingConv::ID CC) const {
  if (IsO32())
    return CC != CallingConv::Fast ? 16 : 0;
  if (IsN32() || IsN64())
    return 0;
  llvm_unreachable("Unhandled ABI");
}

MipsABIInfo MipsABIInfo::computeTargetABI(const Triple &TT, StringRef CPU,
                                          const MCTargetOptions &Options) {
  StringRef ABIName = Options.getABIName();
  if (ABIName.startswith("o32"))
    return MipsABIInfo::O32();
  if (ABIName.startswith("n32"))
    return MipsABIInfo::N32();
  if (ABIName.startswith("n64"))
    return MipsABIInfo::N64();
  if (ABIName.startswith("sandbox") || ABIName.startswith("purecap"))
    return MipsABIInfo::CheriPureCap(CPU.endswith("128") ? 16 : 32);
  assert(ABIName.empty() && "Unknown ABI option for MIPS");

  if (TT.isMIPS64())
    return MipsABIInfo::N64();
  return MipsABIInfo::O32();
}

unsigned MipsABIInfo::GetStackPtr() const {
  return IsCheriPureCap() ? 
    Mips::C11 :
    (ArePtrs64bit() ? Mips::SP_64 : Mips::SP);
}

unsigned MipsABIInfo::GetUnsafeStackPtr() const {
  return IsCheriPureCap() ?
         Mips::C10 :
         (0);
}

unsigned MipsABIInfo::GetFramePtr() const {
  return IsCheriPureCap() ? 
    Mips::C24 :
    (ArePtrs64bit() ? Mips::FP_64 : Mips::FP);
}

unsigned MipsABIInfo::GetBasePtr() const {
  // FIXME: $c25 is probably not sensible here.
  return IsCheriPureCap() ? 
    Mips::C25 :
    (ArePtrs64bit() ? Mips::S7_64 : Mips::S7);
}

unsigned MipsABIInfo::GetDefaultDataCapability() const { return Mips::DDC; }

unsigned MipsABIInfo::GetGlobalCapability() const {
  return Mips::C25;
}

unsigned MipsABIInfo::GetLocalCapability() const {
  return Mips::C26;
}

bool MipsABIInfo::UsesCapabilityTable() const {
  return IsCheriPureCap() && MCTargetOptions::cheriUsesCapabilityTable();
}

const MipsABIInfo::TemporalABILayout* MipsABIInfo::GetTABILayout() const {
  return isCheriPureCap ? &temporalABILayout : nullptr;
}

unsigned MipsABIInfo::GetReturnAddress() const {
  return IsCheriPureCap() ?
    Mips::C17 :
    (ArePtrs64bit() ? Mips::RA_64 : Mips::RA);
}

unsigned MipsABIInfo::GetReturnData() const {
  return IsCheriPureCap() ?
         Mips::C18 :
         (0);
}

unsigned MipsABIInfo::GetReturnSelector() const {
    return IsCheriPureCap() ?
           2 :
           (0);
}

unsigned MipsABIInfo::GetGlobalPtr() const {
  //assert(!UsesCapabilityTable());
  return ArePtrs64bit() ? Mips::GP_64 : Mips::GP;
}

unsigned MipsABIInfo::GetNullPtr() const {
  return ArePtrs64bit() ? Mips::ZERO_64 : Mips::ZERO;
}

unsigned MipsABIInfo::GetZeroReg() const {
  return AreGprs64bit() ? Mips::ZERO_64 : Mips::ZERO;
}

unsigned MipsABIInfo::GetPtrAdduOp() const {
  return ArePtrs64bit() ? Mips::DADDu : Mips::ADDu;
}

unsigned MipsABIInfo::GetPtrAddiuOp() const {
  return ArePtrs64bit() ? Mips::DADDiu : Mips::ADDiu;
}

unsigned MipsABIInfo::GetPtrSubuOp() const {
  return ArePtrs64bit() ? Mips::DSUBu : Mips::SUBu;
}

unsigned MipsABIInfo::GetPtrAndOp() const {
  return ArePtrs64bit() ? Mips::AND64 : Mips::AND;
}

unsigned MipsABIInfo::GetGPRMoveOp() const {
  return ArePtrs64bit() ? Mips::OR64 : Mips::OR;
}

unsigned MipsABIInfo::GetSPMoveOp() const {
  return IsCheriPureCap() ?
    Mips::CIncOffset :
    (ArePtrs64bit() ? Mips::OR64 : Mips::OR);
}

unsigned MipsABIInfo::GetEhDataReg(unsigned I) const {
  static const unsigned EhDataReg[] = {
    Mips::A0, Mips::A1, Mips::A2, Mips::A3
  };
  static const unsigned EhDataReg64[] = {
    Mips::A0_64, Mips::A1_64, Mips::A2_64, Mips::A3_64
  };

  return IsN64() ? EhDataReg64[I] : EhDataReg[I];
}

