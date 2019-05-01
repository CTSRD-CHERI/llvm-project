//===-- RISCVInstrInfoXCheri.td - XCheri RISCV instructions -*- tblgen-*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "RISCVInstrFormatsXCheri.td"

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

let Predicates = [HasCheri, IsRV32] in {
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

let Predicates = [HasCheri, IsRV64] in {
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
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//

class PatGpcrGpr<SDPatternOperator OpNode, RVInstR Inst>
    : Pat<(OpNode GPCR:$rs1, GPR:$rs2), (Inst GPCR:$rs1, GPR:$rs2)>;

class PatGpcrSimm12<SDPatternOperator OpNode, RVInstI Inst>
    : Pat<(OpNode GPCR:$rs1, simm12:$imm12), (Inst GPCR:$rs1, simm12:$imm12)>;

/// Capability-Modification Instructions

def : PatGpcrGpr<ptradd, CIncOffset>;
def : PatGpcrSimm12<ptradd, CIncOffsetImm>;

/// Pointer-Arithmetic Instructions

def : Pat<(inttoptr GPR:$rs2), (CFromPtr DDC, $rs2)>;
def : Pat<(ptrtoint GPCR:$rs1), (CToPtr $rs1, DDC)>;

/// Memory-Access with Explicit Address Type Instructions

multiclass CheriLdPat<PatFrag LoadOp, RVInst Inst, RegisterClass AddrTy> {
  def : Pat<(LoadOp AddrTy:$rs1), (Inst AddrTy:$rs1)>;
}

multiclass CheriStPat<PatFrag StoreOp, RVInst Inst, RegisterClass StTy,
                      RegisterClass AddrTy> {
  def : Pat<(StoreOp StTy:$rs2, AddrTy:$rs1), (Inst StTy:$rs2, AddrTy:$rs1)>;
}

/// DDC-relative loads

let Predicates = [HasCheri] in {
defm : CheriLdPat<sextloadi8, LBddc, GPR>;
defm : CheriLdPat<extloadi8, LBddc, GPR>;
defm : CheriLdPat<sextloadi16, LHddc, GPR>;
defm : CheriLdPat<extloadi16, LHddc, GPR>;
defm : CheriLdPat<zextloadi8, LBUddc, GPR>;
defm : CheriLdPat<zextloadi16, LHUddc, GPR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriLdPat<load, LWddc, GPR>;
defm : CheriLdPat<load, LCddc_64, GPR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriLdPat<sextloadi32, LWddc, GPR>;
defm : CheriLdPat<extloadi32, LWddc, GPR>;
defm : CheriLdPat<load, LDddc, GPR>;
defm : CheriLdPat<zextloadi32, LWUddc, GPR>;
defm : CheriLdPat<load, LCddc_128, GPR>;
}

/// DDC-relative stores

let Predicates = [HasCheri] in {
defm : CheriStPat<truncstorei8, SBddc, GPR, GPR>;
defm : CheriStPat<truncstorei16, SHddc, GPR, GPR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriStPat<store, SWddc, GPR, GPR>;
defm : CheriStPat<store, SCddc_64, GPCR, GPR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriStPat<truncstorei32, SWddc, GPR, GPR>;
defm : CheriStPat<store, SDddc, GPR, GPR>;
defm : CheriStPat<store, SCddc_128, GPCR, GPR>;
}

/// Capability loads

let Predicates = [HasCheri] in {
defm : CheriLdPat<sextloadi8, LBcap, GPCR>;
defm : CheriLdPat<extloadi8, LBcap, GPCR>;
defm : CheriLdPat<sextloadi16, LHcap, GPCR>;
defm : CheriLdPat<extloadi16, LHcap, GPCR>;
defm : CheriLdPat<zextloadi8, LBUcap, GPCR>;
defm : CheriLdPat<zextloadi16, LHUcap, GPCR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriLdPat<load, LWcap, GPCR>;
defm : CheriLdPat<load, LCcap_64, GPCR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriLdPat<sextloadi32, LWcap, GPCR>;
defm : CheriLdPat<extloadi32, LWcap, GPCR>;
defm : CheriLdPat<load, LDcap, GPCR>;
defm : CheriLdPat<zextloadi32, LWUcap, GPCR>;
defm : CheriLdPat<load, LCcap_128, GPCR>;
}

/// Capability stores

let Predicates = [HasCheri] in {
defm : CheriStPat<truncstorei8, SBcap, GPR, GPCR>;
defm : CheriStPat<truncstorei16, SHcap, GPR, GPCR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriStPat<store, SWcap, GPR, GPCR>;
defm : CheriStPat<store, SCcap_64, GPCR, GPCR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriStPat<truncstorei32, SWcap, GPR, GPCR>;
defm : CheriStPat<store, SDcap, GPR, GPCR>;
defm : CheriStPat<store, SCcap_128, GPCR, GPCR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : LdPat<load, LC_64>;
defm : StPat<store, SC_64, GPCR>;
} // Predicates = [HasCheri, IsRV32]

let Predicates = [HasCheri, IsRV64] in {
defm : LdPat<load, LC_128>;
defm : StPat<store, SC_128, GPCR>;
} // Predicates = [HasCheri, IsRV64]
