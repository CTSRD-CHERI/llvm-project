//===- lib/MC/MCObjectWriter.cpp - MCObjectWriter implementation ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCFragment.h"
#include "llvm/MC/MCSymbol.h"
namespace llvm {
class MCSection;
}

using namespace llvm;

MCObjectWriter::~MCObjectWriter() = default;

bool MCObjectWriter::isSymbolRefDifferenceFullyResolved(
    const MCAssembler &Asm, const MCSymbolRefExpr *A, const MCSymbolRefExpr *B,
    bool InSet) const {
  // Modified symbol references cannot be resolved.
  if (A->getKind() != MCSymbolRefExpr::VK_None ||
      B->getKind() != MCSymbolRefExpr::VK_None)
    return false;

  const MCSymbol &SA = A->getSymbol();
  const MCSymbol &SB = B->getSymbol();
  assert(!SA.isUndefined() && !SB.isUndefined());
  if (!SA.getFragment() || !SB.getFragment())
    return false;

  return isSymbolRefDifferenceFullyResolvedImpl(Asm, SA, SB, InSet);
}

bool MCObjectWriter::isSymbolRefDifferenceFullyResolvedImpl(
    const MCAssembler &Asm, const MCSymbol &A, const MCSymbol &B,
    bool InSet) const {
  return isSymbolRefDifferenceFullyResolvedImpl(Asm, A, *B.getFragment(), InSet,
                                                false);
}

bool MCObjectWriter::isSymbolRefDifferenceFullyResolvedImpl(
    const MCAssembler &Asm, const MCSymbol &SymA, const MCFragment &FB,
    bool InSet, bool IsPCRel) const {
  const MCSection &SecA = SymA.getSection();
  const MCSection &SecB = *FB.getParent();
  // On ELF and COFF  A - B is absolute if A and B are in the same section.
  return &SecA == &SecB;
}

bool MCObjectWriter::fixupNeedsProvenance(const MCAssembler &Asm,
                                          const MCFixup *Fixup) const {
  const MCFixupKindInfo &FKI =
      Asm.getBackend().getFixupKindInfo(Fixup->getKind());

  return FKI.Flags & MCFixupKindInfo::FKF_Provenance;
}
