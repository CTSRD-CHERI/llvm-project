// ==-- PointerSizeAssumptionsChecker.cpp -------------------------*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This checker detects record fields that will not have precise bounds when
// compiled with
//      -cheri-bounds=subobject-safe
// due to big size and underaligned offset, as narrowed capability will not
// be representable
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
#include "llvm/Support/Morello.h"

using namespace clang;
using namespace ento;

namespace {
class SubObjectRepresentabilityChecker
    : public Checker<check::ASTDecl<RecordDecl>> {
public:
  void checkASTDecl(const RecordDecl *R, AnalysisManager& mgr,
                    BugReporter &BR) const;
};

}

void SubObjectRepresentabilityChecker::checkASTDecl(const RecordDecl *R,
                                                    AnalysisManager &mgr,
                                                    BugReporter &BR) const {
  if (!R->isCompleteDefinition())
    return;

  if (!R->getLocation().isValid())
    return;

  /*
  SrcMgr::CharacteristicKind Kind =
      BR.getSourceManager().getFileCharacteristic(Location);
  // Ignore records in system headers
  if (Kind != SrcMgr::C_User)
    return;
  */
  
  for (FieldDecl *D : R->fields()) {
    QualType T = D->getType();

    ASTContext &ASTCtx = BR.getContext();
    uint64_t Offset = ASTCtx.getFieldOffset(D)/8;
    if (Offset > 0) {
      uint64_t Size = ASTCtx.getTypeSize(T)/8;
      uint64_t ReqAlign = llvm::getMorelloRequiredAlignment(Size);
      if (1 << llvm::countTrailingZeros(Offset) < ReqAlign) {
        /* Emit warning */
        SmallString<1024> Err;
        llvm::raw_svector_ostream OS(Err);
        const PrintingPolicy &PP = ASTCtx.getPrintingPolicy();
        OS << "Field '";
        D->getNameForDiagnostic(OS, PP, false);
        OS << "' of type '" << T.getAsString(PP) << "'";
        OS << " (size " << Size << ")";
        OS << " requires " << ReqAlign << " byte alignment for precise bounds;";
        OS << " field offset is " << Offset;
        
        // Note that this will fire for every translation unit that uses this
        // class.  This is suboptimal, but at least scan-build will merge
        // duplicate HTML reports.
        PathDiagnosticLocation L =
            PathDiagnosticLocation::createBegin(D, BR.getSourceManager());
        BR.EmitBasicReport(R, this, "Field with imprecise subobject bounds",
                           "CHERI portability", OS.str(), L);
      }
    }
  }
}

void ento::registerSubObjectRepresentabilityChecker(CheckerManager &mgr) {
  mgr.registerChecker<SubObjectRepresentabilityChecker>();
}

bool ento::shouldRegisterSubObjectRepresentabilityChecker(
    const CheckerManager &mgr) {
  return true;
}
