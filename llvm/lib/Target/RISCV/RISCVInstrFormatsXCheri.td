//===-- RISCVInstrFormatsXCheri.td ---------------------------*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file describes the RISC-V XCheri extension instruction formats.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Operand definitions.
//===----------------------------------------------------------------------===//

def uimm2 : Operand<XLenVT>, ImmLeaf<XLenVT, [{return isUInt<2>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<2>;
  let DecoderMethod = "decodeUImmOperand<2>";
}

def uimm8 : Operand<XLenVT>, ImmLeaf<XLenVT, [{return isUInt<8>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<8>;
  let DecoderMethod = "decodeUImmOperand<8>";
}

def uimm12 : Operand<XLenVT>, ImmLeaf<XLenVT, [{return isUInt<12>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<12>;
  let DecoderMethod = "decodeUImmOperand<12>";
}

def SpecialCapRegisterAsmOperand : AsmOperandClass {
  let Name = "SpecialCapRegister";
  let ParserMethod = "parseSpecialCapRegister";
  let DiagnosticType = "InvalidSpecialCapRegister";
}

def special_capreg : Operand<XLenVT> {
  let ParserMatchClass = SpecialCapRegisterAsmOperand;
  let PrintMethod = "printSpecialCapRegister";
  let DecoderMethod = "decodeUImmOperand<5>";
}

//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//

// Like an RVInstR, except rs2 is now an additional function code.
class RVInstCheriRR<bits<7> funct7, bits<5> funct5, bits<3> funct3,
                    RISCVOpcode opcode, dag outs, dag ins, string opcodestr,
                    string argstr>
    : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = funct7;
  let Inst{24-20} = funct5;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

// Like an RVInstR, except rs2 is now an immediate.
class RVInstCheriRRI<bits<7> funct7, bits<3> funct3,
                     RISCVOpcode opcode, dag outs, dag ins, string opcodestr,
                     string argstr>
    : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> imm5;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = funct7;
  let Inst{24-20} = imm5;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

// Like an RVInstR, except rd is now an immediate. The instruction also has no
// outputs.
class RVInstCheriZRRI<bits<7> funct7, bits<3> funct3,
                      RISCVOpcode opcode, dag outs, dag ins, string opcodestr,
                      string argstr>
    : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> imm5;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31-25} = funct7;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = imm5;
  let Opcode = opcode.Value;
}

// Like an RVInstR, except rd is now an additional function code. The
// instruction also has no outputs.
class RVInstCheriZRR<bits<7> funct7, bits<5> funct5, bits<3> funct3,
                     RISCVOpcode opcode, dag outs, dag ins, string opcodestr,
                     string argstr>
    : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31-25} = funct7;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = funct5;
  let Opcode = opcode.Value;
}

class RVInstCheriClear<bits<7> funct7, bits<5> funct5, bits<3> funct3,
                       RISCVOpcode opcode, dag outs, dag ins, string opcodestr,
                       string argstr>
    : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<2> quarter;
  bits<8> mask;

  let Inst{31-25} = funct7;
  let Inst{24-20} = funct5;
  let Inst{19-18} = quarter;
  let Inst{17-15} = mask{7-5};
  let Inst{14-12} = funct3;
  let Inst{11-7} = mask{4-0};
  let Opcode = opcode.Value;
}

//===----------------------------------------------------------------------===//
// Instruction Class Templates
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_rc<bits<7> funct7, bits<5> funct5, string opcodestr>
    : RVInstCheriRR<funct7, funct5, 0, OPC_CHERI, (outs GPR:$rd),
                    (ins GPCR:$rs1), opcodestr, "$rd, $rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_ccc<bits<7> funct7, string opcodestr>
    : RVInstR<funct7, 0, OPC_CHERI, (outs GPCR:$rd),
              (ins GPCR:$rs1, GPCR:$rs2), opcodestr, "$rd, $rs1, $rs2">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_ccr<bits<7> funct7, string opcodestr, RegisterClass rs1Class=GPCR>
    : RVInstR<funct7, 0, OPC_CHERI, (outs GPCR:$rd),
              (ins rs1Class:$rs1, GPR:$rs2), opcodestr, "$rd, $rs1, $rs2">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_cci<bits<3> funct3, string opcodestr, bit simm>
    : RVInstI<funct3, OPC_CHERI, (outs GPCR:$rd),
              (ins GPCR:$rs1, !if(simm,simm12,uimm12):$imm12),
              opcodestr, "$rd, $rs1, $imm12">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_cc<bits<7> funct7, bits<5> funct5, string opcodestr>
    : RVInstCheriRR<funct7, funct5, 0, OPC_CHERI, (outs GPCR:$rd),
                    (ins GPCR:$rs1), opcodestr, "$rd, $rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_rcc<bits<7> funct7, string opcodestr, RegisterClass rs2Class=GPCR>
    : RVInstR<funct7, 0, OPC_CHERI, (outs GPR:$rd),
              (ins GPCR:$rs1, rs2Class:$rs2), opcodestr, "$rd, $rs1, $rs2">;

let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
class Cheri_ccs<bits<7> funct7, string opcodestr>
    : RVInstCheriRRI<funct7, 0, OPC_CHERI, (outs GPCR:$rd),
                     (ins GPCR:$rs1, special_capreg:$imm5),
                     opcodestr, "$rd, $rs1, $imm5">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Cheri_clear<bits<7> funct7, bits<5> funct5, string opcodestr>
    : RVInstCheriClear<funct7, funct5, 0, OPC_CHERI, (outs),
                       (ins uimm2:$quarter, uimm8:$mask),
                       opcodestr, "$quarter, $mask">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class CheriLoad_explicit<bits<5> op, string opcodestr, RegisterClass dstClass,
                         RegisterClass addrClass>
    : RVInstCheriRR<0x7d, op, 0, OPC_CHERI, (outs dstClass:$rd),
                    (ins addrClass:$rs1), opcodestr, "$rd, $rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
class CheriStore_explicit<bits<5> op, string opcodestr, RegisterClass srcClass,
                         RegisterClass addrClass>
    : RVInstCheriZRR<0x7c, op, 0, OPC_CHERI, (outs),
                     (ins srcClass:$rs2, addrClass:$rs1),
                     opcodestr, "$rs2, $rs1">;
