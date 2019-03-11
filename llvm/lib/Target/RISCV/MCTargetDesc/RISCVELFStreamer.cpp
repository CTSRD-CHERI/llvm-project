//===-- RISCVELFStreamer.cpp - RISCV ELF Target Streamer Methods ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides RISCV specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "RISCVELFStreamer.h"
#include "MCTargetDesc/RISCVAsmBackend.h"
#include "RISCVFixupKinds.h"
#include "RISCVMCTargetDesc.h"
#include "Utils/RISCVBaseInfo.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCSubtargetInfo.h"

using namespace llvm;

// This part is for ELF object output.
RISCVTargetELFStreamer::RISCVTargetELFStreamer(MCStreamer &S,
                                               const MCSubtargetInfo &STI)
    : RISCVTargetStreamer(S) {
  MCAssembler &MCA = getStreamer().getAssembler();
  const FeatureBitset &Features = STI.getFeatureBits();
  auto &MAB = static_cast<RISCVAsmBackend &>(MCA.getBackend());
  RISCVABI::ABI ABI = MAB.getTargetABI();
  assert(ABI != RISCVABI::ABI_Unknown && "Improperly initialised target ABI");

  unsigned EFlags = MCA.getELFHeaderEFlags();

  if (Features[RISCV::FeatureStdExtC])
    EFlags |= ELF::EF_RISCV_RVC;

  switch (ABI) {
  case RISCVABI::ABI_ILP32:
  case RISCVABI::ABI_LP64:
    break;
  case RISCVABI::ABI_ILP32F:
  case RISCVABI::ABI_LP64F:
    EFlags |= ELF::EF_RISCV_FLOAT_ABI_SINGLE;
    break;
  case RISCVABI::ABI_ILP32D:
  case RISCVABI::ABI_LP64D:
    EFlags |= ELF::EF_RISCV_FLOAT_ABI_DOUBLE;
    break;
  case RISCVABI::ABI_ILP32E:
    EFlags |= ELF::EF_RISCV_RVE;
    break;
  case RISCVABI::ABI_Unknown:
    llvm_unreachable("Improperly initialised target ABI");
  }

  MCA.setELFHeaderEFlags(EFlags);
}

MCELFStreamer &RISCVTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}

void RISCVTargetELFStreamer::emitDirectiveOptionPush() {}
void RISCVTargetELFStreamer::emitDirectiveOptionPop() {}
void RISCVTargetELFStreamer::emitDirectiveOptionRVC() {}
void RISCVTargetELFStreamer::emitDirectiveOptionNoRVC() {}
void RISCVTargetELFStreamer::emitDirectiveOptionRelax() {}
void RISCVTargetELFStreamer::emitDirectiveOptionNoRelax() {}

void RISCVELFStreamer::EmitCheriIntcap(int64_t Value, unsigned CapSize, SMLoc) {
  unsigned XLenInBytes = Is64Bit ? 8 : 4;
  assert(CapSize == 2 * XLenInBytes);

  // Pad to ensure that the (u)intcap_t is aligned
  EmitValueToAlignment(CapSize, 0, 1, 0);

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
                                               const MCExpr *Addend,
                                               unsigned CapSize, SMLoc Loc) {
  assert(Addend && "Should have received a MCConstExpr(0) instead of nullptr");
  visitUsedSymbol(*Symbol);
  MCContext &Context = getContext();

  const MCSymbolRefExpr *SRE =
      MCSymbolRefExpr::create(Symbol, MCSymbolRefExpr::VK_None, Context, Loc);
  const MCBinaryExpr *CapExpr = MCBinaryExpr::createAdd(SRE, Addend, Context);

  // Pad to ensure that the capability is aligned
  EmitValueToAlignment(CapSize, 0, 1, 0);

  MCDataFragment *DF = new MCDataFragment();
  MCFixup CapFixup =
      MCFixup::create(0, CapExpr, MCFixupKind(RISCV::fixup_riscv_capability));
  DF->getFixups().push_back(CapFixup);
  DF->getContents().resize(DF->getContents().size() + CapSize, '\xca');
  insert(DF);
}
