//===-- RISCVELFStreamer.h - RISCV ELF Target Streamer ---------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_RISCVELFSTREAMER_H
#define LLVM_LIB_TARGET_RISCV_RISCVELFSTREAMER_H

#include "RISCVTargetStreamer.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCObjectWriter.h"

namespace llvm {

class RISCVTargetELFStreamer : public RISCVTargetStreamer {
public:
  MCELFStreamer &getStreamer();
  RISCVTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

  virtual void emitDirectiveOptionPush();
  virtual void emitDirectiveOptionPop();
  virtual void emitDirectiveOptionRVC();
  virtual void emitDirectiveOptionNoRVC();
  virtual void emitDirectiveOptionRelax();
  virtual void emitDirectiveOptionNoRelax();
};

class RISCVELFStreamer : public MCELFStreamer {
  bool Is64Bit;

public:
  RISCVELFStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> MAB,
                   std::unique_ptr<MCObjectWriter> OW,
                   std::unique_ptr<MCCodeEmitter> Emitter, bool Is64Bit)
      : MCELFStreamer(Context, std::move(MAB), std::move(OW),
                      std::move(Emitter)),
        Is64Bit(Is64Bit) {}

  void EmitCheriIntcap(int64_t Value, unsigned CapSize, SMLoc Loc) override;

protected:
  void EmitCheriCapabilityImpl(const MCSymbol *Symbol, const MCExpr *Addend,
                               unsigned CapSize, SMLoc Loc) override;
};
}
#endif
