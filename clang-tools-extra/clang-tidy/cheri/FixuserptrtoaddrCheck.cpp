//===--- FixuserptrtoaddrCheck.cpp - clang-tidy ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CheriUtil.h"
#include "FixuserptrtoaddrCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cheri {

void FixuserptrtoaddrCheck::registerMatchers(MatchFinder *Finder) {
  // clang-format off
  Finder->addMatcher(
      callExpr(
          callee(
              functionDecl(
                  hasName("__c_pa")
              )
          )
      ).bind("cpa"),
      this
  );
  Finder->addMatcher(
      callExpr(
          callee(
              functionDecl(
                  hasName("__c_pa_u")
              )
          )
      ).bind("cpau"),
      this
  );
  // clang-format on
}

void FixuserptrtoaddrCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Cpa = Result.Nodes.getNodeAs<CallExpr>("cpa");
  const auto *Cpau = Result.Nodes.getNodeAs<CallExpr>("cpau");
  const CallExpr *Call = Cpa ? Cpa : Cpau;

  if (Cpa && Cpau)
    return;
  if (!Call || Call->getNumArgs() == 0)
    return;

  /*
   * Skip the outer most implicit cast (if any) that converts to the
   * formal type of the function parameter.
   */
  const Expr *Arg = Call->getArg(0);
  if (const auto *Cast = dyn_cast<ImplicitCastExpr>(Arg))
    Arg = Cast->getSubExpr();
  bool user = Util::isUserPtr(Arg);

  const Expr *Callee = Call->getCallee();
  auto R = CharSourceRange::getTokenRange(Callee->getBeginLoc(), Callee->getEndLoc());
  if (user && !Cpau)
    {
      diag(Call->getExprLoc(),
          "CHERI: __c_pa() on user pointer. Should be __c_pa_u()")
          << FixItHint::CreateReplacement(R, "__c_pa_u");
    }
  if (!user && !Cpa)
    {
      diag(Call->getExprLoc(),
          "CHERI: __c_pa_u() on kernel pointer. Should be __c_pa()")
          << FixItHint::CreateReplacement(R, "__c_pa");
    }
}

} // namespace clang::tidy::cheri
