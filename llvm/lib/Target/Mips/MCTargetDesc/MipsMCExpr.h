//===- MipsMCExpr.h - Mips specific MC expression classes -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCEXPR_H
#define LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCEXPR_H

#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCValue.h"

namespace llvm {

class MipsMCExpr : public MCTargetExpr {
public:
  enum MipsExprKind {
    MEK_None,
    MEK_CALL_HI16,
    MEK_CALL_LO16,
    MEK_DTPREL_HI,
    MEK_DTPREL_LO,
    MEK_GOT,
    MEK_GOTTPREL,
    MEK_GOT_CALL,
    MEK_GOT_DISP,
    MEK_GOT_HI16,
    MEK_GOT_LO16,
    MEK_GOT_OFST,
    MEK_GOT_PAGE,
    MEK_GPREL,
    MEK_HI,
    MEK_HIGHER,
    MEK_HIGHEST,
    MEK_LO,
    MEK_NEG,
    MEK_PCREL_HI16,
    MEK_PCREL_LO16,
    MEK_TLSGD,
    MEK_TLSLDM,
    MEK_TPREL_HI,
    MEK_TPREL_LO,
    MEK_CAPTABLE11,
    MEK_CAPTABLE_HI16,
    MEK_CAPTABLE_LO16,
    MEK_CAPTABLE20,
    MEK_CAPCALL11,
    MEK_CAPCALL_HI16,
    MEK_CAPCALL_LO16,
    MEK_CAPCALL20,

    MEK_CHERI_CAP,
    // Like GPREL but the offset from _CHERI_CAPABILITY_TABLE_ to symbol
    MEK_CAPTABLEREL,

    MEK_CAPTAB_TLSGD_HI16,
    MEK_CAPTAB_TLSGD_LO16,
    MEK_CAPTAB_TLSLDM_HI16,
    MEK_CAPTAB_TLSLDM_LO16,
    MEK_CAPTAB_TPREL_HI16,
    MEK_CAPTAB_TPREL_LO16,

    MEK_Special,
  };

private:
  const MipsExprKind Kind;
  const MCExpr *Expr;

  explicit MipsMCExpr(MipsExprKind Kind, const MCExpr *Expr)
      : Kind(Kind), Expr(Expr) {}

public:
  static const MipsMCExpr *create(MipsExprKind Kind, const MCExpr *Expr,
                                  MCContext &Ctx);
  static const MipsMCExpr *createGpOff(MipsExprKind Kind, const MCExpr *Expr,
                                       MCContext &Ctx);
 static const MipsMCExpr *createCaptableOff(MipsExprKind Kind,
                                            const MCExpr *Expr, MCContext &Ctx);

  /// Get the kind of this expression.
  MipsExprKind getKind() const { return Kind; }

  /// Get the child of this expression.
  const MCExpr *getSubExpr() const { return Expr; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;

  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  bool isGpOff(MipsExprKind &Kind) const { return isOffImpl(Kind, MEK_GPREL); }
  bool isGpOff() const {
    MipsExprKind Kind;
    return isGpOff(Kind);
  }

  bool isCaptableOff() const {
    MipsExprKind Kind;
    return isOffImpl(Kind, MEK_CAPTABLEREL);
  }
private:
  bool isOffImpl(MipsExprKind &Kind, MipsExprKind Expected) const;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCEXPR_H
