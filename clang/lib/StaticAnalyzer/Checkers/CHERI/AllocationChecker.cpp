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
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"


using namespace clang;
using namespace ento;
using namespace cheri;

namespace {

class AllocationChecker : public Checker<check::PostStmt<CastExpr>> {
  BugType BT_Default{this, "Allocation partitioning", "CHERI portability"};
  BugType BT_KnownReg{this, "Heap or static allocation partitioning",
                      "CHERI portability"};

  class AllocPartitionBugVisitor : public BugReporterVisitor {
  public:
    AllocPartitionBugVisitor(const MemRegion *R) : Reg(R) {}

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
  void checkPostStmt(const CastExpr *CE, CheckerContext &C) const;

private:
  ExplodedNode *emitAllocationPartitionWarning(const CastExpr *CE,
                                               CheckerContext &C,
                                               const MemRegion *R) const;
};

} // namespace

REGISTER_MAP_WITH_PROGRAMSTATE(AllocMap, const MemRegion *, QualType)
REGISTER_MAP_WITH_PROGRAMSTATE(ShiftMap, const MemRegion *, const MemRegion *)

namespace {
std::pair<const MemRegion *, bool> getAllocationStart(const MemRegion *R,
                                                      CheckerContext &C,
                                                      bool ZeroShift = true) {
  if (const ElementRegion *ER = R->getAs<ElementRegion>()) {
    const MemRegion *Base = ER->getSuperRegion();
    return getAllocationStart(Base, C, ER->getIndex().isZeroConstant());
  }
  if (auto OrigR = C.getState()->get<ShiftMap>(R)) {
    return std::make_pair(*OrigR, false);
  }
  return std::make_pair(R, ZeroShift);
}

bool isAllocation(const MemRegion *R) {
  if (R->getAs<SymbolicRegion>())
    return true;
  if (const TypedValueRegion *TR = R->getAs<TypedValueRegion>()) {
    return TR->getValueType()->isArrayType();
  }
  return false;
}

bool relatedTypes(const Type *Ty1, const Type *Ty2) {
  if (Ty1 == Ty2)
    return true;
  if (Ty1->isArrayType())
    return relatedTypes(Ty1->getArrayElementTypeNoTypeQual(), Ty2);
  return false;
}

bool isGenPtrType(QualType Ty) {
  return Ty->isVoidPointerType() ||
         ((Ty->isPointerType() || Ty->isArrayType()) &&
          Ty->getPointeeOrArrayElementType()->isCharType());
}

Optional<QualType> getPrevType(ProgramStateRef State, const MemRegion *R) {
  if (const QualType *PrevTy = State->get<AllocMap>(R))
    return *PrevTy;
  if (const TypedValueRegion *TR = R->getAs<TypedValueRegion>()) {
    QualType Ty = TR->getValueType();
    if ((Ty->isPointerType() || Ty->isArrayType()) && !isGenPtrType(Ty))
      return Ty;
  }
  return None;
}

} // namespace

ExplodedNode *AllocationChecker::emitAllocationPartitionWarning(
    const CastExpr *CE, CheckerContext &C, const MemRegion *MR) const {
  if (ExplodedNode *ErrNode = C.generateNonFatalErrorNode()) {
    SmallString<256> Buf;
    llvm::raw_svector_ostream OS(Buf);
    OS << "Allocation partition: ";
    describeCast(OS, CE, C.getASTContext().getLangOpts());
    const MemSpaceRegion *MemSpace = MR->getMemorySpace();
    bool KnownReg = isa<HeapSpaceRegion, GlobalsSpaceRegion>(MemSpace);
    auto R = std::make_unique<PathSensitiveBugReport>(
        KnownReg ? BT_KnownReg : BT_Default, OS.str(), ErrNode);
    R->addVisitor(std::make_unique<AllocPartitionBugVisitor>(MR));
    C.emitReport(std::move(R));
    return ErrNode;
  }
  return nullptr;
}

void AllocationChecker::checkPostStmt(const CastExpr *CE,
                                      CheckerContext &C) const {
  if (CE->getCastKind() != CK_BitCast)
    return;
  SVal SrcVal = C.getSVal(CE->getSubExpr());
  const MemRegion *MR = SrcVal.getAsRegion();
  if (!MR)
    return;
  if (MR->getMemorySpace()->getKind() == MemSpaceRegion::CodeSpaceRegionKind)
    return;

  ProgramStateRef State = C.getState();
  bool Updated = false;

  std::pair<const MemRegion *, bool> StartPair = getAllocationStart(MR, C);
  const MemRegion *SR = StartPair.first;
  if (!isAllocation(SR))
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
  if (!DstTy->isPointerType())
    return;
  if (DstTy->isVoidPointerType() || DstTy->getPointeeType()->isCharType())
    return;

  Optional<QualType> PrevTy = getPrevType(State, SR);
  if (PrevTy.hasValue()) {
    if (SR != MR && !ZeroShift) {
      const Type *Ty1 = PrevTy.getValue()
                            ->getPointeeOrArrayElementType()
                            ->getUnqualifiedDesugaredType();
      const Type *Ty2 = DstTy->getPointeeType()->getUnqualifiedDesugaredType();
      if (!relatedTypes(Ty1, Ty2)) {
        emitAllocationPartitionWarning(CE, C, SR);
        return;
      } // else OK
    } // else ??? (ignore for now)
  } else {
    State = State->set<AllocMap>(SR, DstTy);
    Updated = true;
  }

  if (Updated)
    C.addTransition(State);
}

PathDiagnosticPieceRef AllocationChecker::AllocPartitionBugVisitor::VisitNode(
    const ExplodedNode *N, BugReporterContext &BRC,
    PathSensitiveBugReport &BR) {
  const Stmt *S = N->getStmtForDiagnostics();
  if (!S)
    return nullptr;

  const CastExpr *CE = dyn_cast<CastExpr>(S);
  if (!CE || CE->getCastKind() != CK_BitCast)
    return nullptr;

  if (Reg != N->getSVal(CE->getSubExpr()).getAsRegion())
    return nullptr;

  SmallString<256> Buf;
  llvm::raw_svector_ostream OS(Buf);
  OS << "Previous partition: ";
  describeCast(OS, CE, BRC.getASTContext().getLangOpts());

  // Generate the extra diagnostic.
  PathDiagnosticLocation const Pos(S, BRC.getSourceManager(),
                                   N->getLocationContext());
  return std::make_shared<PathDiagnosticEventPiece>(Pos, OS.str(), true);
}

//===----------------------------------------------------------------------===//
// Checker registration.
//===----------------------------------------------------------------------===//

void ento::registerAllocationChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<AllocationChecker>();
}

bool ento::shouldRegisterAllocationChecker(const CheckerManager &Mgr) {
  return true;
}