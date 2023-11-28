//===-- RISCVInstrFormatsZCheriPurecap.td ---------------------*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file describes the RISC-V ZCheriPureCap extension instruction formats.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Operand definitions.
//===----------------------------------------------------------------------===//

def CheriCSRSystemRegister : AsmOperandClass {
  let Name = "CheriCSRSystemRegister";
  let ParserMethod = "parseCSRSystemRegister";
  let DiagnosticType = "InvalidCheriCSRSystemRegister";
}

def cheri_csr_sysreg : Operand<XLenVT> {
  let ParserMatchClass = CheriCSRSystemRegister;
  let PrintMethod = "printCheriCSRSystemRegister";
  let DecoderMethod = "decodeCheriSysReg";
  let OperandType = "OPERAND_UIMM12";
  let OperandNamespace = "RISCVOp";
}

def CSetBndImmOperand : AsmOperandClass {
  let Name = "CSetBndImm";
  let ParserMethod = "parseCSetBndImmOperand";
  let RenderMethod = "addCSetBndImmOperands";
  let DiagnosticType = "InvalidCSetBndImm";
}

def csetbnd_imm : Operand<XLenVT>,
    ComplexPattern<XLenVT, 1, "SelectCSetBndImm", []>{
  let PrintMethod = "printCSetBndImm";
  let EncoderMethod = "getCSetBndImmOpValue";
  let ParserMatchClass = CSetBndImmOperand;
  let DecoderMethod = "DecodeCSetBndImm";
  let MIOperandInfo = (ops i32imm);
}

//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//

// Like an RVInstR, except rs2 is now an additional function code.
class RVInstZCheriSrcDst<bits<7> funct7, bits<5> funct5, bits<3> funct3,
                         RISCVOpcode opcode, dag outs, dag ins,
                         string opcodestr, string argstr>
      : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = funct7;
  let Inst{24-20} = funct5;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7}  = rd;
  let Inst{6-0}   = opcode.Value;
}

class RVInstCheriJump<bits<12> funct12, bits<3> funct3,
                      RISCVOpcode opcode, dag outs, dag ins, string opcodestr,
                      string argstr>
      : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {

  bits<5> rs1;
  bits<5> rd;

  let Inst{31-20} = funct12;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7}  = rd;
  let Inst{6-0}   = opcode.Value;
}

class RVInstCheriSetBoundsImmFmt<bits<6> funct6, bits<3> funct3,
                                 RISCVOpcode opcode, dag outs, dag ins,
                                 string opcodestr, string argstr>
      : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {

  bits<5> rd;
  bits<5> rs1;
  bits<6> imm;

  let Inst{31-26} = funct6;
  let Inst{25}    = imm{5};
  let Inst{24-20} = imm{4-0};
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7}  = rd;
  let Inst{6-0}   = opcode.Value;
}

let hasNoSchedulingInfo = 1,
    hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
class CheriCSR_ir<bits<3> funct3, string opcodestr, RegisterClass rs1Class=GPCR>
    : RVInstI<funct3, OPC_SYSTEM, (outs GPCR:$rd),
              (ins cheri_csr_sysreg:$imm12, rs1Class:$rs1), opcodestr,
              "$rd, $imm12, $rs1">, Sched<[WriteCSR, ReadCSR]>;

let hasNoSchedulingInfo = 1,
    hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
class CheriCSR_ii<bits<3> funct3, string opcodestr>
    : RVInstI<funct3, OPC_SYSTEM, (outs GPCR:$rd),
              (ins cheri_csr_sysreg:$imm12, uimm5:$rs1), opcodestr,
              "$rd, $imm12, $rs1">, Sched<[WriteCSR]>;
