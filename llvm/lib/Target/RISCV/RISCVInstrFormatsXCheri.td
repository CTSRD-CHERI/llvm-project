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
class RVInstCheriSrcDst<bits<7> funct7, bits<5> funct5, bits<3> funct3,
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
class RVInstCheriSCR<bits<7> funct7, bits<3> funct3,
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

// Like an RVInstR, except rd is now an additional function code. The
// instruction also has no outputs.
class RVInstCheriTwoSrc<bits<7> funct7, bits<5> funct5, bits<3> funct3,
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
