//===--- FixinttoptrcastCheck.cpp - clang-tidy ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FixinttoptrcastCheck.h"
#include "CheriUtil.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cheri {

void FixinttoptrcastCheck::registerMatchers(MatchFinder *Finder) {
  // clang-format off
  Finder->addMatcher(
      explicitCastExpr(
          hasDestinationType(
              type(
                  hasUnqualifiedDesugaredType(
                      pointerType()
                  )
              )
          ),
          hasSourceExpression(
              expr(
                  hasType(
                      hasCanonicalType(
                          isInteger()
                      )
                  )
              )
          )
      ) .bind("cast"),
      this
  );
  // clang-format on
}

void FixinttoptrcastCheck::check(const MatchFinder::MatchResult &Result) {
  auto *Ctx = Result.Context;
  auto *SM = Result.SourceManager;
  const auto *C = Result.Nodes.getNodeAs<ExplicitCastExpr>("cast");

  /* Ignore a forced cast. */
  if (Util::isForced(SM, C))
    return;

  /* Only fix if the cast expression is known to be safe. */
  if (!Util::checkCastExpr(Ctx, C) && !Util::checkExprUsage(Ctx, SM, C))
    return;

  /* Find the subexpression removing outer parens. */
  const auto *Sub = C->getSubExpr();
  while (const auto *Paren = dyn_cast<ParenExpr>(Sub))
    Sub = Paren->getSubExpr();

  /* Integer constants don't generate a compiler error. */
  if (Sub->isIntegerConstantExpr(*Ctx))
    return;

  /* Nothing to do if the source expression already is a capability. */
  if (Util::isCapability(Ctx, Sub))
    return;

  diag(Sub->getExprLoc(), "CHERI: Intermediate cast to 'uintptr_t' required");
  if (isa<ExplicitCastExpr>(Sub)) {
    if (const auto *SubCC = dyn_cast<CStyleCastExpr>(Sub)) {
      diag(Sub->getExprLoc(), "Here:") << FixItHint::CreateReplacement(
          SourceRange(SubCC->getLParenLoc(), SubCC->getRParenLoc()),
          "(uintptr_t)");
    }
  } else {
    if (const auto *CC = dyn_cast<CStyleCastExpr>(C)) {
      diag(Sub->getExprLoc(), "Here:")
          << FixItHint::CreateInsertion(CC->getRParenLoc(), ")(uintptr_t");
    }
  }
}

} // namespace clang::tidy::cheri
