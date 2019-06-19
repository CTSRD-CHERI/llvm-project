//===- lib/MC/MCTargetOptions.cpp - MC Target Options ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCTargetOptions.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

static cl::opt<analysis_type> TemporalType("TemporalType",
                                           cl::desc("Tag everything not manually tagged..."),
                                           cl::init(NONE),
                                           cl::values(
                                               clEnumValN(NONE, "none",
                                                          "Do no tagging"),
                                               clEnumValN(ALL_SAFE, "all_safe",
                                                          "as safe"),
                                               clEnumValN(ALL_UNSAFE, "all_unsafe",
                                                          "as unsafe"),
                                               clEnumValN(USE_CAPTURE_TRACK, "capture_track",
                                                          "unsafe if it may be captured"),
                                               clEnumValN(ANALYSIS, "analysis",
                                                          "unsafe if more complex analysis shows it is"),
                                               clEnumValN(ANALYSIS_ALL_SAFE, "analysis_all_safe",
                                                          "safe, but throw an error if something is unsafe")
                                           ));
static cl::opt<int> TemporalExpandLimit("TemporalExpandLimit",
    cl::desc("How many functions to statically expand for escape analysis"),
    cl::init(1));


analysis_type llvm::get_temporal_analysis_type() {
  return TemporalType.getValue();
}

int llvm::get_temporal_expand_limit() {
  return TemporalExpandLimit.getValue();
}

static cl::opt<CheriCapabilityTableABI> CapTableABI(
    "cheri-cap-table-abi", cl::desc("ABI to use for :"),
    cl::init(CheriCapabilityTableABI::Pcrel),
    cl::values(clEnumValN(CheriCapabilityTableABI::Legacy, "legacy",
                          "Disable capability table and use the legacy ABI"),
               clEnumValN(CheriCapabilityTableABI::PLT, "plt",
                          "Use PLT stubs to setup $cgp correctly"),
               clEnumValN(CheriCapabilityTableABI::Pcrel, "pcrel",
                          "Derive $cgp from $pcc in every function"),
               clEnumValN(CheriCapabilityTableABI::FunctionDescriptor,
                          "fn-desc",
                          "Use function descriptors to setup $cgp correctly")));

bool MCTargetOptions::cheriUsesCapabilityTable() {
  return CapTableABI != CheriCapabilityTableABI::Legacy;
}

CheriCapabilityTableABI MCTargetOptions::cheriCapabilityTableABI() {
  return CapTableABI;
}

static cl::opt<CheriCapabilityTlsABI> CapTlsABI(
    "cheri-cap-tls-abi", cl::desc("ABI to use for :"),
    cl::values(clEnumValN(CheriCapabilityTlsABI::Legacy, "legacy",
                          "Disable capability TLS and use the legacy ABI"),
               clEnumValN(CheriCapabilityTlsABI::CapEquiv, "cap-equiv",
                          "Use a capability equivalent of the normal ABI")));

bool MCTargetOptions::cheriUsesCapabilityTls() {
  return cheriCapabilityTlsABI() != CheriCapabilityTlsABI::Legacy &&
         cheriUsesCapabilityTable();
}

CheriCapabilityTlsABI MCTargetOptions::cheriCapabilityTlsABI() {
  if (!cheriUsesCapabilityTable())
    return CheriCapabilityTlsABI::Legacy;
  // For cap-table default to CapEquiv unless there is an explicit =legacy set.
  if (CapTlsABI.getNumOccurrences() > 0)
    return CapTlsABI;
  return CheriCapabilityTlsABI::CapEquiv;
}

MCTargetOptions::MCTargetOptions()
    : SanitizeAddress(false), MCRelaxAll(false), MCNoExecStack(false),
      MCFatalWarnings(false), MCNoWarn(false), MCNoDeprecatedWarn(false),
      MCSaveTempLabels(false), MCUseDwarfDirectory(false),
      MCIncrementalLinkerCompatible(false), MCPIECopyRelocations(false),
      ShowMCEncoding(false), ShowMCInst(false), AsmVerbose(false),
      PreserveAsmComments(true) {}

StringRef MCTargetOptions::getABIName() const {
  return ABIName;
}
