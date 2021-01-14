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
    // TODO: cc64 helpers
    llvm_unreachable("RV32 getRepresentableLength requires cc64_* helper");
  }
}

uint64_t getAlignmentMask(uint64_t Length, bool IsRV64) {
  if (IsRV64) {
    return cc128_get_alignment_mask(Length);
  } else {
    // TODO: cc64 helpers
    llvm_unreachable("RV32 getAlignmentMask requires cc64_* helper");
  }
}

TailPaddingAmount getRequiredTailPadding(uint64_t Size, bool IsRV64) {
  if (IsRV64) {
    return static_cast<TailPaddingAmount>(
        llvm::alignTo(Size, cc128_get_required_alignment(Size)) - Size);
  } else {
    // TODO: cc64 helpers
    return TailPaddingAmount::None;
  }
}

Align getRequiredAlignment(uint64_t Size, bool IsRV64) {
  if (IsRV64) {
    return Align(cc128_get_required_alignment(Size));
  } else {
    // TODO: cc64 helpers
    return Align();
  }
}
} // namespace RISCVCompressedCap
} // namespace llvm
