//===--- PtrtointcastCheck.cpp - clang-tidy -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "PtrtointcastCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cheri {

void PtrtointcastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(castExpr().bind("cast"), this);
}

/*
 * Return true if the source range contains the literal string "__force".
 */
bool PtrtointcastCheck::isForced(SourceManager *SM, SourceLocation Start,
                                 SourceLocation End) {
  const char *S = SM->getCharacterData(Start);
  const char *E = SM->getCharacterData(End);

  if (S == E)
    return false;

  return StringRef(S, E - S).find(StringRef("__force")) != StringRef::npos;
}

/*
 * Return true if the cast is a C-style cast and the target type (i.e. the
 * string in the parentheses) as written in the source code contains the
 * literal string "__force".
 */
bool PtrtointcastCheck::isForced(SourceManager *SM, const CastExpr *C) {
  if (auto *CC = dyn_cast<CStyleCastExpr>(C))
    return isForced(SM, CC->getLParenLoc(), CC->getRParenLoc());

  return false;
}

/*
 * Check the usage of an expression. Return true if the expression is
 * used in a way that can never result in a valid pointer.
 */
bool PtrtointcastCheck::checkExprUsage(ASTContext *Ctx, SourceManager *SM,
                                       const Expr *E, unsigned int AddrWidth) {
  /*
   * If the expression has no parents, we do not know how it
   * is used.
   */
  if (Ctx->getParents(*E).empty())
    return false;

  /* Check all uses (parents) of the expression. */
  for (auto &P : Ctx->getParents(*E)) {
    const auto *Pexpr = P.get<Expr>();

    /* Use is never ok if it is not in an expression. */
    if (!Pexpr)
      return false;

    /* Check usage in cast expressions. */
    if (auto C = dyn_cast<CastExpr>(Pexpr)) {
      const auto *To = C->getType().getTypePtr();

      /* Never ok if the cast is not to an integer type. */
      if (!To->isIntegerType())
        return false;

      /* Cast to something smaller than an address is ok. */
      if (Ctx->getTypeSize(To) < AddrWidth)
        continue;

      /* Forced cast is ok. */
      if (isForced(SM, C))
        continue;

      /* Otherwise check the usage of the parent expression. */
      if (checkExprUsage(Ctx, SM, C, AddrWidth))
        continue;
    }

    /* Check usage in an unary operator. */
    if (auto Unop = dyn_cast<UnaryOperator>(Pexpr)) {
      /* Inconsitent AST. Reject. */
      if (Unop->getSubExpr() != E)
        return false;

      /* Use in a logical NOT is fine. */
      if (Unop->getOpcode() == UO_LNot)
        continue;
    }

    /* Check usage in a binary operator. */
    if (auto Binop = dyn_cast<BinaryOperator>(Pexpr)) {
      Expr *Other = nullptr;

      if (Binop->getLHS() == E)
        Other = Binop->getRHS();
      if (Binop->getRHS() == E)
        Other = Binop->getLHS();
      if (Other == nullptr)
        return false;

      switch (Binop->getOpcode()) {
      case BO_LT:
      case BO_GT:
      case BO_LE:
      case BO_GE:
      case BO_EQ:
      case BO_NE:
      case BO_Cmp:
      case BO_Rem:
      case BO_LAnd:
      case BO_LOr:
        /* Boolean expressions are always ok. */
        continue;
      case BO_And: {
        /*
         * Binary AND ("&") is ok if the other operand
         * is constant and not negative as an int66_t.
         * E.g. (cap & 0xfffUL) is fine because it can
         * never be a pointer but (cap & ~0xfffUL) is not.
         */
        auto Val = Other->getIntegerConstantExpr(*Ctx);
        if (Val && Val->getExtValue() >= 0)
          continue;

        /* Also ok if the parent usage is ok. */
        if (checkExprUsage(Ctx, SM, Pexpr, AddrWidth))
          continue;
        break;
      }
      case BO_Xor:
      case BO_Or: {
        /*
         * For binary OR ("|") and XOR ("^") check
         * the parent's usage.
         */
        if (checkExprUsage(Ctx, SM, Pexpr, AddrWidth))
          continue;
        break;
      }
      case BO_Add:
      case BO_Sub: {
        /*
         * For ADD ("+") and SUB ("-") with a constant
         * expression check the parent usage.
         */
        auto Val = Other->getIntegerConstantExpr(*Ctx);
        if (Val && checkExprUsage(Ctx, SM, Pexpr, AddrWidth))
          continue;
        break;
      }
      default:
        break;
      }

      /*
       * If the expression is part of a binary expression that
       * results in a capability but the other operand provides
       * provenance this is ok. The cast is likely due to integer
       * propagation. For things like "+=" this is only true if
       * the we are on the RHS of the expression.
       */
      switch (Binop->getOpcode()) {
      case BO_AddAssign:
      case BO_SubAssign:
      case BO_AndAssign:
      case BO_OrAssign:
      case BO_XorAssign:
        if (E != Binop->getRHS())
          break;
        LLVM_FALLTHROUGH;
      case BO_Add:
      case BO_Sub:
      case BO_And:
      case BO_Or:
      case BO_Xor: {
        const auto *T = E->getType().getTypePtr();
        const auto *OT = Other->getType().getTypePtr();

        if (!T->isCHERICapabilityType(*Ctx))
          break;
        if (!OT->isCHERICapabilityType(*Ctx))
          break;
        if (!T->canCarryProvenance(*Ctx) && OT->canCarryProvenance(*Ctx))
          continue;
        break;
      }
      default:
        break;
      }
    }

    /* For ParenExpr "(...)" check their respecitve parent. */
    if (isa<ParenExpr>(Pexpr) || isa<CastExpr>(Pexpr)) {
      if (checkExprUsage(Ctx, SM, Pexpr, AddrWidth))
        continue;
    }

    /* Reject all other types of parent expressions. */
    return false;
  }

  return true;
}

/*
 * Check an expression that is cast to a capability/pointer.
 * Return true if we could determine that the expression could
 * not possibly be a valid pointer.
 */
bool PtrtointcastCheck::checkCastExpr(ASTContext *Ctx, const Expr *E,
                                      unsigned int AddrWidth) {
  /*
   * A constant expression cast to a capability type
   * can never be a valid pointer.
   */
  if (E->isIntegerConstantExpr(*Ctx))
    return true;

  /* Check binary expressions. */
  if (auto Binop = dyn_cast<BinaryOperator>(E)) {
    auto Clhs = Binop->getLHS()->getIntegerConstantExpr(*Ctx);
    auto Crhs = Binop->getRHS()->getIntegerConstantExpr(*Ctx);

    switch (Binop->getOpcode()) {
    case BO_And: {
      /*
       * If either side of the binary expression
       * is a positive constant this is ok.
       */
      if (Clhs && Clhs->getExtValue() >= 0)
        return true;
      if (Crhs && Crhs->getExtValue() >= 0)
        return true;
      break;
    }
    default:
      break;
    }
  }

  /* Decend into a ParenEpxr... */
  if (auto Paren = dyn_cast<ParenExpr>(E)) {
    if (checkCastExpr(Ctx, Paren->getSubExpr(), AddrWidth))
      return true;
  }

  /* ... and into another CastExpr. */
  if (auto C = dyn_cast<CastExpr>(E)) {
    if (checkCastExpr(Ctx, C->getSubExpr(), AddrWidth))
      return true;
  }

  return false;
}

void PtrtointcastCheck::checkCast(const MatchFinder::MatchResult &Result) {
  SourceManager *SM = Result.SourceManager;
  auto Ctx = Result.Context;
  const auto *C = Result.Nodes.getNodeAs<CastExpr>("cast");
  const auto *From = C->getSubExpr()->getType().getTypePtr();
  const auto *To = C->getType().getTypePtr();
  bool FromCap = From->isCHERICapabilityType(*Ctx);
  bool ToCap = To->isCHERICapabilityType(*Ctx);
  unsigned int Fromsz = Ctx->getTypeSize(From);
  unsigned int Tosz = Ctx->getTypeSize(To);
  unsigned int AddrWidth;

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
  }

  /* The width of an address is half the size of a pointer. */
  if (FromCap)
    AddrWidth = Fromsz / 2;
  else
    AddrWidth = Tosz / 2;

  /*
   * Casting between something that is smaller than an address
   * and a capability is ok. This cannot be a dereferencable
   * pointer in any case.
   */
  if (FromCap && Tosz < AddrWidth)
    return;
  if (ToCap && Fromsz < AddrWidth)
    return;

  /* If the cast is forced it is ok. */
  if (isForced(SM, C))
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
  if (checkExprUsage(Ctx, SM, C, AddrWidth))
    return;

  /*
   * Check the expression that is subject to a cast. If it
   * can never result in a pointer do not warn.
   */
  if (checkCastExpr(Ctx, C->getSubExpr(), AddrWidth))
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
  if (From->isCHERICapabilityType(*Ctx) == To->isCHERICapabilityType(*Ctx))
    return true;

  /* Exactly one of the types is not a Capability type. Find it. */
  const Type *Cap = From->isCHERICapabilityType(*Ctx) ? From : To;
  const Type *NonCap = To->isCHERICapabilityType(*Ctx) ? From : To;

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
  const auto *C = Result.Nodes.getNodeAs<ExplicitCastExpr>("cast");
  const auto *From = C->getSubExpr()->getType().getTypePtr();
  const auto *To = C->getType().getTypePtr();

  /*
   * This function only cares about the pointer target type in pointer
   * conversions.
   */
  if (!From->isCapabilityPointerType() || !To->isCapabilityPointerType())
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
