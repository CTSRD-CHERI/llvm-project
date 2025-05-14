//===--- PtrtointcastCheck.cpp - clang-tidy -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "PtrtointcastCheck.h"
#include "CheriUtil.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cheri {

void PtrtointcastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(castExpr().bind("cast"), this);
}

void PtrtointcastCheck::checkCast(const MatchFinder::MatchResult &Result) {
  SourceManager *SM = Result.SourceManager;
  auto Ctx = Result.Context;
  const auto *C = Result.Nodes.getNodeAs<CastExpr>("cast");
  const auto *From = C->getSubExpr()->getType().getTypePtr();
  const auto *To = C->getType().getTypePtr();
  bool FromCap = Util::isCapability(Ctx, From);
  bool ToCap = Util::isCapability(Ctx, To);

  /*
   * If either both types are capabilities or none of them is
   * everything is ok.
   */
  if (FromCap == ToCap)
    return;

  /*
   * Do not warn for implicit decay casts.
   */
  if (ToCap && isa<ImplicitCastExpr>(C)) {
    auto Kind = C->getCastKind();
    if (Kind == CK_ArrayToPointerDecay)
      return;
    if (Kind == CK_FunctionToPointerDecay)
      return;
    if (Kind == CK_BuiltinFnToFnPtr)
      return;
  }

  /*
   * Casting between something that is smaller than an address
   * and a capability is ok. This cannot be a dereferencable
   * pointer in any case.
   */
  if (FromCap && Util::isSafeNonPtrType(Ctx, To))
    return;
  if (ToCap && Util::isSafeNonPtrType(Ctx, From))
    return;

  /* If the cast is forced it is ok. */
  if (Util::isForced(SM, C))
    return;

  auto Val = C->getIntegerConstantExpr(*Ctx);
  if (Val)
    return;

  /*
   * Check how the value is used. Do not warn if the use is ok
   * because the value is never stored as or used as something
   * that could later be interpreted as a pointer. This applies
   * to both upcasts and downcasts.
   */
  if (Util::checkExprUsage(Ctx, SM, C))
    return;

  /*
   * Check the expression that is subject to a cast. If it
   * can never result in a pointer do not warn.
   */
  if (Util::checkCastExpr(Ctx, C->getSubExpr()))
    return;

  /* The cast is not ok. Warn! */
  if (FromCap)
    diag(C->getExprLoc(), "CHERI: Invalid capability to integer cast");
  else {
    diag(C->getExprLoc(), "CHERI: Invalid integer to capability cast");
  }
}

/* Check to pointee types for compatibility. */
bool PtrtointcastCheck::checkPointeeTypes(ASTContext *Ctx, const Type *From,
                                          const Type *To) {
  /* Pointers to Array types are in fact pointers to the first element. */
  if (auto FromA = dyn_cast<ArrayType>(From))
    if (FromA->canDecayToPointerType())
      From = FromA->getElementType().getTypePtr();
  if (auto ToA = dyn_cast<ArrayType>(To))
    if (ToA->canDecayToPointerType())
      To = ToA->getElementType().getTypePtr();

  /* If both are real pointers the pointee types must be compatible. */
  if (From->isCapabilityPointerType() && To->isCapabilityPointerType()) {
    auto FromP = From->getPointeeType().getTypePtr();
    auto ToP = To->getPointeeType().getTypePtr();
    return checkPointeeTypes(Ctx, FromP, ToP);
  }

  /* It is fine if either both or none of the types are capabilities. */
  if (Util::isCapability(Ctx, From) == Util::isCapability(Ctx, To))
    return true;

  /* Exactly one of the types is not a Capability type. Find it. */
  const Type *Cap = Util::isCapability(Ctx, From) ? From : To;
  const Type *NonCap = Util::isCapability(Ctx, To) ? From : To;

  /* "void" is ok */
  if (NonCap->isVoidType())
    return true;

  /* Check type sizes */
  uint64_t CapSize = Ctx->getTypeSize(Cap);
  uint64_t NonCapSize = Ctx->getTypeSize(NonCap);

  /* Ok if the sizes match. */
  if (CapSize == NonCapSize)
    return true;

  /* Assume cast to (char *) is fine. */
  if (NonCapSize <= 8)
    return true;

  /*
   * Accept casts between structures and capabilities if the
   * structure has at least capability alignment.
   */
  if (NonCapSize > CapSize && NonCap->isStructureType()) {
    if (Ctx->getTypeAlign(NonCap) >= Ctx->getTypeAlign(Cap))
      return true;
  }

  return false;
}

/*
 * Check for pointer casts where the terminal non-pointer type is
 * a pointer on one side and an address on the other.
 */
void PtrtointcastCheck::checkIndirectCast(
    const MatchFinder::MatchResult &Result) {
  auto Ctx = Result.Context;
  SourceManager *SM = Result.SourceManager;
  const auto *C = Result.Nodes.getNodeAs<ExplicitCastExpr>("cast");
  const auto *From = C->getSubExpr()->getType().getTypePtr();
  const auto *To = C->getType().getTypePtr();

  /*
   * This function only cares about the pointer target type in pointer
   * conversions.
   */
  if (!From->isCapabilityPointerType() || !To->isCapabilityPointerType())
    return;

  /* Ignore a forced cast. */
  if (Util::isForced(SM, C))
    return;

  auto FromP = From->getPointeeType().getTypePtr();
  auto ToP = To->getPointeeType().getTypePtr();
  if (checkPointeeTypes(Ctx, FromP, ToP))
    return;

  diag(C->getExprLoc(), "CHERI: Incompatible pointer target types in cast");
}

void PtrtointcastCheck::check(const MatchFinder::MatchResult &Result) {
  /* Check all casts to/from capability types. */
  if (Result.Nodes.getNodeAs<CastExpr>("cast"))
    checkCast(Result);
  /* Check if target types are suspicious in explicit casts. */
  if (Result.Nodes.getNodeAs<ExplicitCastExpr>("cast"))
    checkIndirectCast(Result);
}

} // namespace clang::tidy::cheri
