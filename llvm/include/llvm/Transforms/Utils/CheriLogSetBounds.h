//===- CheriLogSetBoundsPass.h - A pass to log CSetBounds statistics ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_UTILS_CHERILOGSETBOUNDS_H
#define LLVM_TRANSFORMS_UTILS_CHERILOGSETBOUNDS_H

#include "llvm/IR/PassManager.h"

namespace llvm {
struct CheriLogSetBoundsPass : PassInfoMixin<CheriLogSetBoundsPass> {
  PreservedAnalyses run(Function &, FunctionAnalysisManager &);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_CHERILOGSETBOUNDS_H
