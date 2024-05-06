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
#include "clang/AST/StmtVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/CHERI/cheri-compressed-cap/cheri_compressed_cap.h"

using namespace clang;
using namespace ento;

namespace {
class SubObjectRepresentabilityChecker
    : public Checker<check::ASTDecl<RecordDecl>, check::ASTCodeBody> {
  BugType BT_1{this, "Field with imprecise subobject bounds",
               "CHERI portability"};
  BugType BT_2{this, "Address taken of a field with imprecise subobject bounds",
               "CHERI portability"};

public:
  void checkASTDecl(const RecordDecl *R, AnalysisManager& mgr,
                    BugReporter &BR) const;
  void checkASTCodeBody(const Decl *D, AnalysisManager &mgr,
                        BugReporter &BR) const;
};

} //namespace

namespace {

std::unique_ptr<BasicBugReport>
reportExposedFields(const FieldDecl *D, ASTContext &ASTCtx, BugReporter &BR,
                    uint64_t Base, uint64_t Top,
                    std::unique_ptr<BasicBugReport> Report) {
  const RecordDecl *Parent = D->getParent();
  auto FI = Parent->field_begin();
  while (FI != Parent->field_end() &&
         ASTCtx.getFieldOffset(*FI) / 8 +
                 ASTCtx.getTypeSize(FI->getType()) / 8 <=
             Base)
    FI++;

  bool Before = true;
  while (FI != Parent->field_end() &&
         ASTCtx.getFieldOffset(*FI) / 8 < Top) {
    if (*FI != D) {
      SmallString<1024> Note;
      llvm::raw_svector_ostream OS2(Note);

      uint64_t CurFieldOffset = ASTCtx.getFieldOffset(*FI) / 8;
      uint64_t CurFieldSize = ASTCtx.getTypeSize(FI->getType()) / 8;
      uint64_t BytesExposed =
          Before
              ? std::min(CurFieldSize, CurFieldOffset + CurFieldSize - Base)
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
  return Report;
}

template <typename Handler>
typename Handler::cap_t getBoundedCap(uint64_t ParentSize, uint64_t Offset,
                                      uint64_t Size) {
  typename Handler::addr_t InitLength =
      Handler::representable_length(ParentSize);
  typename Handler::cap_t MockCap =
      Handler::make_max_perms_cap(0, Offset, InitLength);
  bool exact = Handler::setbounds(&MockCap, Size);
  assert(!exact);
  return MockCap;
}

template<typename Handler>
uint64_t getRepresentableAlignment(uint64_t Size) {
  return ~Handler::representable_mask(Size) + 1;
}

template <typename Handler>
std::unique_ptr<BugReport> checkFieldImpl(const FieldDecl *D,
                                          BugReporter &BR,
                                          const BugType &BT) {
  QualType T = D->getType();

  ASTContext &ASTCtx = BR.getContext();
  uint64_t Offset = ASTCtx.getFieldOffset(D) / 8;
  if (Offset > 0) {
    uint64_t Size = ASTCtx.getTypeSize(T) / 8;
    uint64_t ReqAlign = getRepresentableAlignment<Handler>(Size);
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

      const RecordDecl *Parent = D->getParent();
      uint64_t ParentSize = ASTCtx.getTypeSize(Parent->getTypeForDecl()) / 8;
      typename Handler::cap_t MockCap =
          getBoundedCap<Handler>(ParentSize, Offset, Size);
      uint64_t Base = MockCap.base();
      uint64_t Top = MockCap.top();
      OS << " Current bounds: " << Base << "-" << Top;

      // Note that this will fire for every translation unit that uses this
      // class.  This is suboptimal, but at least scan-build will merge
      // duplicate HTML reports.
      PathDiagnosticLocation L =
          PathDiagnosticLocation::createBegin(D, BR.getSourceManager());
      auto Report = std::make_unique<BasicBugReport>(BT, OS.str(), L);
      Report->setDeclWithIssue(D);
      Report->addRange(D->getSourceRange());

      Report = reportExposedFields(D, ASTCtx, BR, Base, Top, std::move(Report));

      return Report;
    }
  }

  return nullptr;
}

std::unique_ptr<BugReport> checkField(const FieldDecl *D,
                                      BugReporter &BR,
                                      const BugType &BT) {
  // TODO: other targets
  return checkFieldImpl<CompressedCap128m>(D, BR, BT);
}

bool supportedTarget(const ASTContext &C) {
  const TargetInfo &TI = C.getTargetInfo();
  return TI.areAllPointersCapabilities()
         && TI.getTriple().isAArch64(); // morello
}

} // namespace


void SubObjectRepresentabilityChecker::checkASTDecl(const RecordDecl *R,
                                                    AnalysisManager &mgr,
                                                    BugReporter &BR) const {
  if (!supportedTarget(mgr.getASTContext()))
    return;

  if (!R->isCompleteDefinition() || R->isDependentType())
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
    auto Report = checkField(D, BR, BT_1);
    if (Report)
      BR.emitReport(std::move(Report));
  }
}

void SubObjectRepresentabilityChecker::checkASTCodeBody(const Decl *D,
                                                        AnalysisManager &mgr,
                                                        BugReporter &BR) const {
  if (!supportedTarget(mgr.getASTContext()))
    return;

  using namespace ast_matchers;
  auto Member = memberExpr().bind("member");
  auto Decay =
      castExpr(hasCastKind(CK_ArrayToPointerDecay), has(Member)).bind("decay");
  auto Addr = unaryOperator(hasOperatorName("&"), has(Member)).bind("addr");

  auto PointerSizeCheck = traverse(TK_AsIs, stmt(anyOf(Decay, Addr)));

  auto Matcher = decl(forEachDescendant(PointerSizeCheck));

  auto Matches = match(Matcher, *D, BR.getContext());
  for (const auto &Match : Matches) {
    if (const CastExpr *CE = Match.getNodeAs<CastExpr>("decay")) {
      if (const MemberExpr *ME = Match.getNodeAs<MemberExpr>("member")) {
        ValueDecl *VD = ME->getMemberDecl();
        if (FieldDecl *FD = dyn_cast<FieldDecl>(VD)) {
          auto Report = checkField(FD, BR, BT_2);
          if (Report) {
            PathDiagnosticLocation LN = PathDiagnosticLocation::createBegin(
                CE, BR.getSourceManager(), mgr.getAnalysisDeclContext(D));
            Report->addNote("Array to pointer decay", LN);
            BR.emitReport(std::move(Report));
          }
        }
      }
    } else if (const UnaryOperator *UO =
                   Match.getNodeAs<UnaryOperator>("addr")) {
      if (const MemberExpr *ME = Match.getNodeAs<MemberExpr>("member")) {
        ValueDecl *VD = ME->getMemberDecl();
        if (FieldDecl *FD = dyn_cast<FieldDecl>(VD)) {
          auto Report = checkField(FD, BR, BT_2);
          if (Report) {
            PathDiagnosticLocation LN = PathDiagnosticLocation::createBegin(
                UO, BR.getSourceManager(), mgr.getAnalysisDeclContext(D));
            Report->addNote("Address of a field taken", LN);
            BR.emitReport(std::move(Report));
          }
        }
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
