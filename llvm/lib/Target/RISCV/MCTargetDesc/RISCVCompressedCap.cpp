//===- RISCVCompressedCap.cpp - CHERI compression helpers ------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RISCVCompressedCap.h"
#include "llvm/CHERI/cheri-compressed-cap/cheri_compressed_cap.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

namespace RISCVCompressedCap {
uint64_t getRepresentableLength(uint64_t Length, bool IsRV64) {
  if (IsRV64) {
    return cc128_get_representable_length(Length);
  } else {
    return cc64_get_representable_length(Length);
  }
}

uint64_t getAlignmentMask(uint64_t Length, bool IsRV64) {
  if (IsRV64) {
    return cc128_get_alignment_mask(Length);
  } else {
    return cc64_get_alignment_mask(Length);
  }
}

TailPaddingAmount getRequiredTailPadding(uint64_t Size, bool IsRV64) {
  if (IsRV64) {
    return static_cast<TailPaddingAmount>(
        llvm::alignTo(Size, cc128_get_required_alignment(Size)) - Size);
  } else {
    return static_cast<TailPaddingAmount>(
        llvm::alignTo(Size, cc64_get_required_alignment(Size)) - Size);
  }
}

Align getRequiredAlignment(uint64_t Size, bool IsRV64) {
  if (IsRV64) {
    return Align(cc128_get_required_alignment(Size));
  } else {
    return Align(cc64_get_required_alignment(Size));
  }
}
} // namespace RISCVCompressedCap
} // namespace llvm
