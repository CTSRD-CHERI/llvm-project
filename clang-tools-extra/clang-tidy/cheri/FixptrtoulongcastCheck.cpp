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
  // clang-format off
  Finder->addMatcher(
      explicitCastExpr(
          hasDestinationType(
              hasCanonicalType(
                  builtinType()
              )
          ),
          hasSourceExpression(
              expr(
                  hasType(
                      type(
                          hasUnqualifiedDesugaredType(
                              pointerType()
                          )
                      ).bind("from")
                  )
              )
          )
      ).bind("cast"),
      this
  );
  // clang-format on
}

void FixptrtoulongcastCheck::check(const MatchFinder::MatchResult &Result) {
  auto *Ctx = Result.Context;
  auto *SM = Result.SourceManager;
  const auto *C = Result.Nodes.getNodeAs<ExplicitCastExpr>("cast");
  const auto *From = Result.Nodes.getNodeAs<Type>("from");
  const auto *To = C->getType().getTypePtr();
  const char *TypeString = "uintptr_t";
  const char *CastString = "(uintptr_t)";

  /* Do not touch a forced cast. */
  if (Util::isForced(Result.SourceManager, C))
    return;

  /* Ignore casts where the "pointer" cannot carry provenance. */
  if (!From->canCarryProvenance(*Ctx))
    return;

  /* Determine what kind of destination type we deal with. */
  bool FixCast = Util::isPlainAddress(Ctx, To);
  if (!FixCast && !Util::isIntegerCapability(Ctx, To))
    return;

  if (Util::checkCastExpr(Ctx, C))
    return;
  if (Util::checkExprUsage(Ctx, SM, C))
    return;

  bool UserPtr = Util::isUserPtr(From);
  if (UserPtr) {
    TypeString = "user_uintptr_t";
    CastString = "(user_uintptr_t)";
  }
  if (!FixCast && UserPtr != Util::isUserIntCapTypedef(To))
    FixCast = true;

  /* Check usage of the cast expression. */
  const auto &Parents = Ctx->getParents(*C);
  if (Parents.empty())
    return;

  const auto *Var = Parents[0].get<VarDecl>();
  if (Var && !Util::isPlainAddress(Ctx, Var)) {
    if (UserPtr == Util::isUserIntCapTypedef(Var->getType().getTypePtr()))
      Var = nullptr;
  }

  if (FixCast)
    diag(C->getExprLoc(), "CHERI: Explicit cast from pointer to 'unsigned "
                          "long'. Cast to '%0' instead")
        << TypeString;
  if (const auto *CC = dyn_cast<CStyleCastExpr>(C)) {
    if (FixCast)
      diag(C->getExprLoc(), "Here:") << FixItHint::CreateReplacement(
          SourceRange(CC->getLParenLoc(), CC->getRParenLoc()), CastString);
  }
  if (Var) {
    auto Begin = Var->getTypeSpecStartLoc();
    auto End = Var->getTypeSpecEndLoc();
    diag(C->getExprLoc(), "CHERI: Variable type should be 'uintptr_t'")
        << FixItHint::CreateReplacement(SourceRange(Begin, End), TypeString);
  }
}

} // namespace clang::tidy::cheri
