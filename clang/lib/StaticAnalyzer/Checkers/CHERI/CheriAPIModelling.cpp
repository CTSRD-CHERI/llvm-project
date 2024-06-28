//===-- AllocationChecker.cpp - Allocation Checker -*- C++ -*--------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Model CHERI API
//
//===----------------------------------------------------------------------===//

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


//namespace {

class CheriAPIModelling : public Checker<eval::Call> {
public:
  bool evalCall(const CallEvent &Call, CheckerContext &C) const;


  typedef void (CheriAPIModelling::*FnCheck)(CheckerContext &,
                                          const CallExpr *) const;
  CallDescriptionMap<FnCheck> Callbacks =  {
    {{"cheri_address_set", 2}, &CheriAPIModelling::evalAddrSet},
    {{"cheri_bounds_set", 2}, &CheriAPIModelling::evalBoundsSet},
    {{"cheri_bounds_set_exact", 2}, &CheriAPIModelling::evalBoundsSet},
    {{"cheri_perms_and", 2}, &CheriAPIModelling::evalBoundsSet},
    {{"cheri_tag_clear", 1}, &CheriAPIModelling::evalBoundsSet}

  };

  void evalBoundsSet(CheckerContext &C, const CallExpr *CE) const;
  void evalAddrSet(CheckerContext &C, const CallExpr *CE) const;
};

//} // namespace

void CheriAPIModelling::evalBoundsSet(CheckerContext &C,
                                    const CallExpr *CE) const {
  auto State = C.getState();
  SVal Cap = C.getSVal(CE->getArg(0));
  State = State->BindExpr(CE, C.getLocationContext(), Cap);
  C.addTransition(State);
}

void CheriAPIModelling::evalAddrSet(CheckerContext &C,
                                      const CallExpr *CE) const {
  auto State = C.getState();
  SVal Addr = C.getSVal(CE->getArg(1));
  State = State->BindExpr(CE, C.getLocationContext(), Addr);
  C.addTransition(State);
}



bool CheriAPIModelling::evalCall(const CallEvent &Call,
                                 CheckerContext &C) const {
  const auto *CE = dyn_cast_or_null<CallExpr>(Call.getOriginExpr());
  if (!CE)
    return false;

  const FnCheck *Handler = Callbacks.lookup(Call);
  if (!Handler)
    return false;

  (this->**Handler)(C, CE);
  return true;
}

//===----------------------------------------------------------------------===//
// Checker registration.
//===----------------------------------------------------------------------===//

void clang::ento::registerCheriAPIModelling(CheckerManager &Mgr) {
  Mgr.registerChecker<CheriAPIModelling>();
}

bool clang::ento::shouldRegisterCheriAPIModelling(const CheckerManager &Mgr) {
  return true;
}
