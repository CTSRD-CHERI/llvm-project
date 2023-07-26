//===-- CHERIUtils.h  -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CHERIUtils.h"

namespace clang {
namespace ento {
namespace cheri {

bool isPureCapMode(const ASTContext &C) {
  return C.getTargetInfo().areAllPointersCapabilities();
}

bool isPointerToCapTy(const QualType Type, ASTContext &Ctx) {
  if (!Type->isPointerType())
    return false;
  return Type->getPointeeType()->isCHERICapabilityType(Ctx, true);
}

CharUnits getCapabilityTypeSize(ASTContext &ASTCtx) {
  return ASTCtx.getTypeSizeInChars(ASTCtx.VoidPtrTy);
}

CharUnits getCapabilityTypeAlign(ASTContext &ASTCtx) {
  return ASTCtx.getTypeAlignInChars(ASTCtx.VoidPtrTy);
}

bool isGenericPointerType(const QualType T) {
  return T->isVoidPointerType() ||
         (T->isPointerType() && T->getPointeeType()->isCharType());
}

} // end of namespace: cheri
} // end of namespace: ento
} // end of namespace: clang