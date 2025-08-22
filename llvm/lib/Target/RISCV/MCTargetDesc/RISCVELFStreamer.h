//===-- RISCVELFStreamer.h - RISC-V ELF Target Streamer ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVELFSTREAMER_H
#define LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVELFSTREAMER_H

#include "RISCVTargetStreamer.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCObjectWriter.h"

using namespace llvm;

class RISCVELFStreamer : public MCELFStreamer {
  void reset() override;

  int64_t MappingSymbolCounter = 0;

  enum ElfMappingSymbol {
    EMS_None,
    EMS_CapMode,
    EMS_NoCapMode,
  };

  struct ElfMappingSymbolInfo {
    ElfMappingSymbolInfo(SMLoc Loc, MCFragment *F, uint64_t O)
        : Loc(Loc), F(F), Offset(O), State(EMS_None) {}
    void ResetInfo() {
      F = nullptr;
      Offset = 0;
    }
    bool hasInfo() { return F != nullptr; }
    SMLoc Loc;
    MCFragment *F;
    uint64_t Offset;
    ElfMappingSymbol State;
  };
  void EmitCapModeMappingSymbol();
  void EmitNoCapModeMappingSymbol();
  void EmitMappingSymbol(StringRef Name);

public:
  RISCVELFStreamer(MCContext &C, std::unique_ptr<MCAsmBackend> MAB,
                   std::unique_ptr<MCObjectWriter> MOW,
                   std::unique_ptr<MCCodeEmitter> MCE)
      : MCELFStreamer(C, std::move(MAB), std::move(MOW), std::move(MCE)) {}
  void emitCheriIntcap(const MCExpr *Expr, unsigned CapSize,
                       SMLoc Loc) override;

protected:
  void emitCheriCapability(const MCExpr *Value, unsigned CapSize,
                           SMLoc Loc) override;

  void changeSection(MCSection *Section, const MCExpr *Subsection) override;
  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;

private:
  DenseMap<const MCSection *, std::unique_ptr<ElfMappingSymbolInfo>>
      LastMappingSymbols;
  std::unique_ptr<ElfMappingSymbolInfo> LastEMSInfo;
};

namespace llvm {

class RISCVTargetELFStreamer : public RISCVTargetStreamer {
private:
  StringRef CurrentVendor;

  MCSection *AttributeSection = nullptr;
  const MCSubtargetInfo &STI;

  void emitAttribute(unsigned Attribute, unsigned Value) override;
  void emitTextAttribute(unsigned Attribute, StringRef String) override;
  void emitIntTextAttribute(unsigned Attribute, unsigned IntValue,
                            StringRef StringValue) override;
  void finishAttributeSection() override;

  void reset() override;

public:
  RISCVELFStreamer &getStreamer();
  RISCVTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

  void emitDirectiveOptionPush() override;
  void emitDirectiveOptionPop() override;
  void emitDirectiveOptionPIC() override;
  void emitDirectiveOptionNoPIC() override;
  void emitDirectiveOptionRVC() override;
  void emitDirectiveOptionNoRVC() override;
  void emitDirectiveOptionRelax() override;
  void emitDirectiveOptionNoRelax() override;
  void emitDirectiveOptionCapMode() override;
  void emitDirectiveOptionNoCapMode() override;
  void emitDirectiveVariantCC(MCSymbol &Symbol) override;

  void finish() override;
};

MCELFStreamer *createRISCVELFStreamer(MCContext &C,
                                      std::unique_ptr<MCAsmBackend> MAB,
                                      std::unique_ptr<MCObjectWriter> MOW,
                                      std::unique_ptr<MCCodeEmitter> MCE,
                                      bool RelaxAll);
}
#endif
