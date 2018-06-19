//=-- MipsInstrFormatsCheri.td- CHERIiInstruction Formats ----*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// BC2F
class CheriFmtCBT<bits<5> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  MipsInst<outs, ins, asmstr, pattern, NoItinerary, FrmOther>, Sched<[]>
{
  bits<5>  cb;
  bits<16> offset;

  let Opcode = 0x12;
  let Inst{25-21} = op;
  let Inst{20-16} = cb;
  let Inst{15-0} = offset;

  let isBranch = 1;
  let isTerminator = 1;
  let hasDelaySlot = 1;
  let hasSideEffects = 0;
}

class CheriFmtRegs<bits<5> op, bits<3> fmt, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  MipsInst<outs, ins, asmstr, pattern, NoItinerary, FrmOther>, Sched<[]>
{
  bits<5> r0;
  bits<5> r1;
  bits<5> r2;


  let Opcode = 0x12;
  let Inst{25-21} = op;
  let Inst{20-16} = r0;
  let Inst{15-11} = r1;
  let Inst{10-6}  = r2;
  let Inst{5-3}  = 0; // Reserved
  let Inst{2-0}   = fmt;
  let hasSideEffects = 0;
}

// CGet*
class CheriFmtGet<bits<3> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  CheriFmtRegs<0x0, op, outs, ins, asmstr, pattern> {
  bits<5>  rd;
  bits<5>  cb;
  let r0 = rd;
  let r1 = cb;
  let r2 = 0;
}
class CheriFmtGetCause<bits<5> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  CheriFmtRegs<0, 4,outs, ins, asmstr, pattern> {
  bits<5>  rd;
  let r0 = rd;
  let r1 = 0;
  let r2 = 0;
}
class CheriFmtSetCause<bits<5> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  CheriFmtRegs<4, 4,outs, ins, asmstr, pattern> {
  bits<5>  rs;
  let r0 = 0;
  let r1 = 0;
  let r2 = rs;
  let hasSideEffects = 1;
}


// CSet*
class CheriFmtSet<bits<3> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  CheriFmtRegs<0x4, op, outs, ins, asmstr, pattern> {
  bits<5>  cd;
  bits<5>  cb;
  bits<5>  rt;
  let r0 = cd;
  let r1 = cb;
  let r2 = rt;
  let hasSideEffects = 0;
}

// 
class CheriFmt2Caps<bits<5> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  CheriFmtRegs<op, 0, outs, ins, asmstr, pattern> {
  bits<5>  cd;
  bits<5>  cs;
  let r0 = cd;
  let r1 = cs;
  let r2 = 0;
}

// CSealData, CUnseal
class CheriFmt3Caps<bits<5> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  CheriFmtRegs<op, 0, outs, ins, asmstr, pattern> {
  bits<5>  cd;
  bits<5>  cs;
  bits<5>  ct;
  let r0 = cd;
  let r1 = cs;
  let r2 = ct;
}

class CheriFmt3CCall<bits<5> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  CheriFmtRegs<op, 0, outs, ins, asmstr, pattern> {
  bits<5>  cs;
  bits<5>  cb;
  bits<11> selector;

  let Opcode = 0x12;
  let Inst{25-21} = 0x05;
  let Inst{20-16} = cs;
  let Inst{15-11} = cb;
  let Inst{10-0} = selector;
}
class CheriFmt2CJMP<bits<5> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  CheriFmtRegs<op, 0, outs, ins, asmstr, pattern> {
  bits<5>  cs;
  bits<5>  rt;
  let r0 = 0;
  let r1 = cs;
  let r2 = rt;
}
class CheriFmt3CRet<bits<5> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  CheriFmtRegs<op, 0, outs, ins, asmstr, pattern> {
  let Opcode = 0x12;
  let Inst{25-21} = 0x05;
  let Inst{20-11} = 0;
  let Inst{10-0} = -1;
}

class CheriFmtClearTag<dag outs, dag ins, string asmstr,
                       list<dag> pattern> :
  CheriFmtSet<0x5, outs, ins, asmstr, pattern>
{
  let rt = 0;
}


// Load / Store capabilities
class CheriFmtCLS<bits<6> op, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  MipsInst<outs, ins, asmstr, pattern, NoItinerary, FrmOther>, Sched<[]>
{
  bits<5>  cs;
  bits<5>  cb;
  bits<5>  rt;
  bits<11> offset;

  let Opcode = op;
  let Inst{25-21} = cs;
  let Inst{20-16} = cb;
  let Inst{15-11} = rt;
  let Inst{10-0}  = offset;
  let hasSideEffects = 0;
}

class CheriFmtCLS_BigImm<bits<6> op, dag outs, dag ins, string asmstr,
                         list<dag> pattern> :
  MipsInst<outs, ins, asmstr, pattern, NoItinerary, FrmOther>, Sched<[]>
{
  bits<5>  cs;
  bits<5>  cb;
  bits<16> offset;

  let Opcode = op;
  let Inst{25-21} = cs;
  let Inst{20-16} = cb;
  let Inst{15-0}  = offset;
  let hasSideEffects = 0;
}

class CheriFmtCLX<bits<2> t, bits<1> e, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  MipsInst<outs, ins, asmstr, pattern, NoItinerary, FrmOther>, Sched<[]>
{
  bits<5>  rd;
  bits<5>  cb;
  bits<5>  rt;
  bits<8>  offset;

  let Opcode = 0x32;
  let Inst{25-21} = rd;
  let Inst{20-16} = cb;
  let Inst{15-11} = rt;
  let Inst{10-3}  = offset;
  let Inst{2}     = e;
  let Inst{1-0}   = t;
  let hasSideEffects = 0;
}

class CheriFmtCSX<bits<2> t, bits<1> e, dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  MipsInst<outs, ins, asmstr, pattern, NoItinerary, FrmOther>, Sched<[]>
{
  bits<5>  rs;
  bits<5>  cb;
  bits<5>  rt;
  bits<8>  offset;

  let Opcode = 0x3a;
  let Inst{25-21} = rs;
  let Inst{20-16} = cb;
  let Inst{15-11} = rt;
  let Inst{10-3}  = offset;
  let Inst{2}     = e;
  let Inst{1-0}   = t;
  let hasSideEffects = 0;
}

class CheriFmtClearRegs<bits<5> regset, dag outs, dag ins, string asmstr,
                        list<dag> pattern> :
  MipsInst<outs, ins, asmstr, pattern, NoItinerary, FrmOther>, Sched<[]>
{
  bits<16> mask;

  let Opcode = 0x12;
  let Inst{25-21} = 0xf;
  let Inst{20-16} = regset;
  let Inst{15-0}  = mask;
  let hasSideEffects = 1;
}

class CheriFmtCLL<bit isSigned, bits<2> type, dag outs, dag ins, string asmstr> :
                  MipsInst<outs, ins, asmstr, [], NoItinerary, FrmOther>, Sched<[]>
{
  bits<5> rd;
  bits<5> cb;

  let Opcode = 0x12;
  let mayLoad = 1;

  let Inst{25-21} = 0x10;
  let Inst{20-16} = rd;
  let Inst{15-11} = cb;
  let Inst{10-2}  = 0; // Reserved
  let Inst{3}   = 1;
  let Inst{2}   = isSigned;
  let Inst{1-0}   = type;
}

class CheriFmtCSC<bits<3> type, dag outs, dag ins, string asmstr> :
                  MipsInst<outs, ins, asmstr, [], NoItinerary, FrmOther>, Sched<[]>
{
  bits<5> rs;
  bits<5> rd;
  bits<5> cb;

  let Opcode = 0x12;
  let mayStore = 1;

  let Inst{25-21} = 0x10;
  let Inst{20-16} = rs;
  let Inst{15-11} = cb;
  let Inst{10-6} = rd;
  let Inst{5-2}  = 0; // Reserved
  let Inst{3}   = 0;
  let Inst{2-0}   = type;
}

class CheriFmt3Op<bits<6> type, dag outs, dag ins, string asmstr, list<dag> pattern> :
                  MipsInst<outs, ins, asmstr, pattern, NoItinerary, FrmOther>, Sched<[]>
{
  bits<5> r1;
  bits<5> r2;
  bits<5> r3;

  let Opcode = 0x12;

  let Inst{25-21} = 0x0;
  let Inst{20-16} = r1;
  let Inst{15-11} = r2;
  let Inst{10-6} = r3;
  let Inst{5-0}   = type;
  let hasSideEffects = 0;
}

class CheriFmtCSetImm<bits<5> type, Operand immOp, PatFrag immPat, string op, string intr=op, string verb="set", string intr_verb=verb> :
                      MipsInst<(outs CheriOpnd:$cd), (ins CheriOpnd:$cb, immOp: $imm),
                               !strconcat("c", verb, op, "\t$cd, $cb, ${imm}"),
                               [(set CheriOpnd:$cd, (!cast<Intrinsic>(!strconcat("int_cheri_cap_", intr, "_", intr_verb)) CheriOpnd:$cb, (i64 immPat: $imm)))],
                               NoItinerary, FrmOther>,
                      Sched<[]>
{
  bits<5> cd;
  bits<5> cb;
  bits<11> imm;

  let Opcode = 0x12;

  let Inst{25-21} = type;
  let Inst{20-16} = cd;
  let Inst{15-11} = cb;
  let Inst{10-0} = imm;
  let hasSideEffects = 0;
}

class CheriFmt2Op<bits<5> type, dag outs, dag ins, string asmstr, list<dag> pattern> :
                  CheriFmt3Op<0x3f, outs, ins, asmstr, pattern>
{
  let r3 = type;
}

class CheriFmt1Op<bits<5> type, dag outs, dag ins, string asmstr, list<dag> pattern> :
                  CheriFmt2Op<0x1f, outs, ins, asmstr, pattern>
{
  let r2 = type;
}

class CheriFmtCGet<bits<5> type, string op, string intr=op> :
                  CheriFmt2Op<type, (outs GPR64Opnd:$r1), (ins CheriOpnd:$r2), 
                              !strconcat("cget", op, "\t$r1, $r2"),
                              [(set GPR64Opnd:$r1, (!cast<Intrinsic>(!strconcat("int_cheri_cap_", intr, "_get")) CheriOpnd:$r2))]>;

class CheriFmtCSet<bits<6> type, string op, string intr=op, string verb="set", string intr_verb=verb> :
                  CheriFmt3Op<type, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, GPR64Opnd:$r3), 
                              !strconcat("c", verb, op, "\t$r1, $r2, $r3"),
                              [(set CheriOpnd:$r1, (!cast<Intrinsic>(!strconcat("int_cheri_cap_", intr, "_", intr_verb)) CheriOpnd:$r2, GPR64Opnd:$r3))]>;

class CheriFmtPtrCmp<bits<6> cmp, RegisterOperand RC, string instr_asm,
                     list<dag> pattern>:
  CheriFmt3Op<cmp, (outs RC: $rd), (ins CheriOpnd: $cb, CheriOpnd: $ct),
           !strconcat(instr_asm, "\t$rd, $cb, $ct"),
           pattern>
{
  bits<5> rd;
  bits<5> cb;
  bits<5> ct;

  let r1 = rd;
  let r2 = cb;
  let r3 = ct;
  let hasSideEffects = 0;
}

