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
#include "clang/Basic/MacroBuilder.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;
using namespace clang::targets;

ArrayRef<const char *> RISCVTargetInfo::getGCCRegNames() const {
  static const char *const GCCRegNames[] = {
      "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",
      "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
      "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
      "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31"};
  return llvm::makeArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> RISCVTargetInfo::getGCCRegAliases() const {
  static const TargetInfo::GCCRegAlias GCCRegAliases[] = {
      {{"zero"}, "x0"}, {{"ra"}, "x1"},  {{"sp"}, "x2"},   {{"gp"}, "x3"},
      {{"tp"}, "x4"},   {{"t0"}, "x5"},  {{"t1"}, "x6"},   {{"t2"}, "x7"},
      {{"s0"}, "x8"},   {{"s1"}, "x9"},  {{"a0"}, "x10"},  {{"a1"}, "x11"},
      {{"a2"}, "x12"},  {{"a3"}, "x13"}, {{"a4"}, "x15"},  {{"a5"}, "x15"},
      {{"a6"}, "x16"},  {{"a7"}, "x17"}, {{"s2"}, "x18"},  {{"s3"}, "x19"},
      {{"s4"}, "x20"},  {{"s5"}, "x21"}, {{"s6"}, "x22"},  {{"s7"}, "x23"},
      {{"s8"}, "x24"},  {{"s9"}, "x25"}, {{"s10"}, "x26"}, {{"s11"}, "x27"},
      {{"t3"}, "x28"},  {{"t4"}, "x29"}, {{"t5"}, "x30"},  {{"t6"}, "x31"}};
  return llvm::makeArrayRef(GCCRegAliases);
}

void RISCVTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  Builder.defineMacro("__ELF__");
  Builder.defineMacro("__riscv");
  bool Is64Bit = getTriple().getArch() == llvm::Triple::riscv64;
  Builder.defineMacro("__riscv_xlen", Is64Bit ? "64" : "32");
  // TODO: modify when more code models and ABIs are supported.
  Builder.defineMacro("__riscv_cmodel_medlow");
  Builder.defineMacro("__riscv_float_abi_soft");

  if (HasM) {
    Builder.defineMacro("__riscv_mul");
    Builder.defineMacro("__riscv_div");
    Builder.defineMacro("__riscv_muldiv");
  }

  if (HasA)
    Builder.defineMacro("__riscv_atomic");

  if (HasF || HasD) {
    Builder.defineMacro("__riscv_flen", HasD ? "64" : "32");
    Builder.defineMacro("__riscv_fdiv");
    Builder.defineMacro("__riscv_fsqrt");
  }

  if (HasC)
    Builder.defineMacro("__riscv_compressed");

  if (HasCheri) {
    // XXX-JC: Do we really want the same ABI constants as CHERI-MIPS?
    if (CapabilityABI) {
      auto CapTableABI = llvm::MCTargetOptions::cheriCapabilityTableABI();
      if (CapTableABI != llvm::CheriCapabilityTableABI::Legacy) {
        Builder.defineMacro("__CHERI_CAPABILITY_TABLE__",
                            Twine(((int)CapTableABI) + 1));
      }
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
  }
}

/// Return true if has this feature, need to sync with handleTargetFeatures.
bool RISCVTargetInfo::hasFeature(StringRef Feature) const {
  bool Is64Bit = getTriple().getArch() == llvm::Triple::riscv64;
  return llvm::StringSwitch<bool>(Feature)
      .Case("riscv", true)
      .Case("riscv32", !Is64Bit)
      .Case("riscv64", Is64Bit)
      .Case("m", HasM)
      .Case("a", HasA)
      .Case("f", HasF)
      .Case("d", HasD)
      .Case("c", HasC)
      .Case("xcheri", HasCheri)
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
    }
  }

  setDataLayout();

  return true;
}
