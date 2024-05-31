//===-- CHERIUtils.h  -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_CHERI_CHERIUTILS_H
#define LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_CHERI_CHERIUTILS_H

#include "clang/StaticAnalyzer/Core/Checker.h"
#include <clang/StaticAnalyzer/Core/PathSensitive/ProgramState.h>
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"

namespace clang {
namespace ento {
namespace cheri {

bool isPureCapMode(const ASTContext &C);

bool isPointerToCapTy(const QualType Type, ASTContext &Ctx);

CharUnits getCapabilityTypeSize(ASTContext &ASTCtx);

CharUnits getCapabilityTypeAlign(ASTContext &ASTCtx);

bool isGenericPointerType(const QualType T, bool AcceptCharPtr = true);

bool hasCapability(const QualType OrigTy, ASTContext &Ctx);

void describeCast(raw_ostream &OS, const CastExpr *CE,
                  const LangOptions &LangOpts);

const DeclRegion *getAllocationDecl(const MemRegion *MR);

} // namespace cheri

template <typename C>
inline typename ProgramStateTrait<C>::key_type
getKey(const std::pair<typename ProgramStateTrait<C>::key_type,
                       typename ProgramStateTrait<C>::value_type> &P) {
  return P.first;
}

template <typename C>
inline typename ProgramStateTrait<C>::key_type
getKey(const typename ProgramStateTrait<C>::key_type &K) {
  return K;
}

inline bool isLive(SymbolReaper &SymReaper, const MemRegion *MR) {
  return SymReaper.isLiveRegion(MR);
}

inline bool isLive(SymbolReaper &SymReaper, SymbolRef Sym) {
  return SymReaper.isLive(Sym);
}

template <typename M>
ProgramStateRef cleanDead(ProgramStateRef State, SymbolReaper &SymReaper,
                         bool &Removed) {
  const typename ProgramStateTrait<M>::data_type &Map = State->get<M>();
  for (const auto &E : Map) {
    const typename ProgramStateTrait<M>::key_type &K = getKey<M>(E);
    if (isLive(SymReaper, K))
      continue;
    State = State->remove<M>(K);
    Removed = true;
  }
  return State;
}

} // namespace ento
} // namespace clang

#endif // LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_CHERI_CHERIUTILS_H
