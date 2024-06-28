// ==-- PointerSizeAssumptionsChecker.cpp -------------------------*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This checker detects code where the pointer size was checked against
// a constant, but the case of capabilities was not addressed explicitly.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

#include "CHERIUtils.h"

using namespace clang;
using namespace ento;
using namespace cheri;
using namespace ast_matchers;

namespace {

// ID of a node at which the diagnostic would be emitted.
constexpr llvm::StringLiteral WarnAtNode = "if_stmt";
constexpr llvm::StringLiteral ConstNode = "size_const";
constexpr llvm::StringLiteral TypeNode = "sizeof_type";

class PointerSizeAssumptionsChecker : public Checker<check::ASTCodeBody> {
public:
  void checkASTCodeBody(const Decl *D, AnalysisManager &mgr,
                        BugReporter &BR) const;
};

auto matchCheckPtrSize() -> decltype(decl()) {
  // sizeof(void*) ...
  auto PtrSize = sizeOfExpr(has(expr(hasType(pointerType())).bind("ptr")));
  
  // ... == 8
  auto SizeConst = ignoringImplicit(integerLiteral().bind(ConstNode));
  
  // ... == sizeof long_var
  auto SizeofFTy =
      sizeOfExpr(unless(has(expr(hasType(pointerType()))))).bind(TypeNode);

  auto Size = expr(anyOf(SizeConst, SizeofFTy));
  
  auto Check = binaryOperation(isComparisonOperator(), has(PtrSize), has(Size));
  auto PointerSizeCheck = traverse(
      TK_AsIs,
      stmt(ifStmt(hasCondition(Check))).bind(WarnAtNode));

  return decl(forEachDescendant(PointerSizeCheck));
}

static void emitDiagnostics(const Expr *WarnStmt, const Decl *D,
                            BugReporter &BR, AnalysisManager &AM,
                            const PointerSizeAssumptionsChecker *Checker) {
  auto *ADC = AM.getAnalysisDeclContext(D);

  auto Range = WarnStmt->getSourceRange();
  auto Location = PathDiagnosticLocation::createBegin(WarnStmt,
                                                      BR.getSourceManager(),
                                                      ADC);
  std::string Diagnostics;
  llvm::raw_string_ostream OS(Diagnostics);

  ASTContext &ASTCtx = AM.getASTContext();
  OS << "This code may fail to consider the case of "
     << ASTCtx.toBits(getCapabilityTypeSize(ASTCtx)) << "-bit "
     << "pointers";

  BR.EmitBasicReport(ADC->getDecl(), Checker,
                     "Only a limited number of pointer sizes checked",
                     "CHERI portability",
                     OS.str(), Location, Range);
}

void PointerSizeAssumptionsChecker::checkASTCodeBody(const Decl *D,
                                                     AnalysisManager &AM,
                                                     BugReporter &BR) const {
  auto MatcherM = matchCheckPtrSize();

  ASTContext &ASTCtx = AM.getASTContext();
  const unsigned CAP_SIZEOF = getCapabilityTypeSize(ASTCtx).getQuantity();
  auto Matches = match(MatcherM, *D, ASTCtx);
  const IfStmt *WarnStmt = nullptr;
  for (const auto &Match : Matches) {
    unsigned s;
    if (const IntegerLiteral *IL = Match.getNodeAs<IntegerLiteral>(ConstNode)) {
      s = IL->getValue().getZExtValue();
    } else {
      const UnaryExprOrTypeTraitExpr *SizeOfExpr =
          Match.getNodeAs<UnaryExprOrTypeTraitExpr>(TypeNode);
      assert(SizeOfExpr);
      s = AM.getASTContext()
              .getTypeSizeInChars(SizeOfExpr->getTypeOfArgument())
              .getQuantity();
    }
    if (s == CAP_SIZEOF)
      return;
    if (!WarnStmt)
      WarnStmt = Match.getNodeAs<IfStmt>(WarnAtNode);
  }
  if (WarnStmt)
    emitDiagnostics(WarnStmt->getCond(), D, BR, AM, this);
}

} // namespace

void ento::registerPointerSizeAssumptionsChecker(CheckerManager &mgr) {
  mgr.registerChecker<PointerSizeAssumptionsChecker>();
}

bool ento::shouldRegisterPointerSizeAssumptionsChecker(
    const CheckerManager &mgr) {
  return true;
}