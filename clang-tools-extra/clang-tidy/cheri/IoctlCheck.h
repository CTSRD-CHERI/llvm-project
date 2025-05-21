//===--- IoctlCheck.h - clang-tidy ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_IOCTLCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_IOCTLCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::cheri {

/// FIXME: Write a short description.
class IoctlCheck : public ClangTidyCheck {
public:
  IoctlCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  void createTypeFixup(const DeclaratorDecl *Decl, SourceManager *SM);
  void createArgFixup(const Expr *Arg, SourceManager *SM);
  void checkFunctionDecl(const ast_matchers::MatchFinder::MatchResult &Result,
                         const FunctionDecl *Func);
  void checkFieldDecl(const ast_matchers::MatchFinder::MatchResult &Result,
                      const FieldDecl *Field);
  void checkInitList(const InitListExpr *InitList);
  void checkFieldInitializer(const Expr *Init, const RecordDecl *Record,
                             const FieldDecl *Field);
  void extractFunctionDecl(const Expr *E, const FunctionDecl *&Decl);
  void extractFieldDecl(const Expr *E, const RecordDecl *&Record,
                        const FieldDecl *&Field);
  void checkCallWithArgs(const ast_matchers::MatchFinder::MatchResult &Result,
                         const CallExpr *Call);
  void checkCallWithArg(const ast_matchers::MatchFinder::MatchResult &Result,
                        const CallExpr *Call, const Expr *Arg);
};

} // namespace clang::tidy::cheri

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CHERI_IOCTLCHECK_H
