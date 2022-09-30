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
def riscv_cap_equal_exact : SDNode<"RISCVISD::CAP_EQUAL_EXACT",
                                   SDT_RISCVCheriBoolBinary>;

//===----------------------------------------------------------------------===//
// Operand and SDNode transformation definitions.
//===----------------------------------------------------------------------===//

def CCallSymbol : AsmOperandClass {
  let Name = "CCallSymbol";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidCCallSymbol";
  let ParserMethod = "parseCallSymbol<true>";
}

def cap_call_symbol : Operand<CLenVT> {
  let ParserMatchClass = CCallSymbol;
}

def PseudoCJumpSymbol : AsmOperandClass {
  let Name = "PseudoCJumpSymbol";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidPseudoCJumpSymbol";
  let ParserMethod = "parsePseudoJumpSymbol<true>";
}

def pseudo_cap_jump_symbol : Operand<CLenVT> {
  let ParserMatchClass = PseudoCJumpSymbol;
}

def TPRelCIncOffsetSymbol : AsmOperandClass {
  let Name = "TPRelCIncOffsetSymbol";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidTPRelCIncOffsetSymbol";
  let ParserMethod = "parseOperandWithModifier";
}

// A bare symbol with the %tprel_add variant.
def tprel_cincoffset_symbol : Operand<XLenVT> {
  let ParserMatchClass = TPRelCIncOffsetSymbol;
}

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

// A 9-bit unsigned immediate where the least significant four bits are zero.
def uimm9_lsb0000 : Operand<XLenVT>,
                    ImmLeaf<XLenVT, [{return isShiftedUInt<5, 4>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<9, "Lsb0000">;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<9>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isShiftedUInt<5, 4>(Imm);
  }];
}

// A 10-bit unsigned immediate where the least significant four bits are zero.
def uimm10_lsb0000 : Operand<XLenVT>,
                     ImmLeaf<XLenVT, [{return isShiftedUInt<6, 4>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<10, "Lsb0000">;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<10>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isShiftedUInt<6, 4>(Imm);
  }];
}

def CapFI64  : ComplexPattern<iFATPTR64,  1, "SelectCapFI", [frameindex], []>;
def CapFI128 : ComplexPattern<iFATPTR128, 1, "SelectCapFI", [frameindex], []>;

//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//

include "RISCVInstrFormatsXCheri.td"

//===----------------------------------------------------------------------===//
// Instruction Class Templates
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_r<bits<5> funct5, string opcodestr, RegisterClass rdClass=GPR,
              DAGOperand rs1Operand=GPCR>
    : RVInstCheriSrcDst<0x7f, funct5, 0, OPC_CHERI, (outs rdClass:$rd),
                        (ins rs1Operand:$rs1), opcodestr, "$rd, $rs1">;

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

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class CheriLoad_ri<bits<3> funct3, string opcodestr>
    : RVInstI<funct3, OPC_LOAD, (outs GPR:$rd), (ins GPCR:$rs1, simm12:$imm12),
              opcodestr, "$rd, ${imm12}(${rs1})">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
class CheriStore_ri<bits<3> funct3, string opcodestr>
    : RVInstS<funct3, OPC_STORE, (outs),
              (ins GPR:$rs2, GPCR:$rs1, simm12:$imm12),
              opcodestr, "$rs2, ${imm12}(${rs1})">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class CLR_r<bit aq, bit rl, bits<3> funct3, string opcodestr>
    : RVInstRAtomic<0b00010, aq, rl, funct3, OPC_AMO,
                    (outs GPR:$rd), (ins GPCRMemAtomic:$rs1),
                    opcodestr, "$rd, $rs1"> {
  let rs2 = 0;
}

multiclass CLR_r_aq_rl<bits<3> funct3, string opcodestr> {
  def ""     : CLR_r<0, 0, funct3, opcodestr>;
  def _AQ    : CLR_r<1, 0, funct3, opcodestr # ".aq">;
  def _RL    : CLR_r<0, 1, funct3, opcodestr # ".rl">;
  def _AQ_RL : CLR_r<1, 1, funct3, opcodestr # ".aqrl">;
}

let hasSideEffects = 0, mayLoad = 1, mayStore = 1 in
class CAMO_rr<bits<5> funct5, bit aq, bit rl, bits<3> funct3, string opcodestr>
    : RVInstRAtomic<funct5, aq, rl, funct3, OPC_AMO,
                    (outs GPR:$rd), (ins GPCRMemAtomic:$rs1, GPR:$rs2),
                    opcodestr, "$rd, $rs2, $rs1">;

multiclass CAMO_rr_aq_rl<bits<5> funct5, bits<3> funct3, string opcodestr> {
  def ""     : CAMO_rr<funct5, 0, 0, funct3, opcodestr>;
  def _AQ    : CAMO_rr<funct5, 1, 0, funct3, opcodestr # ".aq">;
  def _RL    : CAMO_rr<funct5, 0, 1, funct3, opcodestr # ".rl">;
  def _AQ_RL : CAMO_rr<funct5, 1, 1, funct3, opcodestr # ".aqrl">;
}

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class CLR_C_r<bit aq, bit rl, bits<3> funct3, string opcodestr>
    : RVInstRAtomic<0b00010, aq, rl, funct3, OPC_AMO,
                    (outs GPCR:$rd), (ins GPCRMemAtomic:$rs1),
                    opcodestr, "$rd, $rs1"> {
  let rs2 = 0;
}

multiclass CLR_C_r_aq_rl<string clenstr, bits<3> funct3, string opcodestr> {
  def _ # clenstr       : CLR_C_r<0, 0, funct3, opcodestr>;
  def _AQ_ # clenstr    : CLR_C_r<1, 0, funct3, opcodestr # ".aq">;
  def _RL_ # clenstr    : CLR_C_r<0, 1, funct3, opcodestr # ".rl">;
  def _AQ_RL_ # clenstr : CLR_C_r<1, 1, funct3, opcodestr # ".aqrl">;
}

let hasSideEffects = 0, mayLoad = 1, mayStore = 1 in
class CAMO_C_rr<bits<5> funct5, bit aq, bit rl, bits<3> funct3,
                string opcodestr, RegisterClass rdClass>
    : RVInstRAtomic<funct5, aq, rl, funct3, OPC_AMO,
                    (outs rdClass:$rd), (ins GPCRMemAtomic:$rs1, GPCR:$rs2),
                    opcodestr, "$rd, $rs2, $rs1">;

multiclass CAMO_C_rr_aq_rl<string clenstr, bits<5> funct5, bits<3> funct3,
                          string opcodestr, RegisterClass rdClass> {
  def _ # clenstr       : CAMO_C_rr<funct5, 0, 0, funct3, opcodestr, rdClass>;
  def _AQ_ # clenstr    : CAMO_C_rr<funct5, 1, 0, funct3, opcodestr # ".aq",
                                    rdClass>;
  def _RL_ # clenstr    : CAMO_C_rr<funct5, 0, 1, funct3, opcodestr # ".rl",
                                    rdClass>;
  def _AQ_RL_ # clenstr : CAMO_C_rr<funct5, 1, 1, funct3, opcodestr # ".aqrl",
                                    rdClass>;
}

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class CCheriStackLoad<bits<3> funct3, string OpcodeStr,
                      RegisterClass cls, DAGOperand opnd>
    : RVInst16CI<funct3, 0b10, (outs cls:$rd), (ins CSP:$rs1, opnd:$imm),
                 OpcodeStr, "$rd, ${imm}(${rs1})">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
class CCheriStackStore<bits<3> funct3, string OpcodeStr,
                       RegisterClass cls, DAGOperand opnd>
    : RVInst16CSS<funct3, 0b10, (outs), (ins cls:$rs2, CSP:$rs1, opnd:$imm),
                  OpcodeStr, "$rs2, ${imm}(${rs1})">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class CCheriLoad_ri<bits<3> funct3, string OpcodeStr,
                    RegisterClass cls, DAGOperand opnd>
    : RVInst16CL<funct3, 0b00, (outs cls:$rd), (ins GPCRC:$rs1, opnd:$imm),
                 OpcodeStr, "$rd, ${imm}(${rs1})">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
class CCheriStore_rri<bits<3> funct3, string OpcodeStr,
                      RegisterClass cls, DAGOperand opnd>
    : RVInst16CS<funct3, 0b00, (outs), (ins cls:$rs2, GPCRC:$rs1, opnd:$imm),
                 OpcodeStr, "$rs2, ${imm}(${rs1})">;

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
let Constraints = "@traps_if_sealed $rs1" in {
let mayTrap = 1 in {
let defsCanBeSealed = 1 in
def CSeal           : Cheri_rr<0xb, "cseal", GPCR, GPCR>;
def CUnseal         : Cheri_rr<0xc, "cunseal", GPCR, GPCR>;
def CAndPerm        : Cheri_rr<0xd, "candperm">;
} // let mayTrap = 1
def CSetFlags       : Cheri_rr<0xe, "csetflags">;
def CSetOffset      : Cheri_rr<0xf, "csetoffset">;
def CSetAddr        : Cheri_rr<0x10, "csetaddr">;
let isReMaterializable = 1, isAsCheapAsAMove = 1 in
def CIncOffset      : Cheri_rr<0x11, "cincoffset">;
let isReMaterializable = 1, isAsCheapAsAMove = 1 in
def CIncOffsetImm   : Cheri_ri<0x1, "cincoffset", 1>;
// Note: mayTrap is optimized in RISCVInstrInfo::isGuaranteedNotToTrap()
let mayTrap = 1 in {
def CSetBounds      : Cheri_rr<0x8, "csetbounds">;
def CSetBoundsExact : Cheri_rr<0x9, "csetboundsexact">;
def CSetBoundsImm   : Cheri_ri<0x2, "csetbounds", 0>;
} // mayTrap = 1
} // let Constraints = "@traps_if_sealed $rs1"
def CClearTag       : Cheri_r<0xb, "ccleartag", GPCR>;
let mayTrap = 1 in {
let defsCanBeSealed = 1 in
def CBuildCap       : Cheri_rr<0x1d, "cbuildcap", GPCR, GPCR, GPCRC0IsDDC>;
def CCopyType       : Cheri_rr<0x1e, "ccopytype", GPCR, GPCR>;
let defsCanBeSealed = 1 in
def CCSeal          : Cheri_rr<0x1f, "ccseal", GPCR, GPCR>;
let defsCanBeSealed = 1 in
def CSealEntry      : Cheri_r<0x11, "csealentry", GPCR>;
} // let mayTrap = 1

def : InstAlias<"cincoffsetimm $cd, $cs1, $imm",
                (CIncOffsetImm GPCR:$cd, GPCR:$cs1, simm12:$imm), 0>;
def : InstAlias<"csetboundsimm $cd, $cs1, $imm",
                (CSetBoundsImm GPCR:$cd, GPCR:$cs1, uimm12:$imm), 0>;
}

//===----------------------------------------------------------------------===//
// Pointer-Arithmetic Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
let mayTrap = 1 in {
def CToPtr      : Cheri_rr<0x12, "ctoptr", GPR, GPCRC0IsDDC>;
def CFromPtr    : Cheri_rr<0x13, "cfromptr", GPCR, GPR, GPCRC0IsDDC>;
}
def CSub        : Cheri_rr<0x14, "csub", GPR, GPCR>;
let isMoveReg = 1, isReMaterializable = 1, isAsCheapAsAMove = 1,
    defsCanBeSealed = 1 in
def CMove       : Cheri_r<0xa, "cmove", GPCR>;
}

//===----------------------------------------------------------------------===//
// Control-Flow Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def JALR_CAP : RVInstCheriSrcDst<0x7f, 0xc, 0, OPC_CHERI, (outs GPCR:$rd),
                                 (ins GPCR:$rs1), "jalr.cap", "$rd, $rs1">;

let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def JALR_PCC : RVInstCheriSrcDst<0x7f, 0x14, 0, OPC_CHERI, (outs GPR:$rd),
                                 (ins GPR:$rs1), "jalr.pcc", "$rd, $rs1">;

let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def CInvoke : RVInstCheriTwoSrc<0x7e, 0x1, 0, OPC_CHERI, (outs),
                                (ins GPCR:$rs1, GPCR:$rs2),
                                "cinvoke", "$rs1, $rs2">;

def : InstAlias<"jalr.cap $cs1", (JALR_CAP C1, GPCR:$cs1), 1>;
def : InstAlias<"jr.cap $cs1",   (JALR_CAP C0, GPCR:$cs1), 1>;
def : InstAlias<"ret.cap",       (JALR_CAP C0,        C1), 2>;

def : InstAlias<"jalr.pcc $cs1", (JALR_PCC X1, GPR:$cs1), 1>;
def : InstAlias<"jr.pcc $cs1",   (JALR_PCC X0, GPR:$cs1), 1>;
def : InstAlias<"ret.pcc",       (JALR_PCC X0,       X1), 2>;
}

//===----------------------------------------------------------------------===//
// Assertion Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri] in {
def CTestSubset : Cheri_rr<0x20, "ctestsubset", GPR, GPCR, GPCRC0IsDDC>;
def CSEQX       : Cheri_rr<0x21, "csetequalexact", GPR, GPCR, GPCR>;

def : InstAlias<"cseqx $rd, $cs1, $cs2", (CSEQX GPR:$rd, GPCR:$cs1, GPCR:$cs2)>;
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
                (CSpecialRW GPCR:$cd, special_capreg:$scr,           C0)>;
def : InstAlias<"cspecialw $scr, $cs",
                (CSpecialRW       C0, special_capreg:$scr, GPCRNoC0:$cs)>;
}

//===----------------------------------------------------------------------===//
// Fast Register-Clearing Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri], hasSideEffects = 1 in {
def CClear  : Cheri_clear<0xe, "cclear">;
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
// Tag-Memory Access Instructions
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri], hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
def CLoadTags : Cheri_r<0x12, "cloadtags", GPR, GPCRMemAtomic>;

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

let DecoderNamespace = "RISCV32Only_",
    Predicates = [HasCheri, HasStdExtA, IsRV32, NotCapMode] in {
defm LR_C       : LR_C_r_aq_rl<"64", 0b011, "lr.c">;
defm SC_C       : AMO_C_rr_aq_rl<"64", 0b00011, 0b011, "sc.c", GPR>;
defm AMOSWAP_C  : AMO_C_rr_aq_rl<"64", 0b00001, 0b011, "amoswap.c", GPCR>;
}

let Predicates = [HasCheri, HasStdExtA, IsRV64, NotCapMode] in {
defm LR_C       : LR_C_r_aq_rl<"128", 0b100, "lr.c">;
defm SC_C       : AMO_C_rr_aq_rl<"128", 0b00011, 0b100, "sc.c", GPR>;
defm AMOSWAP_C  : AMO_C_rr_aq_rl<"128", 0b00001, 0b100, "amoswap.c", GPCR>;
}

//===----------------------------------------------------------------------===//
// Capability Mode Instructions
//===----------------------------------------------------------------------===//


/// 'I' (Integer) base

let DecoderNamespace = "CapModeOnly_" in {
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
let Predicates = [HasCheri, IsCapMode] in {
def AUIPCC : RVInstU<OPC_AUIPC, (outs GPCR:$rd), (ins uimm20_auipc:$imm20),
                     "auipcc", "$rd, $imm20">;

let isCall = 1 in
def CJAL : RVInstJ<OPC_JAL, (outs GPCR:$rd), (ins simm21_lsb0_jal:$imm20),
                   "cjal", "$rd, $imm20">;

let isCall = 1 in
def CJALR : RVInstI<0b000, OPC_JALR, (outs GPCR:$rd),
                    (ins GPCR:$rs1, simm12:$imm12),
                    "cjalr", "$rd, ${imm12}(${rs1})">;
} // Predicates = [HasCheri, IsCapMode]
} // hasSideEffects = 0, mayLoad = 0, mayStore = 0
} // DecoderNameSpace = "CapModeOnly_"

let Predicates = [HasCheri, IsCapMode] in {
def : InstAlias<"j $offset",    (CJAL C0, simm21_lsb0_jal:$offset)>;
def : InstAlias<"cjal $offset", (CJAL C1, simm21_lsb0_jal:$offset)>;

// Non-canonical form; JAL X0 and CJAL X0 are semantically identical so "j" is
// the canonical form, but we provide this for completeness.
def : InstAlias<"cj $offset",   (CJAL C0, simm21_lsb0_jal:$offset), 0>;

// Non-zero offset aliases of "cjalr" are the lowest weight, followed by the
// two-register form, then the one-register forms and finally "cret".
def : InstAlias<"cjr $rs",                (CJALR       C0, GPCR:$rs, 0), 3>;
def : InstAlias<"cjr ${offset}(${rs})",   (CJALR       C0, GPCR:$rs, simm12:$offset)>;
def : InstAlias<"cjalr $rs",              (CJALR       C1, GPCR:$rs, 0), 3>;
def : InstAlias<"cjalr ${offset}(${rs})", (CJALR       C1, GPCR:$rs, simm12:$offset)>;
def : InstAlias<"cjalr $rd, $rs",         (CJALR GPCR:$rd, GPCR:$rs, 0), 2>;
def : InstAlias<"cret",                   (CJALR       C0,       C1, 0), 4>;

// Non-canonical forms for jump targets also accepted by the assembler.
def : InstAlias<"cjr $rs, $offset",        (CJALR       C0, GPCR:$rs, simm12:$offset), 0>;
def : InstAlias<"cjalr $rs, $offset",      (CJALR       C1, GPCR:$rs, simm12:$offset), 0>;
def : InstAlias<"cjalr $rd, $rs, $offset", (CJALR GPCR:$rd, GPCR:$rs, simm12:$offset), 0>;
} // Predicates = [HasCheri, IsCapMode]

let DecoderNamespace = "CapModeOnly_" in {
let Predicates = [HasCheri, IsCapMode] in {
def CLB  : CheriLoad_ri<0b000, "clb">;
def CLH  : CheriLoad_ri<0b001, "clh">;
def CLW  : CheriLoad_ri<0b010, "clw">;
def CLBU : CheriLoad_ri<0b100, "clbu">;
def CLHU : CheriLoad_ri<0b101, "clhu">;

def CSB : CheriStore_ri<0b000, "csb">;
def CSH : CheriStore_ri<0b001, "csh">;
def CSW : CheriStore_ri<0b010, "csw">;
} // Predicates = [HasCheri, IsCapMode]

let Predicates = [HasCheri, IsRV64, IsCapMode] in {
def CLWU   : CheriLoad_ri<0b110, "clwu">;
def CLD    : CheriLoad_ri<0b011, "cld">;
def CSD    : CheriStore_ri<0b011, "csd">;
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

let DecoderNamespace = "CapModeOnly_" in {
let Predicates = [HasCheri, HasStdExtA, IsCapMode] in {
defm CLR_B       : CLR_r_aq_rl<0b000, "clr.b">;
defm CSC_B       : CAMO_rr_aq_rl<0b00011, 0b000, "csc.b">;

defm CLR_H       : CLR_r_aq_rl<0b001, "clr.h">;
defm CSC_H       : CAMO_rr_aq_rl<0b00011, 0b001, "csc.h">;

defm CLR_W       : CLR_r_aq_rl<0b010, "clr.w">;
defm CSC_W       : CAMO_rr_aq_rl<0b00011, 0b010, "csc.w">;
defm CAMOSWAP_W  : CAMO_rr_aq_rl<0b00001, 0b010, "camoswap.w">;
defm CAMOADD_W   : CAMO_rr_aq_rl<0b00000, 0b010, "camoadd.w">;
defm CAMOXOR_W   : CAMO_rr_aq_rl<0b00100, 0b010, "camoxor.w">;
defm CAMOAND_W   : CAMO_rr_aq_rl<0b01100, 0b010, "camoand.w">;
defm CAMOOR_W    : CAMO_rr_aq_rl<0b01000, 0b010, "camoor.w">;
defm CAMOMIN_W   : CAMO_rr_aq_rl<0b10000, 0b010, "camomin.w">;
defm CAMOMAX_W   : CAMO_rr_aq_rl<0b10100, 0b010, "camomax.w">;
defm CAMOMINU_W  : CAMO_rr_aq_rl<0b11000, 0b010, "camominu.w">;
defm CAMOMAXU_W  : CAMO_rr_aq_rl<0b11100, 0b010, "camomaxu.w">;
} // Predicates = [HasCheri, HasStdExtA, IsCapMode]

let Predicates = [HasCheri, HasStdExtA, IsRV64, IsCapMode] in {
defm CLR_D       : CLR_r_aq_rl<0b011, "clr.d">;
defm CSC_D       : CAMO_rr_aq_rl<0b00011, 0b011, "csc.d">;
defm CAMOSWAP_D  : CAMO_rr_aq_rl<0b00001, 0b011, "camoswap.d">;
defm CAMOADD_D   : CAMO_rr_aq_rl<0b00000, 0b011, "camoadd.d">;
defm CAMOXOR_D   : CAMO_rr_aq_rl<0b00100, 0b011, "camoxor.d">;
defm CAMOAND_D   : CAMO_rr_aq_rl<0b01100, 0b011, "camoand.d">;
defm CAMOOR_D    : CAMO_rr_aq_rl<0b01000, 0b011, "camoor.d">;
defm CAMOMIN_D   : CAMO_rr_aq_rl<0b10000, 0b011, "camomin.d">;
defm CAMOMAX_D   : CAMO_rr_aq_rl<0b10100, 0b011, "camomax.d">;
defm CAMOMINU_D  : CAMO_rr_aq_rl<0b11000, 0b011, "camominu.d">;
defm CAMOMAXU_D  : CAMO_rr_aq_rl<0b11100, 0b011, "camomaxu.d">;
} // Predicates = [HasCheri, HasStdExtA, IsRV64, IsCapMode]
} // DecoderNamespace = "CapModeOnly_"

let DecoderNamespace = "RISCV32CapModeOnly_",
    Predicates = [HasCheri, HasStdExtA, IsRV32, IsCapMode] in {
defm CLR_C       : CLR_C_r_aq_rl<"64", 0b011, "clr.c">;
defm CSC_C       : CAMO_C_rr_aq_rl<"64", 0b00011, 0b011, "csc.c", GPR>;
defm CAMOSWAP_C  : CAMO_C_rr_aq_rl<"64", 0b00001, 0b011, "camoswap.c", GPCR>;
}

let DecoderNamespace = "CapModeOnly_",
    Predicates = [HasCheri, HasStdExtA, IsRV64, IsCapMode] in {
defm CLR_C       : CLR_C_r_aq_rl<"128", 0b100, "clr.c">;
defm CSC_C       : CAMO_C_rr_aq_rl<"128", 0b00011, 0b100, "csc.c", GPR>;
defm CAMOSWAP_C  : CAMO_C_rr_aq_rl<"128", 0b00001, 0b100, "camoswap.c", GPCR>;
}

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

let DecoderNamespace = "CapModeOnly_" in {
let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {

let hasSideEffects = 0, mayLoad = 0, mayStore = 0, Uses = [C2] in
def C_CIncOffsetImm4CSPN : RVInst16CIW<0b000, 0b00, (outs GPCRC:$rd),
                                       (ins CSP:$rs1, uimm10_lsb00nonzero:$imm),
                                       "c.cincoffset4cspn", "$rd, $rs1, $imm"> {
  bits<5> rs1;
  let Inst{12-11} = imm{5-4};
  let Inst{10-7} = imm{9-6};
  let Inst{6} = imm{2};
  let Inst{5} = imm{3};
}

let DecoderNamespace = "RISCV32CapModeOnly_",
    Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode] in
def C_CFLD : CCheriLoad_ri<0b001, "c.cfld", FPR64C, uimm8_lsb000> {
  bits<8> imm;
  let Inst{12-10} = imm{5-3};
  let Inst{6-5} = imm{7-6};
}

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in
def C_CLC_128 : CCheriLoad_ri<0b001, "c.clc", GPCRC, uimm9_lsb0000> {
  bits<9> imm;
  let Inst{12-11} = imm{5-4};
  let Inst{10} = imm{8};
  let Inst{6-5} = imm{7-6};
}

def C_CLW : CCheriLoad_ri<0b010, "c.clw", GPRC, uimm7_lsb00> {
  bits<7> imm;
  let Inst{12-10} = imm{5-3};
  let Inst{6} = imm{2};
  let Inst{5} = imm{6};
}

let DecoderNamespace = "RISCV32CapModeOnly_",
    Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in
def C_CLC_64 : CCheriLoad_ri<0b011, "c.clc", GPCRC, uimm8_lsb000> {
  bits<8> imm;
  let Inst{12-10} = imm{5-3};
  let Inst{6-5} = imm{7-6};
}

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in
def C_CLD : CCheriLoad_ri<0b011, "c.cld", GPRC, uimm8_lsb000> {
  bits<8> imm;
  let Inst{12-10} = imm{5-3};
  let Inst{6-5} = imm{7-6};
}

let DecoderNamespace = "RISCV32CapModeOnly_",
    Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode] in
def C_CFSD : CCheriStore_rri<0b101, "c.cfsd", FPR64C, uimm8_lsb000> {
  bits<8> imm;
  let Inst{12-10} = imm{5-3};
  let Inst{6-5} = imm{7-6};
}

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in
def C_CSC_128 : CCheriStore_rri<0b101, "c.csc", GPCRC, uimm9_lsb0000> {
  bits<9> imm;
  let Inst{12-11} = imm{5-4};
  let Inst{10} = imm{8};
  let Inst{6-5} = imm{7-6};
}

def C_CSW : CCheriStore_rri<0b110, "c.csw", GPRC, uimm7_lsb00> {
  bits<7> imm;
  let Inst{12-10} = imm{5-3};
  let Inst{6} = imm{2};
  let Inst{5} = imm{6};
}

let DecoderNamespace = "RISCV32CapModeOnly_",
    Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]  in
def C_CSC_64 : CCheriStore_rri<0b111, "c.csc", GPCRC, uimm8_lsb000> {
  bits<8> imm;
  let Inst{12-10} = imm{5-3};
  let Inst{6-5} = imm{7-6};
}

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in
def C_CSD : CCheriStore_rri<0b111, "c.csd", GPRC, uimm8_lsb000> {
  bits<8> imm;
  let Inst{12-10} = imm{5-3};
  let Inst{6-5} = imm{7-6};
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isCall = 1,
    DecoderNamespace = "RISCV32CapModeOnly_", Defs = [C1],
    Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]  in
def C_CJAL : RVInst16CJ<0b001, 0b01, (outs), (ins simm12_lsb0:$offset),
                        "c.cjal", "$offset">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def C_CIncOffsetImm16CSP : RVInst16CI<0b011, 0b01, (outs CSP:$rd_wb),
                                      (ins CSP:$rd, simm10_lsb0000nonzero:$imm),
                                      "c.cincoffset16csp", "$rd, $imm"> {
  let Constraints = "$rd = $rd_wb";
  let Inst{12} = imm{9};
  let Inst{11-7} = 2;
  let Inst{6} = imm{4};
  let Inst{5} = imm{6};
  let Inst{4-3} = imm{8-7};
  let Inst{2} = imm{5};
}

let DecoderNamespace = "RISCV32CapModeOnly_",
    Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode] in
def C_CFLDCSP : CCheriStackLoad<0b001, "c.cfldcsp", FPR64, uimm9_lsb000> {
  let Inst{6-5} = imm{4-3};
  let Inst{4-2} = imm{8-6};
}

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in
def C_CLCCSP_128 : CCheriStackLoad<0b001, "c.clccsp", GPCRNoC0, uimm10_lsb0000> {
  let Inst{6} = imm{4};
  let Inst{5-2} = imm{9-6};
}

def C_CLWCSP : CCheriStackLoad<0b010, "c.clwcsp", GPRNoX0, uimm8_lsb00> {
  let Inst{6-4} = imm{4-2};
  let Inst{3-2} = imm{7-6};
}

let DecoderNamespace = "RISCV32CapModeOnly_",
    Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in
def C_CLCCSP_64 : CCheriStackLoad<0b011, "c.clccsp", GPCRNoC0, uimm9_lsb000> {
  let Inst{6-5} = imm{4-3};
  let Inst{4-2} = imm{8-6};
}

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in
def C_CLDCSP : CCheriStackLoad<0b011, "c.cldcsp", GPRNoX0, uimm9_lsb000> {
  let Inst{6-5} = imm{4-3};
  let Inst{4-2} = imm{8-6};
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def C_CJR : RVInst16CR<0b1000, 0b10, (outs), (ins GPCRNoC0:$rs1),
                       "c.cjr", "$rs1"> {
  let isBranch = 1;
  let isBarrier = 1;
  let isTerminator = 1;
  let isIndirectBranch = 1;
  let rs2 = 0;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0,
    isCall = 1, Defs = [C1], rs2 = 0 in
def C_CJALR : RVInst16CR<0b1001, 0b10, (outs), (ins GPCRNoC0:$rs1),
                         "c.cjalr", "$rs1">;

let DecoderNamespace = "RISCV32CapModeOnly_",
    Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode] in
def C_CFSDCSP : CCheriStackStore<0b101, "c.cfsdcsp", FPR64, uimm9_lsb000> {
  let Inst{12-10} = imm{5-3};
  let Inst{9-7}   = imm{8-6};
}

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in
def C_CSCCSP_128 : CCheriStackStore<0b101, "c.csccsp", GPCR, uimm10_lsb0000> {
  let Inst{12-11} = imm{5-4};
  let Inst{10-7}  = imm{9-6};
}

def C_CSWCSP : CCheriStackStore<0b110, "c.cswcsp", GPR, uimm8_lsb00> {
  let Inst{12-9} = imm{5-2};
  let Inst{8-7}  = imm{7-6};
}

let DecoderNamespace = "RISCV32CapModeOnly_",
    Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in
def C_CSCCSP_64 : CCheriStackStore<0b111, "c.csccsp", GPCR, uimm9_lsb000> {
  let Inst{12-10} = imm{5-3};
  let Inst{9-7}   = imm{8-6};
}

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in
def C_CSDCSP : CCheriStackStore<0b111, "c.csdcsp", GPR, uimm9_lsb000> {
  let Inst{12-10} = imm{5-3};
  let Inst{9-7}   = imm{8-6};
}

} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]
} // DecoderNamespace = "CapModeOnly_"

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : InstAlias<"c.cincoffsetimm4cspn $rd, $rs1, $imm",
                (C_CIncOffsetImm4CSPN GPCRC:$rd, CSP:$rs1, uimm10_lsb00nonzero:$imm), 0>;
def : InstAlias<"c.cincoffsetimm16csp $rd, $imm",
                (C_CIncOffsetImm16CSP CSP:$rd, simm10_lsb0000nonzero:$imm), 0>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//

let Predicates = [HasCheri, IsCapMode] in {
let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isCodeGenOnly = 0,
    isAsmParserOnly = 1 in
def PseudoCLLC : Pseudo<(outs GPCR:$dst), (ins bare_symbol:$src), [],
                        "cllc", "$dst, $src">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0, isCodeGenOnly = 0,
    isAsmParserOnly = 1 in
def PseudoCLGC : Pseudo<(outs GPCR:$dst), (ins bare_symbol:$src), [],
                        "clgc", "$dst, $src">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0, isCodeGenOnly = 0,
    isAsmParserOnly = 1 in
def PseudoCLA_TLS_IE : Pseudo<(outs GPR:$dst, GPCR:$tmp),
                              (ins bare_symbol:$src), [],
                              "cla.tls.ie", "$dst, $src, $tmp">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isCodeGenOnly = 0,
    isAsmParserOnly = 1 in
def PseudoCLC_TLS_GD : Pseudo<(outs GPCR:$dst), (ins bare_symbol:$src), [],
                              "clc.tls.gd", "$dst, $src">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isCodeGenOnly = 0 in
def PseudoCIncOffsetTPRel : Pseudo<(outs GPCR:$rd),
                                   (ins GPCR:$rs1, GPR:$rs2,
                                        tprel_cincoffset_symbol:$src), [],
                                   "cincoffset", "$rd, $rs1, $rs2, $src">;
}

let Predicates = [HasCheri, HasStdExtD, IsRV32] in {

// Stores an FPR via splitting to two GPRs.
let hasSideEffects = 0, mayLoad = 0, mayStore = 1,
    Constraints = "@earlyclobber $tmplo,@earlyclobber $tmphi",
    usesCustomInserter = 1 in
def SplitStoreF64Pseudo
    : Pseudo<(outs GPR:$tmplo, GPR:$tmphi, GPR:$tmpdst),
             (ins FPR64:$src, GPR:$dst), []>;

// Stores an FPR via splitting to two GPRs.
let hasSideEffects = 0, mayLoad = 0, mayStore = 1,
    Constraints = "@earlyclobber $tmplo,@earlyclobber $tmphi",
    usesCustomInserter = 1 in
def CheriSplitStoreF64Pseudo
    : Pseudo<(outs GPR:$tmplo, GPR:$tmphi, GPCR:$tmpdst),
             (ins FPR64:$src, GPCR:$dst), []>;

} // Predicates = [HasCheri, HasStdExtD, IsRV32]

class PatGpcr<SDPatternOperator OpNode, RVInstCheriSrcDst Inst>
    : Pat<(OpNode GPCR:$rs1), (Inst GPCR:$rs1)>;

class PatGpcrGpcr<SDPatternOperator OpNode, RVInst Inst>
    : Pat<(OpNode GPCR:$rs1, GPCR:$rs2), (Inst GPCR:$rs1, GPCR:$rs2)>;

class PatGpcrGpr<SDPatternOperator OpNode, RVInst Inst>
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
def : PatGpcr<int_cheri_cap_seal_entry, CSealEntry>;

// TODO: Make this rematerialisable like MIPS
def : PatGpcrGpr<int_cheri_bounded_stack_cap, CSetBounds>;
def : PatGpcrUimm12<int_cheri_bounded_stack_cap, CSetBoundsImm>;
def : PatGpcrGpr<int_cheri_bounded_stack_cap_dynamic, CSetBounds>;
def : PatGpcrUimm12<int_cheri_bounded_stack_cap_dynamic, CSetBoundsImm>;

def : Pat<(ptradd CapFI64:$rs1, simm12:$imm12),
          (CIncOffsetImm CapFI64:$rs1, simm12:$imm12)>;
def : Pat<(ptradd CapFI128:$rs1, simm12:$imm12),
          (CIncOffsetImm CapFI128:$rs1, simm12:$imm12)>;

/// Pointer-Arithmetic Instructions

def : Pat<(int_cheri_cap_to_pointer GPCR:$rs1, GPCRC0IsDDC:$rs2),
          (CToPtr GPCR:$rs1, GPCRC0IsDDC:$rs2)>;
def : Pat<(int_cheri_cap_from_pointer GPCRC0IsDDC:$rs1, GPR:$rs2),
          (CFromPtr GPCRC0IsDDC:$rs1, GPR:$rs2)>;
def : PatGpcrGpcr<int_cheri_cap_diff, CSub>;

let Predicates = [IsPureCapABI] in {
def : Pat<(inttoptr GPR:$rs2), (CIncOffset C0, GPR:$rs2)>;
def : Pat<(inttoptr simm12:$imm12), (CIncOffsetImm C0, simm12:$imm12)>;
def : Pat<(ptrtoint GPCR:$rs1), (CGetAddr GPCR:$rs1)>;
}

let Predicates = [NotPureCapABI] in {
def : Pat<(inttoptr GPR:$rs2), (CFromPtr DDC, GPR:$rs2)>;
def : Pat<(ptrtoint GPCR:$rs1), (CToPtr GPCR:$rs1, DDC)>;
}

def : Pat<(int_cheri_cap_to_pointer (CLenVT (int_cheri_ddc_get)), GPCR:$rs1),
          (CToPtr $rs1, DDC)>;
def : Pat<(int_cheri_cap_from_pointer (CLenVT (int_cheri_ddc_get)), GPR:$rs2),
          (CFromPtr DDC, $rs2)>;

/// Null Capability Patterns

def : Pat<(int_cheri_cap_from_pointer DDC, (XLenVT 0)), (CLenVT (COPY C0))>;
def : Pat<(inttoptr (XLenVT 0)), (CLenVT (COPY C0))>;
def : Pat<(ptrtoint C0), (XLenVT (COPY X0))>;

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

multiclass CheriSetCCPatGpcrImm<PatFrag CondOp, Operand ImmTy, dag GprImmDag> {
  def : Pat<(CondOp GPCR:$cs1, (inttoptr ImmTy:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprImmDag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), ImmTy:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (ptradd (iFATPTR64 (inttoptr (XLenVT 0))), ImmTy:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprImmDag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), ImmTy:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (ptradd (iFATPTR128 (inttoptr (XLenVT 0))), ImmTy:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprImmDag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), ImmTy:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (int_cheri_cap_offset_set
                (iFATPTR64 (inttoptr (XLenVT 0))), ImmTy:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprImmDag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), ImmTy:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (int_cheri_cap_offset_set
                (iFATPTR128 (inttoptr (XLenVT 0))), ImmTy:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprImmDag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), ImmTy:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (int_cheri_cap_address_set
                (iFATPTR64 (inttoptr (XLenVT 0))), ImmTy:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprImmDag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), ImmTy:$imm12)>;
  def : Pat<(CondOp GPCR:$cs1,
              (int_cheri_cap_address_set
                (iFATPTR128 (inttoptr (XLenVT 0))), ImmTy:$imm12)),
            (OutPatFrag<(ops node:$rs1, node:$imm12), GprImmDag>
              (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr), ImmTy:$imm12)>;
}

multiclass CheriSetCCPatGpcrSimm12<PatFrag CondOp, dag GprImmDag>
    : CheriSetCCPatGpcrImm<CondOp, simm12, GprImmDag>;

multiclass CheriSetCCPatGpcrSimm12Plus1<PatFrag CondOp, dag GprImmDag>
    : CheriSetCCPatGpcrImm<CondOp, simm12_plus1, GprImmDag>;

class CheriSetCCPatGpcrNull<PatFrag CondOp, dag GprDag>
    : Pat<(CondOp GPCR:$cs1, (inttoptr (XLenVT 0))),
          (OutPatFrag<(ops node:$rs1), GprDag>
            (EXTRACT_SUBREG GPCR:$cs1, sub_cap_addr))>;

class Swap<PatFrag BinFrag>
    : PatFrag<(ops node:$a, node:$b), (BinFrag $b, $a)>;

// NB: Constants are wrapped as (ptradd (inttoptr 0) C) so the operand order
// isn't canonicalised like for integers so we have to add extra patterns for
// the commutative and inverse cases. Maybe we should support Constant:iFATPTR,
// represent them as (inttoptr C), or just canonicalise the current form?

def  : CheriSetCCPatGpcrGpcr<setlt, (SLT node:$rs1, node:$rs2)>;
defm : CheriSetCCPatGpcrSimm12<setlt, (SLTI node:$rs1, node:$imm12)>;
defm : CheriSetCCPatGpcrSimm12<Swap<setgt>, (SLTI node:$rs1, node:$imm12)>;
def  : CheriSetCCPatGpcrGpcr<setult, (SLTU node:$rs1, node:$rs2)>;
defm : CheriSetCCPatGpcrSimm12<setult, (SLTIU node:$rs1, node:$imm12)>;
defm : CheriSetCCPatGpcrSimm12<Swap<setugt>, (SLTIU node:$rs1, node:$imm12)>;

def  : CheriSetCCPatGpcrNull<seteq, (SLTIU node:$rs1, 1)>;
def  : CheriSetCCPatGpcrGpcr<seteq, (SLTIU (XOR node:$rs1, node:$rs2), 1)>;
defm : CheriSetCCPatGpcrSimm12Plus1<
           seteq, (SLTIU (ADDI node:$rs1, (NegImm node:$imm12)), 1)>;
defm : CheriSetCCPatGpcrSimm12Plus1<
           Swap<seteq>, (SLTIU (ADDI node:$rs1, (NegImm node:$imm12)), 1)>;
def  : CheriSetCCPatGpcrNull<setne, (SLTU X0, node:$rs1)>;
def  : CheriSetCCPatGpcrGpcr<setne, (SLTU X0, (XOR node:$rs1, node:$rs2))>;
defm : CheriSetCCPatGpcrSimm12Plus1<
           setne, (SLTU X0, (ADDI node:$rs1, (NegImm node:$imm12)))>;
defm : CheriSetCCPatGpcrSimm12Plus1<
           Swap<setne>, (SLTU X0, (ADDI node:$rs1, (NegImm node:$imm12)))>;
def  : CheriSetCCPatGpcrGpcr<setugt, (SLTU node:$rs2, node:$rs1)>;
def  : CheriSetCCPatGpcrGpcr<setuge, (XORI (SLTU node:$rs1, node:$rs2), 1)>;
def  : CheriSetCCPatGpcrGpcr<setule, (XORI (SLTU node:$rs2, node:$rs1), 1)>;
def  : CheriSetCCPatGpcrGpcr<setgt, (SLT node:$rs2, node:$rs1)>;
def  : CheriSetCCPatGpcrGpcr<setge, (XORI (SLT node:$rs1, node:$rs2), 1)>;
def  : CheriSetCCPatGpcrGpcr<setle, (XORI (SLT node:$rs2, node:$rs1), 1)>;

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
def : PatGpcrGpcr<riscv_cap_equal_exact, CSEQX>;

/// Special Capability Register Access Instructions

def : Pat<(int_cheri_ddc_get), (CSpecialRW SCR_DDC.Encoding, C0)>;
let Predicates = [HasCheri, IsPureCapABI] in
def : Pat<(int_cheri_stack_cap_get), (CLenVT (COPY C2))>;

let Predicates = [HasCheri, IsCapMode] in
def : Pat<(int_cheri_pcc_get), (AUIPCC 0)>;

let Predicates = [HasCheri, NotCapMode] in
def : Pat<(int_cheri_pcc_get), (CSpecialRW SCR_PCC.Encoding, C0)>;

/// Fast Register-Clearing Instructions

/// Adjusting to Compressed Capability Precision Instructions

def : PatGpr<int_cheri_round_representable_length, CRRL>;
def : PatGpr<int_cheri_representable_alignment_mask, CRAM>;

/// Tag-Memory Access Instructions

def : PatGpcr<int_cheri_cap_load_tags, CLoadTags>;

/// Memory-Access with Explicit Address Type Instructions

multiclass CheriExplicitLdPat<PatFrag LoadOp, RVInst Inst,
                              RegisterClass AddrTy> {
  def : Pat<(LoadOp AddrTy:$rs1), (Inst AddrTy:$rs1)>;
}

multiclass CheriExplicitStPat<PatFrag StoreOp, RVInst Inst,
                              RegisterClass StTy, RegisterClass AddrTy> {
  def : Pat<(StoreOp StTy:$rs2, AddrTy:$rs1), (Inst StTy:$rs2, AddrTy:$rs1)>;
}

/// DDC-relative loads

let Predicates = [HasCheri, IsCapMode] in {
defm : CheriExplicitLdPat<sextloadi8, LB_DDC, GPR>;
defm : CheriExplicitLdPat<extloadi8, LB_DDC, GPR>;
defm : CheriExplicitLdPat<sextloadi16, LH_DDC, GPR>;
defm : CheriExplicitLdPat<extloadi16, LH_DDC, GPR>;
defm : CheriExplicitLdPat<zextloadi8, LBU_DDC, GPR>;
defm : CheriExplicitLdPat<zextloadi16, LHU_DDC, GPR>;
}

let Predicates = [HasCheri, IsCapMode, IsRV32] in {
defm : CheriExplicitLdPat<load, LW_DDC, GPR>;
defm : CheriExplicitLdPat<load, LC_DDC_64, GPR>;
}

let Predicates = [HasCheri, IsCapMode, IsRV64] in {
defm : CheriExplicitLdPat<sextloadi32, LW_DDC, GPR>;
defm : CheriExplicitLdPat<extloadi32, LW_DDC, GPR>;
defm : CheriExplicitLdPat<load, LD_DDC, GPR>;
defm : CheriExplicitLdPat<zextloadi32, LWU_DDC, GPR>;
defm : CheriExplicitLdPat<load, LC_DDC_128, GPR>;
}

let Predicates = [HasCheri, IsCapMode, HasStdExtF] in
def : Pat<(load GPR:$rs1), (FMV_W_X (LW_DDC GPR:$rs1))>;

let Predicates = [HasCheri, IsCapMode, HasStdExtD, IsRV32] in
def : Pat<(load GPR:$rs1),
          (BuildPairF64Pseudo (LW_DDC GPR:$rs1),
                              (LW_DDC (ADDI GPR:$rs1, 4)))>;

let Predicates = [HasCheri, IsCapMode, HasStdExtD, IsRV64] in
def : Pat<(load GPR:$rs1), (FMV_D_X (LD_DDC GPR:$rs1))>;

/// DDC-relative stores

let Predicates = [HasCheri, IsCapMode] in {
defm : CheriExplicitStPat<truncstorei8, SB_DDC, GPR, GPR>;
defm : CheriExplicitStPat<truncstorei16, SH_DDC, GPR, GPR>;
}

let Predicates = [HasCheri, IsCapMode, IsRV32] in {
defm : CheriExplicitStPat<store, SW_DDC, GPR, GPR>;
defm : CheriExplicitStPat<store, SC_DDC_64, GPCR, GPR>;
}

let Predicates = [HasCheri, IsCapMode, IsRV64] in {
defm : CheriExplicitStPat<truncstorei32, SW_DDC, GPR, GPR>;
defm : CheriExplicitStPat<store, SD_DDC, GPR, GPR>;
defm : CheriExplicitStPat<store, SC_DDC_128, GPCR, GPR>;
}

let Predicates = [HasCheri, IsCapMode, HasStdExtF] in
def : Pat<(store FPR32:$rs2, GPR:$rs1),
          (SW_DDC (FMV_X_W FPR32:$rs2), GPR:$rs1)>;

let Predicates = [HasCheri, IsCapMode, HasStdExtD, IsRV32] in
def : Pat<(store FPR64:$rs2, GPR:$rs1),
          (KILL (SplitStoreF64Pseudo FPR64:$rs2, GPR:$rs1))>;

let Predicates = [HasCheri, IsCapMode, HasStdExtD, IsRV64] in
def : Pat<(store FPR64:$rs2, GPR:$rs1),
          (SD_DDC (FMV_X_D FPR64:$rs2), GPR:$rs1)>;

/// Capability loads

let Predicates = [HasCheri, NotCapMode] in {
defm : CheriExplicitLdPat<sextloadi8, LB_CAP, GPCR>;
defm : CheriExplicitLdPat<extloadi8, LB_CAP, GPCR>;
defm : CheriExplicitLdPat<sextloadi16, LH_CAP, GPCR>;
defm : CheriExplicitLdPat<extloadi16, LH_CAP, GPCR>;
defm : CheriExplicitLdPat<zextloadi8, LBU_CAP, GPCR>;
defm : CheriExplicitLdPat<zextloadi16, LHU_CAP, GPCR>;
}

let Predicates = [HasCheri, NotCapMode, IsRV32] in {
defm : CheriExplicitLdPat<load, LW_CAP, GPCR>;
defm : CheriExplicitLdPat<load, LC_CAP_64, GPCR>;
}

let Predicates = [HasCheri, NotCapMode, IsRV64] in {
defm : CheriExplicitLdPat<sextloadi32, LW_CAP, GPCR>;
defm : CheriExplicitLdPat<extloadi32, LW_CAP, GPCR>;
defm : CheriExplicitLdPat<load, LD_CAP, GPCR>;
defm : CheriExplicitLdPat<zextloadi32, LWU_CAP, GPCR>;
defm : CheriExplicitLdPat<load, LC_CAP_128, GPCR>;
}

let Predicates = [HasCheri, NotCapMode, HasStdExtF] in
def : Pat<(load GPCR:$rs1), (FMV_W_X (LW_CAP GPCR:$rs1))>;

let Predicates = [HasCheri, NotCapMode, HasStdExtD, IsRV32] in
def : Pat<(load GPCR:$rs1),
          (BuildPairF64Pseudo (LW_CAP GPCR:$rs1),
                              (LW_CAP (CIncOffsetImm GPCR:$rs1, 4)))>;

let Predicates = [HasCheri, NotCapMode, HasStdExtD, IsRV64] in
def : Pat<(load GPCR:$rs1), (FMV_D_X (LD_CAP GPCR:$rs1))>;

/// Capability stores

let Predicates = [HasCheri, NotCapMode] in {
defm : CheriExplicitStPat<truncstorei8, SB_CAP, GPR, GPCR>;
defm : CheriExplicitStPat<truncstorei16, SH_CAP, GPR, GPCR>;
}

let Predicates = [HasCheri, NotCapMode, IsRV32] in {
defm : CheriExplicitStPat<store, SW_CAP, GPR, GPCR>;
defm : CheriExplicitStPat<store, SC_CAP_64, GPCR, GPCR>;
}

let Predicates = [HasCheri, NotCapMode, IsRV64] in {
defm : CheriExplicitStPat<truncstorei32, SW_CAP, GPR, GPCR>;
defm : CheriExplicitStPat<store, SD_CAP, GPR, GPCR>;
defm : CheriExplicitStPat<store, SC_CAP_128, GPCR, GPCR>;
}

let Predicates = [HasCheri, NotCapMode, HasStdExtF] in
def : Pat<(store FPR32:$rs2, GPCR:$rs1),
          (SW_CAP (FMV_X_W FPR32:$rs2), GPCR:$rs1)>;

let Predicates = [HasCheri, NotCapMode, HasStdExtD, IsRV32] in
def : Pat<(store FPR64:$rs2, GPCR:$rs1),
          (KILL (CheriSplitStoreF64Pseudo FPR64:$rs2, GPCR:$rs1))>;

let Predicates = [HasCheri, NotCapMode, HasStdExtD, IsRV64] in
def : Pat<(store FPR64:$rs2, GPCR:$rs1),
          (SD_CAP (FMV_X_D FPR64:$rs2), GPCR:$rs1)>;

/// Non-Capability Mode Instructions

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

let Predicates = [HasCheri, IsRV32, NotCapMode] in {
defm : LdPat<load, LC_64, iFATPTR64>;
defm : StPat<store, SC_64, GPCR, iFATPTR64>;
} // Predicates = [HasCheri, IsRV32, NotCapMode]

let Predicates = [HasCheri, IsRV64, NotCapMode] in {
defm : LdPat<load, LC_128, iFATPTR128>;
defm : StPat<store, SC_128, GPCR, iFATPTR128>;
} // Predicates = [HasCheri, IsRV64, NotCapMode]

let Predicates = [HasCheri, HasStdExtA, IsRV32, NotCapMode] in {
defm : LdPat<atomic_load_cap, LC_64, iFATPTR64>;
defm : AtomicStPat<atomic_store_cap, SC_64, GPCR, iFATPTR64>;
defm : AMOCapPat<"64", "atomic_swap_cap", "AMOSWAP_C">;
} // Predicates = [HasCheri, HasStdExtA, IsRV32, NotCapMode]

let Predicates = [HasCheri, HasStdExtA, IsRV64, NotCapMode] in {
defm : LdPat<atomic_load_cap, LC_128, iFATPTR128>;
defm : AtomicStPat<atomic_store_cap, SC_128, GPCR, iFATPTR128>;
defm : AMOCapPat<"128", "atomic_swap_cap", "AMOSWAP_C">;
} // Predicates = [HasCheri, HasStdExtA, IsRV64, NotCapMode]

let Predicates = [HasCheri, HasStdExtA] in {
def PseudoAtomicLoadAddCap : PseudoAMO<GPCR>;
def PseudoAtomicLoadSubCap : PseudoAMO<GPCR>;
def PseudoAtomicLoadAndCap : PseudoAMO<GPCR>;
def PseudoAtomicLoadOrCap : PseudoAMO<GPCR>;
def PseudoAtomicLoadXorCap : PseudoAMO<GPCR>;
def PseudoAtomicLoadMaxCap : PseudoAMO<GPCR>;
def PseudoAtomicLoadMinCap : PseudoAMO<GPCR>;
def PseudoAtomicLoadUMaxCap : PseudoAMO<GPCR>;
def PseudoAtomicLoadUMinCap : PseudoAMO<GPCR>;
def PseudoAtomicLoadNandCap : PseudoAMO<GPCR>;
def PseudoCmpXchgCap : PseudoCmpXchg<GPCR>;
} // Predicates = [HasCheri, HasStdExtA]

let Predicates = [HasCheri, HasStdExtA, NotCapMode] in {
defm : PseudoAMOPat<"atomic_load_add_cap", PseudoAtomicLoadAddCap, GPCR>;
defm : PseudoAMOPat<"atomic_load_sub_cap", PseudoAtomicLoadSubCap, GPCR>;
defm : PseudoAMOPat<"atomic_load_and_cap", PseudoAtomicLoadAndCap, GPCR>;
defm : PseudoAMOPat<"atomic_load_or_cap", PseudoAtomicLoadOrCap, GPCR>;
defm : PseudoAMOPat<"atomic_load_xor_cap", PseudoAtomicLoadXorCap, GPCR>;
defm : PseudoAMOPat<"atomic_load_max_cap", PseudoAtomicLoadMaxCap, GPCR>;
defm : PseudoAMOPat<"atomic_load_min_cap", PseudoAtomicLoadMinCap, GPCR>;
defm : PseudoAMOPat<"atomic_load_umax_cap", PseudoAtomicLoadUMaxCap, GPCR>;
defm : PseudoAMOPat<"atomic_load_umin_cap", PseudoAtomicLoadUMinCap, GPCR>;
defm : PseudoAMOPat<"atomic_load_nand_cap", PseudoAtomicLoadNandCap, GPCR>;
defm : PseudoCmpXchgPat<"atomic_cmp_swap_cap", PseudoCmpXchgCap, GPCR>;
} // Predicates = [HasCheri, HasStdExtA, NotCapMode]

/// Capability Mode Instructions

multiclass CheriLdPat<PatFrag LoadOp, RVInst Inst> {
  def : Pat<(LoadOp GPCR:$rs1), (Inst GPCR:$rs1, 0)>;
  def : Pat<(LoadOp CapFI64:$rs1), (Inst CapFI64:$rs1, 0)>;
  def : Pat<(LoadOp CapFI128:$rs1), (Inst CapFI128:$rs1, 0)>;
  def : Pat<(LoadOp (ptradd GPCR:$rs1, simm12:$imm12)),
            (Inst GPCR:$rs1, simm12:$imm12)>;
  def : Pat<(LoadOp (ptradd CapFI64:$rs1, simm12:$imm12)),
            (Inst CapFI64:$rs1, simm12:$imm12)>;
  def : Pat<(LoadOp (ptradd CapFI128:$rs1, simm12:$imm12)),
            (Inst CapFI128:$rs1, simm12:$imm12)>;
}

multiclass CheriStPat<PatFrag StoreOp, RVInst Inst, RegisterClass StTy> {
  def : Pat<(StoreOp StTy:$rs2, GPCR:$rs1), (Inst StTy:$rs2, GPCR:$rs1, 0)>;
  def : Pat<(StoreOp StTy:$rs2, CapFI64:$rs1), (Inst StTy:$rs2, CapFI64:$rs1, 0)>;
  def : Pat<(StoreOp StTy:$rs2, CapFI128:$rs1), (Inst StTy:$rs2, CapFI128:$rs1, 0)>;
  def : Pat<(StoreOp StTy:$rs2, (ptradd GPCR:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, GPCR:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp StTy:$rs2, (ptradd CapFI64:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, CapFI64:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp StTy:$rs2, (ptradd CapFI128:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, CapFI128:$rs1, simm12:$imm12)>;
}

multiclass CheriAtomicStPat<PatFrag StoreOp, RVInst Inst, RegisterClass StTy>
    : CheriStPat<Swap<StoreOp>, Inst, StTy>;

multiclass CheriAMOPat<string AtomicOp, string BaseInst> {
  def : PatGpcrGpr<!cast<PatFrag>(AtomicOp#"_monotonic"),
                   !cast<RVInst>(BaseInst)>;
  def : PatGpcrGpr<!cast<PatFrag>(AtomicOp#"_acquire"),
                   !cast<RVInst>(BaseInst#"_AQ")>;
  def : PatGpcrGpr<!cast<PatFrag>(AtomicOp#"_release"),
                   !cast<RVInst>(BaseInst#"_RL")>;
  def : PatGpcrGpr<!cast<PatFrag>(AtomicOp#"_acq_rel"),
                   !cast<RVInst>(BaseInst#"_AQ_RL")>;
  def : PatGpcrGpr<!cast<PatFrag>(AtomicOp#"_seq_cst"),
                   !cast<RVInst>(BaseInst#"_AQ_RL")>;
}

multiclass CheriAMOSubPat<string AtomicOp, string BaseInst> {
  def : Pat<(!cast<PatFrag>(AtomicOp#"_monotonic") GPCR:$addr, GPR:$incr),
            (!cast<RVInst>(BaseInst) GPCR:$addr, (SUB X0, GPR:$incr))>;
  def : Pat<(!cast<PatFrag>(AtomicOp#"_acquire") GPCR:$addr, GPR:$incr),
            (!cast<RVInst>(BaseInst#"_AQ") GPCR:$addr, (SUB X0, GPR:$incr))>;
  def : Pat<(!cast<PatFrag>(AtomicOp#"_release") GPCR:$addr, GPR:$incr),
            (!cast<RVInst>(BaseInst#"_RL") GPCR:$addr, (SUB X0, GPR:$incr))>;
  def : Pat<(!cast<PatFrag>(AtomicOp#"_acq_rel") GPCR:$addr, GPR:$incr),
            (!cast<RVInst>(BaseInst#"_AQ_RL") GPCR:$addr, (SUB X0, GPR:$incr))>;
  def : Pat<(!cast<PatFrag>(AtomicOp#"_seq_cst") GPCR:$addr, GPR:$incr),
            (!cast<RVInst>(BaseInst#"_AQ_RL") GPCR:$addr, (SUB X0, GPR:$incr))>;
}

class PseudoCheriAMO<RegisterClass ValTy=GPR>
    : Pseudo<(outs ValTy:$res, ValTy:$scratch),
             (ins GPCR:$addr, ValTy:$incr, ixlenimm:$ordering), []> {
  let Constraints = "@earlyclobber $res,@earlyclobber $scratch";
  let mayLoad = 1;
  let mayStore = 1;
  let hasSideEffects = 0;
}

multiclass PseudoCheriAMOPat<string AtomicOp, Instruction BaseInst,
                             RegisterClass ValTy=GPR> {
  def : Pat<(!cast<PatFrag>(AtomicOp#"_monotonic") GPCR:$addr, ValTy:$incr),
            (BaseInst GPCR:$addr, ValTy:$incr, 2)>;
  def : Pat<(!cast<PatFrag>(AtomicOp#"_acquire") GPCR:$addr, ValTy:$incr),
            (BaseInst GPCR:$addr, ValTy:$incr, 4)>;
  def : Pat<(!cast<PatFrag>(AtomicOp#"_release") GPCR:$addr, ValTy:$incr),
            (BaseInst GPCR:$addr, ValTy:$incr, 5)>;
  def : Pat<(!cast<PatFrag>(AtomicOp#"_acq_rel") GPCR:$addr, ValTy:$incr),
            (BaseInst GPCR:$addr, ValTy:$incr, 6)>;
  def : Pat<(!cast<PatFrag>(AtomicOp#"_seq_cst") GPCR:$addr, ValTy:$incr),
            (BaseInst GPCR:$addr, ValTy:$incr, 7)>;
}

class PseudoCheriCmpXchg<RegisterClass ValTy=GPR>
    : Pseudo<(outs ValTy:$res, GPR:$scratch),
             (ins GPCR:$addr, ValTy:$cmpval, ValTy:$newval, ixlenimm:$ordering), []> {
  let Constraints = "@earlyclobber $res,@earlyclobber $scratch";
  let mayLoad = 1;
  let mayStore = 1;
  let hasSideEffects = 0;
}

multiclass PseudoCheriCmpXchgPat<string Op, Pseudo CmpXchgInst,
                                 RegisterClass ValTy=GPR> {
  def : Pat<(!cast<PatFrag>(Op#"_monotonic") GPCR:$addr, ValTy:$cmp, ValTy:$new),
            (CmpXchgInst GPCR:$addr, ValTy:$cmp, ValTy:$new, 2)>;
  def : Pat<(!cast<PatFrag>(Op#"_acquire") GPCR:$addr, ValTy:$cmp, ValTy:$new),
            (CmpXchgInst GPCR:$addr, ValTy:$cmp, ValTy:$new, 4)>;
  def : Pat<(!cast<PatFrag>(Op#"_release") GPCR:$addr, ValTy:$cmp, ValTy:$new),
            (CmpXchgInst GPCR:$addr, ValTy:$cmp, ValTy:$new, 5)>;
  def : Pat<(!cast<PatFrag>(Op#"_acq_rel") GPCR:$addr, ValTy:$cmp, ValTy:$new),
            (CmpXchgInst GPCR:$addr, ValTy:$cmp, ValTy:$new, 6)>;
  def : Pat<(!cast<PatFrag>(Op#"_seq_cst") GPCR:$addr, ValTy:$cmp, ValTy:$new),
            (CmpXchgInst GPCR:$addr, ValTy:$cmp, ValTy:$new, 7)>;
}

multiclass CheriAMOCapPat<string CLenStr, string AtomicOp, string BaseInst> {
  def : PatGpcrGpcr<!cast<PatFrag>(AtomicOp#"_monotonic"),
                    !cast<RVInst>(BaseInst#"_"#CLenStr)>;
  def : PatGpcrGpcr<!cast<PatFrag>(AtomicOp#"_acquire"),
                    !cast<RVInst>(BaseInst#"_AQ_"#CLenStr)>;
  def : PatGpcrGpcr<!cast<PatFrag>(AtomicOp#"_release"),
                    !cast<RVInst>(BaseInst#"_RL_"#CLenStr)>;
  def : PatGpcrGpcr<!cast<PatFrag>(AtomicOp#"_acq_rel"),
                    !cast<RVInst>(BaseInst#"_AQ_RL_"#CLenStr)>;
  def : PatGpcrGpcr<!cast<PatFrag>(AtomicOp#"_seq_cst"),
                    !cast<RVInst>(BaseInst#"_AQ_RL_"#CLenStr)>;
}

let Predicates = [HasCheri, HasStdExtA] in {
def PseudoCheriAtomicSwap8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadAdd8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadSub8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadAnd8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadOr8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadXor8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadMax8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadMin8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadUMax8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadUMin8 : PseudoCheriAMO;
def PseudoCheriAtomicLoadNand8 : PseudoCheriAMO;
def PseudoCheriCmpXchg8 : PseudoCheriCmpXchg;

def PseudoCheriAtomicSwap16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadAdd16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadSub16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadAnd16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadOr16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadXor16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadMax16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadMin16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadUMax16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadUMin16 : PseudoCheriAMO;
def PseudoCheriAtomicLoadNand16 : PseudoCheriAMO;
def PseudoCheriCmpXchg16 : PseudoCheriCmpXchg;

def PseudoCheriAtomicLoadNand32 : PseudoCheriAMO;
def PseudoCheriCmpXchg32 : PseudoCheriCmpXchg;

def PseudoCheriAtomicLoadAddCap : PseudoCheriAMO<GPCR>;
def PseudoCheriAtomicLoadSubCap : PseudoCheriAMO<GPCR>;
def PseudoCheriAtomicLoadAndCap : PseudoCheriAMO<GPCR>;
def PseudoCheriAtomicLoadOrCap : PseudoCheriAMO<GPCR>;
def PseudoCheriAtomicLoadXorCap : PseudoCheriAMO<GPCR>;
def PseudoCheriAtomicLoadMaxCap : PseudoCheriAMO<GPCR>;
def PseudoCheriAtomicLoadMinCap : PseudoCheriAMO<GPCR>;
def PseudoCheriAtomicLoadUMaxCap : PseudoCheriAMO<GPCR>;
def PseudoCheriAtomicLoadUMinCap : PseudoCheriAMO<GPCR>;
def PseudoCheriAtomicLoadNandCap : PseudoCheriAMO<GPCR>;
def PseudoCheriCmpXchgCap : PseudoCheriCmpXchg<GPCR>;
} // Predicates = [HasCheri, HasStdExtA]

let Predicates = [HasCheri, HasStdExtA, IsRV64] in {
def PseudoCheriAtomicLoadNand64 : PseudoCheriAMO;
def PseudoCheriCmpXchg64 : PseudoCheriCmpXchg;
} // Predicates = [HasCheri, HasStdExtA, IsRV64]

/// 'I' (Integer) base

let Predicates = [HasCheri, IsCapMode, IsPureCapABI] in {
let isBarrier = 1, isBranch = 1, isTerminator = 1 in
def PseudoCBR : Pseudo<(outs), (ins simm21_lsb0_jal:$imm20), [(br bb:$imm20)]>,
                PseudoInstExpansion<(CJAL C0, simm21_lsb0_jal:$imm20)>;

let isCall = 1, Defs = [C1] in
let isBarrier = 1, isBranch = 1, isIndirectBranch = 1, isTerminator = 1 in
def PseudoCBRIND : Pseudo<(outs), (ins GPCR:$rs1, simm12:$imm12), []>,
                   PseudoInstExpansion<(CJALR C0, GPCR:$rs1, simm12:$imm12)>;

def : Pat<(brind GPCR:$rs1), (PseudoCBRIND GPCR:$rs1, 0)>;
def : Pat<(brind (ptradd GPCR:$rs1, simm12:$imm12)),
          (PseudoCBRIND GPCR:$rs1, simm12:$imm12)>;
} // Predicates = [HasCheri, IsCapMode, IsPureCapABI]

let Predicates = [HasCheri, IsCapMode],
    isCall = 1, isBarrier = 1, isCodeGenOnly = 0, hasSideEffects = 0,
    mayStore = 0, mayLoad = 0 in
def PseudoCCALLReg : Pseudo<(outs GPCR:$rd),
                            (ins cap_call_symbol:$func), []> {
  let AsmString = "ccall\t$rd, $func";
}

let Predicates = [HasCheri, IsCapMode],
    isCall = 1, Defs = [C1], isCodeGenOnly = 0 in
def PseudoCCALL : Pseudo<(outs), (ins cap_call_symbol:$func), []> {
  let AsmString = "ccall\t$func";
}

let Predicates = [HasCheri, IsCapMode, IsPureCapABI] in {
def : Pat<(riscv_cap_call tglobaladdr:$func),
          (PseudoCCALL tglobaladdr:$func)>;
def : Pat<(riscv_cap_call texternalsym:$func),
          (PseudoCCALL texternalsym:$func)>;
} // Predicates = [HasCheri, IsCapMode, IsPureCapABI]

let Predicates = [HasCheri, IsCapMode, IsPureCapABI],
    isCall = 1, Defs = [C1] in
def PseudoCCALLIndirect : Pseudo<(outs), (ins GPCR:$rs1),
                                 [(riscv_cap_call GPCR:$rs1)]>,
                          PseudoInstExpansion<(CJALR C1, GPCR:$rs1, 0)>;

let Predicates = [HasCheri, IsCapMode, IsPureCapABI],
    isBarrier = 1, isReturn = 1, isTerminator = 1 in
def PseudoCRET : Pseudo<(outs), (ins), [(riscv_ret_flag)]>,
                 PseudoInstExpansion<(CJALR C0, C1, 0)>;

let Predicates = [HasCheri, IsCapMode],
    isCall = 1, isTerminator = 1, isReturn = 1, isBarrier = 1, Uses = [C2],
    isCodeGenOnly = 0 in
def PseudoCTAIL : Pseudo<(outs), (ins cap_call_symbol:$dst), []> {
  let AsmString = "ctail\t$dst";
}

let Predicates = [HasCheri, IsCapMode, IsPureCapABI],
    isCall = 1, isTerminator = 1, isReturn = 1, isBarrier = 1, Uses = [C2] in
def PseudoCTAILIndirect : Pseudo<(outs), (ins GPCRTC:$rs1),
                                 [(riscv_cap_tail GPCRTC:$rs1)]>,
                          PseudoInstExpansion<(CJALR C0, GPCR:$rs1, 0)>;

let Predicates = [HasCheri, IsCapMode, IsPureCapABI] in {
def : Pat<(riscv_cap_tail tglobaladdr:$dst),
          (PseudoCTAIL texternalsym:$dst)>;
def : Pat<(riscv_cap_tail texternalsym:$dst),
          (PseudoCTAIL texternalsym:$dst)>;
} // Predicates = [HasCheri, IsCapMode, IsPureCapABI]

let Predicates = [HasCheri, IsCapMode, IsPureCapABI] in
let isCall = 0, isBarrier = 1, isBranch = 1, isTerminator = 1,
    isCodeGenOnly = 0, hasSideEffects = 0, mayStore = 0, mayLoad = 0 in
def PseudoCJump : Pseudo<(outs GPCR:$rd),
                         (ins pseudo_cap_jump_symbol:$target), []> {
  let AsmString = "cjump\t$target, $rd";
}

defm : CheriLdPat<sextloadi8, CLB>, Requires<[HasCheri, IsCapMode]>;
defm : CheriLdPat<extloadi8, CLB>, Requires<[HasCheri, IsCapMode]>;
defm : CheriLdPat<sextloadi16, CLH>, Requires<[HasCheri, IsCapMode]>;
defm : CheriLdPat<extloadi16, CLH>, Requires<[HasCheri, IsCapMode]>;
defm : CheriLdPat<load, CLW>, Requires<[HasCheri, IsRV32, IsCapMode]>;
defm : CheriLdPat<zextloadi8, CLBU>, Requires<[HasCheri, IsCapMode]>;
defm : CheriLdPat<zextloadi16, CLHU>, Requires<[HasCheri, IsCapMode]>;

defm : CheriStPat<truncstorei8, CSB, GPR>, Requires<[HasCheri, IsCapMode]>;
defm : CheriStPat<truncstorei16, CSH, GPR>, Requires<[HasCheri, IsCapMode]>;
defm : CheriStPat<store, CSW, GPR>, Requires<[HasCheri, IsRV32, IsCapMode]>;

let Predicates = [HasCheri, IsRV64, IsCapMode] in {

/// Loads

defm : CheriLdPat<sextloadi32, CLW>;
defm : CheriLdPat<extloadi32, CLW>;
defm : CheriLdPat<zextloadi32, CLWU>;
defm : CheriLdPat<load, CLD>;

/// Stores

defm : CheriStPat<truncstorei32, CSW, GPR>;
defm : CheriStPat<store, CSD, GPR>;
} // Predicates = [HasCheri, IsRV64, IsCapMode]

/// Other pseudo-instructions

// Pessimistically assume the stack pointer will be clobbered
let Predicates = [HasCheri, IsPureCapABI] in {
let Defs = [C2], Uses = [C2] in {
def ADJCALLSTACKDOWNCAP : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                 [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUPCAP   : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                 [(callseq_end timm:$amt1, timm:$amt2)]>;
} // Defs = [C2], Uses = [C2]
} // Predicates = [HasCheri, IsPureCapABI]

/// 'A' (Atomic Instructions) extension

let Predicates = [HasCheri, HasStdExtA, IsCapMode] in {

/// Atomic loads and stores

// Fences will be inserted for atomic load/stores according to the logic in
// RISCVTargetLowering::{emitLeadingFence,emitTrailingFence}.

defm : CheriLdPat<atomic_load_8,  CLB>;
defm : CheriLdPat<atomic_load_16, CLH>;
defm : CheriLdPat<atomic_load_32, CLW>;

defm : CheriAtomicStPat<atomic_store_8,  CSB, GPR>;
defm : CheriAtomicStPat<atomic_store_16, CSH, GPR>;
defm : CheriAtomicStPat<atomic_store_32, CSW, GPR>;

/// AMOs

defm : CheriAMOPat<"atomic_swap_32", "CAMOSWAP_W">;
defm : CheriAMOPat<"atomic_load_add_32", "CAMOADD_W">;
defm : CheriAMOSubPat<"atomic_load_sub_32", "CAMOADD_W">;
defm : CheriAMOPat<"atomic_load_and_32", "CAMOAND_W">;
defm : CheriAMOPat<"atomic_load_or_32", "CAMOOR_W">;
defm : CheriAMOPat<"atomic_load_xor_32", "CAMOXOR_W">;
defm : CheriAMOPat<"atomic_load_max_32", "CAMOMAX_W">;
defm : CheriAMOPat<"atomic_load_min_32", "CAMOMIN_W">;
defm : CheriAMOPat<"atomic_load_umax_32", "CAMOMAXU_W">;
defm : CheriAMOPat<"atomic_load_umin_32", "CAMOMINU_W">;

/// Pseudo AMOs

defm : PseudoCheriAMOPat<"atomic_swap_8", PseudoCheriAtomicSwap8>;
defm : PseudoCheriAMOPat<"atomic_load_add_8", PseudoCheriAtomicLoadAdd8>;
defm : PseudoCheriAMOPat<"atomic_load_sub_8", PseudoCheriAtomicLoadSub8>;
defm : PseudoCheriAMOPat<"atomic_load_and_8", PseudoCheriAtomicLoadAnd8>;
defm : PseudoCheriAMOPat<"atomic_load_or_8", PseudoCheriAtomicLoadOr8>;
defm : PseudoCheriAMOPat<"atomic_load_xor_8", PseudoCheriAtomicLoadXor8>;
defm : PseudoCheriAMOPat<"atomic_load_max_8", PseudoCheriAtomicLoadMax8>;
defm : PseudoCheriAMOPat<"atomic_load_min_8", PseudoCheriAtomicLoadMin8>;
defm : PseudoCheriAMOPat<"atomic_load_umax_8", PseudoCheriAtomicLoadUMax8>;
defm : PseudoCheriAMOPat<"atomic_load_umin_8", PseudoCheriAtomicLoadUMin8>;
defm : PseudoCheriAMOPat<"atomic_load_nand_8", PseudoCheriAtomicLoadNand8>;

defm : PseudoCheriAMOPat<"atomic_swap_16", PseudoCheriAtomicSwap16>;
defm : PseudoCheriAMOPat<"atomic_load_add_16", PseudoCheriAtomicLoadAdd16>;
defm : PseudoCheriAMOPat<"atomic_load_sub_16", PseudoCheriAtomicLoadSub16>;
defm : PseudoCheriAMOPat<"atomic_load_and_16", PseudoCheriAtomicLoadAnd16>;
defm : PseudoCheriAMOPat<"atomic_load_or_16", PseudoCheriAtomicLoadOr16>;
defm : PseudoCheriAMOPat<"atomic_load_xor_16", PseudoCheriAtomicLoadXor16>;
defm : PseudoCheriAMOPat<"atomic_load_max_16", PseudoCheriAtomicLoadMax16>;
defm : PseudoCheriAMOPat<"atomic_load_min_16", PseudoCheriAtomicLoadMin16>;
defm : PseudoCheriAMOPat<"atomic_load_umax_16", PseudoCheriAtomicLoadUMax16>;
defm : PseudoCheriAMOPat<"atomic_load_umin_16", PseudoCheriAtomicLoadUMin16>;
defm : PseudoCheriAMOPat<"atomic_load_nand_16", PseudoCheriAtomicLoadNand16>;

defm : PseudoCheriAMOPat<"atomic_load_nand_32", PseudoCheriAtomicLoadNand32>;

defm : PseudoCheriAMOPat<"atomic_load_add_cap", PseudoCheriAtomicLoadAddCap, GPCR>;
defm : PseudoCheriAMOPat<"atomic_load_sub_cap", PseudoCheriAtomicLoadSubCap, GPCR>;
defm : PseudoCheriAMOPat<"atomic_load_and_cap", PseudoCheriAtomicLoadAndCap, GPCR>;
defm : PseudoCheriAMOPat<"atomic_load_or_cap", PseudoCheriAtomicLoadOrCap, GPCR>;
defm : PseudoCheriAMOPat<"atomic_load_xor_cap", PseudoCheriAtomicLoadXorCap, GPCR>;
defm : PseudoCheriAMOPat<"atomic_load_max_cap", PseudoCheriAtomicLoadMaxCap, GPCR>;
defm : PseudoCheriAMOPat<"atomic_load_min_cap", PseudoCheriAtomicLoadMinCap, GPCR>;
defm : PseudoCheriAMOPat<"atomic_load_umax_cap", PseudoCheriAtomicLoadUMaxCap, GPCR>;
defm : PseudoCheriAMOPat<"atomic_load_umin_cap", PseudoCheriAtomicLoadUMinCap, GPCR>;
defm : PseudoCheriAMOPat<"atomic_load_nand_cap", PseudoCheriAtomicLoadNandCap, GPCR>;

/// Compare and exchange

defm : PseudoCheriCmpXchgPat<"atomic_cmp_swap_8", PseudoCheriCmpXchg8>;
defm : PseudoCheriCmpXchgPat<"atomic_cmp_swap_16", PseudoCheriCmpXchg16>;
defm : PseudoCheriCmpXchgPat<"atomic_cmp_swap_32", PseudoCheriCmpXchg32>;

defm : PseudoCheriCmpXchgPat<"atomic_cmp_swap_cap", PseudoCheriCmpXchgCap, GPCR>;

} // Predicates = [HasCheri, HasStdExtA, IsCapMode]

let Predicates = [HasCheri, HasStdExtA, IsRV64, IsCapMode] in {

/// 64-bit atomic loads and stores

// Fences will be inserted for atomic load/stores according to the logic in
// RISCVTargetLowering::{emitLeadingFence,emitTrailingFence}.
defm : CheriLdPat<atomic_load_64, CLD>;
defm : CheriAtomicStPat<atomic_store_64, CSD, GPR>;

/// 64-bit AMOs

defm : CheriAMOPat<"atomic_swap_64", "CAMOSWAP_D">;
defm : CheriAMOPat<"atomic_load_add_64", "CAMOADD_D">;
defm : CheriAMOSubPat<"atomic_load_sub_64", "CAMOADD_D">;
defm : CheriAMOPat<"atomic_load_and_64", "CAMOAND_D">;
defm : CheriAMOPat<"atomic_load_or_64", "CAMOOR_D">;
defm : CheriAMOPat<"atomic_load_xor_64", "CAMOXOR_D">;
defm : CheriAMOPat<"atomic_load_max_64", "CAMOMAX_D">;
defm : CheriAMOPat<"atomic_load_min_64", "CAMOMIN_D">;
defm : CheriAMOPat<"atomic_load_umax_64", "CAMOMAXU_D">;
defm : CheriAMOPat<"atomic_load_umin_64", "CAMOMINU_D">;

/// 64-bit pseudo AMOs

defm : PseudoCheriAMOPat<"atomic_load_nand_64", PseudoCheriAtomicLoadNand64>;

/// 64-bit compare and exchange

defm : PseudoCheriCmpXchgPat<"atomic_cmp_swap_64", PseudoCheriCmpXchg64>;

} // Predicates = [HasCheri, HasStdExtA, IsRV64, IsCapMode]

let Predicates = [HasCheri, HasStdExtA, IsRV32, IsCapMode] in {
defm : CheriLdPat<atomic_load_cap, CLC_64>;
defm : CheriAtomicStPat<atomic_store_cap, CSC_64, GPCR>;
defm : CheriAMOCapPat<"64", "atomic_swap_cap", "CAMOSWAP_C">;
} // Predicates = [HasCheri, HasStdExtA, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasStdExtA, IsRV64, IsCapMode] in {
defm : CheriLdPat<atomic_load_cap, CLC_128>;
defm : CheriAtomicStPat<atomic_store_cap, CSC_128, GPCR>;
defm : CheriAMOCapPat<"128", "atomic_swap_cap", "CAMOSWAP_C">;
} // Predicates = [HasCheri, HasStdExtA, IsRV64, IsCapMode]

/// 'F' (Single-Precision Floating-Point) extension

let Predicates = [HasCheri, HasStdExtF, IsCapMode] in {

/// Loads

defm : CheriLdPat<load, CFLW>;

/// Stores

defm : CheriStPat<store, CFSW, FPR32>;

} // Predicates = [HasCheri, HasStdExtF, IsCapMode]

/// 'D' (Single-Precision Floating-Point) extension

let Predicates = [HasCheri, HasStdExtD, IsCapMode] in {

/// Loads

defm : CheriLdPat<load, CFLD>;

/// Stores

defm : CheriStPat<store, CFSD, FPR64>;

} // Predicates = [HasCheri, HasStdExtD, IsCapMode]

/// 'XCheri' extension

let Predicates = [HasCheri, IsRV32, IsCapMode] in {
defm : CheriLdPat<load, CLC_64>;
defm : CheriStPat<store, CSC_64, GPCR>;
} // Predicates = [HasCheri, IsRV32, IsCapMode]

let Predicates = [HasCheri, IsRV64, IsCapMode] in {
defm : CheriLdPat<load, CLC_128>;
defm : CheriStPat<store, CSC_128, GPCR>;
} // Predicates = [HasCheri, IsRV64, IsCapMode]

//===----------------------------------------------------------------------===//
// Compress Instruction tablegen backend.
//===----------------------------------------------------------------------===//

// Quadrant 0
let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CIncOffsetImm GPCRC:$rd, CSP:$rs1, uimm10_lsb00nonzero:$imm),
                  (C_CIncOffsetImm4CSPN GPCRC:$rd, CSP:$rs1, uimm10_lsb00nonzero:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode] in {
def : CompressPat<(CFLD FPR64C:$rd, GPCRC:$rs1, uimm8_lsb000:$imm),
                  (C_CFLD FPR64C:$rd, GPCRC:$rs1, uimm8_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CLC_128 GPCRC:$rd, GPCRC:$rs1, uimm9_lsb0000:$imm),
                  (C_CLC_128 GPCRC:$rd, GPCRC:$rs1, uimm9_lsb0000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CLW GPRC:$rd, GPCRC:$rs1, uimm7_lsb00:$imm),
                  (C_CLW GPRC:$rd, GPCRC:$rs1, uimm7_lsb00:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in {
def : CompressPat<(CLC_64 GPCRC:$rd, GPCRC:$rs1, uimm8_lsb000:$imm),
                  (C_CLC_64 GPCRC:$rd, GPCRC:$rs1, uimm8_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CLD GPRC:$rd, GPCRC:$rs1, uimm8_lsb000:$imm),
                  (C_CLD GPRC:$rd, GPCRC:$rs1, uimm8_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode] in {
def : CompressPat<(CFSD FPR64C:$rs2, GPCRC:$rs1, uimm8_lsb000:$imm),
                  (C_CFSD FPR64C:$rs2, GPCRC:$rs1, uimm8_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CSC_128 GPCRC:$rs2, GPCRC:$rs1, uimm9_lsb0000:$imm),
                  (C_CSC_128 GPCRC:$rs2, GPCRC:$rs1, uimm9_lsb0000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CSW GPRC:$rs2, GPCRC:$rs1, uimm7_lsb00:$imm),
                  (C_CSW GPRC:$rs2, GPCRC:$rs1, uimm7_lsb00:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in {
def : CompressPat<(CSC_64 GPCRC:$rs2, GPCRC:$rs1, uimm8_lsb000:$imm),
                  (C_CSC_64 GPCRC:$rs2, GPCRC:$rs1, uimm8_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CSD GPRC:$rs2, GPCRC:$rs1, uimm8_lsb000:$imm),
                  (C_CSD GPRC:$rs2, GPCRC:$rs1, uimm8_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

// Quadrant 1
let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in {
def : CompressPat<(CJAL C1, simm12_lsb0:$offset),
                  (C_CJAL simm12_lsb0:$offset)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CIncOffsetImm C2, C2, simm10_lsb0000nonzero:$imm),
                  (C_CIncOffsetImm16CSP C2, simm10_lsb0000nonzero:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CJAL C0, simm12_lsb0:$offset),
                  (C_J simm12_lsb0:$offset)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

// Quadrant 2
let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode] in {
def : CompressPat<(CFLD FPR64:$rd, CSP:$rs1, uimm9_lsb000:$imm),
                  (C_CFLDCSP FPR64:$rd, CSP:$rs1, uimm9_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CLC_128 GPCRNoC0:$rd, CSP:$rs1, uimm10_lsb0000:$imm),
                  (C_CLCCSP_128 GPCRNoC0:$rd, CSP:$rs1, uimm10_lsb0000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CLW GPRNoX0:$rd, CSP:$rs1,  uimm8_lsb00:$imm),
                  (C_CLWCSP GPRNoX0:$rd, CSP:$rs1, uimm8_lsb00:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in {
def : CompressPat<(CLC_64 GPCRNoC0:$rd, CSP:$rs1, uimm9_lsb000:$imm),
                  (C_CLCCSP_64 GPCRNoC0:$rd, CSP:$rs1, uimm9_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CLD GPRNoX0:$rd, CSP:$rs1, uimm9_lsb000:$imm),
                  (C_CLDCSP GPRNoX0:$rd, CSP:$rs1, uimm9_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CJALR C0, GPCRNoC0:$rs1, 0),
                  (C_CJR GPCRNoC0:$rs1)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CJALR C1, GPCRNoC0:$rs1, 0),
                  (C_CJALR GPCRNoC0:$rs1)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode] in {
def : CompressPat<(CFSD FPR64:$rs2, CSP:$rs1, uimm9_lsb000:$imm),
                  (C_CFSDCSP FPR64:$rs2, CSP:$rs1, uimm9_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, HasStdExtD, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CSC_128 GPCR:$rs2, CSP:$rs1, uimm10_lsb0000:$imm),
                  (C_CSCCSP_128 GPCR:$rs2, CSP:$rs1, uimm10_lsb0000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode] in {
def : CompressPat<(CSW GPR:$rs2, CSP:$rs1, uimm8_lsb00:$imm),
                  (C_CSWCSP GPR:$rs2, CSP:$rs1, uimm8_lsb00:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode] in {
def : CompressPat<(CSC_64 GPCR:$rs2, CSP:$rs1, uimm9_lsb000:$imm),
                  (C_CSCCSP_64 GPCR:$rs2, CSP:$rs1, uimm9_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV32, IsCapMode]

let Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode] in {
def : CompressPat<(CSD GPR:$rs2, CSP:$rs1, uimm9_lsb000:$imm),
                  (C_CSDCSP GPR:$rs2, CSP:$rs1, uimm9_lsb000:$imm)>;
} // Predicates = [HasCheri, HasCheriRVC, HasStdExtC, IsRV64, IsCapMode]
