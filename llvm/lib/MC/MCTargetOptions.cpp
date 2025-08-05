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

static cl::opt<bool> CheriTLSUseTGOT(
    "cheri-tgot-tls", cl::desc("Use TGOT for CHERI TLS instead of legacy ABI"),
    cl::init(false));

bool MCTargetOptions::cheriTLSUseTGOT() {
  return CheriTLSUseTGOT;
}

MCTargetOptions::MCTargetOptions()
    : MCRelaxAll(false), MCNoExecStack(false), MCFatalWarnings(false),
      MCNoWarn(false), MCNoDeprecatedWarn(false), MCNoTypeCheck(false),
      MCSaveTempLabels(false), MCIncrementalLinkerCompatible(false),
      ShowMCEncoding(false), ShowMCInst(false), AsmVerbose(false),
      PreserveAsmComments(true), Dwarf64(false),
      EmitDwarfUnwind(EmitDwarfUnwindType::Default),
      MCUseDwarfDirectory(DefaultDwarfDirectory),
      EmitCompactUnwindNonCanonical(false) {}

StringRef MCTargetOptions::getABIName() const {
  return ABIName;
}

StringRef MCTargetOptions::getAssemblyLanguage() const {
  return AssemblyLanguage;
}
