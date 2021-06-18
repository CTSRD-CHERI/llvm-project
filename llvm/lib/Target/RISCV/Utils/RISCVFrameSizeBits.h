//===- RISCVFrameSizeBits.h - Size bits for stack frames ------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_UTILS_RISCVFRAMESIZEBITS_H
#define LLVM_LIB_TARGET_RISCV_UTILS_RISCVFRAMESIZEBITS_H

#include <cstdint>

namespace llvm {
namespace RISCVFrameSizeBits {

/// Reports whether a stack frame is too large to be protected by the CDL
/// scheme.
bool frameTooLargeForCDL(uint64_t StackFrameSize);

/// Returns the number of bits of alignment we require for a given stack frame
/// size.
unsigned getNumBitsAlignmentRequired(uint64_t StackFrameSize);

/// Returns the stack frame size bits to emit for a function's frame pointer.
unsigned getFrameSizeBits(uint64_t StackFrameSize);

} // namespace RISCVFrameSizeBits
} // namespace llvm

#endif
