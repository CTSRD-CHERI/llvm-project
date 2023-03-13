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
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

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
                                             check::Bind,
                                             check::BranchCondition> {
  std::unique_ptr<BugType> UseCapAsNonCap;
  std::unique_ptr<BugType> StoreCapAsNonCap;

public:
  CapabilityCopyChecker();

  void checkPostStmt(const BinaryOperator *BO, CheckerContext &C) const;
  void checkLocation(SVal l, bool isLoad, const Stmt *S,
                     CheckerContext &C) const;
  void checkBind(SVal L, SVal V, const Stmt *S, CheckerContext &C) const;
  void checkBranchCondition(const Stmt *Condition, CheckerContext &Ctx) const;

private:
  bool checkBinaryOpArg(CheckerContext &C, const Expr* E) const;

};
} // namespace

REGISTER_SET_WITH_PROGRAMSTATE(VoidPtrArgDeref, const MemRegion *)
REGISTER_LIST_WITH_PROGRAMSTATE(WhileBoundVar, SymbolRef)

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

static auto whileConditionMatcher() {
  using namespace clang::ast_matchers;

  // (--len)
  // FIXME: PreDec for do-while; PostDec for while
  auto U =
      unaryOperator(hasOperatorName("--"), hasUnaryOperand(expr().bind("len")));
  // (--len > 0)
  auto BO = binaryOperation(
      hasAnyOperatorName("!=", ">"),
      hasLHS(U),
      hasRHS(ignoringImplicit(integerLiteral(equals(0))))
  );
  return stmt(anyOf(U, BO));
}

static bool isInsideSmallConstantBoundLoop(const Stmt *S, CheckerContext &C,
                                           unsigned BlockSize) {
  ASTContext &ASTCtx = C.getASTContext();
  SValBuilder &SVB = C.getSValBuilder();
  unsigned CapSize = ASTCtx.getTypeSize(ASTCtx.VoidPtrTy);
  unsigned IterNum4CapCopy = CapSize / BlockSize;
  const NonLoc &ItVal = SVB.makeIntVal(IterNum4CapCopy, true);

  auto BoundVarList = C.getState()->get<WhileBoundVar>();
  for (auto &&V : BoundVarList) {
    auto SmallLoop =
        SVB.evalBinOpNN(C.getState(), clang::BO_GE, nonloc::SymbolVal(V), ItVal,
                        SVB.getConditionType());
    if (auto LC = SmallLoop.getAs<DefinedOrUnknownSVal>())
      if (!C.getState()->assume(*LC, true))
        return true;
    return false;
  }

  using namespace clang::ast_matchers;
  if (C.blockCount() == 1) {
    // first iter
    auto doWhileStmtMatcher = doStmt(hasCondition(whileConditionMatcher()));
    auto M = match(stmt(hasParent(doWhileStmtMatcher)), *S, C.getASTContext());
    if (!M.empty())
      return true; // decide on second iteration
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
  if (!ValTag.isTagged() && !ValTag.mayBeTagged())
    return;

  if (!isCapabilityStorage(C, MR))
    return;

  // Skip if loop bound is not big enough to copy capability
  unsigned BlockSize = C.getASTContext().getTypeSize(PointeeTy);
  if (isInsideSmallConstantBoundLoop(S, C, BlockSize))
    return;

  /* Storing capability to capability storage as non-cap*/
  if (ExplodedNode *ErrNode = C.generateNonFatalErrorNode()) {
    auto W = std::make_unique<PathSensitiveBugReport>(
        *StoreCapAsNonCap, "Tag-stripping store of a capability", ErrNode);
    W->addRange(S->getSourceRange());
    C.emitReport(std::move(W));
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

void CapabilityCopyChecker::checkBranchCondition(const Stmt *Condition,
                                                 CheckerContext &Ctx) const {
  using namespace clang::ast_matchers;
  // TODO: do-while, merge with second matcher
  auto childOfWhile = stmt(hasParent(stmt(anyOf(whileStmt(), doStmt()))));
  auto L = match(childOfWhile, *Condition, Ctx.getASTContext());
  if (L.empty())
    return;

  auto whileCond = whileConditionMatcher();

  auto M = match(whileCond, *Condition, Ctx.getASTContext());
  if (M.size() != 1)
    return;

  const SVal &CondVal = Ctx.getSVal(Condition);
  if (Ctx.getSVal(Condition).isUnknownOrUndef())
    return;

  ProgramStateRef ThenSt, ElseSt;
  std::tie(ThenSt, ElseSt) =
      Ctx.getState()->assume(CondVal.castAs<DefinedOrUnknownSVal>());
  for (auto I : M) {
    auto L = I.getNodeAs<Expr>("len");
    if (SymbolRef ISym = Ctx.getSVal(L).getAsSymbol()) {
      if (ThenSt)
        ThenSt = ThenSt->add<WhileBoundVar>(ISym);
      if (ElseSt)
        ElseSt = ElseSt->set<WhileBoundVar>(llvm::ImmutableList<SymbolRef>());
    } else
      return;
  }
  if (ThenSt)
    Ctx.addTransition(ThenSt);
  if (ElseSt)
    Ctx.addTransition(ElseSt);
}

void ento::registerCapabilityCopyChecker(CheckerManager &mgr) {
  mgr.registerChecker<CapabilityCopyChecker>();

}

bool ento::shouldRegisterCapabilityCopyChecker(const CheckerManager &Mgr) {
  return true;
}
