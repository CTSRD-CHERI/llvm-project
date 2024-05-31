//===-- AllocationChecker.cpp - Allocation Checker -*- C++ -*--------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// TODO: description
//
//===----------------------------------------------------------------------===//
#include "CHERIUtils.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include <clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h>


using namespace clang;
using namespace ento;
using namespace cheri;


struct EscapeInfo {
  PointerEscapeKind Kind;
  EscapeInfo(PointerEscapeKind K) : Kind(K) {};
  bool operator==(const EscapeInfo &X) const {
    return Kind == X.Kind;
  }
  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddInteger(Kind);
  }
};
using EscapePair = std::pair<const MemRegion*, EscapeInfo>;

namespace {
class AllocationChecker : public Checker<check::PostStmt<CastExpr>,
                                         check::PreCall,
                                         check::PostCall,
                                         check::Bind,
                                         check::EndFunction> {
  BugType BT_Default{this, "Allocation partitioning", "CHERI portability"};
  BugType BT_UnknownReg{this, "Unknown allocation partitioning",
                      "CHERI portability"};

  const CallDescriptionSet IgnoreFnSet {
      {"free", 1},
  };

  const CallDescriptionSet CheriBoundsFnSet {
      {"cheri_bounds_set", 2},
      {"cheri_bounds_set_exact", 2},
  };


  class AllocPartitionBugVisitor : public BugReporterVisitor {
  public:
    AllocPartitionBugVisitor(const MemRegion *P, const MemRegion *A)
        : PrevAlloc(P), SubAlloc(A) {}

    void Profile(llvm::FoldingSetNodeID &ID) const override {
      static int X = 0;
      ID.AddPointer(&X);
      ID.AddPointer(PrevAlloc);
      ID.AddPointer(SubAlloc);

    }

    PathDiagnosticPieceRef VisitNode(const ExplodedNode *N,
                                     BugReporterContext &BRC,
                                     PathSensitiveBugReport &BR) override;

  private:
    const MemRegion *PrevAlloc;
    const MemRegion *SubAlloc;
    bool PrevReported = false;
  };

public:
  void checkPostStmt(const CastExpr *CE, CheckerContext &C) const;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  void checkBind(SVal L, SVal V, const Stmt *S, CheckerContext &C) const;
  void checkEndFunction(const ReturnStmt *RS, CheckerContext &Ctx) const;

  bool ReportForUnknownAllocations;

private:
  ExplodedNode *emitAllocationPartitionWarning(CheckerContext &C,
                                               const MemRegion *MR,
                                               SourceRange SR,
                                               StringRef Msg) const;
};

} // namespace

REGISTER_MAP_WITH_PROGRAMSTATE(AllocMap, const MemRegion *, QualType)
REGISTER_MAP_WITH_PROGRAMSTATE(ShiftMap, const MemRegion *, const MemRegion *)
REGISTER_SET_WITH_PROGRAMSTATE(SuballocationSet, const MemRegion *)
REGISTER_SET_WITH_PROGRAMSTATE(BoundedSet, const MemRegion *)


namespace {
std::pair<const MemRegion *, bool> getAllocationStart(const ASTContext &ASTCtx,
                                                      const MemRegion *R,
                                                      ProgramStateRef State,
                                                      bool ZeroShift = true) {
  if (const ElementRegion *ER = R->getAs<ElementRegion>()) {
    const MemRegion *Base = ER->getSuperRegion();
    return getAllocationStart(ASTCtx, Base, State,
                              ZeroShift && ER->getIndex().isZeroConstant());
  }
  if (const auto *OrigR = State->get<ShiftMap>(R)) {
    return std::make_pair(*OrigR, false);
  }
  return std::make_pair(R, ZeroShift);
}

bool isAllocation(const MemRegion *R, const AllocationChecker* Chk) {
  if (!Chk->ReportForUnknownAllocations) {
    const MemSpaceRegion *MemSpace = R->getMemorySpace();
    if (!isa<HeapSpaceRegion, GlobalsSpaceRegion, StackSpaceRegion>(MemSpace))
      return false;
  }

  if (R->getAs<SymbolicRegion>())
    return true;
  if (const TypedValueRegion *TR = R->getAs<TypedValueRegion>()) {
    return TR->getValueType()->isArrayType();
  }
  return false;
}

bool relatedTypes(const ASTContext &ASTCtx, const Type *Ty1, const Type *Ty2) {
  if (Ty1 == Ty2)
    return true;
  if (Ty1->isIntegerType()) {
    if (Ty2->isIntegerType())
      return ASTCtx.getTypeSize(Ty1) == ASTCtx.getTypeSize(Ty2);
    return false;
  }
  if (Ty1->isArrayType())
    return relatedTypes(ASTCtx, Ty1->getArrayElementTypeNoTypeQual(), Ty2);
  if (Ty1->isRecordType()) {
    if (RecordDecl *RD = Ty1->getAs<RecordType>()->getAsRecordDecl()) {
      const RecordDecl::field_iterator &FirstField = RD->field_begin();
      if (FirstField != RD->field_end()) {
        const Type *FFTy = FirstField->getType()->getUnqualifiedDesugaredType();
        return relatedTypes(ASTCtx, FFTy, Ty2);
      }
    }
  }
  return false;
}

bool hasFlexibleArrayMember(const Type *PTy) {
  const RecordType *RTy = dyn_cast<RecordType>(PTy);
  if (!RTy)
    return false;

  RecordDecl *RD = RTy->getDecl();
  if (RD->hasFlexibleArrayMember())
    return true;

  // check last field
  FieldDecl *LastField = nullptr;
  for (auto i = RD->field_begin(), end = RD->field_end(); i != end; ++i)
    LastField = *i;
  if (!LastField)
    return false;

  QualType FieldTy = LastField->getType();
  if (FieldTy->isVariableArrayType() || FieldTy->isIncompleteArrayType())
    return true;

  if (const ConstantArrayType *CAT =
          dyn_cast<ConstantArrayType>(FieldTy.getTypePtr())) {
    return CAT->getSize() == 0 || CAT->getSize() == 1;
  }
  return false;
}

bool reportForType(QualType Ty) {
  if (Ty->isVoidPointerType())
    return false;
  if (Ty->isPointerType() || Ty->isArrayType()) {
    const Type *PTy = Ty->getPointeeOrArrayElementType();
    PTy = PTy->getUnqualifiedDesugaredType();
    if (PTy->isCharType())
      return false;
    if (PTy->isPointerType())
      return false;
    if (hasFlexibleArrayMember(PTy))
      return false;
    return true;
  }
  return false;
}

Optional<QualType> getPrevType(ProgramStateRef State, const MemRegion *R) {
  if (const QualType *PrevTy = State->get<AllocMap>(R))
    return *PrevTy;
  if (const TypedValueRegion *TR = R->getAs<TypedValueRegion>()) {
    QualType Ty = TR->getValueType();
    if (reportForType(Ty))
      return Ty;
  }
  return None;
}

} // namespace

ExplodedNode *AllocationChecker::emitAllocationPartitionWarning(
    CheckerContext &C, const MemRegion *MR, SourceRange SR,
    StringRef Msg = "") const {
  if (ExplodedNode *ErrNode = C.generateNonFatalErrorNode()) {

    const MemRegion *PrevAlloc =
        getAllocationStart(C.getASTContext(), MR, C.getState()).first;
    const MemSpaceRegion *MS =
        PrevAlloc ? PrevAlloc->getMemorySpace() : MR->getMemorySpace();
    const BugType &BT =
        isa<HeapSpaceRegion, GlobalsSpaceRegion, StackSpaceRegion>(MS)
            ? BT_Default
            : BT_UnknownReg;
    auto R = std::make_unique<PathSensitiveBugReport>(BT, Msg, ErrNode);
    R->addRange(SR);
    R->markInteresting(MR);

    R->addVisitor(std::make_unique<AllocPartitionBugVisitor>(
        PrevAlloc == MR ? nullptr : PrevAlloc, MR));

    if (const DeclRegion *PrevDecl = getAllocationDecl(PrevAlloc)) {
      auto DeclLoc = PathDiagnosticLocation::create(PrevDecl->getDecl(),
                                                    C.getSourceManager());
      R->addNote("Original allocation", DeclLoc);
    }

    C.emitReport(std::move(R));
    return ErrNode;
  }
  return nullptr;
}

void AllocationChecker::checkPostStmt(const CastExpr *CE,
                                      CheckerContext &C) const {
  if (!isPureCapMode(C.getASTContext()))
    return;

  if (CE->getCastKind() != CK_BitCast)
    return;
  SVal SrcVal = C.getSVal(CE->getSubExpr());
  const MemRegion *MR = SrcVal.getAsRegion();
  if (!MR)
    return;

  ProgramStateRef State = C.getState();
  if (State->contains<BoundedSet>(MR))
    return;

  const ASTContext &ASTCtx = C.getASTContext();
  bool Updated = false;
  std::pair<const MemRegion *, bool> StartPair =
      getAllocationStart(ASTCtx, MR, State);

  const MemRegion *SR = StartPair.first;
  if (!isAllocation(SR, this))
    return;
  bool ZeroShift = StartPair.second;

  SVal DstVal = C.getSVal(CE);
  const MemRegion *DMR = DstVal.getAsRegion();
  if (MR->getAs<ElementRegion>() && (!DMR || !DMR->getAs<ElementRegion>())) {
    if (DstVal.isUnknown()) {
      const LocationContext *LCtx = C.getLocationContext();
      DstVal = C.getSValBuilder().conjureSymbolVal(
          nullptr, CE, LCtx, CE->getType(), C.blockCount());
      State = State->BindExpr(CE, LCtx, DstVal);
      DMR = DstVal.getAsRegion();
    }
    if (DMR) {
      State = State->set<ShiftMap>(DMR, SR);
      Updated = true;
    }
  }

  QualType DstTy = CE->getType().getUnqualifiedType();
  if (!reportForType(DstTy))
    return;

  Optional<QualType> PrevTy = getPrevType(State, SR);
  if (PrevTy.hasValue()) {
    if (SR != MR && !ZeroShift) {
      const Type *Ty1 = PrevTy.getValue()
                            ->getPointeeOrArrayElementType()
                            ->getUnqualifiedDesugaredType();
      const Type *Ty2 = DstTy->getPointeeType()->getUnqualifiedDesugaredType();
      if (!relatedTypes(ASTCtx, Ty1, Ty2)) {
        if (!State->contains<SuballocationSet>(SR)) {
          State = State->add<SuballocationSet>(SR);
          Updated = true;
        }
        if (DMR && !State->contains<SuballocationSet>(DMR)) {
          State = State->add<SuballocationSet>(DMR);
          Updated = true;
        }
      } // else OK
    } // else ??? (ignore for now)
  } else {
    State = State->set<AllocMap>(SR, DstTy);
    Updated = true;
  }

  if (Updated)
    C.addTransition(State);
}

void AllocationChecker::checkPreCall(const CallEvent &Call,
                                     CheckerContext &C) const {
  if (!isPureCapMode(C.getASTContext()))
    return;

  if (IgnoreFnSet.contains(Call) || CheriBoundsFnSet.contains(Call))
    return;

  ProgramStateRef State = C.getState();
  ExplodedNode *N = nullptr;
  bool Updated  = false;
  for (unsigned Arg = 0; Arg < Call.getNumArgs(); ++Arg) {
    const Expr *ArgExpr = Call.getArgExpr(Arg);
    if (const MemRegion *MR = C.getSVal(ArgExpr).getAsRegion()) {
      if (State->contains<SuballocationSet>(MR)) {
        SmallString<256> Buf;
        llvm::raw_svector_ostream OS(Buf);
        OS << "Pointer to suballocation passed to function as " << (Arg+1);
        if (Arg + 1 < 11) {
          switch (Arg+1) {
          case 1: OS << "st"; break;
          case 2: OS << "nd"; break;
          case 3: OS << "rd"; break;
          default: OS << "th"; break;
          }
        }

        OS << " argument";
        OS << " (consider narrowing the bounds for suballocation)";
        N = emitAllocationPartitionWarning(
            C, MR, ArgExpr->getSourceRange(),
            OS.str());
        if (N) {
          State = State->remove<SuballocationSet>(MR);
          Updated = true;
        }
      }
    }
  }
  if (Updated)
    C.addTransition(State, N ? N : C.getPredecessor());
}

void AllocationChecker::checkPostCall(const CallEvent &Call,
                                      CheckerContext &C) const {
  if (!isPureCapMode(C.getASTContext()))
    return;

  if (!CheriBoundsFnSet.contains(Call))
    return;
  const MemRegion *MR = C.getSVal(Call.getArgExpr(0)).getAsRegion();
  const MemRegion *ResMR = C.getSVal(Call.getOriginExpr()).getAsRegion();
  if (!MR || !ResMR)
    return;

  ProgramStateRef State = C.getState();
  if (!State->contains<SuballocationSet>(MR) ||
      !State->contains<SuballocationSet>(ResMR))
    return;

  State = State->remove<SuballocationSet>(ResMR);
  State = State->add<BoundedSet>(ResMR);
  C.addTransition(State);
}

void AllocationChecker::checkBind(SVal L, SVal V, const Stmt *S,
                                  CheckerContext &C) const {
  if (!isPureCapMode(C.getASTContext()))
    return;

  const MemRegion *Dst = L.getAsRegion();
  if (!Dst || !isa<FieldRegion>(Dst))
    return;

  ProgramStateRef State = C.getState();
  const MemRegion *Val = V.getAsRegion();
  if (Val && State->contains<SuballocationSet>(Val)) {
    ExplodedNode *N = emitAllocationPartitionWarning(
        C, Val, S->getSourceRange(),
        "Pointer to suballocation escaped on assign"
        " (consider narrowing the bounds for suballocation)");
    if (N) {
      State = State->remove<SuballocationSet>(Val);
      C.addTransition(State, N);
    }
    return;
  }
}

void AllocationChecker::checkEndFunction(const ReturnStmt *RS,
                                         CheckerContext &C) const {
  if (!isPureCapMode(C.getASTContext()))
    return;

  if (!RS)
    return;
  const Expr *RV = RS->getRetValue();
  if (!RV)
    return;

  llvm::SmallVector<EscapePair, 2> Escaped;
  if (const MemRegion *MR = C.getSVal(RV).getAsRegion()) {
    if (C.getState()->contains<SuballocationSet>(MR)) {
      ExplodedNode *N = emitAllocationPartitionWarning(
          C, MR, RS->getSourceRange(),
          "Pointer to suballocation returned from function"
          " (consider narrowing the bounds for suballocation)");
      if (N) {
        ProgramStateRef State = N->getState()->remove<SuballocationSet>(MR);
        C.addTransition(State, N);
      }
      return;
    }
  }
}

PathDiagnosticPieceRef AllocationChecker::AllocPartitionBugVisitor::VisitNode(
    const ExplodedNode *N, BugReporterContext &BRC,
    PathSensitiveBugReport &BR) {
  const Stmt *S = N->getStmtForDiagnostics();
  if (!S)
    return nullptr;

  if (const CastExpr *CE = dyn_cast<CastExpr>(S)) {
    if (CE->getCastKind() != CK_BitCast)
      return nullptr;

    SmallString<256> Buf;
    llvm::raw_svector_ostream OS(Buf);

    if (!PrevReported && PrevAlloc &&
        PrevAlloc == N->getSVal(CE->getSubExpr()).getAsRegion()) {
      OS << "Previous partition: ";
      PrevReported = true;
    } else if (SubAlloc == N->getSVal(CE).getAsRegion() &&
               N->getState()->contains<SuballocationSet>(SubAlloc) &&
               !N->getFirstPred()->getState()->contains<SuballocationSet>(
                   SubAlloc)) {
      OS << "Allocation partition: ";
    } else
      return nullptr;

    describeCast(OS, CE, BRC.getASTContext().getLangOpts());
    PathDiagnosticLocation const Pos(S, BRC.getSourceManager(),
                                     N->getLocationContext());
    auto Ev = std::make_shared<PathDiagnosticEventPiece>(Pos, OS.str(), true);
    Ev->setPrunable(false);
    return Ev;
  }
  return nullptr;
}

//===----------------------------------------------------------------------===//
// Checker registration.
//===----------------------------------------------------------------------===//

void ento::registerAllocationChecker(CheckerManager &Mgr) {
  auto *Checker = Mgr.registerChecker<AllocationChecker>();
  Checker->ReportForUnknownAllocations =
      Mgr.getAnalyzerOptions().getCheckerBooleanOption(
          Checker, "ReportForUnknownAllocations");
}

bool ento::shouldRegisterAllocationChecker(const CheckerManager &Mgr) {
  return true;
}