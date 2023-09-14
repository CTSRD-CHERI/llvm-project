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

#include "CHERIUtils.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <clang/ASTMatchers/ASTMatchersInternal.h>
#include <clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h>

using namespace clang;
using namespace ento;
using namespace cheri;

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

class CapabilityCopyChecker :public Checker<check::Location, check::Bind,
                                            check::PostStmt<BinaryOperator>,
                                            check::PostStmt<ArraySubscriptExpr>,
                                            check::BranchCondition,
                                            check::PreCall> {

  std::unique_ptr<BugType> UseCapAsNonCap;
  std::unique_ptr<BugType> StoreCapAsNonCap;

  using CheckFn = std::function<void(const CapabilityCopyChecker *,
                                     const CallEvent &Call, CheckerContext &C)>;
  const CallDescriptionMap<int> CStringFn {
    {{"strlen", 1}, 1},
    {{"strdup", 1}, 1},
    {{"strcpy", 2}, 3},
    {{"strcat", 2}, 3}
  };

public:
  CapabilityCopyChecker();

  void checkLocation(SVal l, bool isLoad, const Stmt *S,
                     CheckerContext &C) const;
  void checkBind(SVal L, SVal V, const Stmt *S, CheckerContext &C) const;

  void checkPostStmt(const BinaryOperator *BO, CheckerContext &C) const;
  void checkPostStmt(const ArraySubscriptExpr *E, CheckerContext &C) const;
  void checkBranchCondition(const Stmt *Cond, CheckerContext &C) const;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  bool ReportForCharPtr = false;

private:
  ExplodedNode *checkBinaryOpArg(CheckerContext &C, const Expr* E) const;

};
} // namespace

REGISTER_SET_WITH_PROGRAMSTATE(VoidPtrArgDeref, const MemRegion *)
REGISTER_SET_WITH_PROGRAMSTATE(UnalignedPtr, const MemRegion *)
REGISTER_SET_WITH_PROGRAMSTATE(CString, const MemRegion *)
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

namespace {

bool isNonCapScalarType(QualType T, ASTContext &C) {
  if (!T->isScalarType())
    return false;
  if (T->isCHERICapabilityType(C, true))
    return false;
  return true;
}

const MemRegion *stripNonCapShift(const MemRegion *R, ASTContext &ASTCtx) {
  const auto *ER = dyn_cast<ElementRegion>(R);
  if (!ER)
    return R;

  if (!isNonCapScalarType(ER->getValueType(), ASTCtx))
    return R;

  return ER->getSuperRegion();
}

bool isVoidOrCharPtrArgRegion(const MemRegion *Reg, bool AcceptCharPtr) {
  if (!Reg)
    return false;

  // 1. Reg is pointed by void* symbol
  const SymbolicRegion *SymReg = Reg->getSymbolicBase();
  if (!SymReg)
    return false;

  SymbolRef Sym = SymReg->getSymbol();
  if (!isGenericPointerType(Sym->getType(), AcceptCharPtr))
    return false;

  // 2. void* symbol is function argument
  const MemRegion *BaseRegOrigin = Sym->getOriginRegion();
  return BaseRegOrigin &&
         BaseRegOrigin->getMemorySpace()->hasStackParametersStorage();
}

CHERITagState getTagState(SVal Val, CheckerContext &C, bool AcceptCharPtr,
                          bool AcceptUnaligned = true) {
  if (Val.isUnknownOrUndef())
    return CHERITagState::getUnknown();

  if (Val.isConstant())
    return CHERITagState::getUntagged();

  if (Val.getAsRegion())
    return CHERITagState::getTagged();

  if (SymbolRef Sym = Val.getAsSymbol()) {
    if (const MemRegion *MR = Sym->getOriginRegion()) {
      const ProgramStateRef S = C.getState();
      const MemRegion *SuperReg = stripNonCapShift(MR, C.getASTContext());
      if (isa<FieldRegion>(SuperReg))
        return CHERITagState::getUnknown(); // not a part of capability
      if (isVoidOrCharPtrArgRegion(SuperReg, AcceptCharPtr) &&
          (AcceptUnaligned || !S->contains<UnalignedPtr>(SuperReg)) &&
          !S->contains<CString>(SuperReg))
        return CHERITagState::getMayBeTagged();
      if (MR != SuperReg && isa<TypedValueRegion>(SuperReg)) {
        const SVal &SuperVal = C.getState()->getSVal(SuperReg);
        if (Val != SuperVal)
          return getTagState(SuperVal, C, AcceptCharPtr);
      }
    }
  }

  return CHERITagState::getUnknown();
}

bool isCapabilityStorage(CheckerContext &C, const MemRegion *R,
                         bool AcceptCharPtr,
                         bool AcceptUnaligned = true) {
  const MemRegion *BaseReg = stripNonCapShift(R, C.getASTContext());
  if (!AcceptUnaligned && C.getState()->contains<UnalignedPtr>(BaseReg))
    return false;
  if (C.getState()->contains<CString>(BaseReg))
    return false;
  if (const auto *SymR = dyn_cast<SymbolicRegion>(BaseReg)) {
    QualType const Ty = SymR->getSymbol()->getType();
    if (isGenericPointerType(Ty, AcceptCharPtr))
      return true;
    return isPointerToCapTy(Ty, C.getASTContext());
  }
  return false;
}

} //namespace

void CapabilityCopyChecker::checkLocation(SVal l, bool isLoad, const Stmt *S,
                                          CheckerContext &C) const {
  if (!isLoad)
    return;
  const ASTContext &ASTCtx = C.getASTContext();
  if (!isPureCapMode(ASTCtx))
    return;

  const MemRegion *R = l.getAsRegion();
  if (!R || !R->hasStackParametersStorage())
    return;

  // Get ArgVal
  ProgramStateRef State = C.getState();
  QualType const Ty = l.getType(ASTCtx);
  QualType const ArgValTy = Ty->getPointeeType();
  if (!isGenericPointerType(ArgValTy, ReportForCharPtr))
    return;

  /* Loading VoidPtr function argument */
  const SVal ArgVal = State->getSVal(R, ArgValTy);
  const MemRegion *ArgValAsRegion = ArgVal.getAsRegion();
  if (!ArgValAsRegion)
    return;

  // If argument has value from caller, CharTy will not be used
  const SVal ArgValDeref = State->getSVal(ArgValAsRegion, ASTCtx.CharTy);

  bool T;
  if (const auto *ArgValDerefAsRegion = ArgValDeref.getAsRegion()) {
    State = State->add<VoidPtrArgDeref>(ArgValDerefAsRegion);
    T = true;
  } else if (ArgValDeref.getAsSymbol()) {
    T = false;
  } else
    return;

  const NoteTag *Tag =
      C.getNoteTag([T, ArgValTy](PathSensitiveBugReport &BR) -> std::string {
        SmallString<80> Msg;
        llvm::raw_svector_ostream OS(Msg);
        OS << ArgValTy.getAsString();
        if (T)
          OS << " argument points to capability";
        else
          OS << " argument may be a pointer to capability";
        return std::string(OS.str());
      });
  C.addTransition(State, C.getPredecessor(), Tag);
}

namespace {


auto whileConditionMatcher() {
  using namespace clang::ast_matchers;

  // (--len)
  // FIXME: PreDec for do-while; PostDec for while
  auto U =
      unaryOperator(hasOperatorName("--"), hasUnaryOperand(expr().bind("len")));
  // (--len > 0)
  auto BO =
      binaryOperation(hasAnyOperatorName("!=", ">"), hasLHS(U),
                      hasRHS(ignoringImplicit(integerLiteral(equals(0)))));
  return stmt(anyOf(U, BO));
}

bool isInsideSmallConstantBoundLoop(const Stmt *S, CheckerContext &C, long N) {
  SValBuilder &SVB = C.getSValBuilder();
  const NonLoc &ItVal = SVB.makeIntVal(N, true);

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

CharUnits getNonCapShift(SVal V) {
  if (SymbolRef Sym = V.getAsSymbol())
    if (const MemRegion *MR = Sym->getOriginRegion())
      if (const auto *ER = dyn_cast<ElementRegion>(MR)) {
        const RegionRawOffset &Offset = ER->getAsArrayOffset();
        if (Offset.getRegion())
          return Offset.getOffset();
      }

  return CharUnits::Zero();
}

bool isPartOfCopyingSequence(const QualType &CopyTy, SVal V, const Stmt *S,
                             CheckerContext &C) {
  ASTContext &ASTCtx = C.getASTContext();
  const CharUnits CopySize = ASTCtx.getTypeSizeInChars(CopyTy);
  const CharUnits CapSize = getCapabilityTypeSize(ASTCtx);

  /* False if loop bound is not big enough to copy capability */
  const long N = CapSize.alignTo(CopySize) / CopySize;
  if (isInsideSmallConstantBoundLoop(S, C, N))
    return false;

  /* True if copy is in loop */
  using namespace clang::ast_matchers;
  auto L = anyOf(forStmt(), whileStmt(), doStmt());
  auto LB = expr(hasAncestor(stmt(L)));
  if (!match(LB, *S, C.getASTContext()).empty())
    return true;

  /* Sequence of individual assignments */
  CharUnits Shift = getNonCapShift(V);
  return (Shift + CopySize) >= CapSize;
}

bool isAssignmentInCondition(const Stmt *S, CheckerContext &C) {
  using namespace clang::ast_matchers;
  auto A = binaryOperation(hasOperatorName("=")).bind("T");
  auto L = anyOf(
        forStmt(hasCondition(expr(hasDescendant(A)))),
        whileStmt(hasCondition(expr(hasDescendant(A)))),
        doStmt(hasCondition(expr(hasDescendant(A))))
      );

  auto LB = expr(hasAncestor(stmt(L)), equalsBoundNode("T"));
  auto M = match(LB, *S, C.getASTContext());
  return !M.empty();
}

const MemRegion *isCapAlignCheck(const BinaryOperator *BO, CheckerContext &C) {
  if (BO->getOpcode() != clang::BO_And)
    return nullptr;

  const long CapAlignMask =
      getCapabilityTypeAlign(C.getASTContext()).getQuantity() - 1;

  const SVal &RHSVal = C.getSVal(BO->getRHS());
  if (!RHSVal.isConstant(CapAlignMask))
    return nullptr;

  return C.getSVal(BO->getLHS()).getAsRegion();
}

bool handleAlignmentCheck(const BinaryOperator *BO, ProgramStateRef State,
                          CheckerContext &C, bool AcceptCharPtr) {
  auto ExprPtrReg = isCapAlignCheck(BO, C);
  if (!ExprPtrReg)
    return false;

  if (!isVoidOrCharPtrArgRegion(ExprPtrReg, AcceptCharPtr))
    return false;

  SVal AndVal = C.getSVal(BO);
  if (AndVal.isUnknown()) {
    const LocationContext *LCtx = C.getLocationContext();
    AndVal = C.getSValBuilder().conjureSymbolVal(nullptr, BO, LCtx,
                                                 BO->getType(), C.blockCount());
    State = State->BindExpr(BO, LCtx, AndVal);
  }

  SValBuilder &SVB = C.getSValBuilder();
  auto PtrIsCapAligned = SVB.evalEQ(State, AndVal, SVB.makeIntVal(0, true));
  ProgramStateRef Aligned, NotAligned;
  std::tie(Aligned, NotAligned) =
      State->assume(PtrIsCapAligned.castAs<DefinedOrUnknownSVal>());

  if (NotAligned) {
    // If void* argument value is not capability aligned, then it cannot
    // be a pointer to capability
    NotAligned = NotAligned->add<UnalignedPtr>(ExprPtrReg->StripCasts());
    C.addTransition(NotAligned);
  }

  if (Aligned)
    C.addTransition(Aligned);

  return NotAligned || Aligned;
}
} //namespace

void CapabilityCopyChecker::checkBind(SVal L, SVal V, const Stmt *S,
                                      CheckerContext &C) const {
  ASTContext &ASTCtx = C.getASTContext();
  if (!isPureCapMode(ASTCtx))
    return;

  const MemRegion *MR = L.getAsRegion();
  if (!MR)
    return;

  const QualType &PointeeTy = L.getType(ASTCtx)->getPointeeType();
  if (!isNonCapScalarType(PointeeTy, ASTCtx))
    return;
  /* Non-capability scalar store */

  const CHERITagState &ValTag = getTagState(V, C, ReportForCharPtr);
  if (!ValTag.isTagged() && !ValTag.mayBeTagged())
    return;

  if (!isCapabilityStorage(C, MR, ReportForCharPtr))
    return;

  if (ValTag.mayBeTagged() && !isPartOfCopyingSequence(PointeeTy, V, S, C))
    return;

  // Suppress for `while ((*dst++=src++));`
  if (isAssignmentInCondition(S, C))
    return;

  /* Storing capability to capability storage as non-cap*/
  if (ExplodedNode *ErrNode = C.generateNonFatalErrorNode()) {
    auto W = std::make_unique<PathSensitiveBugReport>(
        *StoreCapAsNonCap, "Tag-stripping store of a capability", ErrNode);
    W->addRange(S->getSourceRange());
    C.emitReport(std::move(W));
  }
}

namespace {
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;

ProgramStateRef markOriginAsCStringForMatch(const Stmt *E, Matcher<Stmt> D,
                                            CheckerContext &C,
                                            bool AcceptCharPtr) {
  auto M = match(D, *E, C.getASTContext());

  bool Updated = false;
  ProgramStateRef State = C.getState();
  for (auto I : M) {
    auto CExpr = I.getNodeAs<Expr>("c");
    const SVal &CVal = C.getSVal(CExpr);

    const MemRegion *R = CVal.getAsRegion();
    if (!R)
      if (SymbolRef Sym = CVal.getAsSymbol())
        R = Sym->getOriginRegion();
    if (!R)
      continue;

    const MemRegion *BaseReg = stripNonCapShift(R, C.getASTContext());

    if (isVoidOrCharPtrArgRegion(BaseReg, AcceptCharPtr)) {
      /* It's probably a string, so it's not to be regarded as a potential
       * pointer to capability */
      State = State->add<CString>(BaseReg);
      Updated = true;
    }
  }
  return Updated ? State : nullptr;
}


bool checkForWhileBoundVar(const Stmt *Condition, CheckerContext &C,
                           ProgramStateRef PrevSt) {
  ASTContext &ASTCtx = C.getASTContext();

  using namespace clang::ast_matchers;
  // TODO: do-while, merge with second matcher
  auto childOfWhile = stmt(hasParent(stmt(anyOf(whileStmt(), doStmt()))));
  auto L = match(childOfWhile, *Condition, ASTCtx);
  if (L.empty())
    return false;

  auto whileCond = whileConditionMatcher();

  auto M = match(whileCond, *Condition, ASTCtx);
  if (M.size() != 1)
    return false;

  const SVal &CondVal = C.getSVal(Condition);
  if (C.getSVal(Condition).isUnknownOrUndef())
    return false;

  ProgramStateRef ThenSt, ElseSt;
  std::tie(ThenSt, ElseSt) =
      PrevSt->assume(CondVal.castAs<DefinedOrUnknownSVal>());
  for (auto I : M) {
    auto L = I.getNodeAs<Expr>("len");
    if (SymbolRef ISym = C.getSVal(L).getAsSymbol()) {
      if (ThenSt)
        ThenSt = ThenSt->add<WhileBoundVar>(ISym);
      if (ElseSt)
        ElseSt = ElseSt->set<WhileBoundVar>(llvm::ImmutableList<SymbolRef>());
    } else
      return false;
  }
  if (ThenSt)
    C.addTransition(ThenSt);
  if (ElseSt)
    C.addTransition(ElseSt);
  return true;
}
} // namespace

void CapabilityCopyChecker::checkBranchCondition(const Stmt *Cond,
                                                 CheckerContext &C) const {
  if (!isPureCapMode(C.getASTContext()))
    return;

  auto CharMatcher = expr(hasType(isAnyCharacter())).bind("c");
  auto CondMatcher = stmt(expr(ignoringParenCasts(CharMatcher)));
  const ProgramStateRef N =
      markOriginAsCStringForMatch(Cond, CondMatcher, C, ReportForCharPtr);
  bool updated = checkForWhileBoundVar(Cond, C, N ? N : C.getState());
  if (!updated && N)
    C.addTransition(N);
}

void CapabilityCopyChecker::checkPostStmt(const ArraySubscriptExpr *E,
                                          CheckerContext &C) const {
  if (!isPureCapMode(C.getASTContext()))
    return;

  const Expr *Index = E->getIdx();
  auto CharMatcher = expr(hasType(isAnyCharacter())).bind("c");
  auto IndexMatcher = stmt(expr(ignoringParenCasts(CharMatcher)));

  const ProgramStateRef N =
      markOriginAsCStringForMatch(Index, IndexMatcher, C, ReportForCharPtr);
  if (N)
    C.addTransition(N);
}

ExplodedNode *CapabilityCopyChecker::checkBinaryOpArg(CheckerContext &C,
                                                      const Expr* E) const {
  ASTContext &ASTCtx = C.getASTContext();
  if (!isPureCapMode(ASTCtx))
    return nullptr;
  if (!isNonCapScalarType(E->getType(), ASTCtx))
    return nullptr;

  const SVal &Val = C.getSVal(E);
  const MemRegion *MR = Val.getAsRegion();
  if (!MR) {
    // Check if Val is a part of capability
    SymbolRef Sym = Val.getAsSymbol();
    if (!Sym)
      return nullptr;
    const MemRegion *OrigRegion = Sym->getOriginRegion();
    if (!OrigRegion)
      return nullptr;
    const MemRegion *SReg = stripNonCapShift(OrigRegion, C.getASTContext());
    const SVal &WiderVal = C.getState()->getSVal(SReg, ASTCtx.CharTy);
    MR = WiderVal.getAsRegion();
    if (!MR)
      return nullptr;
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
      return ErrNode;
    }
  }
  return nullptr;
}

void CapabilityCopyChecker::checkPostStmt(const BinaryOperator *BO,
                                          CheckerContext &C) const {
  const ASTContext &ASTCtx = C.getASTContext();
  if (!isPureCapMode(ASTCtx))
    return;

  /* Check for capability repr bytes used in arithmetic */
  ExplodedNode *N = nullptr;
  if (!BO->isAssignmentOp() || BO->isCompoundAssignmentOp()) {
    if (!(N = checkBinaryOpArg(C, BO->getLHS())))
      N = checkBinaryOpArg(C, BO->getRHS());
  }
  if (!N)
    N = C.getPredecessor();
  ProgramStateRef State = N->getState();

  /* CString character check */
  using namespace clang::ast_matchers;
  auto CharMatcher = ignoringParenCasts(expr(hasType(isAnyCharacter())).bind("c"));
  auto Cmp = binaryOperation(isComparisonOperator(), has(CharMatcher));

  /* Not a c-string, but rather an array of individual bytes.
   * We shall handle it the same way as c-string */
  auto BitOps = hasAnyOperatorName("&", "&=", "^", "^=", "|", "|=");
  auto Arithm = binaryOperation(BitOps, has(CharMatcher));

  auto BOM = anyOf(Cmp, Arithm);
  const ProgramStateRef NewState =
      markOriginAsCStringForMatch(BO, BOM, C, ReportForCharPtr);
  if (NewState)
    State = NewState;

  bool updated = handleAlignmentCheck(BO, State, C, ReportForCharPtr);
  if (!updated && NewState)
    C.addTransition(NewState);
}

void CapabilityCopyChecker::checkPreCall(const CallEvent &Call,
                                        CheckerContext &C) const {
  if (!Call.isGlobalCFunction())
    return;

  const int *StrParams = CStringFn.lookup(Call);
  if (!StrParams)
    return;

  ProgramStateRef State = C.getState();
  int i = 0, P = *StrParams;
  while (P) {
    if (P & 1)
      if (const MemRegion *Str = Call.getArgSVal(i).getAsRegion())
        State = State->add<CString>(Str);
    i++;
    P >>= 1;
  }

  C.addTransition(State);
}

void ento::registerCapabilityCopyChecker(CheckerManager &mgr) {
  auto *Checker = mgr.registerChecker<CapabilityCopyChecker>();
  Checker->ReportForCharPtr = mgr.getAnalyzerOptions().getCheckerBooleanOption(
            Checker, "ReportForCharPtr");
}

bool ento::shouldRegisterCapabilityCopyChecker(const CheckerManager &Mgr) {
  return true;
}
