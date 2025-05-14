//===--- FixptrtoulongcastCheck.cpp - clang-tidy --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FixptrtoulongcastCheck.h"
#include "CheriUtil.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cheri {

void FixptrtoulongcastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      explicitCastExpr(
          hasDestinationType(hasCanonicalType(builtinType())),
          hasSourceExpression(expr(hasType(pointerType().bind("from")))))
          .bind("cast"),
      this);
}

void FixptrtoulongcastCheck::check(const MatchFinder::MatchResult &Result) {
  auto *Ctx = Result.Context;
  auto *SM = Result.SourceManager;
  const auto *C = Result.Nodes.getNodeAs<ExplicitCastExpr>("cast");
  const auto *From = Result.Nodes.getNodeAs<Type>("from");
  const auto *To = C->getType().getTypePtr();

  /* Do not touch a forced cast. */
  if (Util::isForced(Result.SourceManager, C))
    return;

  /* Ignore casts where the "pointer" cannot carry provenance. */
  if (!From->canCarryProvenance(*Ctx))
    return;

  /* Determine what kind of destination type we deal with. */
  bool ToAddr = Util::isPlainAddress(Ctx, To);
  bool ToCap = Util::isIntegerCapability(Ctx, To);
  if (!ToAddr && !ToCap)
    return;

  if (Util::checkCastExpr(Ctx, C))
    return;
  if (Util::checkExprUsage(Ctx, SM, C))
    return;

  /* Check usage of the cast expression. */
  const auto &Parents = Ctx->getParents(*C);
  if (Parents.empty())
    return;

  const auto *Var = Parents[0].get<VarDecl>();
  if (Var && !Util::isPlainAddress(Ctx, Var))
    Var = nullptr;

  if (ToAddr)
    diag(C->getExprLoc(), "CHERI: Explicit cast from pointer to 'unsigned "
                          "long'. Cast to 'uintptr_t' instead");
  if (const auto *CC = dyn_cast<CStyleCastExpr>(C)) {
    if (ToAddr)
      diag(C->getExprLoc(), "Here:") << FixItHint::CreateReplacement(
          SourceRange(CC->getLParenLoc(), CC->getRParenLoc()), "(uintptr_t)");
  }
  if (Var) {
    auto Begin = Var->getTypeSpecStartLoc();
    auto End = Var->getTypeSpecEndLoc();
    diag(C->getExprLoc(), "CHERI: Variable type should be 'uintptr_t'")
        << FixItHint::CreateReplacement(SourceRange(Begin, End), "uintptr_t");
  }
}

} // namespace clang::tidy::cheri
