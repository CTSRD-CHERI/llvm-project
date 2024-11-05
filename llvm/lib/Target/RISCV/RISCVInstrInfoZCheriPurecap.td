//===-- RISCVInstrInfoZCheriPurecap.td - RISCV instructions -*- tblgen-*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// RISC-V specific DAG Nodes.
//===----------------------------------------------------------------------===//
// Target-dependent type requirements.


// Target-dependent nodes.


//===----------------------------------------------------------------------===//
// Operand and SDNode transformation definitions.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//

include "RISCVInstrFormatsZCheriPurecap.td"

//===----------------------------------------------------------------------===//
// Instruction Class Templates
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ZCheri_r<bits<5> funct5, string opcodestr, RegisterClass rdClass=GPR,
               DAGOperand rs1Operand=GPCR>
      : RVInstZCheriSrcDst<0x8, funct5, 0x0, OPC_OP, (outs rdClass:$rd),
                          (ins rs1Operand:$rs1), opcodestr, "$rd, $rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ZCheri_rr<bits<7> funct7, bits<3> funct3, string opcodestr,
                   RegisterClass rdClass=GPCR, RegisterClass rs1Class=GPCR,
                   RegisterClass rs2Class=GPR>
      : RVInstR<funct7, funct3, OPC_OP, (outs rdClass:$rd),
                (ins rs1Class:$rs1, rs2Class:$rs2), opcodestr, "$rd, $rs1, $rs2"
               >;
//===----------------------------------------------------------------------===//
// Capability-Inspection Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasZCheriPurecap] in {
def GCTAG  : ZCheri_r<0x0, "gctag" >;
def GCPERM : ZCheri_r<0x1, "gcperm">;
def GCHI   : ZCheri_r<0x4, "gchi"  >;
def GCBASE : ZCheri_r<0x5, "gcbase">;
def GCLEN  : ZCheri_r<0x6, "gclen" >;
def GCTYPE : ZCheri_r<0x2, "gctype">;
} // Predicates = [HasZCheriPurecap]

let Predicates = [HasZCheriHybrid] in {
def GCMODE : ZCheri_r<0x3, "gcmode">;
} // Predicates = [HasZCheriHybrid]

//===----------------------------------------------------------------------===//
// Capability-Modification Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasZCheriPurecap] in {
def SCADDR        : ZCheri_rr<0x6, 0x1, "scaddr">;
def ACPERM        : ZCheri_rr<0x6, 0x2, "acperm">;
def SCHI          : ZCheri_rr<0x6, 0x3, "schi">;
let isReMaterializable = 1, isAsCheapAsAMove = 1 in {
def CADD          : CheriRVInstR<0x6, 0x0, OPC_OP, "cadd", GPCR, GPCR, GPRNoX0>;
def CADDI         : CheriRVInstrI<0x2, OPC_OP_IMM_32, "caddi", 1>;
}
let defsCanBeSealed = 1 in {
def SENTRY        : ZCheri_r<0x8, "sentry", GPCR>;
let mayTrap = 1 in
def CBLD          : ZCheri_rr<0x6, 0x5, "cbld", GPCR, GPCRNoC0, GPCR>;
} // defsCanBeSealed = 1

let mayTrap = 1 in {
def SCBNDS        : ZCheri_rr<0x7, 0x0, "scbnds">;
def SCBNDSR       : ZCheri_rr<0x7, 0x1, "scbndsr">;
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def SCBNDSI
    : RVInstCheriSetBoundsImmFmt<0x1, 0x5, OPC_OP_IMM, (outs GPCR:$rd),
                                 (ins GPCR:$rs1, csetbnd_imm:$imm),
                                 "scbndsi", "$rd, $rs1, $imm"
                                >;
} // mayTrap = 1

def : InstAlias<"scbnds $cd, $cs1, $imm",
                (SCBNDSI GPCR:$cd, GPCR:$cs1, csetbnd_imm:$imm), 0>;
def : InstAlias<"add $cd, $cs1, $imm",
                (CADDI GPCR:$cd, GPCR:$cs1, simm12:$imm), 0>;
def : InstAlias<"add $cd, $cs1, $rs2",
                (CADD GPCR:$cd, GPCR:$cs1, GPRNoX0:$rs2), 0>;
} // HasZCheriPurecap

let Predicates = [HasZCheriHybrid] in {
def SCMODE        : ZCheri_rr<0x6, 0x7, "scmode">;
}

//===----------------------------------------------------------------------===//
// Mode Switch Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasZCheriHybrid] in {
def MODESW_CAP : CheriModeSwitchInstr<0x9, OPC_OP, "modesw.cap">;
def MODESW_INT : CheriModeSwitchInstr<0xA, OPC_OP, "modesw.int">;
} // HasZCheriHybrid

//===----------------------------------------------------------------------===//
// Pointer-Arithmetic Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasZCheriPurecap] in {
let isMoveReg = 1, isReMaterializable = 1, isAsCheapAsAMove = 1,
    defsCanBeSealed = 1 in
def CMV       : Cheri_SrcDst<0x6, 0x0, 0x0, OPC_OP, "cmv", GPCR>;
def : InstAlias<"mv $cd, $cs1",
                (CMV GPCR:$cd, GPCR:$cs1), 0>;
} // HasZCheriPurecap


//===----------------------------------------------------------------------===//
// Adjusting to Compressed Capability Precision Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasZCheriPurecap] in {
def CRAM_ : ZCheri_r<0x7, "cram", GPR, GPR>;

} // HasZCheriPurecap

//===----------------------------------------------------------------------===//
// Assertion Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasZCheriPurecap] in {
def SCSS : ZCheri_rr<0x6, 0x6, "scss", GPR, GPCRNoC0, GPCR>;
def SCEQ : ZCheri_rr<0x6, 0x4, "sceq", GPR, GPCR, GPCR>;
}

//===----------------------------------------------------------------------===//
// Control-Flow Instructions
//===----------------------------------------------------------------------===//

let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
  let Predicates = [IsCapMode, HasZCheriPurecap], DecoderNamespace="CapModeOnly_" in {
    def JALR_MODE_CAP : RVInstI<0b001, OPC_JALR, (outs GPR:$rd),
                                (ins GPR:$rs1, immzero:$imm12),
                                "jalr.mode", "$rd, ${imm12}(${rs1})">,
                        Sched<[WriteJalr, ReadJalr]>;
  } // Predicates = [IsCapMode, HasZCheriPurecap, NotHasZCheriHybrid]

  let Predicates = [NotCapMode, HasZCheriHybrid] in {
    def JALR_MODE_LEG : RVInstI<0b001, OPC_JALR, (outs GPCR:$rd),
                                (ins GPCR:$rs1, immzero:$imm12),
                                "jalr.mode", "$rd, ${imm12}(${rs1})">,
                        Sched<[WriteJalr, ReadJalr]>;

  } // Predicates = [NotCapMode, HasZCheriHybrid]
} // isCall, hasSideEffects, mayLoad, mayStore

let Predicates = [IsCapMode, HasZCheriPurecap] in {
  def : InstAlias<"jalr $rd, ${imm12}(${rs1})", (JALR_MODE_CAP GPR:$rd, GPR:$rs1, immzero:$imm12), 2>;
  def : InstAlias<"jalr $rd, $rs1"            , (JALR_MODE_CAP GPR:$rd, GPR:$rs1, 0             ), 2>;
  def : InstAlias<"jalr $rs1"                 , (JALR_MODE_CAP X1     , GPR:$rs1, 0             ), 2>;
  def : InstAlias<"jr $rs1"                   , (JALR_MODE_CAP X0     , GPR:$rs1, 0             ), 2>;
  def : InstAlias<"ret.mode"                  , (JALR_MODE_CAP X0     , X1      , 0             ), 2>;
} // Predicates = [IsCapMode, HasZCheriPurecap]

let Predicates = [NotCapMode, HasZCheriHybrid] in {
  def : InstAlias<"jalr $rd, ${imm12}(${rs1})", (JALR_MODE_LEG GPCR:$rd, GPCR:$rs1, immzero:$imm12), 2>;
  def : InstAlias<"jalr $rd, $rs1"            , (JALR_MODE_LEG GPCR:$rd, GPCR:$rs1, 0             ), 2>;
  def : InstAlias<"jalr $rs1"                 , (JALR_MODE_LEG C1      , GPCR:$rs1, 0             ), 2>;
  def : InstAlias<"jr $rs1"                   , (JALR_MODE_LEG C0      , GPCR:$rs1, 0             ), 2>;
  def : InstAlias<"ret.mode"                  , (JALR_MODE_LEG C0      , C1       , 0             ), 2>;
} // Predicates = [NotCapMode, HasZCheriHybrid]


//===----------------------------------------------------------------------===//
// Special Capabilty Register Access Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasZCheriPurecap] in {

let DecoderNamespace = "CapModeOnly_" in {
def CHERI_CSRRW : CheriCSR_ir<0b001, "csrrw">;
def CHERI_CSRRS : CheriCSR_ir<0b010, "csrrs", GPR>;
def CHERI_CSRRC : CheriCSR_ir<0b011, "csrrc", GPR>;
def CHERI_CSRRWI: CheriCSR_ii<0b101, "csrrwi">;
def CHERI_CSRRSI: CheriCSR_ii<0b110, "csrrsi">;
def CHERI_CSRRCI: CheriCSR_ii<0b111, "csrrci">;
}

def : InstAlias<"csrr $cd, $csr",
                (CHERI_CSRRS GPCR: $cd, cheri_csr_sysreg:$csr,           X0)>;
def : InstAlias<"csrw $csr, $cs",
                (CHERI_CSRRW        C0, cheri_csr_sysreg:$csr,     GPCR:$cs)>;
def : InstAlias<"csrs $csr, $cs",
                (CHERI_CSRRS        C0, cheri_csr_sysreg:$csr,      GPR:$cs)>;
def : InstAlias<"csrc $csr, $cs",
                (CHERI_CSRRC        C0, cheri_csr_sysreg:$csr,      GPR:$cs)>;

def : InstAlias<"csrwi $csr, $imm",
                (CHERI_CSRRWI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrsi $csr, $imm",
                (CHERI_CSRRSI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrci $csr, $imm",
                (CHERI_CSRRCI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;

let EmitPriority = 0 in {
def : InstAlias<"csrw $csr, $imm",
                (CHERI_CSRRWI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrs $csr, $imm",
                (CHERI_CSRRSI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrc $csr, $imm",
                (CHERI_CSRRCI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;

def : InstAlias<"csrrw $cd, $csr, $imm",
                (CHERI_CSRRWI GPCR:$cd, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrrs $cd, $csr, $imm",
                (CHERI_CSRRSI GPCR:$cd, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrrc $cd, $csr, $imm",
                (CHERI_CSRRCI GPCR:$cd, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
}

} // Predicates = [HasZCheriPurecap]

//===----------------------------------------------------------------------===//
// Memory-Access Instructions
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Capability Mode Instructions
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//
class PatCapInspect<SDPatternOperator OpNode, RVInstZCheriSrcDst Inst>
    : Pat<(XLenVT (OpNode GPCR:$rs1)), (Inst GPCR:$rs1)>;
class PatZpGpcr<SDPatternOperator OpNode, RVInstZCheriSrcDst Inst>
    : Pat<(OpNode GPCR:$rs1), (Inst GPCR:$rs1)>;

/// Capability-Inspection Instructions

let Predicates = [HasZCheriPurecap] in {
def : PatCapInspect<riscv_cap_tag_get,        GCTAG >;
def : PatCapInspect<int_cheri_cap_perms_get,  GCPERM>;
def : PatCapInspect<int_cheri_cap_high_get,   GCHI  >;
def : PatCapInspect<int_cheri_cap_base_get,   GCBASE>;
def : PatCapInspect<int_cheri_cap_length_get, GCLEN >;
def : PatCapInspect<riscv_cap_sealed_get,     GCTYPE>;
def : PatCapInspect<int_cheri_cap_type_get,   GCTYPE>;
let Predicates = [HasZCheriHybrid] in
def : PatCapInspect<int_cheri_cap_flags_get,  GCMODE>;

def : Pat<(XLenVT (int_cheri_cap_offset_get GPCR:$cs1)),
          (SUB (XLenVT (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr)),
               (GCBASE GPCR:$cs1))>;
def : Pat<(int_cheri_cap_offset_set GPCR:$cs1, (XLenVT GPR:$rs2)),
          (CADD (SCADDR GPCR:$cs1, (GCBASE GPCR:$cs1)),
                      GPR:$rs2)>;
def : PatCapInspect<int_cheri_cap_copy_from_high, GCHI>;
} // Predicates [HasZCheriPurecap]


// Capability-Modification Instructions

let Predicates = [HasZCheriPurecap] in {
def : PatGpcrGpr<int_cheri_cap_address_set, SCADDR>;
def : PatGpcrGpr<int_cheri_cap_perms_and, ACPERM>;
def : PatGpcrGpr<int_cheri_cap_high_set, SCHI>;
def : PatGpcrGpr<int_cheri_cap_copy_to_high, SCHI>;
def : PatZpGpcr<int_cheri_cap_seal_entry, SENTRY>;
def : PatGpcrGpcr<int_cheri_cap_build, CBLD, CLenVT>;
def : PatGpcrGpr<int_cheri_cap_bounds_set, SCBNDSR>;
def : PatGpcrGpr<int_cheri_cap_bounds_set_exact, SCBNDS>;
def : Pat<(int_cheri_cap_bounds_set_exact GPCR:$rs1, csetbnd_imm:$imm),
          (SCBNDSI GPCR:$rs1, csetbnd_imm:$imm)>;
def : PatGpcrGpr<cptradd, CADD>;
def : PatGpcrSimm12<cptradd, CADDI>;

// TODO: Make this rematerialisable like MIPS
def : PatGpcrGpr<int_cheri_bounded_stack_cap, SCBNDSR>;
def : Pat<(int_cheri_bounded_stack_cap GPCR:$rs1, csetbnd_imm:$imm),
          (SCBNDSI GPCR:$rs1, csetbnd_imm:$imm)>;
def : PatGpcrGpr<int_cheri_bounded_stack_cap_dynamic, SCBNDSR>;
def : Pat<(int_cheri_bounded_stack_cap_dynamic GPCR:$rs1, csetbnd_imm:$imm),
          (SCBNDSI GPCR:$rs1, csetbnd_imm:$imm)>;
def : Pat<(CapFrameAddrRegImm GPCR:$rs1, simm12:$imm12),
          (CADDI GPCR:$rs1, simm12:$imm12)>;

} // HasZCheriPurecap

let Predicates = [HasZCheriHybrid] in
def : PatGpcrGpr<int_cheri_cap_flags_set, SCMODE>;

// bit 25 is the flags bit (32-bit)
let Predicates = [HasZCheriPurecap, NotHasZCheriHybrid, IsRV32] in
def: Pat<(int_cheri_cap_flags_set GPCR:$cs1, (XLenVT GPR:$rs2)),
         (SCHI GPCR:$cs1, (OR (AND (GCHI GPCR:$cs1),
                                   (ADDI (LUI 1040384), -1)),
                              (SLLI (ANDI (XLenVT GPR:$rs2), 1), 25)))>;

// bit 52 is the flags bit (64-bit)
let Predicates = [HasZCheriPurecap, NotHasZCheriHybrid, IsRV64] in
def: Pat<(int_cheri_cap_flags_set GPCR:$cs1, (XLenVT GPR:$rs2)),
         (SCHI GPCR:$cs1, (OR (AND (GCHI GPCR:$cs1),
                                   (ADDI (SLLI (XLenVT -1), 52), (XLenVT -1))),
                              (XLenVT (SLLI (ANDI (XLenVT GPR:$rs2), 1), 52))))>;

/// Pointer-Arithmetic Instructions
let Predicates = [IsPureCapABI, HasZCheriPurecap] in {
def : Pat<(inttoptr (XLenVT GPR:$rs2)), (CADD C0, GPR:$rs2)>;
def : Pat<(inttoptr simm12:$imm12), (CADDI C0, simm12:$imm12)>;
def : Pat<(XLenVT (ptrtoint GPCR:$rs1)), (PseudoCGetAddr GPCR:$rs1)>;
}

let Predicates = [HasZCheriPurecap] in {
def : Pat<(XLenVT (int_cheri_round_representable_length (XLenVT GPR:$rs1))),
          (AND (ADD GPR:$rs1, (XORI (CRAM_ GPR:$rs1), -1)), (CRAM_ GPR:$rs1))>;
def : PatGpr<int_cheri_representable_alignment_mask, CRAM_>;
}

/// Assertion Instructions

def : PatGpcrGpcr<riscv_cap_subset_test, SCSS, XLenVT>;
def : PatGpcrGpcr<riscv_cap_equal_exact, SCEQ, XLenVT>;

/// Special Capability Register Access Instructions

let Predicates = [HasZCheriPurecap, HasZCheriHybrid] in
def : Pat<(int_cheri_ddc_get), (CHERI_CSRRC CSR_DDC.Encoding, (XLenVT X0))>;

/// Capability loads

let Predicates = [HasZCheriPurecap, NotCapMode, HasStdExtD, IsRV32] in
def : Pat<(load GPCR:$rs1),
          (BuildPairF64Pseudo (LW_CAP GPCR:$rs1),
                              (LW_CAP (CADDI GPCR:$rs1, 4)))>;

//===----------------------------------------------------------------------===//
// Compress Instruction tablegen backend.
//===----------------------------------------------------------------------===//

// Quadrant 0
let Predicates = [HasZCheriPurecap, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CADDI GPCRC:$rd, CSP:$rs1, uimm10_lsb00nonzero:$imm),
                  (C_CIncOffsetImm4CSPN GPCRC:$rd, CSP:$rs1, uimm10_lsb00nonzero:$imm)>;
} // Predicates = [HasZCheriPurecap, HasCheriRVC, HasStdExtC, IsCapMode]

// Quadrant 1
let Predicates = [HasZCheriPurecap, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CADDI C2, C2, simm10_lsb0000nonzero:$imm),
                  (C_CIncOffsetImm16CSP C2, simm10_lsb0000nonzero:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

//===----------------------------------------------------------------------===//
// Cheri Hypervisor extensions.
//===----------------------------------------------------------------------===//

let DecoderNamespace = "CapModeOnly_" in {
let Predicates = [HasStdExtH, HasZCheriPurecap, IsCapMode] in {
def CHLV_B   : CHLoad_r<0b0110000, 0b00000, "hlv.b">, Sched<[]>;
def CHLV_BU  : CHLoad_r<0b0110000, 0b00001, "hlv.bu">, Sched<[]>;
def CHLV_H   : CHLoad_r<0b0110010, 0b00000, "hlv.h">, Sched<[]>;
def CHLV_HU  : CHLoad_r<0b0110010, 0b00001, "hlv.hu">, Sched<[]>;
def CHLVX_HU : CHLoad_r<0b0110010, 0b00011, "hlvx.hu">, Sched<[]>;
def CHLV_W   : CHLoad_r<0b0110100, 0b00000, "hlv.w">, Sched<[]>;
def CHLVX_WU : CHLoad_r<0b0110100, 0b00011, "hlvx.wu">, Sched<[]>;
def CHLV_C   : CHLoad_r<0b0111000, 0b00000, "hlv.c">, Sched<[]>;
def CHSV_B   : CHStore_rr<0b0110001, "hsv.b">, Sched<[]>;
def CHSV_H   : CHStore_rr<0b0110011, "hsv.h">, Sched<[]>;
def CHSV_W   : CHStore_rr<0b0110101, "hsv.w">, Sched<[]>;
def CHSV_C   : CHStore_rr<0b0111001, "hsv.c">, Sched<[]>;
}
let Predicates = [IsRV64, HasStdExtH, HasZCheriPurecap, IsCapMode] in {
def CHLV_WU  : CHLoad_r<0b0110100, 0b00001, "hlv.wu">, Sched<[]>;
def CHLV_D   : CHLoad_r<0b0110110, 0b00000, "hlv.d">, Sched<[]>;
def CHSV_D   : CHStore_rr<0b0110111, "hsv.d">, Sched<[]>;
}
}

let Predicates = [HasStdExtH, HasZCheriPurecap, NotCapMode] in {
def HLV_C    : CHLoad_r<0b0111000, 0b00000, "hlv.c", GPRMemZeroOffset>, Sched<[]>;
def HSV_C    : CHStore_rr<0b0111001, "hsv.c", GPRMemZeroOffset>, Sched<[]>;
}
