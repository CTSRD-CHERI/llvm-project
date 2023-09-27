//=== PointerAlignmentChecker.cpp - Capability Alignment Checker -*- C++ ==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Tracks pointer values alignment and reports pointer casts of underaligned
// values to types with strict alignment requirements.
//   * Cast Align Bug
//       Cast of underaligned pointer value to the pointer type with stricter
//       alignment requirements.
//   * CapCastAlignBug
//       Special case for CHERI: casts to pointer to capability. Storing
//       capabilities at not capability-aligned addressed will result in
//       stored capability losing its tag.
//
// Currently, this checker (deliberately) does not take into account:
//    - alignment of static and automatic allocations, enforced by ABI (other
//    than implied by variable types),
//    - alignment of fields that is guaranteed implicitly by the alignment of
//    the whole object and current field offset.
// Relying on this could make the code less portable and easier to break, but
// it may be a good idea to introduce a separate warning type that will not
// be reported for pointer values properly aligned due to the reasons above,
// so that it will produce just a few reports for critical bugs only.
//
//===----------------------------------------------------------------------===//

#include "CHERI/CHERIUtils.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/StaticAnalyzer/Core/BugReporter/BugType.h>
#include <clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h>

using namespace clang;
using namespace ento;
using namespace cheri;

namespace {
class PointerAlignmentChecker
    : public Checker<check::PreStmt<CastExpr>, check::PostStmt<CastExpr>,
                     check::PostStmt<BinaryOperator>, check::DeadSymbols> {
  std::unique_ptr<BugType> CastAlignBug;
  std::unique_ptr<BugType> CapCastAlignBug;


public:
  PointerAlignmentChecker();

  void checkPostStmt(const BinaryOperator *BO, CheckerContext &C) const;
  void checkPostStmt(const CastExpr *BO, CheckerContext &C) const;
  void checkPreStmt(const CastExpr *BO, CheckerContext &C) const;
  void checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const;


private:
  ExplodedNode *emitCastAlignWarn(CheckerContext &C, unsigned SrcAlign,
                                  unsigned DstReqAlign,
                                  const CastExpr *CE) const;

  class AlignmentBugVisitor : public BugReporterVisitor {
  public:
    AlignmentBugVisitor(SymbolRef SR) : Sym(SR) {}

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
};

} // namespace

REGISTER_MAP_WITH_PROGRAMSTATE(TrailingZerosMap, SymbolRef, int)

PointerAlignmentChecker::PointerAlignmentChecker() {
  CastAlignBug.reset(new BugType(this,
      "Cast increases required alignment",
      "Type Error"));
  CapCastAlignBug.reset(new BugType(this,
      "Cast increases required alignment to capability alignment",
      "CHERI portability"));
}

namespace {

int getTrailingZerosCount(const SVal &V, ProgramStateRef State,
                          ASTContext &ASTCtx);

int getTrailingZerosCount(SymbolRef Sym, ProgramStateRef State,
                          ASTContext &ASTCtx) {
  const int *Align = State->get<TrailingZerosMap>(Sym);
  if (Align)
    return *Align;

  // Is function argument or global?
  if (const MemRegion *BaseRegOrigin = Sym->getOriginRegion())
    if (BaseRegOrigin->getMemorySpace()->hasGlobalsOrParametersStorage()) {
      const QualType &SymTy = Sym->getType();
      if (SymTy->isPointerType() && !isGenericPointerType(SymTy)) {
        const QualType &PT = SymTy->getPointeeType();
        if (!PT->isIncompleteType()) {
          unsigned A = ASTCtx.getTypeAlignInChars(PT).getQuantity();
          return llvm::APSInt::getUnsigned(A).countTrailingZeros();
        }
      }
  }

  return -1;
}

int getTrailingZerosCount(const MemRegion *R, ProgramStateRef State,
                          ASTContext &ASTCtx) {
  R = R->StripCasts();

  if (const SymbolicRegion *SR = R->getAs<SymbolicRegion>())
    return getTrailingZerosCount(SR->getSymbol(), State, ASTCtx);

  if (const TypedValueRegion *TR = R->getAs<TypedValueRegion>()) {
    const QualType PT = TR->getDesugaredValueType(ASTCtx);
    if (PT->isIncompleteType())
      return -1;
    unsigned NaturalAlign = ASTCtx.getTypeAlignInChars(PT).getQuantity();

    if (const ElementRegion *ER = R->getAs<ElementRegion>()) {
      int ElTyTZ = llvm::APSInt::getUnsigned(NaturalAlign).countTrailingZeros();

      const MemRegion *Base = ER->getSuperRegion();
      int BaseTZC = getTrailingZerosCount(Base, State, ASTCtx);
      if (BaseTZC < 0)
        return ElTyTZ > 0 ? ElTyTZ : -1;

      int IdxTZC = getTrailingZerosCount(ER->getIndex(), State, ASTCtx);
      if (IdxTZC < 0 && NaturalAlign == 1)
        return -1;

      return std::min(BaseTZC, std::max(IdxTZC, 0) + ElTyTZ);
    }

    unsigned AlignAttrVal = 0;
    if (auto DR = R->getAs<DeclRegion>()) {
      if (auto AA = DR->getDecl()->getAttr<AlignedAttr>()) {
        if (!AA->isAlignmentDependent())
          AlignAttrVal = AA->getAlignment(ASTCtx) / ASTCtx.getCharWidth();
      }
    }

    unsigned A = std::max(NaturalAlign, AlignAttrVal);
    return llvm::APSInt::getUnsigned(A).countTrailingZeros();
  }
  return -1;
}

int getTrailingZerosCount(const SVal &V, ProgramStateRef State,
                          ASTContext &ASTCtx) {
  if (V.isUnknownOrUndef())
    return -1;

  if (V.isConstant()) {
    if (Optional<loc::ConcreteInt> LV = V.getAs<loc::ConcreteInt>())
        return LV->getValue().countTrailingZeros();
    if (Optional<nonloc::ConcreteInt> NV = V.getAs<nonloc::ConcreteInt>())
        return NV->getValue().countTrailingZeros();
    return -1;
  }

  if (SymbolRef Sym = V.getAsSymbol()) {
    int Res = getTrailingZerosCount(Sym, State, ASTCtx);
    if (Res >=0)
        return Res;
  }

  if (const MemRegion *R = V.getAsRegion()) {
    return getTrailingZerosCount(R, State, ASTCtx);
  }

  return -1;
}

int getTrailingZerosCount(const Expr *E, CheckerContext &C) {
  const SVal &V = C.getSVal(E);
  return getTrailingZerosCount(V, C.getState(), C.getASTContext());
}

/* Introduced by clang, not in C standard */
bool isImplicitConversionFromVoidPtr(const Stmt *S, CheckerContext &C) {
  using namespace clang::ast_matchers;
  auto CmpM = binaryOperation(isComparisonOperator());
  auto VoidPtr = expr(hasType(pointerType(pointee(voidType()))));
  auto CastM = implicitCastExpr(has(VoidPtr), hasType(pointerType()),
                                hasCastKind(CK_BitCast), hasParent(CmpM));
  auto M = match(expr(CastM), *S, C.getASTContext());
  return !M.empty();
}

} // namespace

void PointerAlignmentChecker::checkPreStmt(const CastExpr *CE,
                                               CheckerContext &C) const {
  CastKind CK = CE->getCastKind();
  if (CK != CastKind::CK_BitCast && CK != CK_IntegralToPointer)
    return;
  if (isImplicitConversionFromVoidPtr(CE, C))
    return;

  const QualType &DstType = CE->getType();
  if (!DstType->isPointerType())
    return;
  const QualType &DstPointeeTy = DstType->getPointeeType();
  if (DstPointeeTy->isIncompleteType())
    return;

  const SVal &SrcVal = C.getSVal(CE->getSubExpr());
  if (SrcVal.isConstant())
    return; // special value
  int SrcTZC = getTrailingZerosCount(SrcVal, C.getState(), C.getASTContext());
  if (SrcTZC < 0)
    return;
  if ((unsigned)SrcTZC >= sizeof(unsigned int)*8)
    return; // Too aligned, probably a Zero
  unsigned SrcAlign = (1U << SrcTZC);

  ASTContext &ASTCtx = C.getASTContext();
  unsigned DstReqAlign = ASTCtx.getTypeAlignInChars(DstPointeeTy).getQuantity();
  if (SrcAlign < DstReqAlign) {
    emitCastAlignWarn(C, SrcAlign, DstReqAlign, CE);
  }
}

void PointerAlignmentChecker::checkPostStmt(const CastExpr *CE,
                                            CheckerContext &C) const {
  CastKind CK = CE->getCastKind();
  if (CK != CastKind::CK_BitCast && CK != CK_PointerToIntegral &&
      CK != CK_IntegralToPointer)
    return;

  int DstTZC = getTrailingZerosCount(CE, C);
  int SrcTZC = getTrailingZerosCount(CE->getSubExpr(), C);

  ASTContext &ASTCtx = C.getASTContext();
  int DstReqTZC = -1;
  if (CE->getType()->isPointerType()) {
    if (!isGenericPointerType(CE->getType(), true)) {
      const QualType &DstPTy = CE->getType()->getPointeeType();
      if (!DstPTy->isIncompleteType()) {
        unsigned ReqAl = ASTCtx.getTypeAlignInChars(DstPTy).getQuantity();
        DstReqTZC = llvm::APSInt::getUnsigned(ReqAl).countTrailingZeros();
      }
    }
  }

  int NewAlign = std::max(SrcTZC, DstReqTZC);
  if (DstTZC < NewAlign) {
    SVal DstVal = C.getSVal(CE);
    ProgramStateRef State = C.getState();
    if (DstVal.isUnknown()) {
        const LocationContext *LCtx = C.getLocationContext();
        DstVal = C.getSValBuilder().conjureSymbolVal(
            nullptr, CE, LCtx, CE->getType(), C.blockCount());
        State = State->BindExpr(CE, LCtx, DstVal);
    }
    if (SymbolRef Sym = DstVal.getAsSymbol()) {
        State = State->set<TrailingZerosMap>(Sym, NewAlign);
        C.addTransition(State);
    }
  }
}

bool valueIsLTPow2(const Expr *E, unsigned P, CheckerContext &C) {
  if (P >= sizeof(uint64_t) * 8)
    return true;
  SValBuilder &SVB = C.getSValBuilder();
  const NonLoc &B = SVB.makeIntVal((uint64_t)1 << P, true);

  ProgramStateRef State = C.getState();
  const SVal &V = C.getSVal(E);
  auto LT = SVB.evalBinOp(State, BO_LT, V, B, E->getType());
  return !State->assume(LT.castAs<DefinedOrUnknownSVal>(), false);
}

void PointerAlignmentChecker::checkPostStmt(const BinaryOperator *BO,
                                               CheckerContext &C) const {
  int LeftTZ = getTrailingZerosCount(BO->getLHS(), C);
  if (LeftTZ < 0)
    return;
  int RightTZ = getTrailingZerosCount(BO->getRHS(), C);
  if (RightTZ < 0 && !BO->isShiftOp() && !BO->isShiftAssignOp())
    return;

  ProgramStateRef State = C.getState();
  SVal ResVal = C.getSVal(BO);
  if (!ResVal.isUnknown() && !ResVal.getAsSymbol())
    return;

  const SVal &RHSVal = C.getSVal(BO->getRHS());
  int BitWidth = C.getASTContext().getTypeSize(BO->getType());
  int Res = 0;
  int RHSConst = 0;
  BinaryOperator::Opcode OpCode = BO->getOpcode();
  switch (OpCode) {
  case clang::BO_And:
  case clang::BO_AndAssign:
    if (valueIsLTPow2(BO->getLHS(), RightTZ, C) ||
        valueIsLTPow2(BO->getRHS(), LeftTZ, C)) {
        /* Align check: p & (ALIGN - 1)*/
        if (ResVal.isUnknown()) {
          ResVal = C.getSValBuilder().makeIntVal(0, true);
          State = State->BindExpr(BO, C.getLocationContext(), ResVal);
          C.addTransition(State);
          return;
        } else {
          Res = BitWidth;
        }
    } else
        Res = std::max(LeftTZ, RightTZ);
    break;
  case clang::BO_Or:
  case clang::BO_OrAssign:
    Res = std::min(LeftTZ, RightTZ);
    break;
  case clang::BO_Add:
  case clang::BO_AddAssign:
  case clang::BO_Sub:
  case clang::BO_SubAssign:
    if (BO->getLHS()->getType()->isPointerType()) {
        const QualType &PointeeTy = BO->getLHS()->getType()->getPointeeType();
        const CharUnits A = C.getASTContext().getTypeAlignInChars(PointeeTy);
        RightTZ +=
            llvm::APSInt::getUnsigned(A.getQuantity()).countTrailingZeros();
    }
    Res = std::min(LeftTZ, RightTZ);
    break;
  case clang::BO_Mul:
  case clang::BO_MulAssign:
    Res = LeftTZ + RightTZ;
    break;
  case clang::BO_Div:
  case clang::BO_DivAssign:
    Res = LeftTZ - RightTZ;
    break;
  case clang::BO_Shl:
  case clang::BO_ShlAssign:
  case clang::BO_Shr:
  case clang::BO_ShrAssign:
    if (RHSVal.isConstant()) {
      if (auto NV = RHSVal.getAs<nonloc::ConcreteInt>())
        RHSConst = NV.getValue().getValue().getExtValue();
    }
    if (BO->getOpcode() == BO_Shl || BO->getOpcode() == BO_ShlAssign)
      Res = LeftTZ + RHSConst;
    else
      Res = RHSVal.isConstant() ? LeftTZ - RHSConst : BitWidth;
    break;
  default:
    return;
  }

  if (Res <= 0)
    Res = 0;
  if (Res > BitWidth)
    Res = BitWidth;

  if (ResVal.isUnknown()) {
    const LocationContext *LCtx = C.getLocationContext();
    ResVal = C.getSValBuilder().conjureSymbolVal(
        nullptr, BO, LCtx, BO->getType(), C.blockCount());
    State = State->BindExpr(BO, LCtx, ResVal);
  }
  State = State->set<TrailingZerosMap>(ResVal.getAsSymbol(), Res);
  C.addTransition(State);
}

void PointerAlignmentChecker::checkDeadSymbols(SymbolReaper &SymReaper,
                                                  CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  TrailingZerosMapTy TZMap = State->get<TrailingZerosMap>();
  bool Updated = false;
  for (TrailingZerosMapTy::iterator I = TZMap.begin(),
                                    E = TZMap.end(); I != E; ++I) {
    if (SymReaper.isDead(I->first)) {
      State = State->remove<TrailingZerosMap>(I->first);
      Updated = true;
    }
  }
  if (Updated)
    C.addTransition(State);
}

namespace {

bool hasCapability(const QualType OrigTy, ASTContext &Ctx) {
  QualType Ty = OrigTy.getCanonicalType();
  if (Ty->isCHERICapabilityType(Ctx, true))
    return true;
  if (const auto *Record = dyn_cast<RecordType>(Ty)) {
    for (const auto *Field : Record->getDecl()->fields()) {
      if (hasCapability(Field->getType(), Ctx))
        return true;
    }
    return false;
  }
  if (const auto *Array = dyn_cast<ArrayType>(Ty)) {
    return hasCapability(Array->getElementType(), Ctx);
  }
  return false;
}

void printAlign(raw_ostream &OS, unsigned TZC) {
  OS << "aligned(";
  if (TZC < sizeof(unsigned long)*8)
    OS << (1LU << TZC);
  else
    OS << "2^(" << TZC << ")";
  OS << ")";
}

void describeOriginalAllocation(const MemRegion *MR, PathSensitiveBugReport &W,
                                const SourceManager &SM,
                                ASTContext &ASTCtx) {
  if (const DeclRegion *DR = MR->getAs<DeclRegion>()) {
    const ValueDecl *SrcDecl = DR->getDecl();
    SmallString<350> Note;
    llvm::raw_svector_ostream OS2(Note);
    const QualType &AllocType = SrcDecl->getType().getCanonicalType();
    OS2 << "Original allocation of type ";
    OS2 << "'" << AllocType.getAsString() << "'";
    OS2 << " which has an alignment requirement ";
    OS2 << ASTCtx.getTypeAlignInChars(AllocType).getQuantity();
    OS2 << " bytes";
    W.addNote(Note, PathDiagnosticLocation::create(SrcDecl, SM));
  } else if (const ElementRegion *ER = MR->getAs<ElementRegion>())
    describeOriginalAllocation(ER->getSuperRegion(), W, SM, ASTCtx);
}

} // namespace

ExplodedNode *
PointerAlignmentChecker::emitCastAlignWarn(
    CheckerContext &C, unsigned SrcAlign, unsigned DstReqAlign,
    const CastExpr *CE) const {
  ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
  if (!ErrNode)
    return nullptr;

  ASTContext &ASTCtx = C.getASTContext();
  const QualType &DstTy = CE->getType();
  bool DstAlignIsCap = hasCapability(DstTy->getPointeeType(), ASTCtx);

  SmallString<350> ErrorMessage;
  llvm::raw_svector_ostream OS(ErrorMessage);
  OS << "Pointer value aligned to a " << SrcAlign << " byte boundary";
  OS << " cast to type '" << DstTy.getAsString() << "'";
  OS << " with required";
  if (DstAlignIsCap)
    OS << " capability";
  OS << " alignment " << DstReqAlign;
  OS << " bytes";

  auto W = std::make_unique<PathSensitiveBugReport>(
      DstAlignIsCap ? *CapCastAlignBug : *CastAlignBug, ErrorMessage, ErrNode);
  W->addRange(CE->getSourceRange());

  const SVal &SrcVal = C.getSVal(CE->getSubExpr());
  W->markInteresting(SrcVal);
  if (SymbolRef S = SrcVal.getAsSymbol())
    W->addVisitor(std::make_unique<AlignmentBugVisitor>(S));
  else if (const MemRegion *MR = SrcVal.getAsRegion()) {
    describeOriginalAllocation(MR, *W, C.getSourceManager(), C.getASTContext());
  }

  C.emitReport(std::move(W));
  return ErrNode;
}

PathDiagnosticPieceRef PointerAlignmentChecker::AlignmentBugVisitor::VisitNode(
    const ExplodedNode *N, BugReporterContext &BRC,
    PathSensitiveBugReport &BR) {

  const Stmt *S = N->getStmtForDiagnostics();
  if (!S)
    return nullptr;
  const Expr *E = dyn_cast<Expr>(S);
  if (!E)
    return nullptr;

  SmallString<256> Buf;
  llvm::raw_svector_ostream OS(Buf);

  if (Sym != N->getSVal(S).getAsSymbol())
    return nullptr;

  ProgramStateRef State = N->getState();
  ProgramStateRef Pred = N->getFirstPred()->getState();
  const int *NewAlign = State->get<TrailingZerosMap>(Sym);
  const int *OldAlign = Pred->get<TrailingZerosMap>(Sym);
  if (!NewAlign || (OldAlign && *NewAlign == *OldAlign))
    return nullptr;

  if (const CastExpr *CE = dyn_cast<CastExpr>(E)) {
    if (SymbolRef SrcSym = N->getSVal(CE->getSubExpr()).getAsSymbol())
      if (const int *SrcAlign = State->get<TrailingZerosMap>(SrcSym))
        if (*SrcAlign == *NewAlign)
          return nullptr;
  }

  OS << "Alignment: ";
  if (const BinaryOperator *BO = dyn_cast<BinaryOperator>(E)) {
    BinaryOperatorKind const OpCode = BO->getOpcode();
    if (!BO->isShiftOp() && !BO->isShiftAssignOp()) {
      ASTContext &ASTCtx = N->getCodeDecl().getASTContext();
      int LTZ = getTrailingZerosCount(N->getSVal(BO->getLHS()), State, ASTCtx);
      int RTZ = getTrailingZerosCount(N->getSVal(BO->getRHS()), State, ASTCtx);
      if (LTZ >= 0 && RTZ >= 0) {
        printAlign(OS, LTZ);
        OS << " " << BinaryOperator::getOpcodeStr(OpCode) << " ";
        printAlign(OS, RTZ);
        OS << " => ";
      }
    }
  }
  printAlign(OS, *NewAlign);

  // Generate the extra diagnostic.
  PathDiagnosticLocation const Pos(S, BRC.getSourceManager(),
                                   N->getLocationContext());
  return std::make_shared<PathDiagnosticEventPiece>(Pos, OS.str(), true);
}

void ento::registerPointerAlignmentChecker(CheckerManager &mgr) {
  mgr.registerChecker<PointerAlignmentChecker>();
}

bool ento::shouldRegisterPointerAlignmentChecker(const CheckerManager &Mgr) {
  return true;
}