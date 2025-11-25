//===-- RISCVInstrInfoY.td - RISCV instructions -------------*- tblgen-*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//

include "RISCVInstrFormatsY.td"

//===----------------------------------------------------------------------===//
// Instruction Class Templates
//===----------------------------------------------------------------------===//

multiclass ZCheri_r<bits<5> funct5, string opcodestr, RegisterClass rdClass=GPR,
               DAGOperand rs1Operand=GPCR, list<string> altMnemonics = [],
               bit defsSealed = false> {
let hasSideEffects = 0, mayLoad = 0, mayStore = 0, defsCanBeSealed = defsSealed in
def "" : RVInstZCheriSrcDst<0x8, funct5, 0x0, OPC_OP, (outs rdClass:$rd),
                          (ins rs1Operand:$rs1), opcodestr, "$rd, $rs1">;
foreach alt = altMnemonics in {
def : InstAlias<alt # " $rd, $rs1",
                (!cast<Instruction>(NAME) rdClass:$rd, rs1Operand:$rs1), 0>;
}
}

multiclass ZCheri_rr<bits<7> funct7, bits<3> funct3, string opcodestr,
                   RegisterClass rdClass=GPCR, RegisterClass rs1Class=GPCR,
                   RegisterClass rs2Class=GPR, list<string> altMnemonics = [],
                   bit defsSealed = false> {
let hasSideEffects = 0, mayLoad = 0, mayStore = 0, defsCanBeSealed = defsSealed in
def "" : RVInstR<funct7, funct3, OPC_OP, (outs rdClass:$rd),
          (ins rs1Class:$rs1, rs2Class:$rs2), opcodestr, "$rd, $rs1, $rs2"
         >;
foreach alt = altMnemonics in {
def : InstAlias<alt # " $rd, $rs1, $rs2",
                (!cast<Instruction>(NAME) rdClass:$rd, rs1Class:$rs1, rs2Class:$rs2), 0>;
}
}

multiclass ZCheri_setboundsimm<string opcodestr, list<string> altMnemonics = []> {
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def "" : RVInstCheriSetBoundsImmFmt<0x1, 0x5, OPC_OP_IMM, (outs GPCR:$rd),
                                    (ins GPCR:$rs1, csetbnd_imm:$imm),
                                    opcodestr, "$rd, $rs1, $imm"
                                   >;
foreach alt = altMnemonics in {
def : InstAlias<alt # " $rd, $rs1, $rs2",
                (!cast<Instruction>(NAME) GPCR:$rd, GPCR:$rs1, csetbnd_imm:$rs2), 0>;
}
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_SrcDst<bits<7> funct7, bits<5> funct5, bits<3> funct3, RISCVOpcode opcode,
                   string opcodestr, RegisterClass rdClass=GPR,
                   DAGOperand rs1Operand=GPCR>
      : RVInstCheriSrcDst<funct7, funct5, funct3, opcode, (outs rdClass:$rd),
                          (ins rs1Operand:$rs1), opcodestr, "$rd, $rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class CheriRVInstR<bits<7> funct7, bits<3> funct3, RISCVOpcode opcode,
                   string opcodestr, RegisterClass rdClass=GPR,
                   RegisterClass rs1Class=GPR, RegisterClass rs2Class=GPR>
      : RVInstR<funct7, funct3, opcode, (outs rdClass:$rd),
                (ins rs1Class:$rs1, rs2Class:$rs2), opcodestr, "$rd, $rs1, $rs2"
               >;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class CheriRVInstrI<bits<3> funct3, RISCVOpcode opcode, string opcodestr, bit simm>
      : RVInstI<funct3, opcode, (outs GPCR:$rd),
                (ins GPCR:$rs1, !if(simm,simm12,uimm12):$imm12),
                opcodestr, "$rd, $rs1, $imm12">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0, isCodeGenOnly = 1 in
class CheriPseudoLoad<RegisterClass rdClass, RegisterOperand rs1Operand>
        : Pseudo<(outs rdClass:$rd), (ins rs1Operand:$rs1), []>;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1, isCodeGenOnly = 1 in
class CheriPseudoStore<RegisterClass rs2Class, RegisterOperand rs1Operand>
        : Pseudo<(outs), (ins rs2Class:$rs2, rs1Operand:$rs1), []>;

multiclass LR_C_r_aq_rl_aliases<list<string> altMnemonics, RegisterOperand rs1Class = GPRMemZeroOffset> {
foreach alias = altMnemonics in {
  def : InstAlias<alias # " $rd, $rs1",
                  (!cast<Instruction>(NAME) GPCR:$rd, rs1Class:$rs1)>;
  def : InstAlias<alias # ".aq $rd, $rs1",
                  (!cast<Instruction>(NAME # _AQ) GPCR:$rd, rs1Class:$rs1)>;
  def : InstAlias<alias # ".rl $rd, $rs1",
                  (!cast<Instruction>(NAME # _RL) GPCR:$rd, rs1Class:$rs1)>;
  def : InstAlias<alias # ".aqrl $rd, $rs1",
                  (!cast<Instruction>(NAME # _AQ_RL) GPCR:$rd, rs1Class:$rs1)>;
}
}

multiclass AMO_C_rr_aq_rl_aliases<list<string> altMnemonics, RegisterClass rdClass,
                                  RegisterOperand rs1Class = GPRMemZeroOffset> {
foreach alias = altMnemonics in {
  def : InstAlias<alias # " $rd, $rs2, $rs1",
                 (!cast<Instruction>(NAME) rdClass:$rd, rs1Class:$rs1, GPCR:$rs2), 0>;
  def : InstAlias<alias # ".aq $rd, $rs2, $rs1",
                 (!cast<Instruction>(NAME # _AQ) rdClass:$rd, rs1Class:$rs1, GPCR:$rs2), 0>;
  def : InstAlias<alias # ".rl $rd, $rs2, $rs1",
                 (!cast<Instruction>(NAME # _RL) rdClass:$rd, rs1Class:$rs1, GPCR:$rs2), 0>;
  def : InstAlias<alias # ".aqrl $rd, $rs2, $rs1",
                 (!cast<Instruction>(NAME # _AQ_RL) rdClass:$rd, rs1Class:$rs1, GPCR:$rs2), 0>;
}
}


//===----------------------------------------------------------------------===//
// Capability-Inspection Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtY] in {
defm YTAGR  : ZCheri_r<0x0, "ytagr", GPR, GPCR, ["cgettag", "gctag"]>;
defm YPERMR : ZCheri_r<0x1, "ypermr", GPR, GPCR,  ["gcperm", "cgetperm"]>;
defm YHIR   : ZCheri_r<0x4, "yhir", GPR, GPCR, ["gchi", "cgethigh"]>;
defm YBASER : ZCheri_r<0x5, "ybaser", GPR, GPCR, ["gcbase", "cgetbase"]>;
defm YLENR  : ZCheri_r<0x6, "ylenr", GPR, GPCR, ["gclen", "cgetlen"]>;
defm YTYPER : ZCheri_r<0x2, "ytyper", GPR, GPCR, ["gctype", "cgettype"]>;
} // Predicates = [HasStdExtY]

let Predicates = [HasStdExtZYHybrid] in {
defm YMODER : ZCheri_r<0x3, "ymoder", GPR, GPCR, ["gcmode"]>;
} // Predicates = [HasStdExtZYHybrid]

//===----------------------------------------------------------------------===//
// Capability-Modification Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtY] in {
defm YADDRW        : ZCheri_rr<0x6, 0x1, "yaddrw", GPCR, GPCR, GPR, ["scaddr", "csetaddr"]>;
defm YPERMC        : ZCheri_rr<0x6, 0x2, "ypermc", GPCR, GPCR, GPR, ["acperm", "candperm"]>;
defm YHIW          : ZCheri_rr<0x6, 0x3, "yhiw", GPCR, GPCR, GPR, ["schi", "csethigh"]>;
let isReMaterializable = 1, isAsCheapAsAMove = 1 in {
def ADDY          : CheriRVInstR<0x6, 0x0, OPC_OP, "addy", GPCR, GPCR, GPRNoX0>;
def ADDIY         : CheriRVInstrI<0x2, OPC_OP_IMM_32, "addiy", 1>;
}
defm YSENTRY      : ZCheri_r<0x8, "ysentry", GPCR, GPCR, ["sentry", "csealentry"], 1>;
defm YBLD        : ZCheri_rr<0x6, 0x5, "ybld", GPCR, GPCRNoC0, GPCR, ["cbld", "cbuildcap"], 1>;

defm YBNDSW        : ZCheri_rr<0x7, 0x0, "ybndsw", GPCR, GPCR, GPR, ["scbnds", "csetboundsexact"]>;
defm YBNDSRW       : ZCheri_rr<0x7, 0x1, "ybndsrw", GPCR, GPCR, GPR, ["scbndsr", "csetbounds"]>;
defm YBNDSIW       : ZCheri_setboundsimm<"ybndsiw", ["ybndsw", "scbndsi", "scbnds", "csetbounds", "csetboundsimm"]>;

foreach alias = ["add", "cadd", "cincoffset"] in
def : InstAlias<alias # " $cd, $cs1, $rs2",
                (ADDY GPCR:$cd, GPCR:$cs1, GPRNoX0:$rs2), 0>;

foreach alias = ["add", "cadd", "caddi", "cincoffset", "cincoffsetimm"] in
def : InstAlias<alias # " $cd, $cs1, $imm",
                (ADDIY GPCR:$cd, GPCR:$cs1, simm12:$imm), 0>;
} // HasStdExtY

let Predicates = [HasStdExtZYHybrid] in {
defm YMODEW        : ZCheri_rr<0x6, 0x7, "ymodew">;
def : InstAlias<"scmode $cd, $cs1, $rs2",
                (YMODEW GPCR:$cd, GPCR:$cs1, GPR:$rs2), 0>;
}

//===----------------------------------------------------------------------===//
// Mode Switch Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtZYHybrid] in {
def YMODESWY : CheriModeSwitchInstr<0x9, OPC_OP, "ymodeswy">;
def : InstAlias<"modesw.cap", (YMODESWY)>;
def YMODESWI : CheriModeSwitchInstr<0xA, OPC_OP, "ymodeswi">;
def : InstAlias<"modesw.int", (YMODESWI)>;
} // HasStdExtZYHybrid

//===----------------------------------------------------------------------===//
// Pointer-Arithmetic Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtY] in {
let isMoveReg = 1, isReMaterializable = 1, isAsCheapAsAMove = 1,
    defsCanBeSealed = 1 in
def YMV       : Cheri_SrcDst<0x6, 0x0, 0x0, OPC_OP, "ymv", GPCR>;

foreach alias = ["mv", "cmv", "cmove"] in
def : InstAlias<alias # " $cd, $cs1",
                (YMV GPCR:$cd, GPCR:$cs1), 0>;
} // HasStdExtY


//===----------------------------------------------------------------------===//
// Adjusting to Compressed Capability Precision Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtY] in {
defm YAMASK : ZCheri_r<0x7, "yamask", GPR, GPR, ["cram", "crepresentablealignmentmask"]>;
} // HasStdExtY

//===----------------------------------------------------------------------===//
// Assertion Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtY] in {
defm YLT  : ZCheri_rr<0x6, 0x6, "ylt", GPR, GPCRNoC0, GPCR, ["scss", "ctestsubset"]>;
defm SYEQ : ZCheri_rr<0x6, 0x4, "syeq", GPR, GPCR, GPCR, ["sceq", "csetequalexact"]>;
}

//===----------------------------------------------------------------------===//
// Special Capabilty Register Access Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtY] in {

let DecoderNamespace = "CapModeOnly_" in {
def YCSRRW : CheriCSR_ir<0b001, "csrrw">;
def YCSRRS : CheriCSR_ir<0b010, "csrrs", GPR>;
def YCSRRC : CheriCSR_ir<0b011, "csrrc", GPR>;
def YCSRRWI: CheriCSR_ii<0b101, "csrrwi">;
def YCSRRSI: CheriCSR_ii<0b110, "csrrsi">;
def YCSRRCI: CheriCSR_ii<0b111, "csrrci">;
}

def : InstAlias<"csrr $cd, $csr",
                (YCSRRS GPCR: $cd, cheri_csr_sysreg:$csr,           X0)>;
def : InstAlias<"csrw $csr, $cs",
                (YCSRRW        C0, cheri_csr_sysreg:$csr,     GPCR:$cs)>;
def : InstAlias<"csrs $csr, $cs",
                (YCSRRS        C0, cheri_csr_sysreg:$csr,      GPR:$cs)>;
def : InstAlias<"csrc $csr, $cs",
                (YCSRRC        C0, cheri_csr_sysreg:$csr,      GPR:$cs)>;

def : InstAlias<"csrwi $csr, $imm",
                (YCSRRWI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrsi $csr, $imm",
                (YCSRRSI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrci $csr, $imm",
                (YCSRRCI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;

let EmitPriority = 0 in {
def : InstAlias<"csrw $csr, $imm",
                (YCSRRWI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrs $csr, $imm",
                (YCSRRSI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrc $csr, $imm",
                (YCSRRCI       C0, cheri_csr_sysreg:$csr,   uimm5:$imm)>;

def : InstAlias<"csrrw $cd, $csr, $imm",
                (YCSRRWI GPCR:$cd, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrrs $cd, $csr, $imm",
                (YCSRRSI GPCR:$cd, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
def : InstAlias<"csrrc $cd, $csr, $imm",
                (YCSRRCI GPCR:$cd, cheri_csr_sysreg:$csr,   uimm5:$imm)>;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isCodeGenOnly = 1 in
def PseudoPCCGet : Pseudo<(outs GPCR:$rd), (ins), []>;

} // Predicates = [HasStdExtY]

//===----------------------------------------------------------------------===//
// Memory-Access with Explicit Address Type Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtZYHybrid, NotCapMode] in {
def PseudoLB_CAP  : CheriPseudoLoad<GPR, GPCRMemZeroOffset>;
def PseudoLH_CAP  : CheriPseudoLoad<GPR, GPCRMemZeroOffset>;
def PseudoLW_CAP  : CheriPseudoLoad<GPR, GPCRMemZeroOffset>;
def PseudoLBU_CAP : CheriPseudoLoad<GPR, GPCRMemZeroOffset>;
def PseudoLHU_CAP : CheriPseudoLoad<GPR, GPCRMemZeroOffset>;
def PseudoLC_CAP  : CheriPseudoLoad<GPCR, GPCRMemZeroOffset>;
}

let Predicates = [HasStdExtZYHybrid, IsRV64, NotCapMode] in {
def PseudoLWU_CAP : CheriPseudoLoad<GPR, GPCRMemZeroOffset>;
def PseudoLD_CAP  : CheriPseudoLoad<GPR, GPCRMemZeroOffset>;
}


let Predicates = [HasStdExtZYHybrid, NotCapMode] in {
def PseudoSB_CAP  : CheriPseudoStore<GPR, GPCRMemZeroOffset>;
def PseudoSH_CAP  : CheriPseudoStore<GPR, GPCRMemZeroOffset>;
def PseudoSW_CAP  : CheriPseudoStore<GPR, GPCRMemZeroOffset>;
def PseudoSC_CAP  : CheriPseudoStore<GPCR, GPCRMemZeroOffset>;
}

let Predicates = [HasStdExtZYHybrid, IsRV64] in {
def PseudoSD_CAP  : CheriPseudoStore<GPR, GPCRMemZeroOffset>;
}

//===----------------------------------------------------------------------===//
// Memory-Access Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasStdExtY, NotCapMode] in {
let DecoderNamespace = "RVYOnly_", hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
def LY  : RVInstI<0b100, OPC_MISC_MEM, (outs GPCR:$rd),
                      (ins GPR:$rs1, simm12:$imm12),
                      "ly", "$rd, ${imm12}(${rs1})">;

let DecoderNamespace = "RVYOnly_", hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
def SY  : RVInstS<0b100, OPC_STORE, (outs),
                      (ins GPCR:$rs2, GPR:$rs1, simm12:$imm12),
                      "sy", "$rs2, ${imm12}(${rs1})">;

foreach alias = ["ly", "lc"] in {
def : InstAlias<alias # " $rd, ${imm12}(${rs1})",
                (LY  GPCR:$rd, GPR:$rs1, simm12:$imm12), 0>;
def : InstAlias<alias # " $rd, (${rs1})",
                (LY  GPCR:$rd, GPR:$rs1, 0), 0>;
}

foreach alias = ["sy", "sc"] in {
def : InstAlias<"sc $rs2, ${imm12}(${rs1})",
                (SY  GPCR:$rs2, GPR:$rs1, simm12:$imm12), 0>;
def : InstAlias<alias # " $rs2, (${rs1})",
                (SY  GPCR:$rs2, GPR:$rs1, 0), 0>;
}
} // Predicates = [HasStdExtY, NotCapMode]

let Predicates = [HasStdExtY, HasStdExtA, NotCapMode] in {
let DecoderNamespace = "RVYOnly_" in {
defm LR_Y      : LR_C_r_aq_rl<"", 0b100, "lr.y">;
defm SC_Y      : AMO_C_rr_aq_rl<"", 0b00011, 0b100, "sc.y", GPR>;
defm AMOSWAP_Y : AMO_C_rr_aq_rl<"", 0b00001, 0b100, "amoswap.y", GPCR>;
}
defm LR_Y      : LR_C_r_aq_rl_aliases<["lr.c"]>;
defm SC_Y      : AMO_C_rr_aq_rl_aliases<["sc.c"], GPR>;
defm AMOSWAP_Y : AMO_C_rr_aq_rl_aliases<["amoswap.c"], GPCR>;
} // Predicates = [HasStdExtY, HasStdExtA, NotCapMode]

//===----------------------------------------------------------------------===//
// Capability Mode Instructions
//===----------------------------------------------------------------------===//

/// 'I' (Integer) base

let Predicates = [HasStdExtY, IsCapMode] in {
def : InstAlias<"auipc $rd, $imm20",
                (AUIPCC GPCR:$rd, uimm20_auipc:$imm20)>;
def : InstAlias<"call $rd, $func",
                (PseudoCCALLReg GPCR:$rd, cap_call_symbol:$func)>;
def : InstAlias<"call $func",
                (PseudoCCALL cap_call_symbol:$func)>;
def : InstAlias<"tail $dst",
                (PseudoCTAIL cap_call_symbol:$dst)>;
def : InstAlias<"jump $target, $rd",
                (PseudoCJump GPCR:$rd, pseudo_cap_jump_symbol:$target)>;
}

let Predicates = [HasStdExtY, IsCapMode] in {
let DecoderNamespace = "RVYCapModeOnly_",
    hasSideEffects = 0, mayLoad = 1, mayStore = 0 in {
def CLY : RVInstI<0b100, OPC_MISC_MEM, (outs GPCR:$rd),
                  (ins GPCR:$rs1, simm12:$imm12),
                  "ly", "$rd, ${imm12}(${rs1})">;
}
foreach alias = ["ly", "lc", "clc"] in {
def : InstAlias<alias # " $rd, ${imm12}(${rs1})",
                (CLY GPCR:$rd, GPCR:$rs1, simm12:$imm12), 0>;
def : InstAlias<alias # " $rd, (${rs1})",
                (CLY  GPCR:$rd, GPCR:$rs1, 0)>;
}
} // Predicates = [HasStdExtY, IsCapMode]

let Predicates = [HasStdExtY, IsCapMode] in {
let DecoderNamespace = "RVYCapModeOnly_", hasSideEffects = 0,
    mayLoad = 0, mayStore = 1 in
def CSY : RVInstS<0b100, OPC_STORE, (outs),
                       (ins GPCR:$rs2, GPCR:$rs1, simm12:$imm12),
                       "sy", "$rs2, ${imm12}(${rs1})">;
foreach alias = ["sy", "sc", "csc"] in {
def : InstAlias<alias # " $rs2, ${imm12}(${rs1})",
                (CSY  GPCR:$rs2, GPCR:$rs1, simm12:$imm12), 0>;
def : InstAlias<alias # " $rs2, (${rs1})",
                (CSY  GPCR:$rs2, GPCR:$rs1, 0)>;
}
} // Predicates = [HasStdExtY, IsCapMode]


/// 'A' (Atomic Instructions) extension

let Predicates = [HasStdExtY, HasStdExtA, IsCapMode] in {
defm CLR_Y       : CLR_C_r_aq_rl<"", 0b100, "lr.y", "RVYCapModeOnly_">;
defm CSC_Y       : CAMO_C_rr_aq_rl<"", 0b00011, 0b100, "sc.y", GPR, "RVYCapModeOnly_">;
defm CAMOSWAP_Y  : CAMO_C_rr_aq_rl<"", 0b00001, 0b100, "amoswap.y", GPCR, "RVYCapModeOnly_">;

defm CLR_Y : LR_C_r_aq_rl_aliases<["lr.c"], GPCRMemZeroOffset>;
defm CSC_Y : AMO_C_rr_aq_rl_aliases<["sc.c"], GPR, GPCRMemZeroOffset>;
defm CAMOSWAP_Y  : AMO_C_rr_aq_rl_aliases<["amoswap.c"], GPCR, GPCRMemZeroOffset>;
}


//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//
let Predicates = [HasStdExtY, IsCapMode] in {
def : InstAlias<"cadd $rd, $rs1, $rs2, $src",
                (PseudoCIncOffsetTPRel GPCR:$rd, GPCR:$rs1, GPRNoX0:$rs2, tprel_add_symbol:$src), 0>;
def : InstAlias<"llc $dst, $src", (PseudoCLLC GPCR:$dst, bare_symbol:$src), 0>;
def : InstAlias<"lgc $dst, $src", (PseudoCLGC GPCR:$dst, bare_symbol:$src), 0>;
}

class PatCapInspect<SDPatternOperator OpNode, RVInstZCheriSrcDst Inst>
    : Pat<(XLenVT (OpNode GPCR:$rs1)), (Inst GPCR:$rs1)>;
class PatZpGpcr<SDPatternOperator OpNode, RVInstZCheriSrcDst Inst>
    : Pat<(OpNode GPCR:$rs1), (Inst GPCR:$rs1)>;

/// Capability-Inspection Instructions

let Predicates = [HasStdExtY] in {
def : PatCapInspect<riscv_cap_tag_get,        YTAGR >;
def : PatCapInspect<int_cheri_cap_perms_get,  YPERMR>;
def : PatCapInspect<int_cheri_cap_high_get,   YHIR  >;
def : PatCapInspect<int_cheri_cap_base_get,   YBASER>;
def : PatCapInspect<int_cheri_cap_length_get, YLENR >;
def : PatCapInspect<riscv_cap_sealed_get,     YTYPER>;
def : PatCapInspect<int_cheri_cap_type_get,   YTYPER>;
let Predicates = [HasStdExtZYHybrid] in
def : PatCapInspect<int_cheri_cap_flags_get,  YMODER>;

def : Pat<(XLenVT (int_cheri_cap_offset_get GPCR:$cs1)),
          (SUB (XLenVT (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr)),
               (YBASER GPCR:$cs1))>;
def : Pat<(int_cheri_cap_offset_set GPCR:$cs1, (XLenVT GPR:$rs2)),
          (ADDY (YADDRW GPCR:$cs1, (YBASER GPCR:$cs1)),
                      GPR:$rs2)>;
} // Predicates [HasStdExtY]


// Capability-Modification Instructions

let Predicates = [HasStdExtY] in {
def : PatGpcrGpr<int_cheri_cap_address_set, YADDRW>;
def : PatGpcrGpr<int_cheri_cap_perms_and, YPERMC>;
def : PatGpcrGpr<int_cheri_cap_high_set, YHIW>;
def : PatZpGpcr<int_cheri_cap_seal_entry, YSENTRY>;
def : PatGpcrGpcr<int_cheri_cap_build, YBLD, CLenVT>;
def : PatGpcrGpr<int_cheri_cap_bounds_set, YBNDSRW>;
def : PatGpcrGpr<int_cheri_cap_bounds_set_exact, YBNDSW>;
def : Pat<(int_cheri_cap_bounds_set_exact GPCR:$rs1, csetbnd_imm:$imm),
          (YBNDSIW GPCR:$rs1, csetbnd_imm:$imm)>;
def : PatGpcrGpr<cptradd, ADDY>;
def : PatGpcrSimm12<cptradd, ADDIY>;

def : PatGpcrGpr<int_cheri_bounded_stack_cap, YBNDSRW>;
def : Pat<(int_cheri_bounded_stack_cap GPCR:$rs1, csetbnd_imm:$imm),
          (YBNDSIW GPCR:$rs1, csetbnd_imm:$imm)>;
def : PatGpcrGpr<int_cheri_bounded_stack_cap_dynamic, YBNDSRW>;
def : Pat<(int_cheri_bounded_stack_cap_dynamic GPCR:$rs1, csetbnd_imm:$imm),
          (YBNDSIW GPCR:$rs1, csetbnd_imm:$imm)>;
def : Pat<(CapFrameAddrRegImm GPCR:$rs1, simm12:$imm12),
          (ADDIY GPCR:$rs1, simm12:$imm12)>;

} // HasStdExtY

let Predicates = [HasStdExtZYHybrid] in
def : PatGpcrGpr<int_cheri_cap_flags_set, YMODEW>;

/// Pointer-Arithmetic Instructions
let Predicates = [IsPureCapABI, HasStdExtY] in {
def : Pat<(inttoptr (XLenVT GPR:$rs2)), (ADDY C0, GPR:$rs2)>;
def : Pat<(inttoptr simm12:$imm12), (ADDIY C0, simm12:$imm12)>;
def : Pat<(XLenVT (ptrtoint GPCR:$rs1)), (PseudoCGetAddr GPCR:$rs1)>;
}

/// Assertion Instructions

let Predicates = [HasStdExtY] in {
def : PatGpcrGpcr<riscv_cap_subset_test, YLT, XLenVT>;
def : PatGpcrGpcr<riscv_cap_equal_exact, SYEQ, XLenVT>;
}

/// Special Capability Register Access Instructions

let Predicates = [HasStdExtZYHybrid] in
def : Pat<(int_cheri_ddc_get), (YCSRRC CSR_DDC.Encoding, (XLenVT X0))>;

let Predicates = [HasStdExtZYHybrid, NotCapMode] in
def : Pat<(int_cheri_pcc_get), (PseudoPCCGet)>;

/// Adjusting to Compressed Capability Precision Instructions

let Predicates = [HasStdExtY] in {
def : Pat<(XLenVT (int_cheri_round_representable_length (XLenVT GPR:$rs1))),
          (AND (ADD GPR:$rs1, (XORI (YAMASK GPR:$rs1), -1)), (YAMASK GPR:$rs1))>;
def : PatGpr<int_cheri_representable_alignment_mask, YAMASK>;
}

/// Capability loads

let Predicates = [HasStdExtZYHybrid, NotCapMode] in {
defm : CheriExplicitLdPat<sextloadi8,  PseudoLB_CAP,  GPCR, CLenVT, XLenVT>;
defm : CheriExplicitLdPat<extloadi8,   PseudoLB_CAP,  GPCR, CLenVT, XLenVT>;
defm : CheriExplicitLdPat<sextloadi16, PseudoLH_CAP,  GPCR, CLenVT, XLenVT>;
defm : CheriExplicitLdPat<extloadi16,  PseudoLH_CAP,  GPCR, CLenVT, XLenVT>;
defm : CheriExplicitLdPat<zextloadi8,  PseudoLBU_CAP, GPCR, CLenVT, XLenVT>;
defm : CheriExplicitLdPat<zextloadi16, PseudoLHU_CAP, GPCR, CLenVT, XLenVT>;
defm : CheriExplicitLdPat<load,        PseudoLC_CAP,  GPCR, CLenVT, CLenVT>;
}

let Predicates = [HasStdExtZYHybrid, NotCapMode, IsRV32] in {
defm : CheriExplicitLdPat<load, PseudoLW_CAP, GPCR, CLenVT, XLenVT>;
}

let Predicates = [HasStdExtZYHybrid, NotCapMode, IsRV64] in {
defm : CheriExplicitLdPat<sextloadi32, PseudoLW_CAP,  GPCR, CLenVT, XLenVT>;
defm : CheriExplicitLdPat<extloadi32,  PseudoLW_CAP,  GPCR, CLenVT, XLenVT>;
defm : CheriExplicitLdPat<load,        PseudoLD_CAP,  GPCR, CLenVT, XLenVT>;
defm : CheriExplicitLdPat<zextloadi32, PseudoLWU_CAP, GPCR, CLenVT, XLenVT>;
}

let Predicates = [HasStdExtZYHybrid, NotCapMode, HasStdExtF] in
def : Pat<(load GPCR:$rs1), (FMV_W_X (PseudoLW_CAP GPCR:$rs1))>;

let Predicates = [HasStdExtZYHybrid, NotCapMode, HasStdExtD, IsRV32] in
def : Pat<(load GPCR:$rs1),
          (BuildPairF64Pseudo (PseudoLW_CAP GPCR:$rs1),
                              (PseudoLW_CAP (CIncOffsetImm GPCR:$rs1, 4)))>;

let Predicates = [HasCheriOrRVY, NotCapMode, HasStdExtD, IsRV64] in
def : Pat<(load GPCR:$rs1), (FMV_D_X (PseudoLD_CAP GPCR:$rs1))>;

/// Capability Stores

let Predicates = [HasStdExtZYHybrid, NotCapMode] in {
defm : CheriExplicitStPat<truncstorei8,  PseudoSB_CAP, GPR,  GPCR, XLenVT, CLenVT>;
defm : CheriExplicitStPat<truncstorei16, PseudoSH_CAP, GPR,  GPCR, XLenVT, CLenVT>;
defm : CheriExplicitStPat<store,         PseudoSC_CAP, GPCR, GPCR, CLenVT, CLenVT>;
}

let Predicates = [HasStdExtZYHybrid, NotCapMode, IsRV32] in {
defm : CheriExplicitStPat<store, PseudoSW_CAP, GPR, GPCR, XLenVT, CLenVT>;
}

let Predicates = [HasCheriOrRVY, NotCapMode, IsRV64] in {
defm : CheriExplicitStPat<truncstorei32, PseudoSW_CAP, GPR, GPCR, XLenVT, CLenVT>;
defm : CheriExplicitStPat<store,         PseudoSD_CAP, GPR, GPCR, XLenVT, CLenVT>;
}

let Predicates = [HasCheriOrRVY, NotCapMode, HasStdExtF] in
def : Pat<(store FPR32:$rs2, GPCR:$rs1),
          (PseudoSW_CAP (FMV_X_W FPR32:$rs2), GPCR:$rs1)>;

let Predicates = [HasCheriOrRVY, NotCapMode, HasStdExtD, IsRV64] in
def : Pat<(store FPR64:$rs2, GPCR:$rs1),
          (PseudoSD_CAP (FMV_X_D FPR64:$rs2), GPCR:$rs1)>;

/// Memory-Access Instructions

let Predicates = [HasStdExtY, NotCapMode] in {
def  : LdPat<load, LY, CLenVT>;
def  : StPat<store, SY, GPCR, CLenVT>;
def  : LdPat<atomic_load_cap, LY, CLenVT>;
def  : AtomicStPat<atomic_store_cap, SY, GPCR, CLenVT>;
defm : AMOCapPat<"", "atomic_swap_cap", "AMOSWAP_Y">;
}

let Predicates = [HasStdExtY, HasStdExtA, IsCapMode] in {
defm : CheriLdPat<atomic_load_cap, CLY, CLenVT>;
defm : CheriAtomicStPat<atomic_store_cap, CSY, GPCR, CLenVT>;
defm : CheriAMOCapPat<"","atomic_swap_cap", "CAMOSWAP_Y">;
}

let Predicates = [HasStdExtY, IsCapMode] in {
defm : CheriLdPat<load, CLY, CLenVT>;
defm : CheriStPat<store, CSY, GPCR, CLenVT>;
}

//===----------------------------------------------------------------------===//
// Compress Instruction tablegen backend.
//===----------------------------------------------------------------------===//

// Quadrant 0
let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(ADDIY GPCRC:$rd, CSP:$rs1, uimm10_lsb00nonzero:$imm),
                  (C_CIncOffsetImm4CSPN GPCRC:$rd, CSP:$rs1, uimm10_lsb00nonzero:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsCapMode]

let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CLY GPCRC:$rd, GPCRC:$rs1, uimm9_lsb0000:$imm),
                  (C_CLC_128 GPCRC:$rd, GPCRC:$rs1, uimm9_lsb0000:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in {
def : CompressPat<(CLY GPCRC:$rd, GPCRC:$rs1, uimm8_lsb000:$imm),
                  (C_CLC_64 GPCRC:$rd, GPCRC:$rs1, uimm8_lsb000:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]

let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CSY GPCRC:$rs2, GPCRC:$rs1, uimm9_lsb0000:$imm),
                  (C_CSC_128 GPCRC:$rs2, GPCRC:$rs1, uimm9_lsb0000:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in {
def : CompressPat<(CSY GPCRC:$rs2, GPCRC:$rs1, uimm8_lsb000:$imm),
                  (C_CSC_64 GPCRC:$rs2, GPCRC:$rs1, uimm8_lsb000:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]

// Quadrant 1
let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(ADDIY C2, C2, simm10_lsb0000nonzero:$imm),
                  (C_CIncOffsetImm16CSP C2, simm10_lsb0000nonzero:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsCapMode]

// Quadrant 2
let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CLY GPCRNoC0:$rd, CSP:$rs1, uimm10_lsb0000:$imm),
                  (C_CLCCSP_128 GPCRNoC0:$rd, CSP:$rs1, uimm10_lsb0000:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in {
def : CompressPat<(CLY GPCRNoC0:$rd, CSP:$rs1, uimm9_lsb000:$imm),
                  (C_CLCCSP_64 GPCRNoC0:$rd, CSP:$rs1, uimm9_lsb000:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]

let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CSY GPCR:$rs2, CSP:$rs1, uimm10_lsb0000:$imm),
                  (C_CSCCSP_128 GPCR:$rs2, CSP:$rs1, uimm10_lsb0000:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in {
def : CompressPat<(CSY GPCR:$rs2, CSP:$rs1, uimm9_lsb000:$imm),
                  (C_CSCCSP_64 GPCR:$rs2, CSP:$rs1, uimm9_lsb000:$imm)>;
} // Predicates = [HasStdExtY, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]
