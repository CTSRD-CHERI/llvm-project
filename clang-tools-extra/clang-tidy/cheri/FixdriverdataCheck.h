//===--- FixdriverdataCheck.h - clang-tidy ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXDRIVERDATACHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXDRIVERDATACHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::cheri {

/// Static initializations of driver data fields (fields name "driver_data"
/// or "driver_info") should use a cast to uintptr_t instead of some
/// variant of unsigned long.
class FixdriverdataCheck : public ClangTidyCheck {
public:
  FixdriverdataCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  bool isDriverData(ASTContext *Ctx, const FieldDecl *Field);
};

} // namespace clang::tidy::cheri

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXDRIVERDATACHECK_H
