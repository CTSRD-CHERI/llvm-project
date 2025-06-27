//===--- FixuserptrtoaddrCheck.h - clang-tidy -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXUSERPTRTOADDRCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXUSERPTRTOADDRCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::cheri {

/// In the CHERI Linux Kernel __c_ua() must be used to convert
/// a kernel pointer to an address while __c_ua_u() must be used
/// for user pointers. Check that this is consistently done even
/// if the current configuration does the same thing for both.
class FixuserptrtoaddrCheck : public ClangTidyCheck {
public:
  FixuserptrtoaddrCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace clang::tidy::cheri

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_FIXUSERPTRTOADDRCHECK_H
