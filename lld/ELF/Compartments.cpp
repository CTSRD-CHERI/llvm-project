//===- Compartments.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains support for sub-object compartments.
//
//===----------------------------------------------------------------------===//

#include "Compartments.h"
#include "InputFiles.h"
#include "SyntheticSections.h"
#include "Writer.h"
#include "Arch/Cheri.h"

using namespace llvm;

namespace lld {
namespace elf {

void addCompartment(StringRef name) {
    compartments.emplace_back();
    Compartment &newCompart = compartments.back();
    newCompart.name = name;
    newCompart.index = compartments.size();
}

Compartment *findCompartment(StringRef name) {
  if (name.empty())
    return nullptr;
  for (Compartment &compart : compartments) {
    if (compart.name.compare(name) == 0)
      return &compart;
  }
  return nullptr;
}

void assignSectionsToCompartments() {
  for (Compartment &c : compartments) {
    c.suffix = "." + c.name.str();
  }

  for (InputSectionBase *s : ctx.inputSections) {
    if (s->file == nullptr) {
      continue;
    }

    Compartment *c = s->file->compartment;
    if (c == nullptr) {
      continue;
    }

    if (s->name.startswith(".text") || isRelroSection(s->name)) {
      s->compartment = c;
    }
  }
}

}
}
