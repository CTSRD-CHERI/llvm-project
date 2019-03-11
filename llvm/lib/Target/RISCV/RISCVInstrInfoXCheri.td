//===-- RISCVInstrInfoXCheri.td - XCheri RISCV instructions -*- tblgen-*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

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
