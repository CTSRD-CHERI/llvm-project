//===- lib/MC/MCTargetOptions.cpp - MC Target Options ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCTargetOptions.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace escape;

static cl::opt<analysis_type> TemporalType(
    "TemporalType", cl::desc("Tag everything not manually tagged..."),
    cl::init(NONE),
    cl::values(clEnumValN(NONE, "none", "Do no tagging"),
               clEnumValN(ALL_SAFE, "all_safe", "as safe"),
               clEnumValN(ALL_UNSAFE, "all_unsafe", "as unsafe"),
               clEnumValN(USE_CAPTURE_TRACK, "capture_track",
                          "unsafe if it may be captured"),
               clEnumValN(ANALYSIS, "analysis",
                          "unsafe if more complex analysis shows it is"),
               clEnumValN(ANALYSIS_ALL_SAFE, "analysis_all_safe",
                          "safe, but throw an error if something is unsafe")));
static cl::opt<int> TemporalExpandLimit(
    "TemporalExpandLimit",
    cl::desc("How many functions to statically expand for escape analysis"),
    cl::init(1));

analysis_type llvm::getTemporalAnalysisType() {
  return TemporalType.getValue();
}

int llvm::getTemporalExpandLimit() { return TemporalExpandLimit.getValue(); }

static cl::opt<CheriCapabilityTableABI> CapTableABI(
    "cheri-cap-table-abi", cl::desc("ABI to use for :"),
    cl::init(CheriCapabilityTableABI::Pcrel),
    cl::values(clEnumValN(CheriCapabilityTableABI::PLT, "plt",
                          "Use PLT stubs to setup $cgp correctly"),
               clEnumValN(CheriCapabilityTableABI::Pcrel, "pcrel",
                          "Derive $cgp from $pcc in every function"),
               clEnumValN(CheriCapabilityTableABI::FunctionDescriptor,
                          "fn-desc",
                          "Use function descriptors to setup $cgp correctly")));

CheriCapabilityTableABI MCTargetOptions::cheriCapabilityTableABI() {
  return CapTableABI;
}

static cl::opt<bool> IsCheriOSABI("is-cherios",
                                  cl::desc("Use the CheriOS style purecap ABI"),
                                  cl::init(false));

bool MCTargetOptions::isCheriOSABI() { return IsCheriOSABI.getValue(); }

MCTargetOptions::MCTargetOptions()
    : MCRelaxAll(false), MCNoExecStack(false), MCFatalWarnings(false),
      MCNoWarn(false), MCNoDeprecatedWarn(false),
      MCNoTypeCheck(false), MCSaveTempLabels(false),
      MCUseDwarfDirectory(false), MCIncrementalLinkerCompatible(false),
      ShowMCEncoding(false), ShowMCInst(false), AsmVerbose(false),
      PreserveAsmComments(true), Dwarf64(false) {}

StringRef MCTargetOptions::getABIName() const {
  return ABIName;
}

StringRef MCTargetOptions::getAssemblyLanguage() const {
  return AssemblyLanguage;
}
