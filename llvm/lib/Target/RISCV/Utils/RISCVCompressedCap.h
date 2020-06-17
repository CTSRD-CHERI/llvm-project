//===- RISCVCompressedCap.h - CHERI compression helpers --------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_COMPRESSEDCAP_H
#define LLVM_LIB_TARGET_RISCV_COMPRESSEDCAP_H

#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/Alignment.h"
#include <cstdint>

namespace llvm {

namespace RISCVCompressedCap {
uint64_t getRepresentableLength(uint64_t Length, bool IsRV64);

uint64_t getAlignmentMask(uint64_t Length, bool IsRV64);

TailPaddingAmount getRequiredTailPadding(uint64_t Size, bool IsRV64);

Align getRequiredAlignment(uint64_t Size, bool IsRV64);
} // namespace RISCVCompressedCap
} // namespace llvm
#endif
