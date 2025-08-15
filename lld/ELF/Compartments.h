//===- Compartments.h -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLD_ELF_COMPARTMENTS_H
#define LLD_ELF_COMPARTMENTS_H

#include "lld/Common/LLVM.h"
#include "llvm/Support/MemoryBufferRef.h"

namespace lld {
namespace elf {

struct Compartment;
class InputSectionBase;
class Symbol;

void readCompartmentPolicy(MemoryBufferRef mb);

void assignSectionsToCompartments();

void verifyExecSymbol(const Compartment &c, const InputSectionBase &sec,
                      const Symbol &sym, uint64_t off);
void verifyAccessSymbol(const Compartment &c, const InputSectionBase &sec,
                        const Symbol &sym, uint64_t off);

} // namespace elf
} // namespace lld

#endif
