//===-- RISCVELFStreamer.cpp - ELF Object Output for RISCV ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RISCVELFStreamer.h"
#include "RISCVFixupKinds.h"

using namespace llvm;

void RISCVELFStreamer::EmitCheriIntcap(int64_t Value, unsigned CapSize, SMLoc) {
  unsigned XLenInBytes = Is64Bit ? 8 : 4;
  assert(CapSize == 2 * XLenInBytes);

  if (Value == 0) {
    EmitZeros(CapSize);
  } else {
    // TODO: we should probably move the CHERI capability encoding somewhere else.
    // Maybe to BinaryFormat or Object?
    // NB: Opposite order to MIPS due to endianness.
    EmitIntValue(Value, XLenInBytes);
    EmitIntValue(0, XLenInBytes);
  }
}

void RISCVELFStreamer::EmitCheriCapabilityImpl(const MCSymbol *Symbol,
                                              int64_t Offset, unsigned CapSize,
                                              SMLoc Loc) {
  visitUsedSymbol(*Symbol);
  MCContext &Context = getContext();

  const MCSymbolRefExpr *SRE =
      MCSymbolRefExpr::create(Symbol, MCSymbolRefExpr::VK_None, Context);
  const MCBinaryExpr *CapExpr =
      MCBinaryExpr::createAdd(SRE,
                              MCConstantExpr::create(Offset, Context),
                              Context);

  const unsigned ByteAlignment = CapSize;
  insert(new MCAlignFragment(ByteAlignment, 0, 1, ByteAlignment));
  // Update the maximum alignment on the current section if necessary.
  MCSection *CurSec = getCurrentSectionOnly();
  if (ByteAlignment > CurSec->getAlignment())
    CurSec->setAlignment(ByteAlignment);

  MCDataFragment *DF = new MCDataFragment();
  MCFixup CapFixup =
      MCFixup::create(0, CapExpr, MCFixupKind(RISCV::fixup_riscv_capability));
  DF->getFixups().push_back(CapFixup);
  DF->getContents().resize(DF->getContents().size() + CapSize, '\xca');
  insert(DF);
}
