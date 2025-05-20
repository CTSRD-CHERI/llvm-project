//===--- FixinttoptrcastCheck.h - clang-tidy --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXINTTOPTRCASTCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXINTTOPTRCASTCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::cheri {

/// Warn about and fix casts where a smaller integer type is cast
/// to a pointer directly. This results in a compiler error on CHERI
/// and can be fixed by adding an intermediate cast to uintptr_t.
///
/// Warn about cases where we have determined that the cast is ok
/// and suggest the insertion of the intermediate cast.
class FixinttoptrcastCheck : public ClangTidyCheck {
public:
  FixinttoptrcastCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace clang::tidy::cheri

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXINTTOPTRCASTCHECK_H
