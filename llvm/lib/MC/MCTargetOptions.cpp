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
    : MCRelaxAll(false), MCNoExecStack(false), MCFatalWarnings(false),
      MCNoWarn(false), MCNoDeprecatedWarn(false), MCSaveTempLabels(false),
      MCUseDwarfDirectory(false), MCIncrementalLinkerCompatible(false),
      ShowMCEncoding(false), ShowMCInst(false), AsmVerbose(false),
      PreserveAsmComments(true) {}

StringRef MCTargetOptions::getABIName() const {
  return ABIName;
}
