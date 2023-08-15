//=== CapabilityAlignmentChecker.cpp - Capability Alignment Checker -*- C++ ==//
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

#include "CHERIUtils.h"
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
class CapabilityAlignmentChecker
    : public Checker<check::PreStmt<CastExpr>, check::PostStmt<CastExpr>,
                     check::PostStmt<BinaryOperator>> {
  std::unique_ptr<BugType> CastAlignBug;


public:
  CapabilityAlignmentChecker();

  void checkPostStmt(const BinaryOperator *BO, CheckerContext &C) const;
  void checkPostStmt(const CastExpr *BO, CheckerContext &C) const;
  void checkPreStmt(const CastExpr *BO, CheckerContext &C) const;

private:
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

CapabilityAlignmentChecker::CapabilityAlignmentChecker() {
  CastAlignBug.reset(
      new BugType(this, "Cast increases required alignment", "CHERI portability"));
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

  if (const ElementRegion *ER = R->getAs<ElementRegion>()) {
    const MemRegion *Base = ER->getSuperRegion();
    int BaseTZC = -1;
    if (const SymbolicRegion *SymbBase = Base->getSymbolicBase())
      BaseTZC = getTrailingZerosCount(SymbBase->getSymbol(), State, ASTCtx);
    else
      BaseTZC = getTrailingZerosCount(Base, State, ASTCtx);
    int IdxTZC = getTrailingZerosCount(ER->getIndex(), State, ASTCtx);
    if (BaseTZC >=0) {
      const QualType ElemTy = ER->getElementType();
      unsigned ElAlign = ASTCtx.getTypeAlignInChars(ElemTy).getQuantity();
      if (IdxTZC < 0 && ElAlign == 1)
          return -1;
      int ElemTyTZC = llvm::APSInt::getUnsigned(ElAlign).countTrailingZeros();
      return std::min(BaseTZC, std::max(IdxTZC, 0) + ElemTyTZC);
    }
    return -1;
  }

  if (R->getSymbolicBase())
    return -1;

  const TypedValueRegion *TR = R->getAs<TypedValueRegion>();
  if (!TR)
    return -1;
  const QualType PT = TR->getDesugaredValueType(ASTCtx);
  if (PT->isIncompleteType())
    return -1;
  unsigned NaturalAlign = ASTCtx.getTypeAlignInChars(PT).getQuantity();

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

void CapabilityAlignmentChecker::checkPreStmt(const CastExpr *CE,
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
    if (ExplodedNode *ErrNode = C.generateNonFatalErrorNode()) {
        SmallString<350> ErrorMessage;
        llvm::raw_svector_ostream OS(ErrorMessage);
        OS << "Cast increases required alignment: ";
        OS << SrcAlign;
        OS << " -> ";
        OS << DstReqAlign;
        OS << ")";
        auto W = std::make_unique<PathSensitiveBugReport>(
            *CastAlignBug, ErrorMessage, ErrNode);
        W->addRange(CE->getSourceRange());
        if (SymbolRef S = SrcVal.getAsSymbol())
          W->addVisitor(std::make_unique<AlignmentBugVisitor>(S));
        W->markInteresting(SrcVal);
        C.emitReport(std::move(W));
    }
  }
}

void printAlign(raw_ostream &OS, unsigned TZC) {
  OS << "aligned(";
  if (TZC < sizeof(unsigned long)*8)
    OS << (1LU << TZC);
  else
    OS << "2^(" << TZC << ")";
  OS << ")";
}

void CapabilityAlignmentChecker::checkPostStmt(const CastExpr *CE,
                                            CheckerContext &C) const {
  CastKind CK = CE->getCastKind();
  if (CK != CastKind::CK_BitCast && CK != CK_PointerToIntegral &&
      CK != CK_IntegralToPointer)
    return;

  int DstTZC = getTrailingZerosCount(CE, C);
  int SrcTZC = getTrailingZerosCount(CE->getSubExpr(), C);

  if (DstTZC < SrcTZC) {
    SVal DstVal = C.getSVal(CE);
    ProgramStateRef State = C.getState();
    if (DstVal.isUnknown()) {
        const LocationContext *LCtx = C.getLocationContext();
        DstVal = C.getSValBuilder().conjureSymbolVal(
            nullptr, CE, LCtx, CE->getType(), C.blockCount());
        State = State->BindExpr(CE, LCtx, DstVal);
    }
    if (SymbolRef Sym = DstVal.getAsSymbol()) {
        State = State->set<TrailingZerosMap>(Sym, SrcTZC);
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

void CapabilityAlignmentChecker::checkPostStmt(const BinaryOperator *BO,
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
      if (Optional<nonloc::ConcreteInt> NV = RHSVal.getAs<nonloc::ConcreteInt>())
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

PathDiagnosticPieceRef
CapabilityAlignmentChecker::AlignmentBugVisitor::VisitNode(
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

  ProgramStateRef State = N->getState();
  const int *NewAlign = State->get<TrailingZerosMap>(N->getSVal(E).getAsSymbol());
  if (!NewAlign)
    return nullptr;

  if (const CastExpr *CE = dyn_cast<CastExpr>(E)) {
    if (Sym != N->getSVal(S).getAsSymbol())
      return nullptr;
    if (SymbolRef SrcSym = N->getSVal(CE->getSubExpr()).getAsSymbol())
      if (const int *OldAlign = State->get<TrailingZerosMap>(SrcSym))
        if (*OldAlign == *NewAlign)
          return nullptr;

    OS << "Alignment: ";
    printAlign(OS, *NewAlign);
  } else if (const BinaryOperator *BO = dyn_cast<BinaryOperator>(E)) {
    BinaryOperatorKind const OpCode = BO->getOpcode();
    OS << "Alignment: ";
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
    printAlign(OS, *NewAlign);
  } else
    return nullptr;

  // Generate the extra diagnostic.
  PathDiagnosticLocation const Pos(S, BRC.getSourceManager(),
                                   N->getLocationContext());
  return std::make_shared<PathDiagnosticEventPiece>(Pos, OS.str(), true);
}

void ento::registerCapabilityAlignmentChecker(CheckerManager &mgr) {
  mgr.registerChecker<CapabilityAlignmentChecker>();
}

bool ento::shouldRegisterCapabilityAlignmentChecker(const CheckerManager &Mgr) {
  return true;
}