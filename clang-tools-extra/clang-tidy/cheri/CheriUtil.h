//===--- CheriUtil.h - clang-tidy -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Preprocessor.h"

namespace clang::tidy::cheri {

/* A class with helper functions that are useful in multiple cheri checks. */
class Util {
public:
  /*
   * Return true if the source range contains the literal string "__force".
   */
  static bool isForced(SourceManager *SM, SourceLocation Start,
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
  static bool isForced(SourceManager *SM, const CastExpr *C) {
    if (auto *CC = dyn_cast<CStyleCastExpr>(C))
      return isForced(SM, CC->getLParenLoc(), CC->getRParenLoc());

    return false;
  }

  static bool isUserAttr(const AttributedType *AT) {
#ifdef NOTYET
    const auto *TA = dyn_cast<TypeAttr>(AT->getAttr());
    if (!TA)
      return false;
    const auto *N = TA->getAttrName();
    if (!N)
      return false;

    return N->getName() == StringRef("user");
#else
    return AT->getAttrKind() == attr::AnnotateType;
#endif
  }

  static bool isUserPtr(const Type *OrigT) {
    const auto *PT = OrigT->getAs<PointerType>();
    if (!PT)
      return false;
    const Type *T = PT->getPointeeType().getTypePtr();
    while (1) {
      const auto *AT = T->getAs<AttributedType>();
      if (!AT)
        return false;
      if (isUserAttr(AT))
        return true;
      T = AT->getModifiedType().getTypePtr();
    }
  }

  static bool isUserPtr(const Expr *E) {
    return isUserPtr(E->getType().getTypePtr());
  }

  /*
   * Given a type of something that is or can be called find the
   * type of the actual function that is called as a FunctionProtoType.
   * If present this removes typedefs, parenthesis and one level of
   * pointer indirection.
   * Returns a nullptr if the result is not a FunctionProtoType.
   */
  static const FunctionProtoType *extractFunctionProtoType(const Type *T) {
    if (const auto *PT = T->getAs<PointerType>())
      T = PT->getPointeeType().getTypePtr();

    return T->getAs<FunctionProtoType>();
  }

  /* Convenience wrapper */
  static const FunctionProtoType *extractFunctionProtoType(const Expr *E) {
    return extractFunctionProtoType(E->getType().getTypePtr());
  }

  /* Convenience wrapper */
  static const FunctionProtoType *extractFunctionProtoType(const ValueDecl *D) {
    return extractFunctionProtoType(D->getType().getTypePtr());
  }

  /*
   * Return true if this is a typdef type with a name that indicates
   * an integer capability type.
   */
  static bool isIntCapTypedef(const Type *T) {
    const auto *TD = T->getAs<TypedefType>();
    if (TD) {
      const auto &Name = TD->getDecl()->getName();
      if (Name == StringRef("uintptr_t"))
        return true;
      if (Name == StringRef("intptr_t"))
        return true;
      if (Name == StringRef("user_uintptr_t"))
        return true;
      if (Name == StringRef("user_intptr_t"))
        return true;
      if (Name == StringRef("__kernel_uintptr_t"))
        return true;
      if (Name == StringRef("register_t"))
        return true;
      if (Name == StringRef("__intcap_t"))
        return true;
      if (Name == StringRef("__uintcap_t"))
        return true;
    }

    return false;
  }

  static bool isUserIntCapTypedef(const Type *T) {
    const auto *TD = T->getAs<TypedefType>();
    if (TD) {
      const auto &Name = TD->getDecl()->getName();
      if (Name == StringRef("user_uintptr_t"))
        return true;
      if (Name == StringRef("user_intptr_t"))
        return true;
      if (Name == StringRef("__kernel_uintptr_t"))
        return true;
    }

    return false;
  }

  static bool isNonCapTypedef(const Type *T) {
    const auto *TD = T->getAs<TypedefType>();
    if (TD) {
      const auto &Name = TD->getDecl()->getName();
      if (Name == StringRef("__ptraddr_t"))
        return true;
      if (Name == StringRef("__ptraddr64_t"))
        return true;
      if (Name == StringRef("resource_size_t"))
        return true;
      if (Name == StringRef("phys_addr_t"))
        return true;
    }

    return false;
  }

  /*
   * Return true if the type refers to something that can be a
   * plain address (not a capability). If CHERI is enabled the
   * detection of capability types is precise. Otherwise only
   * typedefs to uintptr_t and user_uintptr_t are recognized as
   * capabilities.
   */
  static bool isPlainAddress(ASTContext *Ctx, const Type *OrigT) {
    const auto *CT =
        Ctx->getCanonicalType(OrigT->getUnqualifiedDesugaredType());
    const auto *BT = dyn_cast<BuiltinType>(CT);

    /* The underlying canonical type of an address must be unsigned long */
    if (!BT)
      return false;
    if (BT->getKind() != BuiltinType::ULong && BT->getKind() != BuiltinType::ULongLong)
      return false;

    /* If it is a typedef to uintptr_t it is not a plain address. */
    return !isIntCapTypedef(OrigT);
  }

  /* Convenience wrapper. */
  static bool isPlainAddress(ASTContext *Ctx, const ValueDecl *D) {
    return isPlainAddress(Ctx, D->getType().getTypePtr());
  }

  static bool isPlainAddress(ASTContext *Ctx, const Expr *E) {
    return isPlainAddress(Ctx, E->getType().getTypePtr());
  }

  static bool isIntegerCapability(ASTContext *Ctx, const Type *OrigT) {
    const auto *CT =
        Ctx->getCanonicalType(OrigT->getUnqualifiedDesugaredType());
    const auto *BT = dyn_cast<BuiltinType>(CT);

    if (!BT)
      return false;
    switch (BT->getKind()) {
    case BuiltinType::UIntCap:
    case BuiltinType::IntCap:
      return true;
    case BuiltinType::ULong:
    case BuiltinType::Long:
    case BuiltinType::ULongLong:
    case BuiltinType::LongLong:
    case BuiltinType::UInt128:
    case BuiltinType::Int128:
      return isIntCapTypedef(OrigT);
    default:
      return false;
    }
  }

  static bool isCapability(ASTContext *Ctx, const Type *T) {
    bool ptr = T->isPointerType();
    bool intcap = isIntegerCapability(Ctx, T);
    return ptr || intcap;
  }

  static bool isCapability(ASTContext *Ctx, const Expr *E) {
    return isCapability(Ctx, E->getType().getTypePtr());
  }

  static bool isSafeNonPtrType(ASTContext *Ctx, const Type *OrigT) {
    const auto *CT =
        Ctx->getCanonicalType(OrigT->getUnqualifiedDesugaredType());

    if (OrigT->isEnumeralType())
      return true;
    if (CT->isVoidType())
      return true;

    if (const auto *BT = dyn_cast<BuiltinType>(CT)) {
      switch (BT->getKind()) {
      case BuiltinType::Bool:
      case BuiltinType::Char_S:
      case BuiltinType::Char_U:
      case BuiltinType::SChar:
      case BuiltinType::UChar:
      case BuiltinType::Short:
      case BuiltinType::UShort:
      case BuiltinType::Int:
      case BuiltinType::UInt:
      case BuiltinType::Char8:
      case BuiltinType::Char16:
      case BuiltinType::Char32:
      case BuiltinType::WChar_S:
      case BuiltinType::WChar_U:
        return true;
      case BuiltinType::ULong:
      case BuiltinType::Long:
      case BuiltinType::ULongLong:
      case BuiltinType::LongLong:
        return isNonCapTypedef(OrigT);
      default:
        return false;
      }
    }

    if (const auto *BitT = dyn_cast<BitIntType>(CT)) {
      if (BitT->getNumBits() < 64)
        return true;
    }

    return false;
  }

  /* Convenience wrapper. */
  static bool isIntegerCapability(ASTContext *Ctx, const ValueDecl *D) {
    return isIntegerCapability(Ctx, D->getType().getTypePtr());
  }

  /*
   * Return true if expression is an explicit downcast from a
   * capability to address.
   */
  static bool isExplicitDowncastFromCap(ASTContext *Ctx, const Expr *E) {
    while (1) {
      if (const auto *Paren = dyn_cast<ParenExpr>(E)) {
        E = Paren->getSubExpr();
        continue;
      }
      if (const auto *C = dyn_cast<ExplicitCastExpr>(E))
        return isPlainAddress(Ctx, E) && isCapability(Ctx, C->getSubExpr());
      break;
    }

    return false;
  }

  /*
   * Check if the expression has an upstream use as an argumetn in a
   * call to a variadic function.
   */
  static bool isVarArgsArgument(ASTContext *Ctx, const Expr *E) {
    for (const auto &P : Ctx->getParents(*E)) {
      const auto *Pexpr = P.get<Expr>();

      /* If the use is not in an expression ignore it. */
      if (!Pexpr)
        continue;

      /* Check use in a call expression. */
      if (const auto *C = dyn_cast<CallExpr>(Pexpr)) {
        const Expr *Callee = C->getCallee();

        /* Skip if it is the callee and not an argument. */
        if (Callee == E)
          continue;

        /* Report true if the called object is variadic. */
        if (const auto *T = extractFunctionProtoType(C->getCallee()))
          if (T->isVariadic())
            return true;

        /* Do not continue further upwards through a call expression. */
        continue;
      }

      /*
       * Blacklist some expressions where the value is not propagated.
       * We should rather have a whitelist here but let's see if we get
       * away with this.
       */
      if (isa<ArraySubscriptExpr>(Pexpr))
        continue;
      if (isa<MemberExpr>(Pexpr))
        continue;
      if (auto Unop = dyn_cast<UnaryOperator>(Pexpr)) {
        auto K = Unop->getOpcode();
        if (K == UO_AddrOf || K == UO_Deref)
            continue;
      }

      /* Not a call expression: Check upstream use. */
      if (isVarArgsArgument(Ctx, Pexpr))
        return true;
    }

    return false;
  }

  /*
   * Check the usage of an expression. Return true if the expression is
   * used in a way that can never result in a valid pointer.
   */
  static bool checkExprUsage(ASTContext *Ctx, SourceManager *SM,
                             const Expr *E) {
    /*
     * If the expression type is not something that can be an address
     * we are fine.
     */
    if (isSafeNonPtrType(Ctx, E->getType().getTypePtr()))
      return true;

    /*
     * If the use is in a call to a variadic function assume that the use
     * is ok.
     */
    if (Util::isVarArgsArgument(Ctx, E))
      return true;

    /*
     * If the expression has no parents, we do not know how it
     * is used.
     */
    if (Ctx->getParents(*E).empty())
      return false;

    /* Check all uses (parents) of the expression. */
    for (auto &P : Ctx->getParents(*E)) {
      /* Special case: Do not complain about the argument of "typeof()" */
      if (auto t = P.get<TypeLoc>()) {
        auto tof = dyn_cast<TypeOfExprType>(t->getType().getTypePtr());
        if (tof && tof->getUnderlyingExpr() == E)
          continue;
      }

      /*
       * If the expression is used in as the condition in a
       * conditional statement it is implicitly converted to
       * bool and thus ok.
       */
      if (const auto *S = P.get<IfStmt>())
        if (S->getCond() == E)
          continue;
      if (const auto *S = P.get<WhileStmt>())
        if (S->getCond() == E)
          continue;
      if (const auto *S = P.get<DoStmt>())
        if (S->getCond() == E)
          continue;
      if (const auto *S = P.get<ForStmt>())
        if (S->getCond() == E)
          continue;
      if (const auto *S = P.get<ConditionalOperator>())
        if (S->getCond() == E)
          continue;

      /* Otherwise use is never ok if it is not in an expression. */
      const auto *Pexpr = P.get<Expr>();
      if (!Pexpr)
        return false;

      /* Check usage in cast expressions. */
      if (auto C = dyn_cast<CastExpr>(Pexpr)) {
        const auto *To = C->getType().getTypePtr();

        /*
         * Check cast to integer types. The check at the end of the
         * loop will handle other cases by checking the parent.
         */
        if (To->isIntegerType()) {
          /* Cast to something smaller than an address is ok. */
          if (Util::isSafeNonPtrType(Ctx, To))
            continue;

          /* Forced cast is ok. */
          if (Util::isForced(SM, C))
            continue;

          /* Otherwise check the usage of the parent expression. */
          if (checkExprUsage(Ctx, SM, C))
            continue;
        }
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
        case BO_LAnd:
        case BO_LOr:
          /* Boolean expressions are always ok. */
        case BO_Rem:
        case BO_Div:
          /* Division and modulus expressions are always ok. */
          continue;
        case BO_And: {
          /*
           * Binary AND ("&") is ok if the other operand
           * is constant and not negative as an int66_t.
           * E.g. (cap & 0xfffUL) is fine because it can
           * never be a pointer but (cap & ~0xfffUL) is not.
           */
          auto Val = Other->getIntegerConstantExpr(*Ctx);
          if (Val && Val->sge(0))
            continue;

          /* Also ok if the parent usage is ok. */
          if (checkExprUsage(Ctx, SM, Pexpr))
            continue;
          break;
        }
        case BO_Xor:
        case BO_Or: {
          /*
           * For binary OR ("|") and XOR ("^") check
           * the parent's usage.
           */
          if (checkExprUsage(Ctx, SM, Pexpr))
            continue;
          break;
        }
        case BO_Add:
        case BO_Sub: {
          /*
           * For ADD ("+") and SUB ("-") with a constant
           * expression check the parent usage.
           */
          if (checkExprUsage(Ctx, SM, Pexpr))
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

          if (!Util::isCapability(Ctx, T))
            break;
          if (!Util::isCapability(Ctx, OT))
            break;
          if (!T->canCarryProvenance(*Ctx) && OT->canCarryProvenance(*Ctx))
            continue;
          break;
        }
        default:
          break;
        }

        if (Binop->getOpcode() == BO_Sub) {
          if (Util::isExplicitDowncastFromCap(Ctx, E) &&
              Util::isExplicitDowncastFromCap(Ctx, Other))
            continue;
        }
      }

      /*
       * If it is a call expression check the argument type.
       * This catches the __ptraddr_t case where no cast is necessary.
       */
      if (auto Call = dyn_cast<CallExpr>(Pexpr)) {
        unsigned int ArgPos = ~0U;
        for (unsigned int i = 0; i < Call->getNumArgs(); ++i) {
          if (Call->getArg(i) == E) {
            ArgPos = i;
            break;
          }
        }
        if (ArgPos != ~0U) {
          const auto *F = extractFunctionProtoType(Call->getCallee());
          if (F) {
            if (F->isVariadic())
              continue;
            if (ArgPos < F->getNumParams() &&
                isSafeNonPtrType(Ctx, F->getParamType(ArgPos).getTypePtr()))
              continue;
          }
        }
      }

      /*
       * For certain expression type uses we can check the parent
       * to determine if the use is ok.
       */
      if (isa<ParenExpr>(Pexpr) || isa<CastExpr>(Pexpr) ||
          isa<AbstractConditionalOperator>(Pexpr)) {
        if (checkExprUsage(Ctx, SM, Pexpr))
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
  static bool checkCastExpr(ASTContext *Ctx, const Expr *E) {
    /*
     * A constant expression cast to a capability type
     * can never be a valid pointer.
     */
    if (E->getIntegerConstantExpr(*Ctx))
      return true;

    /* If the expression is too small to be an adress it is ok, too. */
    if (Util::isSafeNonPtrType(Ctx, E->getType().getTypePtr()))
      return true;

    /* Check binary expressions. */
    if (auto Binop = dyn_cast<BinaryOperator>(E)) {
      auto Clhs = Binop->getLHS()->getIntegerConstantExpr(*Ctx);
      auto Crhs = Binop->getRHS()->getIntegerConstantExpr(*Ctx);

      switch (Binop->getOpcode()) {
      case BO_And:
      case BO_Rem: {
        /*
         * If either side of and "and" or "remainder" expression
         * is a positive constant this is ok.
         */
        if (Clhs && Clhs->sge(0))
          return true;
        if (Crhs && Crhs->sge(0))
          return true;
        break;
      }
      default:
        break;
      }
    }

    /* Descend into a ParenEpxr... */
    if (auto Paren = dyn_cast<ParenExpr>(E)) {
      if (checkCastExpr(Ctx, Paren->getSubExpr()))
        return true;
    }

    /* ... and into another CastExpr. */
    if (auto C = dyn_cast<CastExpr>(E)) {
      if (checkCastExpr(Ctx, C->getSubExpr()))
        return true;
    }

    return false;
  }
};

} // namespace clang::tidy::cheri
