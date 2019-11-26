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

void MipsELFStreamer::EmitInstruction(const MCInst &Inst,
                                      const MCSubtargetInfo &STI) {
  MCELFStreamer::EmitInstruction(Inst, STI);

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

void MipsELFStreamer::EmitCFIStartProcImpl(MCDwarfFrameInfo &Frame) {
  Frame.Begin = getContext().createTempSymbol();
  MCELFStreamer::EmitLabel(Frame.Begin);
}

MCSymbol *MipsELFStreamer::EmitCFILabel() {
  MCSymbol *Label = getContext().createTempSymbol("cfi", true);
  MCELFStreamer::EmitLabel(Label);
  return Label;
}

void MipsELFStreamer::EmitCFIEndProcImpl(MCDwarfFrameInfo &Frame) {
  Frame.End = getContext().createTempSymbol();
  MCELFStreamer::EmitLabel(Frame.End);
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

void MipsELFStreamer::EmitLabel(MCSymbol *Symbol, SMLoc Loc) {
  MCELFStreamer::EmitLabel(Symbol);
  Labels.push_back(Symbol);
}

void MipsELFStreamer::SwitchSection(MCSection *Section,
                                    const MCExpr *Subsection) {
  MCELFStreamer::SwitchSection(Section, Subsection);
  Labels.clear();
}

void MipsELFStreamer::EmitValueImpl(const MCExpr *Value, unsigned Size,
                                    SMLoc Loc) {
  MCELFStreamer::EmitValueImpl(Value, Size, Loc);
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
  EmitValueToAlignment(CapSize, 0, 1, 0);

  MCDataFragment *DF = new MCDataFragment();
  MCFixup cheriFixup = MCFixup::create(
      0, CapExpr, MCFixupKind(Mips::fixup_CHERI_CAPABILITY), Loc);
  DF->getFixups().push_back(cheriFixup);
  DF->getContents().resize(DF->getContents().size() + CapSize, '\xca');
  insert(DF);
}

void MipsELFStreamer::EmitCheriIntcap(int64_t Value, unsigned CapSize, SMLoc) {
  assert(CapSize == 32 || CapSize == 16);
  // Pad to ensure that the (u)intcap_t is aligned
  EmitValueToAlignment(CapSize, 0, 1, 0);
  if (Value == 0) {
    EmitZeros(CapSize);
  } else {
    // TODO: we should probably move the CHERI capability encoding somewhere else.
    // Maybe to BinaryFormat or Object?
    EmitIntValue(0, 8);
    EmitIntValue(Value, 8);
    if (CapSize == 32) {
      EmitIntValue(0, 8);
      EmitIntValue(0, 8);
    }
  }
}

void MipsELFStreamer::EmitIntValue(uint64_t Value, unsigned Size) {
  MCELFStreamer::EmitIntValue(Value, Size);
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
