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

#include "CHERIUtils.h"
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
#include "llvm/CHERI/cheri-compressed-cap/cheri_compressed_cap.h"


using namespace clang;
using namespace ento;

namespace {
class SubObjectRepresentabilityChecker
    : public Checker<check::ASTDecl<RecordDecl>> {
  BugType BT_1{this, "Field with imprecise subobject bounds",
               "CHERI portability"};

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
      uint64_t CurAlign = 1 << llvm::countTrailingZeros(Offset);
      if (CurAlign < ReqAlign) {
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
        OS << " (aligned to " << CurAlign << ");";

        //FIXME: other targets
        using Handler = CompressedCap128;
        uint64_t ParentSize = ASTCtx.getTypeSize(R->getTypeForDecl())/8;
        Handler::addr_t InitLength = Handler ::representable_length(ParentSize);
        Handler::cap_t MockCap = Handler::make_max_perms_cap(0, 0, InitLength);
        bool exact = Handler::setbounds(&MockCap, Offset, Offset + Size);
        assert(!exact);
        auto Base = MockCap.base();
        Handler::addr_t Top = MockCap.top();
        OS << " Current bounds: " << Base << "-" << Top;

        PathDiagnosticLocation L =
            PathDiagnosticLocation::createBegin(D, BR.getSourceManager());
        auto Report =
            std::make_unique<BasicBugReport>(BT_1, OS.str(), L);
        Report->setDeclWithIssue(D);
        Report->addRange(D->getSourceRange());


        auto FI = R->field_begin();
        while (FI != R->field_end() &&
               ASTCtx.getFieldOffset(*FI)/8
                + ASTCtx.getTypeSize(FI->getType())/8 <= Base)
          FI++;

        bool Before = true;
        while (FI != R->field_end() && ASTCtx.getFieldOffset(*FI)/8 < Top) {
          if (*FI != D) {
            SmallString<1024> Note;
            llvm::raw_svector_ostream OS2(Note);

            uint64_t CurFieldOffset = ASTCtx.getFieldOffset(*FI)/8;
            uint64_t CurFieldSize = ASTCtx.getTypeSize(FI->getType())/8;
            uint64_t BytesExposed =
                Before ? std::min(CurFieldSize,
                                  CurFieldOffset + CurFieldSize - Base)
                       : std::min(CurFieldSize, Top - CurFieldOffset);
            OS2 << BytesExposed << "/" << CurFieldSize << " bytes exposed";
            if (cheri::hasCapability(FI->getType(), ASTCtx))
              OS2 << " (may expose capability!)";

            PathDiagnosticLocation LN =
                PathDiagnosticLocation::createBegin(*FI, BR.getSourceManager());
            Report->addNote(OS2.str(), LN);
          } else
            Before = false;
          FI++;
        }

        // Note that this will fire for every translation unit that uses this
        // class.  This is suboptimal, but at least scan-build will merge
        // duplicate HTML reports.
        BR.emitReport(std::move(Report));
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
