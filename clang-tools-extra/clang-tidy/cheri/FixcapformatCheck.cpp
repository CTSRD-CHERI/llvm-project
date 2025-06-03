//===--- FixcapformatCheck.cpp - clang-tidy -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FixcapformatCheck.h"
#include "CheriUtil.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cheri {

void FixcapformatCheck::registerMatchers(MatchFinder *Finder) {
  // clang-format off
  Finder->addMatcher(
      callExpr(
          callee(
              expr(
                  hasType(
                      type(
                          hasUnqualifiedDesugaredType(
                              pointerType(
                                  pointee(
                                      type(
                                          hasUnqualifiedDesugaredType(
                                              functionProtoType().bind("ftype") 
                                          )
                                      )
                                  )
                              )
                          )
                      )
                  )
              )
          )
      ).bind("call"),
      this
  );
  // clang-format on
}

void FixcapformatCheck::check(const MatchFinder::MatchResult &Result) {
  auto *Ctx = Result.Context;
  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("call");
  const auto *FType = Result.Nodes.getNodeAs<FunctionProtoType>("ftype");

  if (!Call || !FType || !FType->isVariadic())
    return;

  for (unsigned int i = FType->getNumParams(); i < Call->getNumArgs(); ++i) {
    const auto *A = Call->getArg(i);
    if (!Util::isIntegerCapability(Ctx, A->getType().getTypePtr()))
      continue;

    diag(A->getExprLoc(),
         "CHERI: Integer capability passed to a variadic function. "
         "Need cast to 'unsigned long'")
        << FixItHint::CreateInsertion(A->getBeginLoc(), "(unsigned long)");
  }
}

} // namespace clang::tidy::cheri
