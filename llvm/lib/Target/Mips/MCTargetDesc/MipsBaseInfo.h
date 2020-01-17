//===-- MipsBaseInfo.h - Top level definitions for MIPS MC ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone helper functions and enum definitions for
// the Mips target useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSBASEINFO_H
#define LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSBASEINFO_H

#include "MipsFixupKinds.h"
#include "MipsMCTargetDesc.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

namespace Mips {
enum OperandType {
  /// Operands with register or 32-bit immediate
  OPERAND_CHERI_GPR_OR_DDC = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_CHERI_GPR_OR_NULL
};
}

/// MipsII - This namespace holds all of the target specific flags that
/// instruction info tracks.
///
namespace MipsII {
  /// Target Operand Flag enum.
  enum TOF {
    //===------------------------------------------------------------------===//
    // Mips Specific MachineOperand flags.

    MO_NO_FLAG,

    /// MO_GOT - Represents the offset into the global offset table at which
    /// the address the relocation entry symbol resides during execution.
    MO_GOT,

    /// MO_GOT_CALL - Represents the offset into the global offset table at
    /// which the address of a call site relocation entry symbol resides
    /// during execution. This is different from the above since this flag
    /// can only be present in call instructions.
    MO_GOT_CALL,

    /// MO_GPREL - Represents the offset from the current gp value to be used
    /// for the relocatable object file being produced.
    MO_GPREL,

    /// MO_ABS_HI/LO - Represents the hi or low part of an absolute symbol
    /// address.
    MO_ABS_HI,
    MO_ABS_LO,

    /// MO_TLSGD - Represents the offset into the global offset table at which
    // the module ID and TSL block offset reside during execution (General
    // Dynamic TLS).
    MO_TLSGD,

    /// MO_TLSLDM - Represents the offset into the global offset table at which
    // the module ID and TSL block offset reside during execution (Local
    // Dynamic TLS).
    MO_TLSLDM,
    MO_DTPREL_HI,
    MO_DTPREL_LO,

    /// MO_GOTTPREL - Represents the offset from the thread pointer (Initial
    // Exec TLS).
    MO_GOTTPREL,

    /// MO_TPREL_HI/LO - Represents the hi and low part of the offset from
    // the thread pointer (Local Exec TLS).
    MO_TPREL_HI,
    MO_TPREL_LO,

    // N32/64 Flags.
    MO_GPOFF_HI,
    MO_GPOFF_LO,
    MO_GOT_DISP,
    MO_GOT_PAGE,
    MO_GOT_OFST,

    /// MO_HIGHER/HIGHEST - Represents the highest or higher half word of a
    /// 64-bit symbol address.
    MO_HIGHER,
    MO_HIGHEST,

    /// MO_GOT_HI16/LO16, MO_CALL_HI16/LO16 - Relocations used for large GOTs.
    MO_GOT_HI16,
    MO_GOT_LO16,
    MO_CALL_HI16,
    MO_CALL_LO16,

    /// Helper operand used to generate R_MIPS_JALR
    MO_JALR,

    /// MO_PCREL_HI/LO - Represents the hi or low part of an PC-relative symbol
    /// address.
    MO_PCREL_HI,
    MO_PCREL_LO,

    /// CHERI capability relocations:
    MO_CAPTAB11,       // offset into the capability table
    MO_CAPTAB_CALL11,  // same as above but only for calls
    // same with 16-bit immediate shifted by 4 version using new clc/csc instructions:
    MO_CAPTAB20,
    MO_CAPTAB_CALL20,
    /// Same relocations with large offsets:
    MO_CAPTAB_LO16,
    MO_CAPTAB_HI16,
    MO_CAPTAB_CALL_LO16,
    MO_CAPTAB_CALL_HI16,

    // Offset to the capability table
    MO_CAPTABLE_OFF_HI,
    MO_CAPTABLE_OFF_LO,

    /// MO_CAPTAB_TLSGD* - Represents the offset into the capability table at
    // which the module ID and TSL block offset reside during execution (General
    // Dynamic TLS).
    MO_CAPTAB_TLSGD_HI16,
    MO_CAPTAB_TLSGD_LO16,

    /// MO_CAPTAB_TLSLDM* - Represents the offset into the capability table at
    // which the module ID and TSL block offset reside during execution (Local
    // Dynamic TLS).
    MO_CAPTAB_TLSLDM_HI16,
    MO_CAPTAB_TLSLDM_LO16,

    /// MO_CAPTAB_TPREL* - Represents the offset into the capability table at
    // which the offset from the thread pointer resides during execution
    // (Initial Exec TLS).
    MO_CAPTAB_TPREL_HI16,
    MO_CAPTAB_TPREL_LO16,
  };

  enum {
    //===------------------------------------------------------------------===//
    // Instruction encodings.  These are the standard/most common forms for
    // Mips instructions.
    //

    // Pseudo - This represents an instruction that is a pseudo instruction
    // or one that has not been implemented yet.  It is illegal to code generate
    // it, but tolerated for intermediate implementation stages.
    Pseudo   = 0,

    /// FrmR - This form is for instructions of the format R.
    FrmR  = 1,
    /// FrmI - This form is for instructions of the format I.
    FrmI  = 2,
    /// FrmJ - This form is for instructions of the format J.
    FrmJ  = 3,
    /// FrmFR - This form is for instructions of the format FR.
    FrmFR = 4,
    /// FrmFI - This form is for instructions of the format FI.
    FrmFI = 5,
    /// FrmOther - This form is for instructions that have no specific format.
    FrmOther = 6,

    FormMask = 15,
    /// IsCTI - Instruction is a Control Transfer Instruction.
    IsCTI = 1 << 4,
    /// HasForbiddenSlot - Instruction has a forbidden slot.
    HasForbiddenSlot = 1 << 5,
    /// HasFCCRegOperand - Instruction uses an $fcc<x> register.
    HasFCCRegOperand = 1 << 6

  };

  enum OperandType : unsigned {
    OPERAND_FIRST_MIPS_MEM_IMM = MCOI::OPERAND_FIRST_TARGET,
    OPERAND_MEM_SIMM9 = OPERAND_FIRST_MIPS_MEM_IMM,
    OPERAND_LAST_MIPS_MEM_IMM = OPERAND_MEM_SIMM9
  };
}
}

#endif
