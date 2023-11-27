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

namespace clang {
namespace ento {
namespace cheri {

bool isPureCapMode(const ASTContext &C);

bool isPointerToCapTy(const QualType Type, ASTContext &Ctx);

CharUnits getCapabilityTypeSize(ASTContext &ASTCtx);

CharUnits getCapabilityTypeAlign(ASTContext &ASTCtx);

bool isGenericPointerType(const QualType T, bool AcceptCharPtr = true);

bool hasCapability(const QualType OrigTy, ASTContext &Ctx);

} // end of namespace: cheri
} // end of namespace: ento
} // end of namespace: clang

#endif // LLVM_CLANG_LIB_STATICANALYZER_CHECKERS_CHERI_CHERIUTILS_H
