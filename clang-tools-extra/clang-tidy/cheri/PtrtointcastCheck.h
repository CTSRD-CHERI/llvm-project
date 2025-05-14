//===--- PtrtointcastCheck.h - clang-tidy -----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_PTRTOINTCASTCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_PTRTOINTCASTCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::cheri {

/// Detect cast from CHERI capabilities to plain addresses (and vice versa)
/// that may be unintended.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/cheri/PtrToIntCast.html
class PtrtointcastCheck : public ClangTidyCheck {
public:
  PtrtointcastCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  void checkCast(const ast_matchers::MatchFinder::MatchResult &Result);
  void checkIndirectCast(const ast_matchers::MatchFinder::MatchResult &Result);

private:
  static bool checkPointeeTypes(ASTContext *Ctx, const Type *From,
                                const Type *To);
};

} // namespace clang::tidy::cheri

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_PTRTOINTCASTCHECK_H
