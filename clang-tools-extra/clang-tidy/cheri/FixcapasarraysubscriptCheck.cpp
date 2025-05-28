//===--- FixcapasarraysubscriptCheck.cpp - clang-tidy ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FixcapasarraysubscriptCheck.h"
#include "CheriUtil.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cheri {

void FixcapasarraysubscriptCheck::registerMatchers(MatchFinder *Finder) {
  // clang-format off
  Finder->addMatcher(
      arraySubscriptExpr(
          hasIndex(
              expr(
                  anything()
              ).bind("index")
          )
      ).bind("expr"),
      this
  );
  // clang-format on
}

void FixcapasarraysubscriptCheck::check(
    const MatchFinder::MatchResult &Result) {
  ASTContext *Ctx = Result.Context;
  const auto *ArrayExpr = Result.Nodes.getNodeAs<ArraySubscriptExpr>("expr");
  const auto *Index = Result.Nodes.getNodeAs<Expr>("index");

  /* Should not happen. */
  if (!ArrayExpr || !Index)
    return;

  const auto *T = Index->getType().getTypePtr();

  /* Only warn if we know that the array index is a capability. */
  if (!T->isIntegerType() || !Util::isCapability(Ctx, T))
    return;

  /* Ok, we have to fix it. */
  const auto Begin = Index->getBeginLoc();
  const auto End = ArrayExpr->getRBracketLoc();
  diag(Begin,
       "CHERI: Array index expression has capability type. Wrap it in __c_ua()")
      << FixItHint::CreateInsertion(Begin, "__c_ua(")
      << FixItHint::CreateReplacement(SourceRange(End, End), ")]");
}

} // namespace clang::tidy::cheri
