//===-- RISCVInstrInfoXCheri.td - XCheri RISCV instructions -*- tblgen-*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "RISCVInstrFormatsXCheri.td"

//===----------------------------------------------------------------------===//
// RISC-V specific DAG Nodes.
//===----------------------------------------------------------------------===//

// Target-dependent type requirements.
def SDT_RISCVCapCall        : SDTypeProfile<0, -1, [SDTCisVT<0, CLenVT>]>;

def SDT_RISCVCheriBoolUnary : SDTypeProfile<1, 1, [
  SDTCisInt<0>, SDTCisVT<1, CLenVT>
]>;

def SDT_RISCVCheriBoolBinary : SDTypeProfile<1, 2, [
  SDTCisInt<0>, SDTCisVT<1, CLenVT>, SDTCisVT<2, CLenVT>
]>;

// Target-dependent nodes.
def riscv_cap_call        : SDNode<"RISCVISD::CAP_CALL", SDT_RISCVCapCall,
                                   [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                                    SDNPVariadic]>;
def riscv_cap_tail        : SDNode<"RISCVISD::CAP_TAIL", SDT_RISCVCapCall,
                                   [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                                    SDNPVariadic]>;
def riscv_cap_tag_get     : SDNode<"RISCVISD::CAP_TAG_GET",
                                   SDT_RISCVCheriBoolUnary>;
def riscv_cap_sealed_get  : SDNode<"RISCVISD::CAP_SEALED_GET",
                                   SDT_RISCVCheriBoolUnary>;
def riscv_cap_subset_test : SDNode<"RISCVISD::CAP_SUBSET_TEST",
                                   SDT_RISCVCheriBoolBinary>;

//===----------------------------------------------------------------------===//
// Operand and SDNode transformation definitions.
//===----------------------------------------------------------------------===//

def CapFI64  : ComplexPattern<iFATPTR64,  1, "SelectCapFI", [frameindex], []>;
def CapFI128 : ComplexPattern<iFATPTR128, 1, "SelectCapFI", [frameindex], []>;

//===----------------------------------------------------------------------===//
// Capability-Inspection Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CGetPerm   : Cheri_rc<0x7f, 0x0, "cgetperm">;
def CGetType   : Cheri_rc<0x7f, 0x1, "cgettype">;
def CGetBase   : Cheri_rc<0x7f, 0x2, "cgetbase">;
def CGetLen    : Cheri_rc<0x7f, 0x3, "cgetlen">;
def CGetTag    : Cheri_rc<0x7f, 0x4, "cgettag">;
def CGetSealed : Cheri_rc<0x7f, 0x5, "cgetsealed">;
def CGetOffset : Cheri_rc<0x7f, 0x6, "cgetoffset">;
def CGetFlags  : Cheri_rc<0x7f, 0x7, "cgetflags">;
def CGetAddr   : Cheri_rc<0x7f, 0xf, "cgetaddr">;
}

//===----------------------------------------------------------------------===//
// Capability-Modification Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CSeal           : Cheri_ccc<0xb, "cseal">;
def CUnseal         : Cheri_ccc<0xc, "cunseal">;
def CAndPerm        : Cheri_ccr<0xd, "candperm">;
def CSetFlags       : Cheri_ccr<0xe, "csetflags">;
def CSetOffset      : Cheri_ccr<0xf, "csetoffset">;
def CIncOffset      : Cheri_ccr<0x11, "cincoffset">;
def CIncOffsetImm   : Cheri_cci<0x1, "cincoffset", 1>;
def CSetBounds      : Cheri_ccr<0x8, "csetbounds">;
def CSetBoundsExact : Cheri_ccr<0x9, "csetboundsexact">;
def CSetBoundsImm   : Cheri_cci<0x2, "csetbounds", 0>;
def CClearTag       : Cheri_cc<0x7f, 0xb, "ccleartag">;
def CBuildCap       : Cheri_ccc<0x1d, "cbuildcap">;
def CCopyType       : Cheri_ccc<0x1e, "ccopytype">;
def CCSeal          : Cheri_ccc<0x1f, "ccseal">;

def : InstAlias<"cincoffsetimm $cd, $cb, $imm",
                (CIncOffsetImm GPCR:$cd, GPCR:$cb, simm12:$imm), 0>;
def : InstAlias<"csetboundsimm $cd, $cb, $imm",
                (CSetBoundsImm GPCR:$cd, GPCR:$cb, uimm12:$imm), 0>;
}

//===----------------------------------------------------------------------===//
// Pointer-Arithmetic Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CToPtr      : Cheri_rcc<0x12, "ctoptr", GPCRC0IsDDC>;
def CFromPtr    : Cheri_ccr<0x13, "cfromptr", GPCRC0IsDDC>;
def CSub        : Cheri_rcc<0x14, "csub">;
def CMove       : Cheri_cc<0x7f, 0xa, "cmove">;
def CSpecialRW  : Cheri_ccs<0x1, "cspecialrw">;
}

//===----------------------------------------------------------------------===//
// Control-Flow Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def CJALR : RVInstCheriRR<0x7f, 0xc, 0, OPC_CHERI, (outs GPCR:$rd),
                          (ins GPCR:$rs1), "cjalr", "$rd, $rs1">;

let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def CCall : RVInstCheriZRRI<0x7e, 0, OPC_CHERI, (outs),
                            (ins GPCR:$rs1, GPCR:$rs2, uimm5:$imm5),
                            "ccall", "$rs1, $rs2, $imm5">;

def : InstAlias<"cjr $cs", (CJALR C0, GPCR:$cs)>;
def : InstAlias<"cret", (CJALR C0, C1), 2>;
def : InstAlias<"creturn", (CCall C0, C0, 0)>;
def : InstAlias<"ccall $cs, $cb", (CCall GPCR:$cs, GPCR:$cb, 0)>;
}

let Predicates = [HasCheri, IsPureCapABI],
    isCall = 1, Defs = [C1] in
def PseudoCapCALLIndirect : Pseudo<(outs), (ins GPCR:$rs1),
                                   [(riscv_cap_call GPCR:$rs1)]>,
                            PseudoInstExpansion<(CJALR C1, GPCR:$rs1)>;

let Predicates = [HasCheri, IsPureCapABI],
    isBarrier = 1, isReturn = 1, isTerminator = 1 in
def PseudoCapRET : Pseudo<(outs), (ins), [(riscv_ret_flag)]>,
                   PseudoInstExpansion<(CJALR C0, C1)>;

let Predicates = [HasCheri, IsPureCapABI],
    isCall = 1, isTerminator = 1, isReturn = 1, isBarrier = 1, Uses = [C2] in
def PseudoCapTAILIndirect : Pseudo<(outs), (ins GPCRTC:$rs1),
                                   [(riscv_cap_tail GPCRTC:$rs1)]>,
                            PseudoInstExpansion<(CJALR C0, GPCR:$rs1)>;

//===----------------------------------------------------------------------===//
// Assertion Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CTestSubset : Cheri_rcc<0x20, "ctestsubset">;
}

//===----------------------------------------------------------------------===//
// Fast Register-Clearing Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def Clear   : Cheri_clear<0x7f, 0xd, "clear">;
def FPClear : Cheri_clear<0x7f, 0x10, "fpclear">;
}

//===----------------------------------------------------------------------===//
// Memory-Access with Explicit Address Type Instructions
//===----------------------------------------------------------------------===//

// TODO: {LR,SC}{ddc,cap}.{B,H,W,D,C}

let Predicates = [HasCheri] in {
def LBddc  : CheriLoad_explicit<0b00000, "lb.ddc",  GPR, GPR>;
def LHddc  : CheriLoad_explicit<0b00001, "lh.ddc",  GPR, GPR>;
def LWddc  : CheriLoad_explicit<0b00010, "lw.ddc",  GPR, GPR>;
def LBUddc : CheriLoad_explicit<0b00100, "lbu.ddc", GPR, GPR>;
def LHUddc : CheriLoad_explicit<0b00101, "lhu.ddc", GPR, GPR>;
}

let Predicates = [HasCheri, IsRV64] in {
def LWUddc : CheriLoad_explicit<0b00110, "lwu.ddc", GPR, GPR>;
def LDddc  : CheriLoad_explicit<0b00011, "ld.ddc",  GPR, GPR>;
}

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, IsRV32] in
def LCddc_64  : CheriLoad_explicit<0b00011, "lc.ddc",  GPCR, GPR>;

let Predicates = [HasCheri, IsRV64] in
def LCddc_128 : CheriLoad_explicit<0b10111, "lc.ddc",  GPCR, GPR>;

let Predicates = [HasCheri] in {
def SBddc  : CheriStore_explicit<0b00000, "sb.ddc",  GPR, GPR>;
def SHddc  : CheriStore_explicit<0b00001, "sh.ddc",  GPR, GPR>;
def SWddc  : CheriStore_explicit<0b00010, "sw.ddc",  GPR, GPR>;
}

let Predicates = [HasCheri, IsRV64] in {
def SDddc  : CheriStore_explicit<0b00011, "sd.ddc",  GPR, GPR>;
}

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, IsRV32] in
def SCddc_64  : CheriStore_explicit<0b00011, "sc.ddc",  GPCR, GPR>;

let Predicates = [HasCheri, IsRV64] in
def SCddc_128 : CheriStore_explicit<0b00100, "sc.ddc",  GPCR, GPR>;

let Predicates = [HasCheri] in {
def LBcap  : CheriLoad_explicit<0b01000, "lb.cap",  GPR, GPCR>;
def LHcap  : CheriLoad_explicit<0b01001, "lh.cap",  GPR, GPCR>;
def LWcap  : CheriLoad_explicit<0b01010, "lw.cap",  GPR, GPCR>;
def LBUcap : CheriLoad_explicit<0b01100, "lbu.cap", GPR, GPCR>;
def LHUcap : CheriLoad_explicit<0b01101, "lhu.cap", GPR, GPCR>;
}

let Predicates = [HasCheri, IsRV64] in {
def LWUcap : CheriLoad_explicit<0b01110, "lwu.cap", GPR, GPCR>;
def LDcap  : CheriLoad_explicit<0b01011, "ld.cap",  GPR, GPCR>;
}

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, IsRV32] in
def LCcap_64  : CheriLoad_explicit<0b01011, "lc.cap",  GPCR, GPCR>;

let Predicates = [HasCheri, IsRV64] in
def LCcap_128 : CheriLoad_explicit<0b11111, "lc.cap",  GPCR, GPCR>;

let Predicates = [HasCheri] in {
def SBcap  : CheriStore_explicit<0b01000, "sb.cap",  GPR, GPCR>;
def SHcap  : CheriStore_explicit<0b01001, "sh.cap",  GPR, GPCR>;
def SWcap  : CheriStore_explicit<0b01010, "sw.cap",  GPR, GPCR>;
}

let Predicates = [HasCheri, IsRV64] in {
def SDcap  : CheriStore_explicit<0b01011, "sd.cap",  GPR, GPCR>;
}

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, IsRV32] in
def SCcap_64  : CheriStore_explicit<0b01011, "sc.cap",  GPCR, GPCR>;

let Predicates = [HasCheri, IsRV64] in
def SCcap_128 : CheriStore_explicit<0b01100, "sc.cap",  GPCR, GPCR>;

//===----------------------------------------------------------------------===//
// Memory-Access Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri, IsRV32, NotCapMode] in {
let DecoderNamespace = "RISCV32Only_",
    hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
def LC_64  : RVInstI<0x3, OPC_LOAD, (outs GPCR:$rd),
                     (ins GPR:$rs1, simm12:$imm12),
                     "lc", "$rd, ${imm12}(${rs1})">;

let DecoderNamespace = "RISCV32Only_",
    hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
def SC_64  : RVInstS<0x3, OPC_STORE, (outs),
                     (ins GPCR:$rs2, GPR:$rs1, simm12:$imm12),
                     "sc", "$rs2, ${imm12}(${rs1})">;

let EmitPriority = 0 in {
def : InstAlias<"lc $rd, (${rs1})",
                (LC_64  GPCR:$rd, GPR:$rs1, 0)>;
def : InstAlias<"sc $rs2, (${rs1})",
                (SC_64  GPCR:$rs2, GPR:$rs1, 0)>;
}
}

let Predicates = [HasCheri, IsRV64, NotCapMode] in {
let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
def LC_128  : RVInstI<0x2, OPC_MISC_MEM, (outs GPCR:$rd),
                      (ins GPR:$rs1, simm12:$imm12),
                      "lc", "$rd, ${imm12}(${rs1})">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
def SC_128  : RVInstS<0x4, OPC_STORE, (outs),
                      (ins GPCR:$rs2, GPR:$rs1, simm12:$imm12),
                      "sc", "$rs2, ${imm12}(${rs1})">;

let EmitPriority = 0 in {
def : InstAlias<"lc $rd, (${rs1})",
                (LC_128  GPCR:$rd, GPR:$rs1, 0)>;
def : InstAlias<"sc $rs2, (${rs1})",
                (SC_128  GPCR:$rs2, GPR:$rs1, 0)>;
}
}

//===----------------------------------------------------------------------===//
// Capability Mode Instructions
//===----------------------------------------------------------------------===//


/// 'I' (Integer) base

let DecoderNamespace = "CapModeOnly_" in {
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
let Predicates = [HasCheri, IsCapMode] in
def AUIPCC : RVInstU<OPC_AUIPC, (outs GPCR:$rd), (ins uimm20_auipc:$imm20),
                     "auipcc", "$rd, $imm20">;
} // hasSideEffects = 0, mayLoad = 0, mayStore = 0

let Predicates = [HasCheri, IsCapMode] in {
def CLB  : CapModeLoad_ri<0b000, "clb">;
def CLH  : CapModeLoad_ri<0b001, "clh">;
def CLW  : CapModeLoad_ri<0b010, "clw">;
def CLBU : CapModeLoad_ri<0b100, "clbu">;
def CLHU : CapModeLoad_ri<0b101, "clhu">;

def CSB : CapModeStore_rri<0b000, "csb">;
def CSH : CapModeStore_rri<0b001, "csh">;
def CSW : CapModeStore_rri<0b010, "csw">;
} // Predicates = [HasCheri, IsCapMode]

let Predicates = [HasCheri, IsRV64, IsCapMode] in {
def CLWU   : CapModeLoad_ri<0b110, "clwu">;
def CLD    : CapModeLoad_ri<0b011, "cld">;
def CSD    : CapModeStore_rri<0b011, "csd">;
} // Predicates = [HasCheri, IsRV64, IsCapMode]
} // DecoderNameSpace = "CapModeOnly_"

let Predicates = [HasCheri, IsRV32, IsCapMode] in {
let DecoderNamespace = "RISCV32CapModeOnly_",
    hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
def CLC_64  : RVInstI<0x3, OPC_LOAD, (outs GPCR:$rd),
                      (ins GPCR:$rs1, simm12:$imm12),
                      "clc", "$rd, ${imm12}(${rs1})">;

let DecoderNamespace = "RISCV32CapModeOnly_",
    hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
def CSC_64  : RVInstS<0x3, OPC_STORE, (outs),
                      (ins GPCR:$rs2, GPCR:$rs1, simm12:$imm12),
                      "csc", "$rs2, ${imm12}(${rs1})">;

let EmitPriority = 0 in {
def : InstAlias<"clc $rd, (${rs1})",
                (CLC_64  GPCR:$rd, GPCR:$rs1, 0)>;
def : InstAlias<"csc $rs2, (${rs1})",
                (CSC_64  GPCR:$rs2, GPCR:$rs1, 0)>;
}
}

let Predicates = [HasCheri, IsRV64, IsCapMode] in {
let DecoderNamespace = "CapModeOnly_",
    hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
def CLC_128  : RVInstI<0x2, OPC_MISC_MEM, (outs GPCR:$rd),
                       (ins GPCR:$rs1, simm12:$imm12),
                       "clc", "$rd, ${imm12}(${rs1})">;

let DecoderNamespace = "CapModeOnly_",
    hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
def CSC_128  : RVInstS<0x4, OPC_STORE, (outs),
                       (ins GPCR:$rs2, GPCR:$rs1, simm12:$imm12),
                       "csc", "$rs2, ${imm12}(${rs1})">;

let EmitPriority = 0 in {
def : InstAlias<"clc $rd, (${rs1})",
                (CLC_128  GPCR:$rd, GPCR:$rs1, 0)>;
def : InstAlias<"csc $rs2, (${rs1})",
                (CSC_128  GPCR:$rs2, GPCR:$rs1, 0)>;
}
}

let EmitPriority = 0 in {
let Predicates = [HasCheri, IsCapMode] in {
def : InstAlias<"clb $rd, (${rs1})",
                (CLB  GPR:$rd, GPCR:$rs1, 0)>;
def : InstAlias<"clh $rd, (${rs1})",
                (CLH  GPR:$rd, GPCR:$rs1, 0)>;
def : InstAlias<"clw $rd, (${rs1})",
                (CLW  GPR:$rd, GPCR:$rs1, 0)>;
def : InstAlias<"clbu $rd, (${rs1})",
                (CLBU  GPR:$rd, GPCR:$rs1, 0)>;
def : InstAlias<"clhu $rd, (${rs1})",
                (CLHU  GPR:$rd, GPCR:$rs1, 0)>;

def : InstAlias<"csb $rs2, (${rs1})",
                (CSB  GPR:$rs2, GPCR:$rs1, 0)>;
def : InstAlias<"csh $rs2, (${rs1})",
                (CSH  GPR:$rs2, GPCR:$rs1, 0)>;
def : InstAlias<"csw $rs2, (${rs1})",
                (CSW  GPR:$rs2, GPCR:$rs1, 0)>;
} // Predicates = [HasCheri, IsCapMode]

let Predicates = [HasCheri, IsRV64, IsCapMode] in {
def : InstAlias<"clwu $rd, (${rs1})",
                (CLWU  GPR:$rd, GPCR:$rs1, 0)>;
def : InstAlias<"cld $rd, (${rs1})",
                (CLD  GPR:$rd, GPCR:$rs1, 0)>;
def : InstAlias<"csd $rs2, (${rs1})",
                (CSD  GPR:$rs2, GPCR:$rs1, 0)>;
} // Predicates = [HasCheri, IsRV64, IsCapMode]
}

/// 'A' (Atomic Instructions) extension

// TODO

/// 'F' (Single-Precision Floating-Point) extension

let Predicates = [HasCheri, HasStdExtF, IsCapMode] in {
let DecoderNamespace = "CapModeOnly_",
    hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
def CFLW : RVInstI<0b010, OPC_LOAD_FP, (outs FPR32:$rd),
                   (ins GPCR:$rs1, simm12:$imm12),
                   "cflw", "$rd, ${imm12}(${rs1})">;

let DecoderNamespace = "CapModeOnly_",
    hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
def CFSW : RVInstS<0b010, OPC_STORE_FP, (outs),
                   (ins FPR32:$rs2, GPCR:$rs1, simm12:$imm12),
                   "cfsw", "$rs2, ${imm12}(${rs1})">;

def : InstAlias<"cflw $rd, (${rs1})",  (CFLW FPR32:$rd,  GPCR:$rs1, 0), 0>;
def : InstAlias<"cfsw $rs2, (${rs1})", (CFSW FPR32:$rs2, GPCR:$rs1, 0), 0>;
} // Predicates = [HasCheri, HasStdExtF, IsCapMode]

/// 'D' (Single-Precision Floating-Point) extension

let Predicates = [HasCheri, HasStdExtD, IsCapMode] in {
let DecoderNamespace = "CapModeOnly_",
    hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
def CFLD : RVInstI<0b011, OPC_LOAD_FP, (outs FPR64:$rd),
                   (ins GPCR:$rs1, simm12:$imm12),
                   "cfld", "$rd, ${imm12}(${rs1})">;

let DecoderNamespace = "CapModeOnly_",
    hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
def CFSD : RVInstS<0b011, OPC_STORE_FP, (outs),
                   (ins FPR64:$rs2, GPCR:$rs1, simm12:$imm12),
                   "cfsd", "$rs2, ${imm12}(${rs1})">;

def : InstAlias<"cfld $rd, (${rs1})",  (CFLD FPR64:$rd,  GPCR:$rs1, 0), 0>;
def : InstAlias<"cfsd $rs2, (${rs1})", (CFSD FPR64:$rs2, GPCR:$rs1, 0), 0>;
} // Predicates = [HasCheri, HasStdExtD, IsCapMode]

/// 'C' (Compressed Instructions) extension

// TODO

//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri, IsCapMode] in {
let hasSideEffects = 0, mayLoad = 1, mayStore = 0, isCodeGenOnly = 0,
    isAsmParserOnly = 1 in
def PseudoCLGC : Pseudo<(outs GPCR:$dst), (ins bare_symbol:$src), [],
                        "clgc", "$dst, $src">;
}

class PatGpcr<SDPatternOperator OpNode, RVInstCheriRR Inst>
    : Pat<(OpNode GPCR:$rs1), (Inst GPCR:$rs1)>;

class PatGpcrGpcr<SDPatternOperator OpNode, RVInstR Inst>
    : Pat<(OpNode GPCR:$rs1, GPCR:$rs2), (Inst GPCR:$rs1, GPCR:$rs2)>;

class PatGpcrGpr<SDPatternOperator OpNode, RVInstR Inst>
    : Pat<(OpNode GPCR:$rs1, GPR:$rs2), (Inst GPCR:$rs1, GPR:$rs2)>;

class PatGpcrSimm12<SDPatternOperator OpNode, RVInstI Inst>
    : Pat<(OpNode GPCR:$rs1, simm12:$imm12), (Inst GPCR:$rs1, simm12:$imm12)>;

class PatGpcrUimm12<SDPatternOperator OpNode, RVInstI Inst>
    : Pat<(OpNode GPCR:$rs1, uimm12:$imm12), (Inst GPCR:$rs1, uimm12:$imm12)>;

/// Capability-Inspection Instructions

def : PatGpcr<int_cheri_cap_perms_get, CGetPerm>;
def : PatGpcr<int_cheri_cap_type_get, CGetType>;
def : PatGpcr<int_cheri_cap_base_get, CGetBase>;
def : PatGpcr<int_cheri_cap_length_get, CGetLen>;
def : PatGpcr<riscv_cap_tag_get, CGetTag>;
def : PatGpcr<riscv_cap_sealed_get, CGetSealed>;
def : PatGpcr<int_cheri_cap_offset_get, CGetOffset>;
def : PatGpcr<int_cheri_cap_flags_get, CGetFlags>;
def : PatGpcr<int_cheri_cap_address_get, CGetAddr>;

/// Capability-Modification Instructions

def : PatGpcrGpcr<int_cheri_cap_seal, CSeal>;
def : PatGpcrGpcr<int_cheri_cap_unseal, CUnseal>;
def : PatGpcrGpr<int_cheri_cap_perms_and, CAndPerm>;
def : PatGpcrGpr<int_cheri_cap_flags_set, CSetFlags>;
def : PatGpcrGpr<int_cheri_cap_offset_set, CSetOffset>;
def : PatGpcrGpr<int_cheri_cap_offset_increment, CIncOffset>;
def : PatGpcrSimm12<int_cheri_cap_offset_increment, CIncOffsetImm>;
def : PatGpcrGpr<int_cheri_cap_bounds_set, CSetBounds>;
def : PatGpcrGpr<int_cheri_cap_bounds_set_exact, CSetBoundsExact>;
def : PatGpcrUimm12<int_cheri_cap_bounds_set, CSetBoundsImm>;
def : PatGpcr<int_cheri_cap_tag_clear, CClearTag>;
def : PatGpcrGpcr<int_cheri_cap_build, CBuildCap>;
def : PatGpcrGpcr<int_cheri_cap_type_copy, CCopyType>;
def : PatGpcrGpcr<int_cheri_cap_conditional_seal, CCSeal>;

def : PatGpcrGpr<ptradd, CIncOffset>;
def : PatGpcrSimm12<ptradd, CIncOffsetImm>;

/// Pointer-Arithmetic Instructions

def : Pat<(int_cheri_cap_to_pointer GPCR:$rs1, GPCRC0IsDDC:$rs2),
          (CToPtr GPCR:$rs1, GPCRC0IsDDC:$rs2)>;
def : Pat<(int_cheri_cap_from_pointer GPCRC0IsDDC:$rs1, GPR:$rs2),
          (CFromPtr GPCRC0IsDDC:$rs1, GPR:$rs2)>;
def : Pat<(int_cheri_cap_from_ddc GPR:$rs2), (CFromPtr DDC, $rs2)>;
def : PatGpcrGpcr<int_cheri_cap_diff, CSub>;

// TODO: Only for hybrid; use CIncOffset C0 etc for CheriABI
def : Pat<(inttoptr GPR:$rs2), (CFromPtr DDC, $rs2)>;
def : Pat<(ptrtoint GPCR:$rs1), (CToPtr $rs1, DDC)>;

/// Control-Flow Instructions

/// Assertion Instructions

def : PatGpcrGpcr<riscv_cap_subset_test, CTestSubset>;

// TODO: int_cheri_cap_load_tags, int_cheri_bounded_stack_cap
// No int_cheri_cap_perms_check/int_cheri_cap_type_check

/// Memory-Access with Explicit Address Type Instructions

// TODO: Need int<->float patterns for pointers in capabilty mode and
// capabilities without capability mode (which requires BuildPairF64 and
// SplitF64 for doubles on RV32I).

multiclass CheriExplicitLdPat<PatFrag LoadOp, RVInst Inst,
                              RegisterClass AddrTy> {
  let AddedComplexity = -10 in
  def : Pat<(LoadOp AddrTy:$rs1), (Inst AddrTy:$rs1)>;
}

multiclass CheriExplicitStPat<PatFrag StoreOp, RVInst Inst,
                              RegisterClass StTy, RegisterClass AddrTy> {
  let AddedComplexity = -10 in
  def : Pat<(StoreOp StTy:$rs2, AddrTy:$rs1), (Inst StTy:$rs2, AddrTy:$rs1)>;
}

/// DDC-relative loads

let Predicates = [HasCheri] in {
defm : CheriExplicitLdPat<sextloadi8, LBddc, GPR>;
defm : CheriExplicitLdPat<extloadi8, LBddc, GPR>;
defm : CheriExplicitLdPat<sextloadi16, LHddc, GPR>;
defm : CheriExplicitLdPat<extloadi16, LHddc, GPR>;
defm : CheriExplicitLdPat<zextloadi8, LBUddc, GPR>;
defm : CheriExplicitLdPat<zextloadi16, LHUddc, GPR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriExplicitLdPat<load, LWddc, GPR>;
defm : CheriExplicitLdPat<load, LCddc_64, GPR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriExplicitLdPat<sextloadi32, LWddc, GPR>;
defm : CheriExplicitLdPat<extloadi32, LWddc, GPR>;
defm : CheriExplicitLdPat<load, LDddc, GPR>;
defm : CheriExplicitLdPat<zextloadi32, LWUddc, GPR>;
defm : CheriExplicitLdPat<load, LCddc_128, GPR>;
}

/// DDC-relative stores

let Predicates = [HasCheri] in {
defm : CheriExplicitStPat<truncstorei8, SBddc, GPR, GPR>;
defm : CheriExplicitStPat<truncstorei16, SHddc, GPR, GPR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriExplicitStPat<store, SWddc, GPR, GPR>;
defm : CheriExplicitStPat<store, SCddc_64, GPCR, GPR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriExplicitStPat<truncstorei32, SWddc, GPR, GPR>;
defm : CheriExplicitStPat<store, SDddc, GPR, GPR>;
defm : CheriExplicitStPat<store, SCddc_128, GPCR, GPR>;
}

/// Capability loads

let Predicates = [HasCheri] in {
defm : CheriExplicitLdPat<sextloadi8, LBcap, GPCR>;
defm : CheriExplicitLdPat<extloadi8, LBcap, GPCR>;
defm : CheriExplicitLdPat<sextloadi16, LHcap, GPCR>;
defm : CheriExplicitLdPat<extloadi16, LHcap, GPCR>;
defm : CheriExplicitLdPat<zextloadi8, LBUcap, GPCR>;
defm : CheriExplicitLdPat<zextloadi16, LHUcap, GPCR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriExplicitLdPat<load, LWcap, GPCR>;
defm : CheriExplicitLdPat<load, LCcap_64, GPCR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriExplicitLdPat<sextloadi32, LWcap, GPCR>;
defm : CheriExplicitLdPat<extloadi32, LWcap, GPCR>;
defm : CheriExplicitLdPat<load, LDcap, GPCR>;
defm : CheriExplicitLdPat<zextloadi32, LWUcap, GPCR>;
defm : CheriExplicitLdPat<load, LCcap_128, GPCR>;
}

/// Capability stores

let Predicates = [HasCheri] in {
defm : CheriExplicitStPat<truncstorei8, SBcap, GPR, GPCR>;
defm : CheriExplicitStPat<truncstorei16, SHcap, GPR, GPCR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriExplicitStPat<store, SWcap, GPR, GPCR>;
defm : CheriExplicitStPat<store, SCcap_64, GPCR, GPCR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriExplicitStPat<truncstorei32, SWcap, GPR, GPCR>;
defm : CheriExplicitStPat<store, SDcap, GPR, GPCR>;
defm : CheriExplicitStPat<store, SCcap_128, GPCR, GPCR>;
}

/// Non-Capability Mode Instructions

let Predicates = [HasCheri, IsRV32, NotCapMode] in {
defm : LdPat<load, LC_64>;
defm : StPat<store, SC_64, GPCR>;
} // Predicates = [HasCheri, IsRV32, NotCapMode]

let Predicates = [HasCheri, IsRV64, NotCapMode] in {
defm : LdPat<load, LC_128>;
defm : StPat<store, SC_128, GPCR>;
} // Predicates = [HasCheri, IsRV64, NotCapMode]

/// Capability Mode Instructions

multiclass CapModeLdPat<PatFrag LoadOp, RVInst Inst> {
  def : Pat<(LoadOp GPCR:$rs1), (Inst GPCR:$rs1, 0)>;
  def : Pat<(LoadOp CapFI64:$rs1), (Inst CapFI64:$rs1, 0)>;
  def : Pat<(LoadOp CapFI128:$rs1), (Inst CapFI128:$rs1, 0)>;
  def : Pat<(LoadOp (ptradd GPCR:$rs1, simm12:$imm12)),
            (Inst GPCR:$rs1, simm12:$imm12)>;
  def : Pat<(LoadOp
              (int_cheri_cap_offset_increment GPCR:$rs1, simm12:$imm12)),
            (Inst GPCR:$rs1, simm12:$imm12)>;
  def : Pat<(LoadOp (ptradd CapFI64:$rs1, simm12:$imm12)),
            (Inst CapFI64:$rs1, simm12:$imm12)>;
  def : Pat<(LoadOp (ptradd CapFI128:$rs1, simm12:$imm12)),
            (Inst CapFI128:$rs1, simm12:$imm12)>;
  def : Pat<(LoadOp
              (int_cheri_cap_offset_increment CapFI64:$rs1, simm12:$imm12)),
            (Inst CapFI64:$rs1, simm12:$imm12)>;
  def : Pat<(LoadOp
              (int_cheri_cap_offset_increment CapFI128:$rs1, simm12:$imm12)),
            (Inst CapFI128:$rs1, simm12:$imm12)>;
}

multiclass CapModeStPat<PatFrag StoreOp, RVInst Inst, RegisterClass StTy> {
  def : Pat<(StoreOp StTy:$rs2, GPCR:$rs1), (Inst StTy:$rs2, GPCR:$rs1, 0)>;
  def : Pat<(StoreOp StTy:$rs2, CapFI64:$rs1), (Inst StTy:$rs2, CapFI64:$rs1, 0)>;
  def : Pat<(StoreOp StTy:$rs2, CapFI128:$rs1), (Inst StTy:$rs2, CapFI128:$rs1, 0)>;
  def : Pat<(StoreOp StTy:$rs2, (ptradd GPCR:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, GPCR:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp StTy:$rs2,
              (int_cheri_cap_offset_increment GPCR:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, GPCR:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp StTy:$rs2, (ptradd CapFI64:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, CapFI64:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp StTy:$rs2, (ptradd CapFI128:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, CapFI128:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp StTy:$rs2,
              (int_cheri_cap_offset_increment CapFI64:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, CapFI64:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp StTy:$rs2,
              (int_cheri_cap_offset_increment CapFI128:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, CapFI128:$rs1, simm12:$imm12)>;
}

/// 'I' (Integer) base

defm : CapModeLdPat<sextloadi8, CLB>, Requires<[HasCheri, IsCapMode]>;
defm : CapModeLdPat<extloadi8, CLB>, Requires<[HasCheri, IsCapMode]>;
defm : CapModeLdPat<sextloadi16, CLH>, Requires<[HasCheri, IsCapMode]>;
defm : CapModeLdPat<extloadi16, CLH>, Requires<[HasCheri, IsCapMode]>;
defm : CapModeLdPat<load, CLW>, Requires<[HasCheri, IsRV32, IsCapMode]>;
defm : CapModeLdPat<zextloadi8, CLBU>, Requires<[HasCheri, IsCapMode]>;
defm : CapModeLdPat<zextloadi16, CLHU>, Requires<[HasCheri, IsCapMode]>;

defm : CapModeStPat<truncstorei8, CSB, GPR>, Requires<[HasCheri, IsCapMode]>;
defm : CapModeStPat<truncstorei16, CSH, GPR>, Requires<[HasCheri, IsCapMode]>;
defm : CapModeStPat<store, CSW, GPR>, Requires<[HasCheri, IsRV32, IsCapMode]>;

let Predicates = [HasCheri, IsRV64, IsCapMode] in {

/// Loads

defm : CapModeLdPat<sextloadi32, CLW>;
defm : CapModeLdPat<extloadi32, CLW>;
defm : CapModeLdPat<zextloadi32, CLWU>;
defm : CapModeLdPat<load, CLD>;

/// Stores

defm : CapModeStPat<truncstorei32, CSW, GPR>;
defm : CapModeStPat<store, CSD, GPR>;
} // Predicates = [HasCheri, IsRV64, IsCapMode]

/// 'A' (Atomic Instructions) extension

// TODO

/// 'F' (Single-Precision Floating-Point) extension

let Predicates = [HasCheri, HasStdExtF, IsCapMode] in {

/// Loads

defm : CapModeLdPat<load, CFLW>;

/// Stores

defm : CapModeStPat<store, CFSW, FPR32>;

} // Predicates = [HasCheri, HasStdExtF, IsCapMode]

/// 'D' (Single-Precision Floating-Point) extension

let Predicates = [HasCheri, HasStdExtD, IsCapMode] in {

/// Loads

defm : CapModeLdPat<load, CFLD>;

/// Stores

defm : CapModeStPat<store, CFSD, FPR64>;

} // Predicates = [HasCheri, HasStdExtD, IsCapMode]

/// 'XCheri' extension

let Predicates = [HasCheri, IsRV32, IsCapMode] in {
defm : CapModeLdPat<load, CLC_64>;
defm : CapModeStPat<store, CSC_64, GPCR>;
} // Predicates = [HasCheri, IsRV32, IsCapMode]

let Predicates = [HasCheri, IsRV64, IsCapMode] in {
defm : CapModeLdPat<load, CLC_128>;
defm : CapModeStPat<store, CSC_128, GPCR>;
} // Predicates = [HasCheri, IsRV64, IsCapMode]
