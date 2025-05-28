//===--- FixcapasarraysubscriptCheck.h - clang-tidy -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXCAPASARRAYSUBSCRIPTCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXCAPASARRAYSUBSCRIPTCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::cheri {

/// Explicitly cast capabilities used as an array index to an unsigned long.
/// Not doing this will crash the compiler with -fsanitize=array-bounds.
class FixcapasarraysubscriptCheck : public ClangTidyCheck {
public:
  FixcapasarraysubscriptCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace clang::tidy::cheri

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXCAPASARRAYSUBSCRIPTCHECK_H
