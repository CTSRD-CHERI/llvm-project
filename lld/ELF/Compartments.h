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

void addCompartment(StringRef name);

Compartment *findCompartment(StringRef name);

void readCompartmentPolicy(MemoryBufferRef mb);

void assignSectionsToCompartments();

void verifyExecSymbol(const Compartment *c, const Symbol &sym);
void verifyAccessSymbol(const Compartment *c, const Symbol &sym);
bool canWriteSymbol(const Compartment *c, const Symbol &sym);
bool canWriteSection(const Compartment *c, const InputSectionBase *isec);

} // namespace elf
} // namespace lld

#endif
