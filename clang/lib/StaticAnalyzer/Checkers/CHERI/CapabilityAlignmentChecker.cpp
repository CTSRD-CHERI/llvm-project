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
#include <clang/StaticAnalyzer/Core/BugReporter/BugType.h>
#include <clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h>
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

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
};

} // namespace

REGISTER_MAP_WITH_PROGRAMSTATE(TrailingZerosMap, SymbolRef, int)

CapabilityAlignmentChecker::CapabilityAlignmentChecker() {
  CastAlignBug.reset(
      new BugType(this, "Cast increases required alignment", "CHERI portability"));
}

namespace {

int getTrailingZerosCount(const SVal &V, CheckerContext &C);

int getTrailingZerosCount(SymbolRef Sym, CheckerContext &C) {
  const int *Align = C.getState()->get<TrailingZerosMap>(Sym);
  if (Align)
    return *Align;

  // Is function argument or global?
  if (const MemRegion *BaseRegOrigin = Sym->getOriginRegion())
    if (BaseRegOrigin->getMemorySpace()->hasGlobalsOrParametersStorage()) {
      const QualType &SymTy = Sym->getType();
      if (SymTy->isPointerType() && !isGenericPointerType(SymTy)) {
        ASTContext &ASTCtx = C.getASTContext();
        const QualType &PT = SymTy->getPointeeType();
        if (!PT->isIncompleteType()) {
          unsigned A = ASTCtx.getTypeAlignInChars(PT).getQuantity();
          return llvm::APSInt::getUnsigned(A).countTrailingZeros();
        }
      }
  }

  return -1;
}

int getTrailingZerosCount(const MemRegion *R, CheckerContext &C) {
  R = R->StripCasts();

  if (const ElementRegion *ER = R->getAs<ElementRegion>()) {
    const MemRegion *Base = ER->getBaseRegion();
    int BaseTZC = -1;
    if (const SymbolicRegion *SymbBase = Base->getSymbolicBase())
      BaseTZC = getTrailingZerosCount(SymbBase->getSymbol(), C);
    else
      BaseTZC = getTrailingZerosCount(Base, C);
    int IdxTZC = getTrailingZerosCount(ER->getIndex(), C);
    if (BaseTZC >=0 && IdxTZC >= 0)
      return std::min(BaseTZC, IdxTZC);
  }

  if (R->getSymbolicBase())
    return -1;

  const TypedValueRegion *TR = R->getAs<TypedValueRegion>();
  if (!TR)
    return -1;

  ASTContext &ASTCtx = C.getASTContext();
  const QualType PT = TR->getDesugaredValueType(ASTCtx);
  if (PT->isIncompleteType())
    return -1;
  unsigned A = ASTCtx.getTypeAlignInChars(PT).getQuantity();
  return llvm::APSInt::getUnsigned(A).countTrailingZeros();
}

int getTrailingZerosCount(const SVal &V, CheckerContext &C) {
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
    int Res = getTrailingZerosCount(Sym, C);
    if (Res >=0)
        return Res;
  }

  if (const MemRegion *R = V.getAsRegion()) {
    return getTrailingZerosCount(R, C);
  }

  return -1;
}

int getTrailingZerosCount(const Expr *E, CheckerContext &C) {
  const SVal &V = C.getSVal(E);
  return getTrailingZerosCount(V, C);
}

} // namespace

void CapabilityAlignmentChecker::checkPreStmt(const CastExpr *CE,
                                               CheckerContext &C) const {
  if (CE->getCastKind() != CastKind::CK_BitCast)
    return;

  const Expr *Src = CE->getSubExpr();
  const QualType &DstType = CE->getType();
  if (!Src->getType()->isPointerType() || !DstType->isPointerType())
    return;
  const QualType &DstPointeeTy = DstType->getPointeeType();
  if (DstPointeeTy->isIncompleteType())
    return;

  const SVal &SrcVal = C.getSVal(CE->getSubExpr());
  if (SrcVal.isConstant())
    return; // special value
  int SrcTZC = getTrailingZerosCount(SrcVal, C);
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
        const NoteTag *Tag =
            C.getNoteTag([SrcTZC](PathSensitiveBugReport &BR) -> std::string {
              SmallString<80> Msg;
              llvm::raw_svector_ostream OS(Msg);
              OS << "Alignment: ";
              printAlign(OS, SrcTZC);
              return std::string(OS.str());
            });
        C.addTransition(State, C.getPredecessor(), Tag);
    }
  }
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
  if (ResVal.isUnknown()) {
    const LocationContext *LCtx = C.getLocationContext();
    ResVal = C.getSValBuilder().conjureSymbolVal(
        nullptr, BO, LCtx, BO->getType(), C.blockCount());
    State = State->BindExpr(BO, LCtx, ResVal);
  }

  SymbolRef ResSymb = ResVal.getAsSymbol();
  if (!ResSymb)
    return;


  const SVal &RHSVal = C.getSVal(BO->getRHS());
  int BitWidth = C.getASTContext().getTypeSize(BO->getType());
  int Res = 0;
  int RHSConst = 0;
  BinaryOperator::Opcode OpCode = BO->getOpcode();
  switch (OpCode) {
  case clang::BO_And:
  case clang::BO_AndAssign:
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

  State = State->set<TrailingZerosMap>(ResSymb, Res);
  const NoteTag *Tag =
      C.getNoteTag([LeftTZ, RightTZ, Res, OpCode]
                   (PathSensitiveBugReport &BR) -> std::string {
        SmallString<80> Msg;
        llvm::raw_svector_ostream OS(Msg);
        OS << "Alignment: ";
        printAlign(OS, LeftTZ);
        OS << " " << BinaryOperator::getOpcodeStr(OpCode) << " ";
        printAlign(OS, RightTZ);
        OS << " => ";
        printAlign(OS, Res);
        return std::string(OS.str());
      });
  C.addTransition(State, C.getPredecessor(), Tag);
}

void ento::registerCapabilityAlignmentChecker(CheckerManager &mgr) {
  mgr.registerChecker<CapabilityAlignmentChecker>();
}

bool ento::shouldRegisterCapabilityAlignmentChecker(const CheckerManager &Mgr) {
  return true;
}