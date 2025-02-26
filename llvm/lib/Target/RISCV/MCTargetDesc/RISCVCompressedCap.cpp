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
uint64_t getRepresentableLength(uint64_t Length, bool IsRV64,
                                bool IsStdCheriRISCV) {
  if (IsRV64) {
    if (IsStdCheriRISCV)
      return cc128r_get_representable_length(Length);
    return cc128_get_representable_length(Length);
  }
  if (IsStdCheriRISCV)
    return cc64r_get_representable_length(Length);
  return cc64_get_representable_length(Length);
}

uint64_t getAlignmentMask(uint64_t Length, bool IsRV64, bool IsStdCheriRISCV) {
  if (IsRV64) {
    if (IsStdCheriRISCV)
      return cc128r_get_alignment_mask(Length);
    return cc128_get_alignment_mask(Length);
  }
  if (IsStdCheriRISCV)
    return cc64r_get_alignment_mask(Length);
  return cc64_get_alignment_mask(Length);
}

TailPaddingAmount getRequiredTailPadding(uint64_t Size, bool IsRV64,
                                         bool IsStdCheriRISCV) {
  Align ReqAlign = getRequiredAlignment(Size, IsRV64, IsStdCheriRISCV);
  return static_cast<TailPaddingAmount>(llvm::alignTo(Size, ReqAlign.value()) - Size);
}

Align getRequiredAlignment(uint64_t Size, bool IsRV64, bool IsStdCheriRISCV) {
  if (IsRV64) {
    if (IsStdCheriRISCV)
      return Align(cc128r_get_required_alignment(Size));
    return Align(cc128_get_required_alignment(Size));
  }
  if (IsStdCheriRISCV)
    return Align(cc64r_get_required_alignment(Size));
  return Align(cc64_get_required_alignment(Size));
}
} // namespace RISCVCompressedCap
} // namespace llvm
