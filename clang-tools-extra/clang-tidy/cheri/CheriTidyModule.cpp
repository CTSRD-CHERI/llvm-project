//===--- CheriTidyModule.cpp - clang-tidy----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "FixcapasarraysubscriptCheck.h"
#include "FixcapformatCheck.h"
#include "FixdriverdataCheck.h"
#include "FixinttoptrcastCheck.h"
#include "FixptrtoulongcastCheck.h"
#include "FixuserptrtoaddrCheck.h"
#include "IoctlCheck.h"
#include "PtrtointcastCheck.h"

namespace clang::tidy {
namespace cheri {

/// This module is for checks specific to the Linux kernel.
class CheriModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<FixcapasarraysubscriptCheck>(
        "cheri-FixCapAsArraySubscript");
    CheckFactories.registerCheck<FixcapformatCheck>(
        "cheri-FixCapFormat");
    CheckFactories.registerCheck<FixdriverdataCheck>(
        "cheri-FixDriverData");
    CheckFactories.registerCheck<FixinttoptrcastCheck>(
        "cheri-FixIntToPtrCast");
    CheckFactories.registerCheck<FixptrtoulongcastCheck>(
        "cheri-FixPtrToUlongCast");
    CheckFactories.registerCheck<FixuserptrtoaddrCheck>(
        "cheri-FixUserPtrToAddr");
    CheckFactories.registerCheck<IoctlCheck>(
        "cheri-Ioctl");
    CheckFactories.registerCheck<PtrtointcastCheck>("cheri-PtrToIntCast");
  }
};
// Register the CheriTidyModule using this statically initialized
// variable.
static ClangTidyModuleRegistry::Add<CheriModule>
    X("cheri-module", "Adds checks specific to the Linux kernel.");
} // namespace cheri

// This anchor is used to force the linker to link in the generated object file
// and thus register the CheriModule.
volatile int CheriModuleAnchorSource = 0;

} // namespace clang::tidy
