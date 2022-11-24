//===-------- MipsELFStreamer.cpp - ELF Object Output ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MipsELFStreamer.h"
#include "MipsFixupKinds.h"
#include "MipsMCExpr.h"
#include "MipsOptionRecord.h"
#include "MipsTargetStreamer.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

MipsELFStreamer::MipsELFStreamer(MCContext &Context,
                                 std::unique_ptr<MCAsmBackend> MAB,
                                 std::unique_ptr<MCObjectWriter> OW,
                                 std::unique_ptr<MCCodeEmitter> Emitter)
    : MCELFStreamer(Context, std::move(MAB), std::move(OW),
                    std::move(Emitter)) {
  RegInfoRecord = new MipsRegInfoRecord(this, Context);
  MipsOptionRecords.push_back(
      std::unique_ptr<MipsRegInfoRecord>(RegInfoRecord));
}

void MipsELFStreamer::emitInstruction(const MCInst &Inst,
                                      const MCSubtargetInfo &STI) {
  MCELFStreamer::emitInstruction(Inst, STI);

  MCContext &Context = getContext();
  const MCRegisterInfo *MCRegInfo = Context.getRegisterInfo();

  for (unsigned OpIndex = 0; OpIndex < Inst.getNumOperands(); ++OpIndex) {
    const MCOperand &Op = Inst.getOperand(OpIndex);

    if (!Op.isReg())
      continue;

    unsigned Reg = Op.getReg();
    RegInfoRecord->SetPhysRegUsed(Reg, MCRegInfo);
  }

  createPendingLabelRelocs();
}

void MipsELFStreamer::emitCFIStartProcImpl(MCDwarfFrameInfo &Frame) {
  Frame.Begin = getContext().createTempSymbol();
  MCELFStreamer::emitLabel(Frame.Begin);
}

MCSymbol *MipsELFStreamer::emitCFILabel() {
  MCSymbol *Label = getContext().createTempSymbol("cfi", true);
  MCELFStreamer::emitLabel(Label);
  return Label;
}

void MipsELFStreamer::emitCFIEndProcImpl(MCDwarfFrameInfo &Frame) {
  Frame.End = getContext().createTempSymbol();
  MCELFStreamer::emitLabel(Frame.End);
}

void MipsELFStreamer::createPendingLabelRelocs() {
  MipsTargetELFStreamer *ELFTargetStreamer =
      static_cast<MipsTargetELFStreamer *>(getTargetStreamer());

  // FIXME: Also mark labels when in MIPS16 mode.
  if (ELFTargetStreamer->isMicroMipsEnabled()) {
    for (auto *L : Labels) {
      auto *Label = cast<MCSymbolELF>(L);
      getAssembler().registerSymbol(*Label);
      Label->setOther(ELF::STO_MIPS_MICROMIPS);
    }
  }

  Labels.clear();
}

void MipsELFStreamer::emitLabel(MCSymbol *Symbol, SMLoc Loc) {
  MCELFStreamer::emitLabel(Symbol);
  Labels.push_back(Symbol);
}

void MipsELFStreamer::switchSection(MCSection *Section,
                                    const MCExpr *Subsection) {
  MCELFStreamer::switchSection(Section, Subsection);
  Labels.clear();
}

void MipsELFStreamer::emitValueImpl(const MCExpr *Value, unsigned Size,
                                    SMLoc Loc) {
  MCELFStreamer::emitValueImpl(Value, Size, Loc);
  Labels.clear();
}

void MipsELFStreamer::EmitCheriCapabilityImpl(const MCSymbol *Symbol,
                                              const MCExpr *Addend,
                                              unsigned CapSize, SMLoc Loc) {
  assert(Addend && "Should have received a MCConstExpr(0) instead of nullptr");
  visitUsedSymbol(*Symbol);
  MCContext &Context = getContext();
  const MCSymbolRefExpr *SRE =
      MCSymbolRefExpr::create(Symbol, MCSymbolRefExpr::VK_None, Context, Loc);
  const MCBinaryExpr *CapExpr = MCBinaryExpr::createAdd(
      MipsMCExpr::create(MipsMCExpr::MEK_CHERI_CAP, SRE, Context), Addend,
      Context);

  // Pad to ensure that the capability is aligned
  emitValueToAlignment(Align(CapSize), 0, 1, 0);

  MCDataFragment *DF = new MCDataFragment();
  MCFixup cheriFixup = MCFixup::create(
      0, CapExpr, MCFixupKind(Mips::fixup_CHERI_CAPABILITY), Loc);
  DF->getFixups().push_back(cheriFixup);
  DF->getContents().resize(DF->getContents().size() + CapSize, '\xca');
  insert(DF);
}

void MipsELFStreamer::emitCheriIntcap(const MCExpr *Expr, unsigned CapSize,
                                      SMLoc Loc) {
  assert(CapSize == 16 && "Only CHERI-128 is supported");
  assert(!getContext().getAsmInfo()->isLittleEndian() &&
         "Only big-endian MIPS is supported");
  emitCheriIntcapGeneric(Expr, CapSize, Loc);
}

void MipsELFStreamer::emitIntValue(uint64_t Value, unsigned Size) {
  MCELFStreamer::emitIntValue(Value, Size);
  Labels.clear();
}

void MipsELFStreamer::EmitMipsOptionRecords() {
  for (const auto &I : MipsOptionRecords)
    I->EmitMipsOptionRecord();
}

MCELFStreamer *llvm::createMipsELFStreamer(
    MCContext &Context, std::unique_ptr<MCAsmBackend> MAB,
    std::unique_ptr<MCObjectWriter> OW, std::unique_ptr<MCCodeEmitter> Emitter,
    bool RelaxAll) {
  return new MipsELFStreamer(Context, std::move(MAB), std::move(OW),
                             std::move(Emitter));
}
