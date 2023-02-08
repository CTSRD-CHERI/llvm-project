//===-- ProvenanceSourceChecker.cpp - Provenance Source Checker -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines checker that detects binary arithmetic expressions with
// capability type operands where provenance source is ambiguous.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include <clang/StaticAnalyzer/Core/BugReporter/BugReporter.h>
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {
class ProvenanceSourceChecker : public Checker<check::PostStmt<CastExpr>,
                                               check::PreStmt<CastExpr>,
                                               check::PostStmt<BinaryOperator>,
                                               check::DeadSymbols> {
  std::unique_ptr<BugType> AmbiguousProvenanceBinOpBugType;
  std::unique_ptr<BugType> AmbiguousProvenancePtrBugType;
  std::unique_ptr<BugType> InvalidCapPtrBugType;
  std::unique_ptr<BugType> PtrdiffAsIntCapBugType;

private:
  class InvalidCapBugVisitor : public BugReporterVisitor {
  public:
    InvalidCapBugVisitor(SymbolRef SR) : Sym(SR) {}

    void Profile(llvm::FoldingSetNodeID &ID) const override {
      static int X = 0;
      ID.AddPointer(&X);
      ID.AddPointer(Sym);
    }

    PathDiagnosticPieceRef VisitNode(const ExplodedNode *N,
                                     BugReporterContext &BRC,
                                     PathSensitiveBugReport &BR) override;
  private:
    SymbolRef Sym;
  };

  class Ptr2IntBugVisitor : public BugReporterVisitor {
  public:
    Ptr2IntBugVisitor(const MemRegion *R) : Reg(R) {}

    void Profile(llvm::FoldingSetNodeID &ID) const override {
      static int X = 0;
      ID.AddPointer(&X);
      ID.AddPointer(Reg);
    }

    PathDiagnosticPieceRef VisitNode(const ExplodedNode *N,
                                     BugReporterContext &BRC,
                                     PathSensitiveBugReport &BR) override;
  private:
    const MemRegion *Reg;
  };

public:
  ProvenanceSourceChecker();

  void checkPostStmt(const CastExpr *CE, CheckerContext &C) const;
  void checkPreStmt(const CastExpr *CE, CheckerContext &C) const;
  void checkPostStmt(const BinaryOperator *BO, CheckerContext &C) const;
  void checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const;

private:
  // Utility
  ExplodedNode *emitAmbiguousProvenanceWarn(const BinaryOperator *BO,
                                            CheckerContext &C, bool LHSIsAddr,
                                            bool LHSIsNullDerived,
                                            bool RHSIsAddr,
                                            bool RHSIsNullDerived) const;

  ExplodedNode *emitPtrdiffAsIntCapWarn(const BinaryOperator *BO,
                                        CheckerContext &C) const;

  static void propagateProvenanceInfoForBinOp(ExplodedNode *N,
                                              const BinaryOperator *BO,
                                              CheckerContext &C,
                                              bool IsInvalidCap);
};
} // namespace

REGISTER_SET_WITH_PROGRAMSTATE(InvalidCap, SymbolRef)
REGISTER_SET_WITH_PROGRAMSTATE(AmbiguousProvenanceSym, SymbolRef)
REGISTER_SET_WITH_PROGRAMSTATE(AmbiguousProvenanceReg, const MemRegion *)
REGISTER_TRAIT_WITH_PROGRAMSTATE(Ptr2IntCapId, unsigned)

ProvenanceSourceChecker::ProvenanceSourceChecker() {
  AmbiguousProvenanceBinOpBugType.reset(
      new BugType(this,
                  "Binary operation with ambiguous provenance",
                  "CHERI portability"));
  AmbiguousProvenancePtrBugType.reset(
      new BugType(this,
                  "Capability with ambiguous provenance used as pointer",
                  "CHERI portability"));
  InvalidCapPtrBugType.reset(
      new BugType(this,
                  "Invalid capability used as pointer",
                  "CHERI portability"));
  PtrdiffAsIntCapBugType.reset(
      new BugType(this,
                  "Pointer difference as capability",
                  "CHERI portability"));
}

static bool isIntegerToIntCapCast(const CastExpr *CE) {
  if (CE->getCastKind() != CK_IntegralCast)
    return false;
  if (!CE->getType()->isIntCapType() ||
      CE->getSubExpr()->getType()->isIntCapType())
    return false;
  return true;
}

static bool isPointerToIntCapCast(const CastExpr *CE) {
  if (CE->getCastKind() != clang::CK_PointerToIntegral)
    return false;
  if (!CE->getType()->isIntCapType())
    return false;
  return true;
}

void ProvenanceSourceChecker::checkPostStmt(const CastExpr *CE,
                                            CheckerContext &C) const {
  if (isIntegerToIntCapCast(CE)) {
    SymbolRef IntCapSym = C.getSVal(CE).getAsSymbol();
    if (!IntCapSym)
      return;

    if (C.getSVal(CE).getAsLocSymbol())
      return; // skip locAsInteger

    ProgramStateRef State = C.getState();
    State = State->add<InvalidCap>(IntCapSym);
    C.addTransition(State);
  } else if (isPointerToIntCapCast(CE)) {
    // Prev node may be reclaimed as "uninteresting", in this case we will not
    // be able to create path diagnostic for it; therefore we modify the state,
    // i.e. create the node that definitely will not be deleted
    ProgramStateRef const State = C.getState();
    C.addTransition(State->set<Ptr2IntCapId>(State->get<Ptr2IntCapId>() + 1));
  }
}

static bool hasAmbiguousProvenance(ProgramStateRef State, const SVal &Val) {
  if (SymbolRef Sym = Val.getAsSymbol())
    return State->contains<AmbiguousProvenanceSym>(Sym);

  if (const MemRegion *Reg = Val.getAsRegion())
    return State->contains<AmbiguousProvenanceReg>(Reg);

  return false;
}

static bool hasNoProvenance(ProgramStateRef State, const SVal &Val) {
  if (Val.isConstant())
    return true;

  if (const auto &LocAsInt = Val.getAs<nonloc::LocAsInteger>())
    return !LocAsInt->hasProvenance();

  SymbolRef Sym = Val.getAsSymbol();
  if (Sym && State->contains<InvalidCap>(Sym))
    return true;
  return false;
}

static bool isAddress(const SVal &Val) {
  if (!Val.getAsLocSymbol(true))
    return false;

  if (const auto &LocAsInt = Val.getAs<nonloc::LocAsInteger>())
    return LocAsInt->hasProvenance();

  return true;
}

static bool isIntToVoidPtrCast(const CastExpr *CE) {
  if (!CE->getType()->isVoidPointerType())
    return false;

  const Expr *Src = CE->getSubExpr();
  if (!Src->getType()->isIntCapType())
    return false;

  if (auto *CE2 = dyn_cast<CastExpr>(Src)) {
    const QualType &T = CE2->getSubExpr()->getType();
    return T->isIntegerType() && !T->isIntCapType();
  }

  return false;
}

// Report intcap with ambiguous or NULL-derived provenance cast to pointer
void ProvenanceSourceChecker::checkPreStmt(const CastExpr *CE,
                                           CheckerContext &C) const {
  if (CE->getCastKind() != clang::CK_IntegralToPointer)
    return;

  ProgramStateRef const State = C.getState();
  const SVal &SrcVal = C.getSVal(CE->getSubExpr());

  std::unique_ptr<PathSensitiveBugReport> R;
  if (hasAmbiguousProvenance(State, SrcVal)) {
    ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode)
      return;
    R = std::make_unique<PathSensitiveBugReport>(
        *AmbiguousProvenancePtrBugType,
        "Capability with ambiguous provenance is used as pointer", ErrNode);
  } else if (hasNoProvenance(State, SrcVal) && !SrcVal.isConstant()
             && !isIntToVoidPtrCast(CE)) {
    ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode)
      return;
    R = std::make_unique<PathSensitiveBugReport>(
        *InvalidCapPtrBugType, "Invalid capability is used as pointer",
        ErrNode);
    if (SymbolRef S = SrcVal.getAsSymbol())
      R->addVisitor(std::make_unique<InvalidCapBugVisitor>(S));
  } else
    return;

  R->markInteresting(SrcVal);
  R->addRange(CE->getSourceRange());
  C.emitReport(std::move(R));
}

static bool justConverted2IntCap(Expr *E, const ASTContext &Ctx) {
  assert(E->getType()->isCHERICapabilityType(Ctx, true));
  if (auto *CE = dyn_cast<CastExpr>(E)) {
    const QualType OrigType = CE->getSubExpr()->getType();
    if (!OrigType->isCHERICapabilityType(Ctx, true))
      return true;
  }
  return false;
}

ExplodedNode *ProvenanceSourceChecker::emitPtrdiffAsIntCapWarn(
    const BinaryOperator *BO, CheckerContext &C) const {
  // Generate the report.
  ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
  if (!ErrNode)
    return nullptr;
  auto R = std::make_unique<PathSensitiveBugReport>(
      *PtrdiffAsIntCapBugType, "Pointer difference as capability",
      ErrNode);
  R->addRange(BO->getSourceRange());

  const SVal &LHSVal = C.getSVal(BO->getLHS());
  R->markInteresting(LHSVal);
  if (const MemRegion *Reg = LHSVal.getAsRegion())
    R->addVisitor(std::make_unique<Ptr2IntBugVisitor>(Reg));

  const SVal &RHSVal = C.getSVal(BO->getRHS());
  R->markInteresting(RHSVal);
  if (const MemRegion *Reg = RHSVal.getAsRegion())
    R->addVisitor(std::make_unique<Ptr2IntBugVisitor>(Reg));

  C.emitReport(std::move(R));
  return ErrNode;
}

ExplodedNode *ProvenanceSourceChecker::emitAmbiguousProvenanceWarn(
    const BinaryOperator *BO, CheckerContext &C,
    bool LHSIsAddr, bool LHSIsNullDerived,
    bool RHSIsAddr, bool RHSIsNullDerived) const {
  SmallString<350> ErrorMessage;
  llvm::raw_svector_ostream OS(ErrorMessage);

  const QualType &T = BO->getType();
  bool const IsUnsigned = T->isUnsignedIntegerType();

  OS << "Result of '"<< BO->getOpcodeStr() << "' on capability type '";
  T.print(OS, PrintingPolicy(C.getASTContext().getLangOpts()));
  OS << "'; it is unclear which side should be used as the source "
        "of provenance; consider indicating the provenance-carrying argument "
        "explicitly by casting the other argument to '"
     << (IsUnsigned ? "size_t" : "ptrdiff_t") << "'. ";

  OS << "Note: along this path, ";
  if (LHSIsAddr && RHSIsAddr)
    OS << "LHS and RHS were derived from pointers";
  else if (LHSIsNullDerived && RHSIsNullDerived) {
    OS << "LHS and RHS were derived from NULL";
  } else {
    if (LHSIsAddr)
      OS << "LHS was derived from pointer";
    if (LHSIsNullDerived)
      OS << "LHS was derived from NULL";
    if ((LHSIsAddr || LHSIsNullDerived) && (RHSIsAddr || RHSIsNullDerived))
      OS << ", ";
    if (RHSIsAddr)
      OS << "RHS was derived from pointer";
    if (RHSIsNullDerived)
      OS << "RHS was derived from NULL";
  }

  // Generate the report.
  ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
  if (!ErrNode)
    return nullptr;
  auto R = std::make_unique<PathSensitiveBugReport>(
      *AmbiguousProvenanceBinOpBugType, ErrorMessage, ErrNode);
  R->addRange(BO->getSourceRange());

  const SVal &LHSVal = C.getSVal(BO->getLHS());
  R->markInteresting(LHSVal);
  if (SymbolRef LS = LHSVal.getAsSymbol())
    R->addVisitor(std::make_unique<InvalidCapBugVisitor>(LS));
  if (const MemRegion *Reg = LHSVal.getAsRegion())
    R->addVisitor(std::make_unique<Ptr2IntBugVisitor>(Reg));

  const SVal &RHSVal = C.getSVal(BO->getRHS());
  R->markInteresting(RHSVal);
  if (SymbolRef RS = RHSVal.getAsSymbol())
    R->addVisitor(std::make_unique<InvalidCapBugVisitor>(RS));
  if (const MemRegion *Reg = RHSVal.getAsRegion())
    R->addVisitor(std::make_unique<Ptr2IntBugVisitor>(Reg));

  C.emitReport(std::move(R));
  return ErrNode;
}

void ProvenanceSourceChecker::propagateProvenanceInfoForBinOp(
    ExplodedNode *N, const BinaryOperator *BO, CheckerContext &C,
    bool IsInvalidCap) {

  ProgramStateRef State = N->getState();
  SVal ResVal = C.getSVal(BO);
  if (ResVal.isUnknown()) {
    const LocationContext *LCtx = C.getLocationContext();
    ResVal = C.getSValBuilder().conjureSymbolVal(
        nullptr, BO, LCtx, BO->getType(), C.blockCount());
    State = State->BindExpr(BO, LCtx, ResVal);
  }

  if (SymbolRef ResSym = ResVal.getAsSymbol())
    State = IsInvalidCap ? State->add<InvalidCap>(ResSym)
                         : State->add<AmbiguousProvenanceSym>(ResSym);
  else if (const MemRegion *Reg = ResVal.getAsRegion())
    State = State->add<AmbiguousProvenanceReg>(Reg);
  else
    return; // no result to propagate to

  const NoteTag *BinOpTag =
      IsInvalidCap ? nullptr // note will be added in BugVisitor
                   : C.getNoteTag("Binary operator has ambiguous provenance");

  C.addTransition(State, N, BinOpTag);
}

// Report intcap binary expressions with ambiguous provenance,
// store them to report again if used as pointer
void ProvenanceSourceChecker::checkPostStmt(const BinaryOperator *BO,
                                            CheckerContext &C) const {
  BinaryOperatorKind const OpCode = BO->getOpcode();
  if (!(BinaryOperator::isAdditiveOp(OpCode)
        || BinaryOperator::isMultiplicativeOp(OpCode)
        || BinaryOperator::isBitwiseOp(OpCode)))
    return;
  bool const IsSub = OpCode == clang::BO_Sub || OpCode == clang::BO_SubAssign;

  Expr *LHS = BO->getLHS();
  Expr *RHS = BO->getRHS();
  if (!LHS->getType()->isIntCapType() || !RHS->getType()->isIntCapType())
    return;

  ProgramStateRef const State = C.getState();

  const SVal &LHSVal = C.getSVal(LHS);
  const SVal &RHSVal = C.getSVal(RHS);

  bool const LHSIsAddr = isAddress(LHSVal);
  bool const LHSIsNullDerived = !LHSIsAddr && hasNoProvenance(State, LHSVal);
  bool const RHSIsAddr = isAddress(RHSVal);
  bool const RHSIsNullDerived = !RHSIsAddr && hasNoProvenance(State, RHSVal);
  if (!LHSIsAddr && !LHSIsNullDerived && !RHSIsAddr && !RHSIsNullDerived)
    return;

  // Operands that were converted to intcap for this binaryOp are not used to
  // derive the provenance of the result
  // FIXME: explicit attr::CHERINoProvenance
  bool const LHSActiveProv = !justConverted2IntCap(LHS, C.getASTContext());
  bool const RHSActiveProv = !justConverted2IntCap(RHS, C.getASTContext());

  ExplodedNode *N;
  bool InvalidCap;
  if (LHSActiveProv && RHSActiveProv && !IsSub) {
    N = emitAmbiguousProvenanceWarn(BO, C, LHSIsAddr, LHSIsNullDerived,
                                      RHSIsAddr, RHSIsNullDerived);
    if (!N)
      N = C.getPredecessor();
    InvalidCap = false;
  } else if (IsSub && LHSIsAddr && RHSIsAddr) {
    N = emitPtrdiffAsIntCapWarn(BO, C);
    if (!N)
      N = C.getPredecessor();
    InvalidCap = true;
  } else if (LHSIsNullDerived && (RHSIsNullDerived || IsSub)) {
    N = C.getPredecessor();
    InvalidCap = true;
  } else
    return;

  // Propagate info for result
  propagateProvenanceInfoForBinOp(N, BO, C, InvalidCap);
}

void ProvenanceSourceChecker::checkDeadSymbols(SymbolReaper &SymReaper,
                                               CheckerContext &C) const {
  ProgramStateRef State = C.getState();

  bool Removed = false;
  const InvalidCapTy &Set = State->get<InvalidCap>();
  for (const auto &Sym : Set) {
    if (!SymReaper.isDead(Sym))
      continue;
    State = State->remove<InvalidCap>(Sym);
    Removed = true;
  }

  const AmbiguousProvenanceSymTy &Set2 = State->get<AmbiguousProvenanceSym>();
  for (const auto &Sym : Set2) {
    if (!SymReaper.isDead(Sym))
      continue;
    State = State->remove<AmbiguousProvenanceSym>(Sym);
    Removed = true;
  }

  if (Removed)
    C.addTransition(State);
}

static void describeCast(raw_ostream &OS, const CastExpr *CE,
                         const LangOptions &LangOpts) {
  OS << (dyn_cast<ImplicitCastExpr>(CE) ? "implicit" : "explicit");
  OS << " cast from '";
  CE->getSubExpr()->getType().print(OS, PrintingPolicy(LangOpts));
  OS << "' to '";
  CE->getType().print(OS, PrintingPolicy(LangOpts));
  OS << "'";
}

PathDiagnosticPieceRef
ProvenanceSourceChecker::InvalidCapBugVisitor::VisitNode(
    const ExplodedNode *N, BugReporterContext &BRC,
    PathSensitiveBugReport &BR) {

  const Stmt *S = N->getStmtForDiagnostics();
  if (!S)
    return nullptr;

  SmallString<256> Buf;
  llvm::raw_svector_ostream OS(Buf);

  if (const CastExpr *CE = dyn_cast<CastExpr>(S)) {
    if (!isIntegerToIntCapCast(CE))
      return nullptr;

    if (Sym != N->getSVal(CE).getAsSymbol())
      return nullptr;

    if (!N->getState()->contains<InvalidCap>(Sym))
      return nullptr;

    OS << "NULL-derived capability: ";
    describeCast(OS, CE, BRC.getASTContext().getLangOpts());
  } else if (const BinaryOperator *BO = dyn_cast<BinaryOperator>(S)) {
    BinaryOperatorKind const OpCode = BO->getOpcode();
    if (!(BinaryOperator::isAdditiveOp(OpCode)
          || BinaryOperator::isMultiplicativeOp(OpCode)
          || BinaryOperator::isBitwiseOp(OpCode)))
      return nullptr;
    bool const IsSub = OpCode == clang::BO_Sub || OpCode == clang::BO_SubAssign;

    if (!BO->getType()->isIntCapType() || Sym != N->getSVal(BO).getAsSymbol())
      return nullptr;

    if (!N->getState()->contains<InvalidCap>(Sym))
      return nullptr;

    OS << "Result of '" << BO->getOpcodeStr() << "'";
    if (hasNoProvenance(N->getState(), N->getSVal(BO->getLHS())) &&
        (hasNoProvenance(N->getState(), N->getSVal(BO->getRHS())) || IsSub))
      OS << " is a NULL-derived capability";
    else
      OS << " is an invalid capability";
  } else
    return nullptr;

  // Generate the extra diagnostic.
  PathDiagnosticLocation const Pos(S, BRC.getSourceManager(),
                                   N->getLocationContext());
  return std::make_shared<PathDiagnosticEventPiece>(Pos, OS.str(), true);
}

PathDiagnosticPieceRef
ProvenanceSourceChecker::Ptr2IntBugVisitor::VisitNode(
    const ExplodedNode *N, BugReporterContext &BRC,
    PathSensitiveBugReport &BR) {

  const Stmt *S = N->getStmtForDiagnostics();
  if (!S)
    return nullptr;

  const CastExpr *CE = dyn_cast<CastExpr>(S);
  if (!CE || !isPointerToIntCapCast(CE))
    return nullptr;

  if (Reg != N->getSVal(CE).getAsRegion())
    return nullptr;

  SmallString<256> Buf;
  llvm::raw_svector_ostream OS(Buf);
  OS << "Capability derived from pointer: ";
  describeCast(OS, CE, BRC.getASTContext().getLangOpts());

  // Generate the extra diagnostic.
  PathDiagnosticLocation const Pos(S, BRC.getSourceManager(),
                                   N->getLocationContext());
  return std::make_shared<PathDiagnosticEventPiece>(Pos, OS.str(), true);
}

void ento::registerProvenanceSourceChecker(CheckerManager &mgr) {
  mgr.registerChecker<ProvenanceSourceChecker>();
}

bool ento::shouldRegisterProvenanceSourceChecker(const CheckerManager &Mgr) {
  return true;
}
