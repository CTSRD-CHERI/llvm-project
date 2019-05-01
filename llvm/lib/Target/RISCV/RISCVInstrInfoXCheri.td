//===-- RISCVInstrInfoXCheri.td - XCheri RISCV instructions -*- tblgen-*---===//
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
def SDT_RISCVCheriBoolUnary : SDTypeProfile<1, 1, [
  SDTCisInt<0>, SDTCisVT<1, CLenVT>
]>;

def SDT_RISCVCheriBoolBinary : SDTypeProfile<1, 2, [
  SDTCisInt<0>, SDTCisVT<1, CLenVT>, SDTCisVT<2, CLenVT>
]>;

// Target-dependent nodes.
def riscv_cap_tag_get     : SDNode<"RISCVISD::CAP_TAG_GET",
                                   SDT_RISCVCheriBoolUnary>;
def riscv_cap_sealed_get  : SDNode<"RISCVISD::CAP_SEALED_GET",
                                   SDT_RISCVCheriBoolUnary>;
def riscv_cap_subset_test : SDNode<"RISCVISD::CAP_SUBSET_TEST",
                                   SDT_RISCVCheriBoolBinary>;

//===----------------------------------------------------------------------===//
// Operand and SDNode transformation definitions.
//===----------------------------------------------------------------------===//

def CheriAtomicMemOpOperand : AsmOperandClass {
  let Name = "CheriAtomicMemOpOperand";
  let RenderMethod = "addRegOperands";
  let PredicateMethod = "isGPCR";
  let ParserMethod = "parseAtomicMemOp";
}

def GPCRMemAtomic : RegisterOperand<GPCR> {
  let ParserMatchClass = CheriAtomicMemOpOperand;
  let PrintMethod = "printAtomicMemOp";
}

//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//

include "RISCVInstrFormatsXCheri.td"

//===----------------------------------------------------------------------===//
// Instruction Class Templates
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_r<bits<5> funct5, string opcodestr, RegisterClass rdClass=GPR,
              RegisterClass rs1Class=GPCR>
    : RVInstCheriSrcDst<0x7f, funct5, 0, OPC_CHERI, (outs rdClass:$rd),
                        (ins rs1Class:$rs1), opcodestr, "$rd, $rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_rr<bits<7> funct7, string opcodestr, RegisterClass rdClass=GPCR,
               RegisterClass rs2Class=GPR, RegisterClass rs1Class=GPCR>
    : RVInstR<funct7, 0, OPC_CHERI, (outs rdClass:$rd),
              (ins rs1Class:$rs1, rs2Class:$rs2),
              opcodestr, "$rd, $rs1, $rs2">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_ri<bits<3> funct3, string opcodestr, bit simm>
    : RVInstI<funct3, OPC_CHERI, (outs GPCR:$rd),
              (ins GPCR:$rs1, !if(simm,simm12,uimm12):$imm12),
              opcodestr, "$rd, $rs1, $imm12">;

let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
class Cheri_clear<bits<5> funct5, string opcodestr>
    : RVInstCheriClear<0x7f, funct5, 0, OPC_CHERI, (outs),
                       (ins uimm2:$quarter, uimm8:$mask),
                       opcodestr, "$quarter, $mask">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class CheriLoad_r<bits<5> op, string opcodestr, RegisterClass rdClass,
                  RegisterOperand rs1Operand>
    : RVInstCheriSrcDst<0x7d, op, 0, OPC_CHERI, (outs rdClass:$rd),
                        (ins rs1Operand:$rs1), opcodestr, "$rd, $rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
class CheriStore_r<bits<5> op, string opcodestr, RegisterClass rs2Class,
                   RegisterOperand rs1Operand>
    : RVInstCheriTwoSrc<0x7c, op, 0, OPC_CHERI, (outs),
                        (ins rs2Class:$rs2, rs1Operand:$rs1),
                        opcodestr, "$rs2, $rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1, Constraints = "$rd = $rs2" in
class CheriStoreCond_r<bits<5> op, string opcodestr, RegisterClass rs2Class,
                       RegisterOperand rs1Operand>
    : RVInstCheriTwoSrc<0x7c, op, 0, OPC_CHERI, (outs rs2Class:$rd),
                        (ins rs2Class:$rs2, rs1Operand:$rs1),
                        opcodestr, "$rs2, $rs1">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class LR_C_r<bit aq, bit rl, bits<3> funct3, string opcodestr>
    : RVInstRAtomic<0b00010, aq, rl, funct3, OPC_AMO,
                    (outs GPCR:$rd), (ins GPRMemAtomic:$rs1),
                    opcodestr, "$rd, $rs1"> {
  let rs2 = 0;
}

multiclass LR_C_r_aq_rl<string clenstr, bits<3> funct3, string opcodestr> {
  def _ # clenstr       : LR_C_r<0, 0, funct3, opcodestr>;
  def _AQ_ # clenstr    : LR_C_r<1, 0, funct3, opcodestr # ".aq">;
  def _RL_ # clenstr    : LR_C_r<0, 1, funct3, opcodestr # ".rl">;
  def _AQ_RL_ # clenstr : LR_C_r<1, 1, funct3, opcodestr # ".aqrl">;
}

let hasSideEffects = 0, mayLoad = 1, mayStore = 1 in
class AMO_C_rr<bits<5> funct5, bit aq, bit rl, bits<3> funct3, string opcodestr,
               RegisterClass rdClass>
    : RVInstRAtomic<funct5, aq, rl, funct3, OPC_AMO,
                    (outs rdClass:$rd), (ins GPRMemAtomic:$rs1, GPCR:$rs2),
                    opcodestr, "$rd, $rs2, $rs1">;

multiclass AMO_C_rr_aq_rl<string clenstr, bits<5> funct5, bits<3> funct3,
                          string opcodestr, RegisterClass rdClass> {
  def _ # clenstr       : AMO_C_rr<funct5, 0, 0, funct3, opcodestr, rdClass>;
  def _AQ_ # clenstr    : AMO_C_rr<funct5, 1, 0, funct3, opcodestr # ".aq",
                                   rdClass>;
  def _RL_ # clenstr    : AMO_C_rr<funct5, 0, 1, funct3, opcodestr # ".rl",
                                   rdClass>;
  def _AQ_RL_ # clenstr : AMO_C_rr<funct5, 1, 1, funct3, opcodestr # ".aqrl",
                                   rdClass>;
}

//===----------------------------------------------------------------------===//
// Capability-Inspection Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CGetPerm   : Cheri_r<0x0, "cgetperm">;
def CGetType   : Cheri_r<0x1, "cgettype">;
def CGetBase   : Cheri_r<0x2, "cgetbase">;
def CGetLen    : Cheri_r<0x3, "cgetlen">;
def CGetTag    : Cheri_r<0x4, "cgettag">;
def CGetSealed : Cheri_r<0x5, "cgetsealed">;
def CGetOffset : Cheri_r<0x6, "cgetoffset">;
def CGetFlags  : Cheri_r<0x7, "cgetflags">;
def CGetAddr   : Cheri_r<0xf, "cgetaddr">;
}

//===----------------------------------------------------------------------===//
// Capability-Modification Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CSeal           : Cheri_rr<0xb, "cseal", GPCR, GPCR>;
def CUnseal         : Cheri_rr<0xc, "cunseal", GPCR, GPCR>;
def CAndPerm        : Cheri_rr<0xd, "candperm">;
def CSetFlags       : Cheri_rr<0xe, "csetflags">;
def CSetOffset      : Cheri_rr<0xf, "csetoffset">;
def CSetAddr        : Cheri_rr<0x10, "csetaddr">;
def CIncOffset      : Cheri_rr<0x11, "cincoffset">;
def CIncOffsetImm   : Cheri_ri<0x1, "cincoffset", 1>;
def CSetBounds      : Cheri_rr<0x8, "csetbounds">;
def CSetBoundsExact : Cheri_rr<0x9, "csetboundsexact">;
def CSetBoundsImm   : Cheri_ri<0x2, "csetbounds", 0>;
def CClearTag       : Cheri_r<0xb, "ccleartag", GPCR>;
def CBuildCap       : Cheri_rr<0x1d, "cbuildcap", GPCR, GPCR>;
def CCopyType       : Cheri_rr<0x1e, "ccopytype", GPCR, GPCR>;
def CCSeal          : Cheri_rr<0x1f, "ccseal", GPCR, GPCR>;

def : InstAlias<"cincoffsetimm $cd, $cs1, $imm",
                (CIncOffsetImm GPCR:$cd, GPCR:$cs1, simm12:$imm), 0>;
def : InstAlias<"csetboundsimm $cd, $cs1, $imm",
                (CSetBoundsImm GPCR:$cd, GPCR:$cs1, uimm12:$imm), 0>;
}

//===----------------------------------------------------------------------===//
// Pointer-Arithmetic Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CToPtr      : Cheri_rr<0x12, "ctoptr", GPR, GPCRC0IsDDC>;
def CFromPtr    : Cheri_rr<0x13, "cfromptr", GPCR, GPR, GPCRC0IsDDC>;
def CSub        : Cheri_rr<0x14, "csub", GPR, GPCR>;
def CMove       : Cheri_r<0xa, "cmove", GPCR>;
}

//===----------------------------------------------------------------------===//
// Control-Flow Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def CJALR : RVInstCheriSrcDst<0x7f, 0xc, 0, OPC_CHERI, (outs GPCR:$rd),
                              (ins GPCR:$rs1), "cjalr", "$rd, $rs1">;

let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def CCall : RVInstCheriTwoSrc<0x7e, 0x1, 0, OPC_CHERI, (outs),
                              (ins GPCR:$rs1, GPCR:$rs2),
                              "ccall", "$rs1, $rs2">;

def : InstAlias<"cjr $cs1", (CJALR C0, GPCR:$cs1)>;
def : InstAlias<"cret", (CJALR C0, C1), 2>;
}

//===----------------------------------------------------------------------===//
// Assertion Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CTestSubset : Cheri_rr<0x20, "ctestsubset", GPR, GPCR>;
}

//===----------------------------------------------------------------------===//
// Special Capabilty Register Access Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
def CSpecialRW : RVInstCheriSCR<0x1, 0, OPC_CHERI, (outs GPCR:$rd),
                                (ins special_capreg:$imm5, GPCR:$rs1),
                                "cspecialrw", "$rd, $imm5, $rs1">;

def : InstAlias<"cspecialr $cd, $scr",
                (CSpecialRW GPCR:$cd, special_capreg:$scr,       C0)>;
def : InstAlias<"cspecialw $scr, $cs",
                (CSpecialRW       C0, special_capreg:$scr, GPCR:$cs)>;
}

//===----------------------------------------------------------------------===//
// Fast Register-Clearing Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def Clear   : Cheri_clear<0xd, "clear">;
def FPClear : Cheri_clear<0x10, "fpclear">;
}

//===----------------------------------------------------------------------===//
// Adjusting to Compressed Capability Precision Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CRRL : Cheri_r<0x8, "croundrepresentablelength", GPR, GPR>;
def CRAM : Cheri_r<0x9, "crepresentablealignmentmask", GPR, GPR>;

def : InstAlias<"crrl $rd, $rs1", (CRRL GPR:$rd, GPR:$rs1)>;
def : InstAlias<"cram $rd, $rs1", (CRAM GPR:$rd, GPR:$rs1)>;
}

//===----------------------------------------------------------------------===//
// Memory-Access with Explicit Address Type Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def LB_DDC  : CheriLoad_r<0b00000, "lb.ddc",  GPR, GPRMemAtomic>;
def LH_DDC  : CheriLoad_r<0b00001, "lh.ddc",  GPR, GPRMemAtomic>;
def LW_DDC  : CheriLoad_r<0b00010, "lw.ddc",  GPR, GPRMemAtomic>;
def LBU_DDC : CheriLoad_r<0b00100, "lbu.ddc", GPR, GPRMemAtomic>;
def LHU_DDC : CheriLoad_r<0b00101, "lhu.ddc", GPR, GPRMemAtomic>;
}

let Predicates = [HasCheri, IsRV64] in {
def LWU_DDC : CheriLoad_r<0b00110, "lwu.ddc", GPR, GPRMemAtomic>;
def LD_DDC  : CheriLoad_r<0b00011, "ld.ddc",  GPR, GPRMemAtomic>;
}

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, IsRV32] in
def LC_DDC_64  : CheriLoad_r<0b00011, "lc.ddc", GPCR, GPRMemAtomic>;

let Predicates = [HasCheri, IsRV64] in
def LC_DDC_128 : CheriLoad_r<0b10111, "lc.ddc", GPCR, GPRMemAtomic>;

let Predicates = [HasCheri] in {
def SB_DDC  : CheriStore_r<0b00000, "sb.ddc", GPR, GPRMemAtomic>;
def SH_DDC  : CheriStore_r<0b00001, "sh.ddc", GPR, GPRMemAtomic>;
def SW_DDC  : CheriStore_r<0b00010, "sw.ddc", GPR, GPRMemAtomic>;
}

let Predicates = [HasCheri, IsRV64] in {
def SD_DDC  : CheriStore_r<0b00011, "sd.ddc", GPR, GPRMemAtomic>;
}

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, IsRV32] in
def SC_DDC_64  : CheriStore_r<0b00011, "sc.ddc", GPCR, GPRMemAtomic>;

let Predicates = [HasCheri, IsRV64] in
def SC_DDC_128 : CheriStore_r<0b00100, "sc.ddc", GPCR, GPRMemAtomic>;

let Predicates = [HasCheri] in {
def LB_CAP  : CheriLoad_r<0b01000, "lb.cap",  GPR, GPCRMemAtomic>;
def LH_CAP  : CheriLoad_r<0b01001, "lh.cap",  GPR, GPCRMemAtomic>;
def LW_CAP  : CheriLoad_r<0b01010, "lw.cap",  GPR, GPCRMemAtomic>;
def LBU_CAP : CheriLoad_r<0b01100, "lbu.cap", GPR, GPCRMemAtomic>;
def LHU_CAP : CheriLoad_r<0b01101, "lhu.cap", GPR, GPCRMemAtomic>;
}

let Predicates = [HasCheri, IsRV64] in {
def LWU_CAP : CheriLoad_r<0b01110, "lwu.cap", GPR, GPCRMemAtomic>;
def LD_CAP  : CheriLoad_r<0b01011, "ld.cap",  GPR, GPCRMemAtomic>;
}

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, IsRV32] in
def LC_CAP_64  : CheriLoad_r<0b01011, "lc.cap", GPCR, GPCRMemAtomic>;

let Predicates = [HasCheri, IsRV64] in
def LC_CAP_128 : CheriLoad_r<0b11111, "lc.cap", GPCR, GPCRMemAtomic>;

let Predicates = [HasCheri] in {
def SB_CAP  : CheriStore_r<0b01000, "sb.cap", GPR, GPCRMemAtomic>;
def SH_CAP  : CheriStore_r<0b01001, "sh.cap", GPR, GPCRMemAtomic>;
def SW_CAP  : CheriStore_r<0b01010, "sw.cap", GPR, GPCRMemAtomic>;
}

let Predicates = [HasCheri, IsRV64] in {
def SD_CAP  : CheriStore_r<0b01011, "sd.cap", GPR, GPCRMemAtomic>;
}

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, IsRV32] in
def SC_CAP_64  : CheriStore_r<0b01011, "sc.cap", GPCR, GPCRMemAtomic>;

let Predicates = [HasCheri, IsRV64] in
def SC_CAP_128 : CheriStore_r<0b01100, "sc.cap", GPCR, GPCRMemAtomic>;

let Predicates = [HasCheri, HasStdExtA] in {
def LR_B_DDC  : CheriLoad_r<0b10000, "lr.b.ddc",  GPR, GPRMemAtomic>;
def LR_H_DDC  : CheriLoad_r<0b10001, "lr.h.ddc",  GPR, GPRMemAtomic>;
def LR_W_DDC  : CheriLoad_r<0b10010, "lr.w.ddc",  GPR, GPRMemAtomic>;
}

let Predicates = [HasCheri, HasStdExtA, IsRV64] in
def LR_D_DDC  : CheriLoad_r<0b10011, "lr.d.ddc",  GPR, GPRMemAtomic>;

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, HasStdExtA, IsRV32] in
def LR_C_DDC_64  : CheriLoad_r<0b10011, "lr.c.ddc", GPCR, GPRMemAtomic>;

let Predicates = [HasCheri, HasStdExtA, IsRV64] in
def LR_C_DDC_128 : CheriLoad_r<0b10100, "lr.c.ddc", GPCR, GPRMemAtomic>;

let Predicates = [HasCheri, HasStdExtA] in {
def LR_B_CAP  : CheriLoad_r<0b11000, "lr.b.cap",  GPR, GPCRMemAtomic>;
def LR_H_CAP  : CheriLoad_r<0b11001, "lr.h.cap",  GPR, GPCRMemAtomic>;
def LR_W_CAP  : CheriLoad_r<0b11010, "lr.w.cap",  GPR, GPCRMemAtomic>;
}

let Predicates = [HasCheri, HasStdExtA, IsRV64] in
def LR_D_CAP  : CheriLoad_r<0b11011, "lr.d.cap",  GPR, GPCRMemAtomic>;

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, HasStdExtA, IsRV32] in
def LR_C_CAP_64  : CheriLoad_r<0b11011, "lr.c.cap", GPCR, GPCRMemAtomic>;

let Predicates = [HasCheri, HasStdExtA, IsRV64] in
def LR_C_CAP_128 : CheriLoad_r<0b11100, "lr.c.cap", GPCR, GPCRMemAtomic>;

let Predicates = [HasCheri, HasStdExtA] in {
def SC_B_DDC  : CheriStoreCond_r<0b10000, "sc.b.ddc",  GPR, GPRMemAtomic>;
def SC_H_DDC  : CheriStoreCond_r<0b10001, "sc.h.ddc",  GPR, GPRMemAtomic>;
def SC_W_DDC  : CheriStoreCond_r<0b10010, "sc.w.ddc",  GPR, GPRMemAtomic>;
}

let Predicates = [HasCheri, HasStdExtA, IsRV64] in
def SC_D_DDC  : CheriStoreCond_r<0b10011, "sc.d.ddc",  GPR, GPRMemAtomic>;

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, HasStdExtA, IsRV32] in
def SC_C_DDC_64  : CheriStoreCond_r<0b10011, "sc.c.ddc", GPCR, GPRMemAtomic>;

let Predicates = [HasCheri, HasStdExtA, IsRV64] in
def SC_C_DDC_128 : CheriStoreCond_r<0b10100, "sc.c.ddc", GPCR, GPRMemAtomic>;

let Predicates = [HasCheri, HasStdExtA] in {
def SC_B_CAP  : CheriStoreCond_r<0b11000, "sc.b.cap",  GPR, GPCRMemAtomic>;
def SC_H_CAP  : CheriStoreCond_r<0b11001, "sc.h.cap",  GPR, GPCRMemAtomic>;
def SC_W_CAP  : CheriStoreCond_r<0b11010, "sc.w.cap",  GPR, GPCRMemAtomic>;
}

let Predicates = [HasCheri, HasStdExtA, IsRV64] in
def SC_D_CAP  : CheriStoreCond_r<0b11011, "sc.d.cap",  GPR, GPCRMemAtomic>;

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, HasStdExtA, IsRV32] in
def SC_C_CAP_64  : CheriStoreCond_r<0b11011, "sc.c.cap", GPCR, GPCRMemAtomic>;

let Predicates = [HasCheri, HasStdExtA, IsRV64] in
def SC_C_CAP_128 : CheriStoreCond_r<0b11100, "sc.c.cap", GPCR, GPCRMemAtomic>;

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

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, HasStdExtA, IsRV32] in {
defm LR_C       : LR_C_r_aq_rl<"64", 0b011, "lr.c">;
defm SC_C       : AMO_C_rr_aq_rl<"64", 0b00011, 0b011, "sc.c", GPR>;
defm AMOSWAP_C  : AMO_C_rr_aq_rl<"64", 0b00001, 0b011, "amoswap.c", GPCR>;
}

let Predicates = [HasCheri, HasStdExtA, IsRV64] in {
defm LR_C       : LR_C_r_aq_rl<"128", 0b100, "lr.c">;
defm SC_C       : AMO_C_rr_aq_rl<"128", 0b00011, 0b100, "sc.c", GPR>;
defm AMOSWAP_C  : AMO_C_rr_aq_rl<"128", 0b00001, 0b100, "amoswap.c", GPCR>;
}

//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//

class PatGpcr<SDPatternOperator OpNode, RVInstCheriSrcDst Inst>
    : Pat<(OpNode GPCR:$rs1), (Inst GPCR:$rs1)>;

class PatGpr<SDPatternOperator OpNode, RVInstCheriSrcDst Inst>
    : Pat<(OpNode GPR:$rs1), (Inst GPR:$rs1)>;

class PatGpcrGpcr<SDPatternOperator OpNode, RVInstR Inst>
    : Pat<(OpNode GPCR:$rs1, GPCR:$rs2), (Inst GPCR:$rs1, GPCR:$rs2)>;

class PatGpcrGpr<SDPatternOperator OpNode, RVInstR Inst>
    : Pat<(OpNode GPCR:$rs1, GPR:$rs2), (Inst GPCR:$rs1, GPR:$rs2)>;

class PatGpcrSimm12<SDPatternOperator OpNode, RVInstI Inst>
    : Pat<(OpNode GPCR:$rs1, simm12:$imm12), (Inst GPCR:$rs1, simm12:$imm12)>;

class PatGpcrUimm12<SDPatternOperator OpNode, RVInstI Inst>
    : Pat<(OpNode GPCR:$rs1, uimm12:$imm12), (Inst GPCR:$rs1, uimm12:$imm12)>;

class PatGprGpcr<SDPatternOperator OpNode, RVInst Inst>
    : Pat<(OpNode GPR:$rs1, GPCR:$rs2), (Inst GPR:$rs1, GPCR:$rs2)>;

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
def : PatGpcrGpr<int_cheri_cap_address_set, CSetAddr>;
def : PatGpcrGpr<ptradd, CIncOffset>;
def : PatGpcrSimm12<ptradd, CIncOffsetImm>;
def : PatGpcrGpr<int_cheri_cap_bounds_set, CSetBounds>;
def : PatGpcrGpr<int_cheri_cap_bounds_set_exact, CSetBoundsExact>;
def : PatGpcrUimm12<int_cheri_cap_bounds_set, CSetBoundsImm>;
def : PatGpcr<int_cheri_cap_tag_clear, CClearTag>;
def : PatGpcrGpcr<int_cheri_cap_build, CBuildCap>;
def : PatGpcrGpcr<int_cheri_cap_type_copy, CCopyType>;
def : PatGpcrGpcr<int_cheri_cap_conditional_seal, CCSeal>;

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

/// Null Capability Patterns

def : Pat<(int_cheri_cap_from_pointer DDC, (XLenVT 0)), (CLenVT (COPY C0))>;
def : Pat<(int_cheri_cap_from_ddc (XLenVT 0)), (CLenVT (COPY C0))>;
def : Pat<(inttoptr (XLenVT 0)), (CLenVT (COPY C0))>;

/// Pointer-Comparison Instructions

// Currently no dedicated instructions, so we:
// * Rely on a merged register file
// * Break untagged < tagged semantics
// * Can't implement exact equality
class CheriSetCCPatGpcrGpcr<PatFrag CondOp, dag GprGprDag>
    : Pat<(CondOp GPCR:$cs1, GPCR:$cs2),
          (OutPatFrag<(ops node:$rs1, node:$rs2), GprGprDag>
            (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr),
            (EXTRACT_SUBREG GPCR:$cs2, sub_cap_addr))>;

multiclass CheriSetCCPatGpcrSimm12<PatFrag CondOp, dag GprSimm12Dag> {
  def : Pat<(CondOp GPCR:$cs1, (inttoptr simm12:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprSimm12Dag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), simm12:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (ptradd (inttoptr (XLenVT 0)), simm12:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprSimm12Dag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), simm12:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (int_cheri_cap_offset_set
                (iFATPTR64 (inttoptr (XLenVT 0))), simm12:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprSimm12Dag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), simm12:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (int_cheri_cap_offset_set
                (iFATPTR64 (inttoptr (XLenVT 0))), simm12:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprSimm12Dag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), simm12:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (int_cheri_cap_address_set
                (iFATPTR64 (inttoptr (XLenVT 0))), simm12:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprSimm12Dag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), simm12:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (int_cheri_cap_address_set
                (iFATPTR128 (inttoptr (XLenVT 0))), simm12:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprSimm12Dag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), simm12:$imm12)>;
}

class CheriSetCCPatGpcrNull<PatFrag CondOp, dag GprDag>
    : Pat<(CondOp GPCR:$cs1, (inttoptr (XLenVT 0))),
          (OutPatFrag<(ops node:$rs1), GprDag>
            (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr))>;

class Swap<PatFrag BinFrag>
    : PatFrag<(ops node:$a, node:$b), (BinFrag $b, $a)>;

def  : CheriSetCCPatGpcrNull<seteq, (SLTIU node:$rs1, 1)>;
def  : CheriSetCCPatGpcrGpcr<seteq, (SLTIU (XOR node:$rs1, node:$rs2), 1)>;
defm : CheriSetCCPatGpcrSimm12<seteq, (SLTIU (XORI node:$rs1, node:$imm12), 1)>;
defm : CheriSetCCPatGpcrSimm12<Swap<seteq>, (SLTIU (XORI node:$rs1, node:$imm12), 1)>;
def  : CheriSetCCPatGpcrNull<setne, (SLTU X0, node:$rs1)>;
def  : CheriSetCCPatGpcrGpcr<setne, (SLTU X0, (XOR node:$rs1, node:$rs2))>;
defm : CheriSetCCPatGpcrSimm12<setne, (SLTU X0, (XORI node:$rs1, node:$imm12))>;
defm : CheriSetCCPatGpcrSimm12<Swap<setne>, (SLTU X0, (XORI node:$rs1, node:$imm12))>;
def  : CheriSetCCPatGpcrGpcr<setugt, (SLTU node:$rs2, node:$rs1)>;
defm : CheriSetCCPatGpcrSimm12<Swap<setugt>, (SLTIU node:$rs1, node:$imm12)>;
def  : CheriSetCCPatGpcrGpcr<setuge, (XORI (SLTU node:$rs1, node:$rs2), 1)>;
def  : CheriSetCCPatGpcrGpcr<setule, (XORI (SLTU node:$rs2, node:$rs1), 1)>;
def  : CheriSetCCPatGpcrGpcr<setult, (SLTU node:$rs1, node:$rs2)>;
defm : CheriSetCCPatGpcrSimm12<setult, (SLTIU node:$rs1, node:$imm12)>;
def  : CheriSetCCPatGpcrGpcr<setgt, (SLT node:$rs2, node:$rs1)>;
defm : CheriSetCCPatGpcrSimm12<Swap<setgt>, (SLTI node:$rs1, node:$imm12)>;
def  : CheriSetCCPatGpcrGpcr<setge, (XORI (SLT node:$rs1, node:$rs2), 1)>;
def  : CheriSetCCPatGpcrGpcr<setle, (XORI (SLT node:$rs2, node:$rs1), 1)>;
def  : CheriSetCCPatGpcrGpcr<setlt, (SLT node:$rs1, node:$rs2)>;
defm : CheriSetCCPatGpcrSimm12<setlt, (SLTI node:$rs1, node:$imm12)>;

def Select_GPCR_Using_CC_GPR : SelectCC_rrirr<GPCR, GPR>;

/// Control-Flow Instructions

// No dedicated instructions; see above

class CheriBccPat<PatFrag CondOp, RVInstB Inst>
    : Pat<(brcond (XLenVT (CondOp GPCR:$rs1, GPCR:$rs2)), bb:$imm12),
          (Inst
            (EXTRACT_SUBREG GPCR:$rs1, sub_cap_addr),
            (EXTRACT_SUBREG GPCR:$rs2, sub_cap_addr),
            simm13_lsb0:$imm12)>;

def : CheriBccPat<seteq, BEQ>;
def : CheriBccPat<setne, BNE>;
def : CheriBccPat<setlt, BLT>;
def : CheriBccPat<setge, BGE>;
def : CheriBccPat<setult, BLTU>;
def : CheriBccPat<setuge, BGEU>;

class CheriBccSwapPat<PatFrag CondOp, RVInst InstBcc>
    : Pat<(brcond (XLenVT (CondOp GPCR:$rs1, GPCR:$rs2)), bb:$imm12),
          (InstBcc
            (EXTRACT_SUBREG GPCR:$rs2, sub_cap_addr),
            (EXTRACT_SUBREG GPCR:$rs1, sub_cap_addr),
            simm13_lsb0:$imm12)>;

def : CheriBccSwapPat<setgt, BLT>;
def : CheriBccSwapPat<setle, BGE>;
def : CheriBccSwapPat<setugt, BLTU>;
def : CheriBccSwapPat<setule, BGEU>;

/// Assertion Instructions

def : PatGpcrGpcr<riscv_cap_subset_test, CTestSubset>;

/// Special Capability Register Access Instructions

def : Pat<(int_cheri_ddc_get), (CSpecialRW SCR_DDC.Encoding, C0)>;

let Predicates = [HasCheri] in
def : Pat<(int_cheri_pcc_get), (CSpecialRW SCR_PCC.Encoding, C0)>;

/// Fast Register-Clearing Instructions

/// Adjusting to Compressed Capability Precision Instructions

def : PatGpr<int_cheri_round_representable_length, CRRL>;
def : PatGpr<int_cheri_representable_alignment_mask, CRAM>;

// TODO: int_cheri_cap_load_tags, int_cheri_bounded_stack_cap

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
defm : CheriExplicitLdPat<sextloadi8, LB_DDC, GPR>;
defm : CheriExplicitLdPat<extloadi8, LB_DDC, GPR>;
defm : CheriExplicitLdPat<sextloadi16, LH_DDC, GPR>;
defm : CheriExplicitLdPat<extloadi16, LH_DDC, GPR>;
defm : CheriExplicitLdPat<zextloadi8, LBU_DDC, GPR>;
defm : CheriExplicitLdPat<zextloadi16, LHU_DDC, GPR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriExplicitLdPat<load, LW_DDC, GPR>;
defm : CheriExplicitLdPat<load, LC_DDC_64, GPR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriExplicitLdPat<sextloadi32, LW_DDC, GPR>;
defm : CheriExplicitLdPat<extloadi32, LW_DDC, GPR>;
defm : CheriExplicitLdPat<load, LD_DDC, GPR>;
defm : CheriExplicitLdPat<zextloadi32, LWU_DDC, GPR>;
defm : CheriExplicitLdPat<load, LC_DDC_128, GPR>;
}

/// DDC-relative stores

let Predicates = [HasCheri] in {
defm : CheriExplicitStPat<truncstorei8, SB_DDC, GPR, GPR>;
defm : CheriExplicitStPat<truncstorei16, SH_DDC, GPR, GPR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriExplicitStPat<store, SW_DDC, GPR, GPR>;
defm : CheriExplicitStPat<store, SC_DDC_64, GPCR, GPR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriExplicitStPat<truncstorei32, SW_DDC, GPR, GPR>;
defm : CheriExplicitStPat<store, SD_DDC, GPR, GPR>;
defm : CheriExplicitStPat<store, SC_DDC_128, GPCR, GPR>;
}

/// Capability loads

let Predicates = [HasCheri] in {
defm : CheriExplicitLdPat<sextloadi8, LB_CAP, GPCR>;
defm : CheriExplicitLdPat<extloadi8, LB_CAP, GPCR>;
defm : CheriExplicitLdPat<sextloadi16, LH_CAP, GPCR>;
defm : CheriExplicitLdPat<extloadi16, LH_CAP, GPCR>;
defm : CheriExplicitLdPat<zextloadi8, LBU_CAP, GPCR>;
defm : CheriExplicitLdPat<zextloadi16, LHU_CAP, GPCR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriExplicitLdPat<load, LW_CAP, GPCR>;
defm : CheriExplicitLdPat<load, LC_CAP_64, GPCR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriExplicitLdPat<sextloadi32, LW_CAP, GPCR>;
defm : CheriExplicitLdPat<extloadi32, LW_CAP, GPCR>;
defm : CheriExplicitLdPat<load, LD_CAP, GPCR>;
defm : CheriExplicitLdPat<zextloadi32, LWU_CAP, GPCR>;
defm : CheriExplicitLdPat<load, LC_CAP_128, GPCR>;
}

/// Capability stores

let Predicates = [HasCheri] in {
defm : CheriExplicitStPat<truncstorei8, SB_CAP, GPR, GPCR>;
defm : CheriExplicitStPat<truncstorei16, SH_CAP, GPR, GPCR>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : CheriExplicitStPat<store, SW_CAP, GPR, GPCR>;
defm : CheriExplicitStPat<store, SC_CAP_64, GPCR, GPCR>;
}

let Predicates = [HasCheri, IsRV64] in {
defm : CheriExplicitStPat<truncstorei32, SW_CAP, GPR, GPCR>;
defm : CheriExplicitStPat<store, SD_CAP, GPR, GPCR>;
defm : CheriExplicitStPat<store, SC_CAP_128, GPCR, GPCR>;
}

/// Memory-Access Instructions

multiclass AMOCapPat<string CLenStr, string AtomicOp, string BaseInst> {
  def : PatGprGpcr<!cast<PatFrag>(AtomicOp#"_monotonic"),
                   !cast<RVInst>(BaseInst#"_"#CLenStr)>;
  def : PatGprGpcr<!cast<PatFrag>(AtomicOp#"_acquire"),
                   !cast<RVInst>(BaseInst#"_AQ_"#CLenStr)>;
  def : PatGprGpcr<!cast<PatFrag>(AtomicOp#"_release"),
                   !cast<RVInst>(BaseInst#"_RL_"#CLenStr)>;
  def : PatGprGpcr<!cast<PatFrag>(AtomicOp#"_acq_rel"),
                   !cast<RVInst>(BaseInst#"_AQ_RL_"#CLenStr)>;
  def : PatGprGpcr<!cast<PatFrag>(AtomicOp#"_seq_cst"),
                   !cast<RVInst>(BaseInst#"_AQ_RL_"#CLenStr)>;
}

let Predicates = [HasCheri, IsRV32] in {
defm : LdPat<load, LC_64>;
defm : StPat<store, SC_64, GPCR>;
} // Predicates = [HasCheri, IsRV32]

let Predicates = [HasCheri, IsRV64] in {
defm : LdPat<load, LC_128>;
defm : StPat<store, SC_128, GPCR>;
} // Predicates = [HasCheri, IsRV64]

let Predicates = [HasCheri, HasStdExtA, IsRV32] in {
defm : LdPat<atomic_load_cap, LC_64>;
defm : AtomicStPat<atomic_store_cap, SC_64, GPCR>;
defm : AMOCapPat<"64", "atomic_swap_cap", "AMOSWAP_C">;
} // Predicates = [HasCheri, HasStdExtA, IsRV32]

let Predicates = [HasCheri, HasStdExtA, IsRV64] in {
defm : LdPat<atomic_load_cap, LC_128>;
defm : AtomicStPat<atomic_store_cap, SC_128, GPCR>;
defm : AMOCapPat<"128", "atomic_swap_cap", "AMOSWAP_C">;
} // Predicates = [HasCheri, HasStdExtA, IsRV64]

let Predicates = [HasCheri, HasStdExtA] in {
def PseudoCmpXchgCap
    : Pseudo<(outs GPCR:$res, GPR:$scratch),
             (ins GPR:$addr, GPCR:$cmpval, GPCR:$newval, ixlenimm:$ordering), []> {
  let Constraints = "@earlyclobber $res,@earlyclobber $scratch";
  let mayLoad = 1;
  let mayStore = 1;
  let hasSideEffects = 0;
}

def : Pat<(atomic_cmp_swap_cap_monotonic GPR:$addr, GPCR:$cmp, GPCR:$new),
          (PseudoCmpXchgCap GPR:$addr, GPCR:$cmp, GPCR:$new, 2)>;
def : Pat<(atomic_cmp_swap_cap_acquire GPR:$addr, GPCR:$cmp, GPCR:$new),
          (PseudoCmpXchgCap GPR:$addr, GPCR:$cmp, GPCR:$new, 4)>;
def : Pat<(atomic_cmp_swap_cap_release GPR:$addr, GPCR:$cmp, GPCR:$new),
          (PseudoCmpXchgCap GPR:$addr, GPCR:$cmp, GPCR:$new, 5)>;
def : Pat<(atomic_cmp_swap_cap_acq_rel GPR:$addr, GPCR:$cmp, GPCR:$new),
          (PseudoCmpXchgCap GPR:$addr, GPCR:$cmp, GPCR:$new, 6)>;
def : Pat<(atomic_cmp_swap_cap_seq_cst GPR:$addr, GPCR:$cmp, GPCR:$new),
          (PseudoCmpXchgCap GPR:$addr, GPCR:$cmp, GPCR:$new, 7)>;
} // Predicates = [HasCheri, HasStdExtA]
