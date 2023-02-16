//===-- CapabilityCopyChecker.cpp - Capability Copy Checker -*- C++ -*-----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines checker that detects tag-stripping loads and stores that
// may be used to copy or swap capabilities
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {

struct CHERITagState {
private:
  enum Kind { Unknown, Tagged, MayBeTagged, Untagged } K;
  CHERITagState(Kind InK) : K(InK) { }
  
public:
  bool isTagged() const { return K == Tagged; }
  bool mayBeTagged() const { return K == MayBeTagged; }
  bool isUntagged() const { return K == Untagged; }
  bool isKnown() const { return K != Unknown; }

  static CHERITagState getTagged() { return CHERITagState(Tagged); }
  static CHERITagState getMayBeTagged() { return CHERITagState(MayBeTagged); }
  static CHERITagState getUntagged() { return CHERITagState(Untagged); }
  static CHERITagState getUnknown() { return CHERITagState(Unknown); }


  bool operator==(const CHERITagState &X) const {
    return K == X.K;
  }
  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddInteger(K);
  }
};

class CapabilityCopyChecker : public Checker<check::PostStmt<BinaryOperator>,
                                             check::Location,
                                             check::Bind> {
  std::unique_ptr<BugType> UseCapAsNonCap;
  std::unique_ptr<BugType> StoreCapAsNonCap;

public:
  CapabilityCopyChecker();

  void checkPostStmt(const BinaryOperator *BO, CheckerContext &C) const;
  void checkLocation(SVal l, bool isLoad, const Stmt *S,
                     CheckerContext &C) const;
  void checkBind(SVal L, SVal V, const Stmt *S, CheckerContext &C) const;

private:
  bool checkBinaryOpArg(CheckerContext &C, const Expr* E) const;

};
} // namespace

REGISTER_SET_WITH_PROGRAMSTATE(VoidPtrArgDeref, const MemRegion *)

CapabilityCopyChecker::CapabilityCopyChecker() {
  UseCapAsNonCap.reset(
      new BugType(this,
                  "Part of capability value used in binary operator",
                  "CHERI portability"));
  StoreCapAsNonCap.reset(
      new BugType(this,
                  "Tag-stripping copy of capability",
                  "CHERI portability"));
}

static bool isPointerToCapTy(const QualType Type, ASTContext &Ctx) {
  if (!Type->isPointerType())
    return false;
  return Type->getPointeeType()->isCHERICapabilityType(Ctx, true);
}

static bool isNonCapScalarType(QualType T, ASTContext &C) {
  if (!T->isScalarType())
    return false;
  if (T->isCHERICapabilityType(C, true))
    return false;
  return true;
}

static const MemRegion *stripNonCapShift(const MemRegion * R, ASTContext &ASTCtx) {
  const auto *ER = dyn_cast<ElementRegion>(R);
  if (!ER)
    return R;

  if (!isNonCapScalarType(ER->getValueType(), ASTCtx))
      return R;

  return ER->getSuperRegion();
}

static bool isVoidPtrArgRegion(const MemRegion *Reg) {
  // 1. Reg is pointed by void* symbol
  const SymbolicRegion *SymReg = Reg->getSymbolicBase();
  if (!SymReg)
      return false;

  SymbolRef Sym = SymReg->getSymbol();
  if (!Sym->getType()->isVoidPointerType())
      return false;

  // 2. void* symbol is function argument
  const MemRegion *BaseRegOrigin = Sym->getOriginRegion();
  return BaseRegOrigin
         && BaseRegOrigin->getMemorySpace()->hasStackParametersStorage();
}

static CHERITagState getTagState(SVal Val, CheckerContext &C) {
  if (Val.isUnknownOrUndef())
    return CHERITagState::getUnknown();

  if (Val.isConstant())
    return CHERITagState::getUntagged();

  if (Val.getAsRegion())
    return CHERITagState::getTagged();

  if (SymbolRef Sym = Val.getAsSymbol()) {
    if (const MemRegion *MR = Sym->getOriginRegion()) {
      const MemRegion *SuperReg = stripNonCapShift(MR, C.getASTContext());
      if (isVoidPtrArgRegion(SuperReg))
        return CHERITagState::getMayBeTagged();
      if (MR != SuperReg && isa<TypedValueRegion>(SuperReg)) {
        const SVal &SuperVal = C.getState()->getSVal(SuperReg);
        if (Val != SuperVal)
          return getTagState(SuperVal, C);
      }
    }
  }

  return CHERITagState::getUnknown();
}

static unsigned getCapabilityTypeSize(ASTContext &ASTCtx) {
  return ASTCtx.getTypeSize(ASTCtx.VoidPtrTy);
}

static CharUnits getCapabilityTypeAlign(ASTContext &ASTCtx) {
  return ASTCtx.getTypeAlignInChars(ASTCtx.VoidPtrTy);
}

void CapabilityCopyChecker::checkLocation(SVal l, bool isLoad, const Stmt *S,
                                          CheckerContext &C) const {
  if (!isLoad)
    return;

  const MemRegion *R = l.getAsRegion();
  if (!R || !R->hasStackParametersStorage())
    return;

  // Get ArgVal
  ProgramStateRef State = C.getState();
  QualType const Ty = l.getType(C.getASTContext());
  QualType const ArgValTy = Ty->getPointeeType();
  if (!ArgValTy->isVoidPointerType())
    return;

  /* Loading VoidPtr function argument */
  SVal ArgVal = State->getSVal(R, ArgValTy);
  const MemRegion *ArgValAsRegion = ArgVal.getAsRegion();
  if (!ArgValAsRegion)
    return;

  // If argument has value from caller, CharTy will not be used
  SVal ArgValDeref = State->getSVal(ArgValAsRegion, C.getASTContext().CharTy);

  const NoteTag *Tag;
  if (const auto *ArgValDerefAsRegion = ArgValDeref.getAsRegion()) {
    Tag = C.getNoteTag("void* argument points to capability");
    State = State->add<VoidPtrArgDeref>(ArgValDerefAsRegion);
  } else if (ArgValDeref.getAsSymbol()) {
    Tag = C.getNoteTag("void* argument may be a pointer to capability");
  } else
    return;

  C.addTransition(State, C.getPredecessor(), Tag);
}

static bool isCapabilityStorage(CheckerContext &C, const MemRegion *R) {
  const MemRegion *BaseReg = stripNonCapShift(R, C.getASTContext());
  if (const auto *SymR = dyn_cast<SymbolicRegion>(BaseReg)) {
    QualType const Ty = SymR->getSymbol()->getType();
    if (Ty->isVoidPointerType())
      return true;
    return isPointerToCapTy(Ty, C.getASTContext());
  }
  return false;
}

void CapabilityCopyChecker::checkBind(SVal L, SVal V, const Stmt *S,
                                      CheckerContext &C) const {
  const MemRegion *MR = L.getAsRegion();
  if (!MR)
    return;

  const QualType &PointeeTy = L.getType(C.getASTContext())->getPointeeType();
  if (!isNonCapScalarType(PointeeTy, C.getASTContext()))
    return;

  /* Non-capability scalar store */
  const CHERITagState &ValTag = getTagState(V, C);
  if ((ValTag.isTagged() || ValTag.mayBeTagged())
      && isCapabilityStorage(C, MR)) {
    /* Storing capability to capability storage as non-cap*/
    if (ExplodedNode *ErrNode = C.generateNonFatalErrorNode()) {
      auto W = std::make_unique<PathSensitiveBugReport>(
          *StoreCapAsNonCap, "Tag-stripping store of a capability",
          ErrNode);
      W->addRange(S->getSourceRange());
      C.emitReport(std::move(W));
      return;
    }
  }
}

bool CapabilityCopyChecker::checkBinaryOpArg(CheckerContext &C, const Expr* E) const {
  ASTContext &ASTCtx = C.getASTContext();
  if (!isNonCapScalarType(E->getType(), ASTCtx))
    return false;

  const SVal &Val = C.getSVal(E);
  const MemRegion *MR = Val.getAsRegion();
  if (!MR) {
    // Check if Val is a part of capability
    SymbolRef Sym = Val.getAsSymbol();
    if (!Sym)
      return false;
    const MemRegion *OrigRegion = Sym->getOriginRegion();
    if (!OrigRegion)
      return false;
    const MemRegion *SReg = stripNonCapShift(OrigRegion, C.getASTContext());
    const SVal &WiderVal = C.getState()->getSVal(SReg, ASTCtx.CharTy);
    MR = WiderVal.getAsRegion();
    if (!MR)
      return false;
  }

  if (C.getState()->contains<VoidPtrArgDeref>(MR)) {
    /* Pointer to capability passed as void* argument */
    if (ExplodedNode *ErrNode = C.generateNonFatalErrorNode()) {
      auto W = std::make_unique<PathSensitiveBugReport>(
          *UseCapAsNonCap,
          "Part of capability representation used as argument in binary "
          "operator",
          ErrNode);
      W->addRange(E->getSourceRange());
      C.emitReport(std::move(W));
      return true;
    }
  }
  return false;
}

void CapabilityCopyChecker::checkPostStmt(const BinaryOperator *BO,
                                          CheckerContext &C) const {
  if (BO->isAssignmentOp())
    return;

  checkBinaryOpArg(C, BO->getLHS()) || checkBinaryOpArg(C, BO->getRHS());
}

void ento::registerCapabilityCopyChecker(CheckerManager &mgr) {
  mgr.registerChecker<CapabilityCopyChecker>();

}

bool ento::shouldRegisterCapabilityCopyChecker(const CheckerManager &Mgr) {
  return true;
}
