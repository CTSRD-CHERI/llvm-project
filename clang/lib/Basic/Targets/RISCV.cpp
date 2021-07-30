//===--- RISCV.cpp - Implement RISCV target feature support ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements RISCV TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "RISCV.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/TargetParser.h"

using namespace clang;
using namespace clang::targets;

ArrayRef<const char *> RISCVTargetInfo::getGCCRegNames() const {
  static const char *const GCCRegNames[] = {
      // Integer registers
      "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",
      "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
      "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
      "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31",

      // Capability registers
      "c0",  "c1",  "c2",  "c3",  "c4",  "c5",  "c6",  "c7",
      "c8",  "c9",  "c10", "c11", "c12", "c13", "c14", "c15",
      "c16", "c17", "c18", "c19", "c20", "c21", "c22", "c23",
      "c24", "c25", "c26", "c27", "c28", "c29", "c30", "c31",

      // Floating point registers
      "f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",
      "f8",  "f9",  "f10", "f11", "f12", "f13", "f14", "f15",
      "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
      "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",

      // Vector registers
      "v0",  "v1",  "v2",  "v3",  "v4",  "v5",  "v6",  "v7",
      "v8",  "v9",  "v10", "v11", "v12", "v13", "v14", "v15",
      "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
      "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"};
  return llvm::makeArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> RISCVTargetInfo::getGCCRegAliases() const {
  static const TargetInfo::GCCRegAlias GCCRegAliases[] = {
      {{"zero"}, "x0"}, {{"ra"}, "x1"},   {{"sp"}, "x2"},    {{"gp"}, "x3"},
      {{"tp"}, "x4"},   {{"t0"}, "x5"},   {{"t1"}, "x6"},    {{"t2"}, "x7"},
      {{"s0"}, "x8"},   {{"s1"}, "x9"},   {{"a0"}, "x10"},   {{"a1"}, "x11"},
      {{"a2"}, "x12"},  {{"a3"}, "x13"},  {{"a4"}, "x14"},   {{"a5"}, "x15"},
      {{"a6"}, "x16"},  {{"a7"}, "x17"},  {{"s2"}, "x18"},   {{"s3"}, "x19"},
      {{"s4"}, "x20"},  {{"s5"}, "x21"},  {{"s6"}, "x22"},   {{"s7"}, "x23"},
      {{"s8"}, "x24"},  {{"s9"}, "x25"},  {{"s10"}, "x26"},  {{"s11"}, "x27"},
      {{"t3"}, "x28"},  {{"t4"}, "x29"},  {{"t5"}, "x30"},   {{"t6"}, "x31"},
      {{"cnull"}, "c0"},{{"cra"}, "c1"},  {{"csp"}, "c2"},   {{"cgp"}, "c3"},
      {{"ctp"}, "c4"},  {{"ct0"}, "c5"},  {{"ct1"}, "c6"},   {{"ct2"}, "c7"},
      {{"cs0"}, "c8"},  {{"cs1"}, "c9"},  {{"ca0"}, "c10"},  {{"ca1"}, "c11"},
      {{"ca2"}, "c12"}, {{"ca3"}, "c13"}, {{"ca4"}, "c14"},  {{"ca5"}, "c15"},
      {{"ca6"}, "c16"}, {{"ca7"}, "c17"}, {{"cs2"}, "c18"},  {{"cs3"}, "c19"},
      {{"cs4"}, "c20"}, {{"cs5"}, "c21"}, {{"cs6"}, "c22"},  {{"cs7"}, "c23"},
      {{"cs8"}, "c24"}, {{"cs9"}, "c25"}, {{"cs10"}, "c26"}, {{"cs11"}, "c27"},
      {{"ct3"}, "c28"}, {{"ct4"}, "c29"}, {{"ct5"}, "c30"},  {{"ct6"}, "c31"},
      {{"ft0"}, "f0"},  {{"ft1"}, "f1"},  {{"ft2"}, "f2"},   {{"ft3"}, "f3"},
      {{"ft4"}, "f4"},  {{"ft5"}, "f5"},  {{"ft6"}, "f6"},   {{"ft7"}, "f7"},
      {{"fs0"}, "f8"},  {{"fs1"}, "f9"},  {{"fa0"}, "f10"},  {{"fa1"}, "f11"},
      {{"fa2"}, "f12"}, {{"fa3"}, "f13"}, {{"fa4"}, "f14"},  {{"fa5"}, "f15"},
      {{"fa6"}, "f16"}, {{"fa7"}, "f17"}, {{"fs2"}, "f18"},  {{"fs3"}, "f19"},
      {{"fs4"}, "f20"}, {{"fs5"}, "f21"}, {{"fs6"}, "f22"},  {{"fs7"}, "f23"},
      {{"fs8"}, "f24"}, {{"fs9"}, "f25"}, {{"fs10"}, "f26"}, {{"fs11"}, "f27"},
      {{"ft8"}, "f28"}, {{"ft9"}, "f29"}, {{"ft10"}, "f30"}, {{"ft11"}, "f31"}};
  return llvm::makeArrayRef(GCCRegAliases);
}

bool RISCVTargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  switch (*Name) {
  default:
    return false;
  case 'I':
    // A 12-bit signed immediate.
    Info.setRequiresImmediate(-2048, 2047);
    return true;
  case 'J':
    // Integer zero.
    Info.setRequiresImmediate(0);
    return true;
  case 'K':
    // A 5-bit unsigned immediate for CSR access instructions.
    Info.setRequiresImmediate(0, 31);
    return true;
  case 'f':
    // A floating-point register.
    Info.setAllowsRegister();
    return true;
  case 'A':
    // An address that is held in a general-purpose register.
    Info.setAllowsMemory();
    return true;
  case 'C':
    // A capability register.
    Info.setAllowsRegister();
    return HasCheri;
  case 'S': // A symbolic address
    Info.setAllowsRegister();
    return true;
  case 'v':
    // A vector register.
    if (Name[1] == 'r' || Name[1] == 'm') {
      Info.setAllowsRegister();
      Name += 1;
      return true;
    }
    return false;
  }
}

std::string RISCVTargetInfo::convertConstraint(const char *&Constraint) const {
  std::string R;
  switch (*Constraint) {
  case 'v':
    R = std::string("^") + std::string(Constraint, 2);
    Constraint += 1;
    break;
  default:
    R = TargetInfo::convertConstraint(Constraint);
    break;
  }
  return R;
}

void RISCVTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  Builder.defineMacro("__ELF__");
  Builder.defineMacro("__riscv");
  bool Is64Bit = getTriple().getArch() == llvm::Triple::riscv64;
  Builder.defineMacro("__riscv_xlen", Is64Bit ? "64" : "32");
  StringRef CodeModel = getTargetOpts().CodeModel;
  if (CodeModel == "default")
    CodeModel = "small";

  if (CodeModel == "small")
    Builder.defineMacro("__riscv_cmodel_medlow");
  else if (CodeModel == "medium")
    Builder.defineMacro("__riscv_cmodel_medany");

  StringRef ABIName = getABI();
  if (ABIName == "ilp32f" || ABIName == "lp64f" ||
      ABIName == "il32pc64f" || ABIName == "l64pc128f")
    Builder.defineMacro("__riscv_float_abi_single");
  else if (ABIName == "ilp32d" || ABIName == "lp64d" ||
           ABIName == "il32pc64d" || ABIName == "l64pc128d")
    Builder.defineMacro("__riscv_float_abi_double");
  else
    Builder.defineMacro("__riscv_float_abi_soft");

  if (ABIName == "ilp32e" || ABIName == "il32pc64e")
    Builder.defineMacro("__riscv_abi_rve");

  Builder.defineMacro("__riscv_arch_test");
  Builder.defineMacro("__riscv_i", "2000000");

  if (HasM) {
    Builder.defineMacro("__riscv_m", "2000000");
    Builder.defineMacro("__riscv_mul");
    Builder.defineMacro("__riscv_div");
    Builder.defineMacro("__riscv_muldiv");
  }

  if (HasA) {
    Builder.defineMacro("__riscv_a", "2000000");
    Builder.defineMacro("__riscv_atomic");
    Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1");
    Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2");
    Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4");
    if (Is64Bit)
      Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8");
  }

  if (HasF || HasD) {
    Builder.defineMacro("__riscv_f", "2000000");
    Builder.defineMacro("__riscv_flen", HasD ? "64" : "32");
    Builder.defineMacro("__riscv_fdiv");
    Builder.defineMacro("__riscv_fsqrt");
  }

  if (HasD)
    Builder.defineMacro("__riscv_d", "2000000");

  if (HasC) {
    Builder.defineMacro("__riscv_c", "2000000");
    Builder.defineMacro("__riscv_compressed");
  }

  if (HasB) {
    Builder.defineMacro("__riscv_b", "93000");
    Builder.defineMacro("__riscv_bitmanip");
  }

  if (HasCheri) {
    // XXX-JC: Do we really want the same ABI constants as CHERI-MIPS?
    if (CapabilityABI) {
      auto CapTableABI = llvm::MCTargetOptions::cheriCapabilityTableABI();
      Builder.defineMacro("__CHERI_CAPABILITY_TABLE__",
                          Twine(((int)CapTableABI) + 1));
    }

    // Macros for use with the set and get permissions builtins.
    Builder.defineMacro("__CHERI_CAP_PERMISSION_GLOBAL__", Twine(1<<0));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_PERMIT_EXECUTE__",
            Twine(1<<1));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_PERMIT_LOAD__", Twine(1<<2));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_PERMIT_STORE__", Twine(1<<3));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__",
            Twine(1<<4));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__",
            Twine(1<<5));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_PERMIT_STORE_LOCAL__",
            Twine(1<<6));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_PERMIT_SEAL__", Twine(1<<7));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_PERMIT_CCALL__", Twine(1<<8));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_PERMIT_UNSEAL__", Twine(1<<9));
    Builder.defineMacro("__CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__", Twine(1<<10));

    Builder.defineMacro("__riscv_clen", Twine(getCHERICapabilityWidth()));
    // TODO: _MIPS_CAP_ALIGN_MASK equivalent?
    // XXX: Temporary to allow software to keep supporting old ISAv8 toolchains
    // where what is now JALR.CAP was called CJALR (which is now the mnemonic
    // for the capability-mode JALR with immediate). Remove after the next
    // CHERI-LLVM "release".
    Builder.defineMacro("__riscv_xcheri_mode_dependent_jumps");
  }

  if (HasV) {
    Builder.defineMacro("__riscv_v", "10000");
    Builder.defineMacro("__riscv_vector");
  }

  if (HasZba)
    Builder.defineMacro("__riscv_zba", "93000");

  if (HasZbb)
    Builder.defineMacro("__riscv_zbb", "93000");

  if (HasZbc)
    Builder.defineMacro("__riscv_zbc", "93000");

  if (HasZbe)
    Builder.defineMacro("__riscv_zbe", "93000");

  if (HasZbf)
    Builder.defineMacro("__riscv_zbf", "93000");

  if (HasZbm)
    Builder.defineMacro("__riscv_zbm", "93000");

  if (HasZbp)
    Builder.defineMacro("__riscv_zbp", "93000");

  if (HasZbproposedc)
    Builder.defineMacro("__riscv_zbproposedc", "93000");

  if (HasZbr)
    Builder.defineMacro("__riscv_zbr", "93000");

  if (HasZbs)
    Builder.defineMacro("__riscv_zbs", "93000");

  if (HasZbt)
    Builder.defineMacro("__riscv_zbt", "93000");

  if (HasZfh)
    Builder.defineMacro("__riscv_zfh", "1000");

  if (HasZvamo)
    Builder.defineMacro("__riscv_zvamo", "10000");

  if (HasZvlsseg)
    Builder.defineMacro("__riscv_zvlsseg", "10000");
}

const Builtin::Info RISCVTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
    {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, FEATURE},
#include "clang/Basic/BuiltinsRISCV.def"
};

ArrayRef<Builtin::Info> RISCVTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, clang::RISCV::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}

bool RISCVTargetInfo::initFeatureMap(
    llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags, StringRef CPU,
    const std::vector<std::string> &FeaturesVec) const {

  if (getTriple().getArch() == llvm::Triple::riscv64)
    Features["64bit"] = true;

  return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
}

/// Return true if has this feature, need to sync with handleTargetFeatures.
bool RISCVTargetInfo::hasFeature(StringRef Feature) const {
  bool Is64Bit = getTriple().getArch() == llvm::Triple::riscv64;
  return llvm::StringSwitch<bool>(Feature)
      .Case("riscv", true)
      .Case("riscv32", !Is64Bit)
      .Case("riscv64", Is64Bit)
      .Case("64bit", Is64Bit)
      .Case("m", HasM)
      .Case("a", HasA)
      .Case("f", HasF)
      .Case("d", HasD)
      .Case("c", HasC)
      .Case("xcheri", HasCheri)
      .Case("experimental-b", HasB)
      .Case("experimental-v", HasV)
      .Case("experimental-zba", HasZba)
      .Case("experimental-zbb", HasZbb)
      .Case("experimental-zbc", HasZbc)
      .Case("experimental-zbe", HasZbe)
      .Case("experimental-zbf", HasZbf)
      .Case("experimental-zbm", HasZbm)
      .Case("experimental-zbp", HasZbp)
      .Case("experimental-zbproposedc", HasZbproposedc)
      .Case("experimental-zbr", HasZbr)
      .Case("experimental-zbs", HasZbs)
      .Case("experimental-zbt", HasZbt)
      .Case("experimental-zfh", HasZfh)
      .Case("experimental-zvamo", HasZvamo)
      .Case("experimental-zvlsseg", HasZvlsseg)
      .Default(false);
}

/// Perform initialization based on the user configured set of features.
bool RISCVTargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                           DiagnosticsEngine &Diags) {
  for (const auto &Feature : Features) {
    if (Feature == "+m")
      HasM = true;
    else if (Feature == "+a")
      HasA = true;
    else if (Feature == "+f")
      HasF = true;
    else if (Feature == "+d")
      HasD = true;
    else if (Feature == "+c")
      HasC = true;
    else if (Feature == "+xcheri") {
      HasCheri = true;
      CapSize = PointerWidth * 2;
    } else if (Feature == "+experimental-b")
      HasB = true;
    else if (Feature == "+experimental-v")
      HasV = true;
    else if (Feature == "+experimental-zba")
      HasZba = true;
    else if (Feature == "+experimental-zbb")
      HasZbb = true;
    else if (Feature == "+experimental-zbc")
      HasZbc = true;
    else if (Feature == "+experimental-zbe")
      HasZbe = true;
    else if (Feature == "+experimental-zbf")
      HasZbf = true;
    else if (Feature == "+experimental-zbm")
      HasZbm = true;
    else if (Feature == "+experimental-zbp")
      HasZbp = true;
    else if (Feature == "+experimental-zbproposedc")
      HasZbproposedc = true;
    else if (Feature == "+experimental-zbr")
      HasZbr = true;
    else if (Feature == "+experimental-zbs")
      HasZbs = true;
    else if (Feature == "+experimental-zbt")
      HasZbt = true;
    else if (Feature == "+experimental-zfh")
      HasZfh = true;
    else if (Feature == "+experimental-zvamo")
      HasZvamo = true;
    else if (Feature == "+experimental-zvlsseg")
      HasZvlsseg = true;
  }

  setDataLayout();

  return true;
}

bool RISCVTargetInfo::validateTarget(DiagnosticsEngine &Diags) const {
  if (CapabilityABI && !HasCheri) {
    Diags.Report(diag::err_riscv_invalid_abi) << ABI
      << "pure capability ABI requires xcheri extension to be specified";
    return false;
  }

  return true;
}

bool RISCV32TargetInfo::isValidCPUName(StringRef Name) const {
  return llvm::RISCV::checkCPUKind(llvm::RISCV::parseCPUKind(Name),
                                   /*Is64Bit=*/false);
}

void RISCV32TargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  llvm::RISCV::fillValidCPUArchList(Values, false);
}

bool RISCV32TargetInfo::isValidTuneCPUName(StringRef Name) const {
  return llvm::RISCV::checkTuneCPUKind(
      llvm::RISCV::parseTuneCPUKind(Name, false),
      /*Is64Bit=*/false);
}

void RISCV32TargetInfo::fillValidTuneCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  llvm::RISCV::fillValidTuneCPUArchList(Values, false);
}

bool RISCV64TargetInfo::isValidCPUName(StringRef Name) const {
  return llvm::RISCV::checkCPUKind(llvm::RISCV::parseCPUKind(Name),
                                   /*Is64Bit=*/true);
}

void RISCV64TargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  llvm::RISCV::fillValidCPUArchList(Values, true);
}

bool RISCV64TargetInfo::isValidTuneCPUName(StringRef Name) const {
  return llvm::RISCV::checkTuneCPUKind(
      llvm::RISCV::parseTuneCPUKind(Name, true),
      /*Is64Bit=*/true);
}

void RISCV64TargetInfo::fillValidTuneCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  llvm::RISCV::fillValidTuneCPUArchList(Values, true);
}
