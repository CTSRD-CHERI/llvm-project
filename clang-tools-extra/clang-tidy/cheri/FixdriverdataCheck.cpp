//===--- FixdriverdataCheck.cpp - clang-tidy ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FixdriverdataCheck.h"
#include "CheriUtil.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::cheri {

void FixdriverdataCheck::registerMatchers(MatchFinder *Finder) {
  // clang-format off
  Finder->addMatcher(
      fieldDecl(
          anyOf(
              hasName("driver_data"),
              hasName("driver_info")),
                hasType(hasCanonicalType(isInteger())))
          .bind("field"),
      this);
  Finder->addMatcher(
      initListExpr(
          hasType(
              hasCanonicalType(
                  type(
                      recordType(
                          hasDeclaration(
                              recordDecl(
                                  has(
                                      fieldDecl(
                                          anyOf(
                                              hasName("driver_data"),
                                              hasName("driver_info")
                                          )
                                      )
                                  )
                              ).bind("record")
                          )
                      )
                  )
              )
          )
      ).bind("init"),
      this
  );
  // clang-format on
}

/* Check if a field is a driver data field. */
bool FixdriverdataCheck::isDriverData(ASTContext *Ctx, const FieldDecl *Field) {
  /* The field must have the correct name. */
  if (Field->getName() != StringRef("driver_data") &&
      Field->getName() != StringRef("driver_info"))
    return false;

  /* If the field is of pointer type we don't touch it. */
  const auto *FT = Field->getType().getTypePtr();
  if (FT->isPointerType())
    return false;

  /* If the field cannot possibly contain a pointer we ignore it. */
  if (Util::isSafeNonPtrType(Ctx, FT))
    return false;

  /* Otherwise it is a candiate for a driver data field. */
  return true;
}

void FixdriverdataCheck::check(const MatchFinder::MatchResult &Result) {
  auto *Ctx = Result.Context;
  auto *SM = Result.SourceManager;
  const auto *Field = Result.Nodes.getNodeAs<FieldDecl>("field");

  if (Field) {
    /* Ignore the FieldDecl if it is not a driver data field. */
    if (!isDriverData(Ctx, Field))
      return;

    /*
     * If the field is already of capability type there is nothring more
     * to do for the declaration.
     */
    if (Util::isCapability(Ctx, Field->getType().getTypePtr()))
      return;

    /* Suggest a fixup. */
    auto Begin = Field->getTypeSpecStartLoc();
    auto End = Field->getTypeSpecEndLoc();

    diag(Field->getLocation(),
         "CHERI: driver data field should have type `uintptr_t`")
        << FixItHint::CreateReplacement(
               CharSourceRange::getTokenRange(Begin, End), "uintptr_t");
  } else {
    const auto *Record = Result.Nodes.getNodeAs<RecordDecl>("record");
    const auto *Init = Result.Nodes.getNodeAs<InitListExpr>("init");

    if (Init->isSyntacticForm())
      Init = Init->getSemanticForm();
    if (!Init)
      return;

    for (const auto F : Record->fields()) {
      /* Skip non driver data fields. */
      if (!isDriverData(Ctx, F))
        continue;

      /* Skip if field is not initialized. */
      if (F->getFieldIndex() >= Init->getNumInits())
        continue;

      /* Skip implicit initializers. */
      const auto *I = Init->getInit(F->getFieldIndex());
      if (!I || isa<ImplicitValueInitExpr>(I))
        continue;

      /* Remove parens and implicit casts. */
      while (1) {
        if (const auto *Paren = dyn_cast<ParenExpr>(I)) {
          I = Paren->getSubExpr();
          continue;
        }
        if (const auto *Cast = dyn_cast<ImplicitCastExpr>(I)) {
          I = Cast->getSubExpr();
          continue;
        }
        break;
      }

      /* Nothing to do if the init expressoin already is a capability. */
      if (Util::isCapability(Ctx, I))
        continue;

      /* If it is an explicit cast, recommend a cast to uintptr_t. */
      if (const auto *CC = dyn_cast<CStyleCastExpr>(I)) {
        /* Ignore forced casts. */
        if (Util::isForced(SM, CC))
          continue;
        diag(CC->getExprLoc(), "CHERI: driver data field should be "
                               "initialized with type 'uintptr_t'")
            << FixItHint::CreateReplacement(
                   SourceRange(CC->getLParenLoc(), CC->getRParenLoc()),
                   "(uintptr_t)");
      }
    }
  }
}

} // namespace clang::tidy::cheri
