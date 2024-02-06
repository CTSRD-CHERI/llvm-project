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
#include <clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h>
#include <clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h>

using namespace clang;
using namespace ento;
using namespace cheri;

namespace {
class PointerAlignmentChecker
    : public Checker<check::PreStmt<CastExpr>, check::PostStmt<CastExpr>,
                     check::PostStmt<BinaryOperator>, check::Bind,
                     check::PreCall, check::DeadSymbols> {
  std::unique_ptr<BugType> CastAlignBug;
  std::unique_ptr<BugType> CapCastAlignBug;
  std::unique_ptr<BugType> GenPtrEscapeAlignBug;
  std::unique_ptr<BugType> MemcpyAlignBug;


  const CallDescriptionMap<std::pair<int, int>> MemCpyFn {
    {{"memcpy", 3}, {0, 1}},
    {{"mempcpy", 3}, {0, 1}},
    {{"memmove", 3}, {0, 1}},
  };

public:
  PointerAlignmentChecker();

  void checkPostStmt(const BinaryOperator *BO, CheckerContext &C) const;
  void checkPostStmt(const CastExpr *BO, CheckerContext &C) const;
  void checkPreStmt(const CastExpr *BO, CheckerContext &C) const;
  void checkBind(SVal L, SVal V, const Stmt *S, CheckerContext &C) const;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const;


private:
  ExplodedNode *emitAlignmentWarning(CheckerContext &C, const SVal &UnderalignedPtrVal,
                                  const BugType &BT,
                                  StringRef ErrorMessage,
                                  const ValueDecl *CapStorageDecl = nullptr,
                                  StringRef CapSrcMsg = StringRef()) const;

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
REGISTER_SET_WITH_PROGRAMSTATE(CapStorageSet, const MemRegion*)

PointerAlignmentChecker::PointerAlignmentChecker() {
  CastAlignBug.reset(new BugType(this,
      "Cast increases required alignment",
      "Type Error"));
  CapCastAlignBug.reset(new BugType(this,
      "Cast increases required alignment to capability alignment",
      "CHERI portability"));
  GenPtrEscapeAlignBug.reset(new BugType(this,
      "Not capability-aligned pointer stored as 'void*'",
      "CHERI portability"));
  MemcpyAlignBug.reset(new BugType(this,
      "Copying capabilities through underaligned memory",
      "CHERI portability"));
}

namespace {

Optional<QualType> globalOrParamPointeeType(SymbolRef Sym) {
  const MemRegion *BaseRegOrigin = Sym->getOriginRegion();
  if (!BaseRegOrigin)
    return llvm::None;

  if (!BaseRegOrigin->getMemorySpace()->hasGlobalsOrParametersStorage())
    return llvm::None;

  const QualType &SymTy = Sym->getType();
  if (SymTy->isPointerType()) {
    const QualType &PT = SymTy->getPointeeType();
    if (!PT->isIncompleteType()) {
      return PT;
    }
  }

  return llvm::None;
}

int getTrailingZerosCount(const SVal &V, ProgramStateRef State,
                          ASTContext &ASTCtx);

int getTrailingZerosCount(SymbolRef Sym, ProgramStateRef State,
                          ASTContext &ASTCtx) {
  const int *Align = State->get<TrailingZerosMap>(Sym);
  if (Align)
    return *Align;

  // Is function argument or global?
  Optional<QualType> GlobalPointeeTy = globalOrParamPointeeType(Sym);
  if (GlobalPointeeTy.hasValue()) {
    QualType &PT = GlobalPointeeTy.getValue();
    unsigned A = ASTCtx.getTypeAlignInChars(PT).getQuantity();
    return llvm::APSInt::getUnsigned(A).countTrailingZeros();
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
    if (const auto *DR = R->getAs<DeclRegion>()) {
      if (auto *AA = DR->getDecl()->getAttr<AlignedAttr>()) {
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

bool isCapabilityStorage(SymbolRef Sym, ASTContext &ASTCtx) {
  const Optional<QualType> GlobalPointeeTy = globalOrParamPointeeType(Sym);
  if (GlobalPointeeTy.hasValue())
    return hasCapability(GlobalPointeeTy.getValue(), ASTCtx);
  return false;
}

bool isCapabilityStorage(const MemRegion *R, ProgramStateRef State,
                         ASTContext &ASTCtx) {
  R = R->StripCasts();

  if (State->contains<CapStorageSet>(R))
    return true;

  if (const SymbolicRegion *SR = R->getAs<SymbolicRegion>())
    return isCapabilityStorage(SR->getSymbol(), ASTCtx);

  if (const TypedValueRegion *TR = R->getAs<TypedValueRegion>()) {
    const QualType PT = TR->getDesugaredValueType(ASTCtx);
    return hasCapability(PT, ASTCtx);
  }

  return false;
}

bool isCapabilityStorage(const SVal &V, ProgramStateRef State,
                         ASTContext &ASTCtx) {
  if (const MemRegion *MR = V.getAsRegion()) {
    return isCapabilityStorage(MR, State, ASTCtx);
  }
  if (SymbolRef Sym = V.getAsSymbol()) {
    return isCapabilityStorage(Sym, ASTCtx);
  }
  return false;
}

Optional<unsigned> getActualAlignment(CheckerContext &C, const SVal &SrcVal) {
  if (SrcVal.isConstant()) // special value
    return llvm::None;

  ASTContext &ASTCtx = C.getASTContext();
  int SrcTZC = getTrailingZerosCount(SrcVal, C.getState(), ASTCtx);
  if (SrcTZC < 0)
    return llvm::None;

  if ((unsigned)SrcTZC >= sizeof(unsigned int) * 8)
    return llvm::None; // Too aligned, probably zero
  return (1U << SrcTZC);
}

Optional<unsigned> getRequiredAlignment(ASTContext &ASTCtx,
                                        const QualType &PtrTy,
                                        bool AssumeCapAlignForVoidPtr) {
  if (!PtrTy->isPointerType())
    return llvm::None;
  const QualType &PointeeTy = PtrTy->getPointeeType();
  if (!PointeeTy->isIncompleteType())
    return ASTCtx.getTypeAlignInChars(PointeeTy).getQuantity();
  if (AssumeCapAlignForVoidPtr && isGenericPointerType(PtrTy, false))
    return getCapabilityTypeAlign(ASTCtx).getQuantity();
  return llvm::None;
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

bool isGenericStorage(SymbolRef Sym, QualType CopyTy) {
  if (!isGenericPointerType(CopyTy, false))
    return false;
  if (const MemRegion *R = Sym->getOriginRegion()) {
    const MemSpaceRegion *MS = R->getMemorySpace();
    if (isa<GlobalsSpaceRegion>(MS))
      return true; // global variable
    if (const auto *SR = dyn_cast<StackArgumentsSpaceRegion>(MS))
      return SR->getStackFrame()->inTopFrame(); // top-level argument

    if (isa<FieldRegion>(R))
      return true; // struct field
  }
  return false;
}

bool isGenericStorage(CheckerContext &C, const Expr *E) {
  if (!isGenericPointerType(E->IgnoreImpCasts()->getType(), false))
    return false;
  if (SymbolRef Sym = C.getSVal(E).getAsSymbol()) {
    return isGenericStorage(Sym, Sym->getType());
  }
  return false;
}

static void describeObjectType(raw_ostream &OS, const QualType &Ty,
                         const LangOptions &LangOpts) {
  if (Ty->isPointerType()) {
    OS << " pointed by '";
    Ty.print(OS, PrintingPolicy(LangOpts));
    OS << "' pointer";
  } else {
    OS << " of type '";
    Ty.print(OS, PrintingPolicy(LangOpts));
    OS << "'";
  }
}

const DeclRegion *getOriginalAllocation(const MemRegion *MR) {
  if (const DeclRegion *DR = MR->getAs<DeclRegion>())
    return DR;
  if (const ElementRegion *ER = MR->getAs<ElementRegion>())
    return getOriginalAllocation(ER->getSuperRegion());
  return nullptr;
}

bool hasCapStorageType(const Expr *E, ASTContext &ASTCtx) {
  const QualType &Ty = E->IgnoreCasts()->getType();
  return Ty->isPointerType() && hasCapability(Ty->getPointeeType(), ASTCtx);
}

} // namespace

void PointerAlignmentChecker::checkPreStmt(const CastExpr *CE,
                                               CheckerContext &C) const {
  CastKind CK = CE->getCastKind();
  if (CK != CastKind::CK_BitCast && CK != CK_IntegralToPointer)
    return;
  if (isImplicitConversionFromVoidPtr(CE, C))
    return;

  ASTContext &ASTCtx = C.getASTContext();

  if (hasCapStorageType(CE->getSubExpr(), ASTCtx)) {
    /* Src value must have been already checked for capability alignment by this time */
    return;
  }

  /* Calculate required alignment */
  const Optional<unsigned int> &DstReqAlign =
      getRequiredAlignment(ASTCtx, CE->getType(), false);
  if (!DstReqAlign.hasValue())
    return;

  /* Calculate actual alignment */
  const SVal &SrcVal = C.getSVal(CE->getSubExpr());
  const Optional<unsigned int> &SrcAlign = getActualAlignment(C, SrcVal);
  if (!SrcAlign.hasValue())
    return;

  if (SrcAlign >= DstReqAlign) // OK
    return;

  /* Emit warning */
  const QualType &DstTy = CE->getType();
  bool DstAlignIsCap = hasCapability(DstTy->getPointeeType(), ASTCtx);
  const BugType &BT= DstAlignIsCap ? *CapCastAlignBug : *CastAlignBug;

  SmallString<350> ErrorMessage;
  llvm::raw_svector_ostream OS(ErrorMessage);
  OS << "Pointer value aligned to a " << SrcAlign << " byte boundary"
     << " cast to type '" << DstTy.getAsString() << "'"
     << " with " << DstReqAlign << "-byte";
  if (DstAlignIsCap)
    OS << " capability";
  OS << " alignment";

  emitAlignmentWarning(C, SrcVal, BT, ErrorMessage);
}

void PointerAlignmentChecker::checkBind(SVal L, SVal V, const Stmt *S,
                                        CheckerContext &C) const {
  ASTContext &ASTCtx = C.getASTContext();
  if (!isPureCapMode(ASTCtx))
    return;

  const BinaryOperator *BO = dyn_cast<BinaryOperator>(S);
  if (!BO || !BO->isAssignmentOp())
    return;

  const QualType &DstTy = BO->getLHS()->getType();
  if (!DstTy->isCHERICapabilityType(ASTCtx, true))
    return;

  if (hasCapStorageType(BO->getRHS(), ASTCtx)) {
    /* Src value must have been already checked for capability alignment by this time */
    return;
  }

  /* Check if dst pointee type contains capabilities or is a generic storage type (can contain arbitrary data) */
  bool DstIsPtr2CapStorage = false, DstIsPtr2GenStorage = false;

  if (DstTy->isPointerType() && hasCapability(DstTy->getPointeeType(), ASTCtx))
    DstIsPtr2CapStorage = true;

  const ValueDecl *CapDstDecl = nullptr;
  if (const MemRegion *MR = L.getAsRegion()) {
    if (const TypedValueRegion *TR = MR->getAs<TypedValueRegion>()) {
      const QualType &Ty = TR->getValueType();
      DstIsPtr2CapStorage |= Ty->isPointerType() && hasCapability(Ty->getPointeeType(), ASTCtx);
      DstIsPtr2GenStorage |= isGenericPointerType(Ty, false) &&
                             (isa<GlobalsSpaceRegion>(TR->getMemorySpace()) || isa<FieldRegion>(TR->StripCasts()));
    }
    if (const DeclRegion *D = getOriginalAllocation(MR))
      CapDstDecl = D->getDecl();
  }

  if (SymbolRef Sym = L.getAsSymbol()) {
    const QualType &SymTy = Sym->getType();
    if (SymTy->isPointerType()) {
      const QualType &CopyTy = SymTy->getPointeeType();
      DstIsPtr2CapStorage |= CopyTy->isPointerType() && hasCapability(CopyTy->getPointeeType(), ASTCtx);
      DstIsPtr2GenStorage |= isGenericStorage(Sym, CopyTy);
    }
  }

  if (!DstIsPtr2CapStorage && !DstIsPtr2GenStorage)
    return;

  /* Source alignment */
  unsigned CapAlign = getCapabilityTypeAlign(ASTCtx).getQuantity();
  const Optional<unsigned int> &SrcAlign = getActualAlignment(C, V);
  if (!SrcAlign.hasValue() || SrcAlign >= CapAlign)
    return;

  if (!DstIsPtr2CapStorage) {
    /* Dst is generic pointer;
     * Skip if src pointee value is known and contains no capabilities  */
    if (const MemRegion *SrcMR = V.getAsRegion()) {
      if (const TypedValueRegion *SrcTR = SrcMR->StripCasts()->getAs<TypedValueRegion>()) {
        const QualType &SrcValTy = SrcTR->getValueType();
        const SVal &SrcDeref = C.getState()->getSVal(SrcMR, SrcValTy);
        SymbolRef DerefSym = SrcDeref.getAsSymbol();
        // Emit if SrcDeref is undef/unknown or represents initial value of this region
        if (!DerefSym || !DerefSym->getOriginRegion() || DerefSym->getOriginRegion()->StripCasts() != SrcTR)
          return;
      }
    }
  }

  /* Emit warning */
  SmallString<350> ErrorMessage;
  llvm::raw_svector_ostream OS(ErrorMessage);
  OS << "Pointer value aligned to a " << SrcAlign << " byte boundary";
  OS << " stored as type '" << BO->getLHS()->getType().getAsString() << "'";
  OS << ". Memory pointed by it";
  if (DstIsPtr2CapStorage)
    OS << " is supposed to";
  else
    OS << " may be used to";
  OS << " hold capabilities, for which " << CapAlign
     << "-byte capability alignment will be required";

  if (CapDstDecl) {
    SmallString<350> Note;
    llvm::raw_svector_ostream OS2(Note);
    OS2 << "Memory pointed by";
    if (DstIsPtr2CapStorage) {
      const QualType &AllocType = CapDstDecl->getType().getCanonicalType();
      OS2 << " '" << AllocType.getAsString() << "'" << " value is supposed to hold capabilities";
    } else
      OS2 << " this value may be used to hold capabilities";
    emitAlignmentWarning(C, V, *GenPtrEscapeAlignBug, ErrorMessage, CapDstDecl, Note);
  } else
    emitAlignmentWarning(C, V, *GenPtrEscapeAlignBug, ErrorMessage);
}

void PointerAlignmentChecker::checkPreCall(const CallEvent &Call,
                                         CheckerContext &C) const {
  ASTContext &ASTCtx = C.getASTContext();
  if (!isPureCapMode(ASTCtx))
    return;

  if (!Call.isGlobalCFunction())
    return;

  const std::pair<int, int> *MemCpyParamPair = MemCpyFn.lookup(Call);
  if (!MemCpyParamPair)
    return;

  unsigned CapAlign = getCapabilityTypeAlign(ASTCtx).getQuantity();

  /* Destination alignment */
  const Expr *DstExpr = Call.getArgExpr(MemCpyParamPair->first);
  const QualType &DstTy = DstExpr->IgnoreImplicit()->getType();
  const SVal &DstVal = C.getSVal(DstExpr);
  const Optional<unsigned> &DstCurAlign = getActualAlignment(C, DstVal);
  bool DstIsCapStorage = isCapabilityStorage(DstVal, C.getState(), ASTCtx);
  bool DstIsGenStorage = isGenericStorage(C, DstExpr);

  /* Source alignment */
  const Expr *SrcExpr = Call.getArgExpr(MemCpyParamPair->second);
  const QualType &SrcTy = SrcExpr->IgnoreImplicit()->getType();
  const SVal &SrcVal = C.getSVal(SrcExpr);
  const Optional<unsigned> &SrcCurAlign = getActualAlignment(C, SrcVal);
  bool SrcIsCapStorage = isCapabilityStorage(SrcVal, C.getState(), ASTCtx);
  bool SrcIsGenStorage = isGenericStorage(C, SrcExpr);

  if ((SrcIsCapStorage || SrcIsGenStorage)
      && DstCurAlign.hasValue() && DstCurAlign < CapAlign) {
    SmallString<350> ErrorMessage;
    llvm::raw_svector_ostream OS(ErrorMessage);
    OS << "Copied memory object";
    describeObjectType(OS, SrcTy, ASTCtx.getLangOpts());
    if (!SrcIsCapStorage)
        OS << " may contain";
    else
        OS << " contains";
    OS << " capabilities that require "
       << CapAlign << "-byte capability alignment.";
    OS << " Destination address alignment is " << DstCurAlign << "."
       << " Storing a capability at an underaligned address"
          " leads to tag stripping.";

    const ValueDecl *CapSrcDecl = nullptr;
    if (const MemRegion *SR = SrcVal.getAsRegion()) {
        if (const DeclRegion *D = getOriginalAllocation(SR))
          CapSrcDecl = D->getDecl();
    }

    if (CapSrcDecl) {
      SmallString<350> Note;
      llvm::raw_svector_ostream OS2(Note);
      const QualType &AllocType = CapSrcDecl->getType().getCanonicalType();
      OS2 << "Capabilities stored in " << "'" << AllocType.getAsString() << "'";
      emitAlignmentWarning(C, DstVal, *MemcpyAlignBug, ErrorMessage, CapSrcDecl, Note);
    } else
      emitAlignmentWarning(C, DstVal, *MemcpyAlignBug, ErrorMessage);
    return;
  }
  
  if ((DstIsCapStorage  || DstIsGenStorage)
      && SrcCurAlign.hasValue() && SrcCurAlign < CapAlign) {
    SmallString<350> ErrorMessage;
    llvm::raw_svector_ostream OS(ErrorMessage);
    OS << "Destination memory object";
    describeObjectType(OS, DstTy, ASTCtx.getLangOpts());
    if (!DstIsCapStorage)
        OS << " may";
    else
        OS << " is supposed to";
    OS << " contain capabilities that require "
       << CapAlign << "-byte capability alignment.";
    OS << " Source address alignment is " << SrcCurAlign << ", which means"
        " that copied object may have its capabilities tags"
        " stripped earlier due to underaligned storage.";


    const ValueDecl *CapDstDecl = nullptr;
    if (const MemRegion *SR = DstVal.getAsRegion()) {
        if (const DeclRegion *D = getOriginalAllocation(SR))
          CapDstDecl = D->getDecl();
    }

    if (CapDstDecl) {
      SmallString<350> Note;
      llvm::raw_svector_ostream OS2(Note);
      const QualType &AllocType = CapDstDecl->getType().getCanonicalType();
      OS2 << "Capabilities stored in " << "'" << AllocType.getAsString() << "'";
      emitAlignmentWarning(C, SrcVal, *MemcpyAlignBug, ErrorMessage, CapDstDecl, Note);
    } else
      emitAlignmentWarning(C, SrcVal, *MemcpyAlignBug, ErrorMessage);
    return;
  }

  /* Propagate CapStorage flag */
  if (SrcIsCapStorage && !DstIsCapStorage) {
    if (const MemRegion *R = DstVal.getAsRegion()) {
        const ProgramStateRef State =
            C.getState()->add<CapStorageSet>(R->StripCasts());
        const NoteTag *Tag =
            C.getNoteTag("Copied memory object contains capabilities");
        C.addTransition(State, C.getPredecessor(), Tag);
    }
  }

  if (!SrcIsCapStorage && DstIsCapStorage) {
    if (const MemRegion *R = SrcVal.getAsRegion()) {
        const ProgramStateRef State =
            C.getState()->add<CapStorageSet>(R->StripCasts());
        const NoteTag *Tag =
            C.getNoteTag("Copied memory object should contain capabilities");
        C.addTransition(State, C.getPredecessor(), Tag);
    }
  }
}

namespace {

bool isNonZeroShift(const SVal& V) {
  if (const MemRegion *MR = V.getAsRegion())
    if (auto *ER = MR->getAs<ElementRegion>())
      if (!ER->getIndex().isZeroConstant())
          return true;
  return false;
}

} // namespace

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
  bool DstIsCapStorage = false;
  if (CE->getType()->isPointerType()) {
    if (!isGenericPointerType(CE->getType(), true)) {
      const QualType &DstPTy = CE->getType()->getPointeeType();
      if (!DstPTy->isIncompleteType()) {
        unsigned ReqAl = ASTCtx.getTypeAlignInChars(DstPTy).getQuantity();
        DstReqTZC = llvm::APSInt::getUnsigned(ReqAl).countTrailingZeros();
        DstIsCapStorage = hasCapability(DstPTy, ASTCtx);
      }
    }
  }

  SVal DstVal = C.getSVal(CE);
  SVal SrcVal = C.getSVal(CE->getSubExpr());
  ProgramStateRef State = C.getState();
  bool Updated = false;

  /* Update TrailingZerosMap */
  int NewAlign = std::max(SrcTZC, DstReqTZC);
  if (DstTZC < NewAlign) {
    if (DstVal.isUnknown()) {
        const LocationContext *LCtx = C.getLocationContext();
        DstVal = C.getSValBuilder().conjureSymbolVal(
            nullptr, CE, LCtx, CE->getType(), C.blockCount());
        State = State->BindExpr(CE, LCtx, DstVal);
    }
    if (SymbolRef Sym = DstVal.getAsSymbol()) {
        State = State->set<TrailingZerosMap>(Sym, NewAlign);
        Updated = true;
    }
  }

  /* Update CapStorageSet */
  const ProgramPointTag *Tag = nullptr;
  if (!isCapabilityStorage(DstVal, State, ASTCtx)) {
    if ((isCapabilityStorage(SrcVal, State, ASTCtx) && !isNonZeroShift(SrcVal))
        || DstIsCapStorage) {
        if (const MemRegion *R = DstVal.getAsRegion()) {
        State = State->add<CapStorageSet>(R->StripCasts());
        Updated = true;
        if (DstIsCapStorage)
          Tag = C.getNoteTag("Cast to capability-containing type");
        }
    }
  }

  if (Updated)
    C.addTransition(State, Tag);
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
  bool Updated = false;
  ProgramStateRef State = C.getState();

  TrailingZerosMapTy TZMap = State->get<TrailingZerosMap>();
  for (TrailingZerosMapTy::iterator I = TZMap.begin(), E = TZMap.end();
                                    I != E; ++I) {
    if (SymReaper.isDead(I->first)) {
      State = State->remove<TrailingZerosMap>(I->first);
      Updated = true;
    }
  }

  if (Updated)
    C.addTransition(State);
}

namespace {

void printAlign(raw_ostream &OS, unsigned TZC) {
  OS << "aligned(";
  if (TZC < sizeof(unsigned long)*8)
    OS << (1LU << TZC);
  else
    OS << "2^(" << TZC << ")";
  OS << ")";
}

void describeOriginalAllocation(const ValueDecl *SrcDecl,
                                PathDiagnosticLocation SrcLoc,
                                PathSensitiveBugReport &W,
                                ASTContext &ASTCtx) {
    SmallString<350> Note;
    llvm::raw_svector_ostream OS2(Note);
    const QualType &AllocType = SrcDecl->getType().getCanonicalType();
    OS2 << "Original allocation of type ";
    OS2 << "'" << AllocType.getAsString() << "'";
    OS2 << " which has an alignment requirement ";
    OS2 << ASTCtx.getTypeAlignInChars(AllocType).getQuantity();
    OS2 << " bytes";
    W.addNote(Note, SrcLoc);
}

void describeCapabilityStorage(const ValueDecl *CapDecl,
                                PathDiagnosticLocation SrcLoc,
                                PathSensitiveBugReport &W,
                                StringRef CapStorageMsg) {
    W.addNote(CapStorageMsg, SrcLoc);
}

} // namespace

ExplodedNode *
PointerAlignmentChecker::emitAlignmentWarning(
    CheckerContext &C,
    const SVal &UnderalignedPtrVal,
    const BugType &BT,
    StringRef ErrorMessage,
    const ValueDecl *CapStorageDecl, StringRef CapStorageMsg) const {
  ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
  if (!ErrNode)
    return nullptr;

  const ValueDecl *MRDecl = nullptr;
  PathDiagnosticLocation MRDeclLoc;
  if (const MemRegion *MR = UnderalignedPtrVal.getAsRegion()) {
    if (const DeclRegion *OriginalAlloc = getOriginalAllocation(MR)) {
      MRDecl = OriginalAlloc->getDecl();
      MRDeclLoc = PathDiagnosticLocation::create(MRDecl, C.getSourceManager());
    }
  }

  auto W = std::make_unique<PathSensitiveBugReport>(BT, ErrorMessage, ErrNode,
                                                    MRDeclLoc, MRDecl);

  W->markInteresting(UnderalignedPtrVal);
  if (SymbolRef S = UnderalignedPtrVal.getAsSymbol())
    W->addVisitor(std::make_unique<AlignmentBugVisitor>(S));

  if (MRDecl) {
    describeOriginalAllocation(MRDecl, MRDeclLoc, *W, C.getASTContext());
  }

  if (CapStorageDecl) {
    auto CapSrcDeclLoc =
        PathDiagnosticLocation::create(CapStorageDecl, C.getSourceManager());
    describeCapabilityStorage(CapStorageDecl, CapSrcDeclLoc, *W, CapStorageMsg);
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