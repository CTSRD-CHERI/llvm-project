//===- RISCVFrameSizeBits.cpp - Size bits for stack frames ----*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RISCVFrameSizeBits.h"

#include "llvm/Support/ErrorHandling.h"

namespace llvm {
namespace RISCVFrameSizeBits {

namespace {
constexpr unsigned MinAlignment = 6;
constexpr unsigned NumRepresentibleSizes = 7;
constexpr unsigned MinRepresentibleFrameSize = 1 << MinAlignment;
constexpr unsigned MaxRepresentibleFrameSize = MinRepresentibleFrameSize
                                               << (NumRepresentibleSizes - 1);
} // anonymous namespace

bool frameTooLargeForCDL(uint64_t StackFrameSize) {
  return StackFrameSize > MaxRepresentibleFrameSize;
}

unsigned getNumBitsAlignmentRequired(uint64_t StackFrameSize) {
  if (StackFrameSize > MaxRepresentibleFrameSize) {
    llvm_unreachable(
        "Stack frame too large to provide lifetime-based temporal safety");
  }

  unsigned Bits = MinAlignment;
  StackFrameSize = (StackFrameSize - 1) / MinRepresentibleFrameSize;
  while (StackFrameSize) {
    Bits++;
    StackFrameSize >>= 1;
  }
  return Bits;
}

unsigned getFrameSizeBits(uint64_t StackFrameSize) {
  return getNumBitsAlignmentRequired(StackFrameSize) - 5;
}

} // namespace RISCVFrameSizeBits
} // namespace llvm
