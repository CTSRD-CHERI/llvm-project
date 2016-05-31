//=- MipsInstrCheri.td - Target Description for CHERI Target -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the CHERI instruction definitions
//
//===----------------------------------------------------------------------===//


def SDT_MipsCBT      : SDTypeProfile<0, 2, [SDTCisVT<0, iFATPTR>]>;
def SDT_MipsPtrToCap     : SDTypeProfile<1, 1, [SDTCisVT<0, iFATPTR>,
                                            SDTCisVT<1, i64>]>;
def MipsSTC : SDNode<"MipsISD::STACKTOCAP", SDT_MipsPtrToCap,
                          []>;

def SDT_CheriJmpLink : SDTypeProfile<0, 2, [SDTCisVT<0, iPTR>, SDTCisVT<1, i32>]>;
def CheriJmpLink : SDNode<"MipsISD::CheriJmpLink",SDT_CheriJmpLink,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;

// Scaled immediate offsets

class SImmScaled<int Width, int Scale> : AsmOperandClass {
  let Name = "SImm" # Width # "s" # Scale;
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledImmediate<" # Width # ", " # Scale # ">";
}

multiclass ShiftImmPattern<int Width, int Shift> {
  def NAME # Pattern : ImmLeaf<i64,
    "return isShiftedInt<" # Width # ", " # Shift # ">(Imm);"
    >;
  def NAME : Operand<i64> {
    let ParserMatchClass = SImmScaled<Width, Shift>;
    let EncoderMethod = "getShiftedImmediate<" # Width # ", " # Shift # ">";
    let DecoderMethod = "DecodeShiftedImmediate<" # Width # ", " # Shift # ">";
  }
}

// 11-bit immediate offset used by C[SL]CR
defm simm11s4  : ShiftImmPattern<11, 4>;
// 8-bit immediate offset used by C[SL][DWHB]
defm simm8s3  : ShiftImmPattern<8, 3>;
defm simm8s2  : ShiftImmPattern<8, 2>;
defm simm8s1  : ShiftImmPattern<8, 1>;
defm simm8    : ShiftImmPattern<8, 0>;


// ClearRegs values
// TODO: Add some nice parsing so that we can get handle register lists for
// these.
def regslist: Operand<i32>;
def reglist_pat : PatLeaf<(imm), [{ return isInt<16>(N->getZExtValue()); }]>;

def uimm32: Operand<i32>;
def uimm32_pat:  PatLeaf<(imm), [{ return isInt<32>(N->getZExtValue()); }]>;

def uimm11: Operand<i32>;
def uimm11_pat:  PatLeaf<(imm), [{ return isUInt<11>(N->getZExtValue()); }]>;


////////////////////////////////////////////////////////////////////////////////
// Flow control
////////////////////////////////////////////////////////////////////////////////
let DecoderNamespace="CHERI", hasDelaySlot=1, isTerminator=1 in {
let isBranch=1 in {
def CBTS : CheriFmtCBT<0x0a, (outs), (ins CheriOpnd:$cb, brtarget:$offset),
                        "cbts\t$cb, $offset",
                        [(brcond (i32 (setne (int_mips_cap_tag_get CheriOpnd:$cb), 0)), bb:$offset)]>;
def CBTU : CheriFmtCBT<0x09, (outs), (ins CheriOpnd:$cb, brtarget:$offset),
                        "cbtu\t$cb, $offset",
                        [(brcond (i32 (seteq (int_mips_cap_tag_get CheriOpnd:$cb), 0)), bb:$offset)]>;
}

def CJR : CheriFmt3Caps<0x08, (outs), (ins CheriOpnd:$cb),
                              "cjr\t${cb}",
                              []>
{
	bits<5> cb;
	let r0 = 0;
	let r1 = cb;
	let r2 = 0;
}
}

// SelectionDAG doesn't know that int_mips_cap_tag_get returns 0 or 1.
def : Pat<(brcond (i32 (seteq (int_mips_cap_tag_get CheriOpnd:$cb), 1)), bb:$offset),
          (CBTS CheriOpnd: $cb, bb:$offset)>;
def : Pat<(brcond (i32 (setne (int_mips_cap_tag_get CheriOpnd:$cb), 1)), bb:$offset),
          (CBTU CheriOpnd: $cb, bb:$offset)>;

let isCall=1, hasDelaySlot=1 in {
def CJALR : CheriFmt3Caps<0x07, (outs CheriOpnd:$cd), (ins CheriOpnd:$cb),
                              "cjalr\t${cd}, ${cb}",
                              []>
{
	bits<5> cb;
	bits<5> cd;
	let r0 = cd;
	let r1 = cb;
	let r2 = 0;
}
def CCall : CheriFmt3CCall<(outs), (ins CheriOpnd:$cs, CheriOpnd:$cb, uimm11: $imm),
                              "ccall\t${cs}, ${cb}, ${imm}",
                              []>;
}
def : InstAlias<"ccall $cs, $cb",
                (CCall CheriOpnd: $cs, CheriOpnd:$cb, 0)>;
def CReturn : CheriFmt3CRet<0x06, (outs), (ins variable_ops),
                              "creturn",
                              []> {
  let isTerminator = 1;
  let isReturn = 1;
  let hasDelaySlot = 1;
}

////////////////////////////////////////////////////////////////////////////////
// Capability-based pointer comparisons
////////////////////////////////////////////////////////////////////////////////
def CEQ : CheriFmtPtrCmp<0, GPR64Opnd, "ceq", []>;
def CNE : CheriFmtPtrCmp<1, GPR64Opnd, "cne", []>;
def CLT : CheriFmtPtrCmp<2, GPR64Opnd, "clt", []>;
def CLE : CheriFmtPtrCmp<3, GPR64Opnd, "cle", []>;
def CLTU : CheriFmtPtrCmp<4, GPR64Opnd, "cltu", []>;
def CLEU : CheriFmtPtrCmp<5, GPR64Opnd, "cleu", []>;
def : InstAlias<"cgt $rd, $cb, $ct",
                (CLT GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
def : InstAlias<"cge $rd, $cb, $ct",
                (CLE GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
def : InstAlias<"cgtu $rd, $cb, $ct",
                (CLTU GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
def : InstAlias<"cgeu $rd, $cb, $ct",
                (CLEU GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
let isCodeGenOnly=1 in {
  def CEQ32 : CheriFmtPtrCmp<0, GPR32Opnd, "ceq", [(set GPR32Opnd: $rd, (seteq CheriOpnd: $cb, CheriOpnd:$ct))]>;
  def CNE32 : CheriFmtPtrCmp<1, GPR32Opnd, "cne", [(set GPR32Opnd: $rd, (setne CheriOpnd: $cb, CheriOpnd:$ct))]>;
  def CLT32 : CheriFmtPtrCmp<2, GPR32Opnd, "clt", [(set GPR32Opnd: $rd, (setlt CheriOpnd: $cb, CheriOpnd:$ct))]>;
  def CLE32 : CheriFmtPtrCmp<3, GPR32Opnd, "cle", [(set GPR32Opnd: $rd, (setle CheriOpnd: $cb, CheriOpnd:$ct))]>;
  def CLTU32 : CheriFmtPtrCmp<2, GPR32Opnd, "cltu", [(set GPR32Opnd: $rd, (setult CheriOpnd: $cb, CheriOpnd:$ct))]>;
  def CLEU32 : CheriFmtPtrCmp<3, GPR32Opnd, "cleu", [(set GPR32Opnd: $rd, (setule CheriOpnd: $cb, CheriOpnd:$ct))]>;
}
def : Pat<(setugt CheriOpnd: $cb, CheriOpnd:$ct), (CLTU32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setuge CheriOpnd: $cb, CheriOpnd:$ct), (CLEU32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setgt CheriOpnd: $cb, CheriOpnd:$ct), (CLT32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setge CheriOpnd: $cb, CheriOpnd:$ct), (CLE32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setogt CheriOpnd: $cb, CheriOpnd:$ct), (CLT32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setoge CheriOpnd: $cb, CheriOpnd:$ct), (CLE32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setolt CheriOpnd: $cb, CheriOpnd:$ct), (CLT32 CheriOpnd: $cb, CheriOpnd:$ct)>;
def : Pat<(setole CheriOpnd: $cb, CheriOpnd:$ct), (CLE32 CheriOpnd: $cb, CheriOpnd:$ct)>;

class CheriPseudo<dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  MipsInst<outs, ins, asmstr, pattern, NoItinerary, Pseudo>
{
  let isPseudo = 1;
  let usesCustomInserter = 1;
}

// Pseudo instruction for capability cmov, expands to a branch sequence
def CAP_SELECT : CheriPseudo<(outs CheriOpnd:$dst), (ins GPR32Opnd:$cond,
                              CheriOpnd:$t, CheriOpnd:$f),
                              "#CAP_SELECT",
                              [(set CheriOpnd: $dst, (select GPR32Opnd:$cond, CheriOpnd:$t, CheriOpnd:$f))]>;


////////////////////////////////////////////////////////////////////////////////
// Capability manipulation
////////////////////////////////////////////////////////////////////////////////
def CGetBase : CheriFmtGet<2, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                           "cgetbase $rd, $cb",
                           [(set GPR64Opnd:$rd, (int_mips_cap_base_get CheriOpnd:$cb))]>;
let isCodeGenOnly=1 in {
def CGetBase32 : CheriFmtGet<2, (outs GPR32:$rd), (ins CheriOpnd:$cb),
                           "cgetbase $rd, $cb", []>;
}


def CGetTag : CheriFmtGet<5, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                          "cgettag $rd, $cb",
                          [(set GPR64Opnd:$rd, (int_mips_cap_tag_get CheriOpnd:$cb))]>;


def CGetLength : CheriFmtGet<3, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                             "cgetlen $rd, $cb",
                             [(set GPR64Opnd:$rd, (int_mips_cap_length_get CheriOpnd:$cb))]>;

def CAndPerm : CheriFmtSet<0, (outs CheriOpnd:$cd), (ins CheriOpnd:$cb, GPR64Opnd:$rt),
                           "candperm\t$cd, $cb, $rt",
                           [(set CheriOpnd:$cd, (int_mips_cap_perms_and CheriOpnd:$cb, GPR64Opnd:$rt))]>;

def CGetPerm : CheriFmtGet<0, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                           "cgetperm $rd, $cb",
                           [(set GPR64Opnd:$rd, (int_mips_cap_perms_get CheriOpnd:$cb))]>;

def CSetBounds : CheriFmtRegs<1, 0, (outs CheriOpnd:$r0), (ins CheriOpnd:$r1, GPR64Opnd:$r2),
                           "csetbounds\t$r0, $r1, $r2",
                           [(set CheriOpnd:$r0, (int_mips_cap_bounds_set CheriOpnd:$r1, GPR64Opnd:$r2))]>;

def CGetType : CheriFmtGet<1, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                           "cgettype $rd, $cb",
                           [(set GPR64Opnd:$rd, (int_mips_cap_type_get CheriOpnd:$cb))]>;

def CSeal : CheriFmt3Caps<0x02, (outs CheriOpnd:$cd), (ins CheriOpnd:$cs, CheriOpnd:$ct),
                              "cseal\t$cd, $cs, $ct",
                              [(set CheriOpnd: $cd, (int_mips_cap_seal CheriOpnd:$cs, CheriOpnd:$ct))]>;

def CUnseal   : CheriFmt3Caps<0x03, (outs CheriOpnd:$cd), (ins CheriOpnd:$cs, CheriOpnd:$ct),
                              "cunseal\t$cd, $cs, $ct",
                              [(set CheriOpnd: $cd, (int_mips_cap_unseal CheriOpnd:$cs, CheriOpnd:$ct))]>;

def CGetUnsealed : CheriFmtGet<6, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                          "cgetsealed\t$rd, $cb",
                          [(set GPR64Opnd:$rd, (int_mips_cap_sealed_get CheriOpnd:$cb))]>;

def CGetPCC : CheriFmt1Op<0x0, (outs CheriOpnd:$r1), (ins),
                          "cgetpcc\t${r1}",
                          [(set CheriOpnd:$r1, (int_mips_pcc_get))]>;

def CSetPCCOffset : CheriFmt2Op<7, (outs CheriOpnd:$cb), (ins GPR64Opnd:$rd),
                                "cgetpccsetoffset\t${cb}, $rd",
                                [(set CheriOpnd:$cb, (int_mips_cap_offset_set (int_mips_pcc_get), GPR64Opnd: $rd))]>;

def CGetCause : CheriFmtGetCause<0, (outs GPR64Opnd:$rd), (ins),
                          "cgetcause\t${rd}",
                          [(set GPR64Opnd:$rd, (int_mips_cap_cause_get))]>;
def CSetCause : CheriFmtSetCause<4, (outs), (ins GPR64Opnd: $rs),
                          "csetcause\t${rs}",
                          [(int_mips_cap_cause_set GPR64Opnd: $rs)]>;


def CCheckPerm : CheriFmtRegs<0x0b, 0, (outs), (ins CheriOpnd:$cs, GPR64Opnd: $rs),
                              "ccheckperm\t${cs}, ${rs}",
                              [(int_mips_cap_perms_check CheriOpnd:$cs,  GPR64Opnd: $rs)]>
{
  bits<5>  cs;
  bits<5>  rs;
  let r0 = cs;
  let r1 = 0;
  let r2 = rs;
}
def CCheckType : CheriFmtRegs<0x0b, 1, (outs), (ins CheriOpnd:$cs, CheriOpnd:$cb),
                              "cchecktype\t${cs}, ${cb}",
                              [(int_mips_cap_type_check CheriOpnd:$cs,  CheriOpnd:$cb)]>
{
  bits<5>  cs;
  bits<5>  cb;
  let r0 = cs;
  let r1 = cb;
  let r2 = 0;
}

// Cursors
def CIncOffset : CheriFmtRegs<0x0d, 0, (outs CheriOpnd: $r0),
  (ins CheriOpnd: $r1, GPR64Opnd: $r2),
  "cincoffset\t$r0, $r1, $r2",
  [(set CheriOpnd:$r0, (ptradd CheriOpnd:$r1, GPR64Opnd:$r2))]>;
def : Pat<(int_mips_cap_offset_increment CheriOpnd: $r1, GPR64Opnd: $r2), (CIncOffset $r1, $r2)>;
def CSetOffset : CheriFmtRegs<0x0d, 1, (outs CheriOpnd: $r0),
  (ins CheriOpnd: $r1, GPR64Opnd: $r2),
  "csetoffset\t$r0, $r1, $r2",
  [(set CheriOpnd: $r0, (int_mips_cap_offset_set CheriOpnd: $r1, GPR64Opnd: $r2))]>;
def CGetOffset : CheriFmtRegs<0x0d, 2, (outs GPR64Opnd:$r0),
  (ins CheriOpnd:$r1),
  "cgetoffset\t$r0, $r1",
  [(set GPR64Opnd: $r0, (int_mips_cap_offset_get CheriOpnd: $r1))]> { let r2 = 0; }

def CMove : CheriPseudo<(outs CheriOpnd: $cd),
  (ins CheriOpnd:$cs),
  "cmove\t$cd,  $cs",
  []>;

// Casting:

def CFromPtr : CheriFmtRegs<4, 7, (outs CheriOpnd: $r0),
  (ins CheriOpnd: $r1, GPR64Opnd: $r2),
  "cfromptr\t$r0, $r1, $r2", [(set CheriOpnd: $r0, (int_mips_cap_from_pointer CheriOpnd: $r1, GPR64Opnd: $r2))]>;
def CToPtr : CheriFmtRegs<0xc, 0, (outs GPR64Opnd: $r0),
  (ins CheriOpnd: $r2, CheriOpnd: $r1),
  "ctoptr $r0, $r1, $r2", [(set GPR64Opnd: $r0, (int_mips_cap_to_pointer CheriOpnd: $r2, CheriOpnd: $r1))]>;
let isCodeGenOnly = 1 in {
def CFromPtr32 : CheriFmtRegs<4, 7, (outs CheriOpnd: $r0),
  (ins CheriOpnd: $r2, GPR32Opnd: $r1),
  "cfromptr\t$r0, $r1, $r2", []>;
def CToPtr32 : CheriFmtRegs<0xc, 0, (outs GPR32Opnd: $r0),
  (ins CheriOpnd: $r2, CheriOpnd: $r1),
  "ctoptr\t$r0, $r1, $r2", []>;
}

// Int to pointer in address space 0
def : Pat<(inttoptr GPR64Opnd:$rt),
          (CFromPtr C0, $rt)>;
def : Pat<(inttoptr GPR32:$rt),
          (CFromPtr32 C0, $rt)>;
def : Pat<(ptrtoint CheriOpnd:$rt),
          (CToPtr C0, $rt)>;
def : Pat<(ptrtoint CheriOpnd:$rt),
          (CToPtr32 C0, $rt)>;


let DecoderNamespace="CHERI" in
def STORECAP : CheriFmtCLS<0x3e, (outs), (ins CheriOpnd:$cs, GPR64Opnd:$rt, simm11s4:$offset, CheriOpnd:$cb),
                         "csc\t$cs, ${rt}, ${offset}(${cb})",
                         [(store CheriOpnd:$cs, (add (add GPR64Opnd:$rt, (ptrtoint CheriOpnd:$cb)), (i64 simm11s4Pattern:$offset)))]>;

def CLEARTAG : CheriFmtClearTag<(outs CheriOpnd:$cd), (ins CheriOpnd:$cb), "ccleartag\t$cd, $cb",
                                [(set CheriOpnd:$cd, (int_mips_cap_tag_clear CheriOpnd:$cb))]>;

def : Pat<(store CheriOpnd:$cs, CheriOpnd:$cb),
          (STORECAP CheriOpnd:$cs, ZERO_64, (i64 0), $cb)>;
def : Pat<(store CheriOpnd:$cs, GPR64Opnd:$rt),
          (STORECAP CheriOpnd:$cs, GPR64Opnd:$rt, (i64 0), C0)>;

// Load a capability
let Predicates = [IsCHERI] in {
def LOADCAP : CheriFmtCLS<0x36, (outs CheriOpnd:$cs), (ins GPR64Opnd:$rt, simm11s4:$offset, CheriOpnd:$cb),
                        "clc\t$cs, ${rt}, ${offset}(${cb})",
                        [(set CheriOpnd:$cs, (load (add (add GPR64Opnd:$rt, (ptrtoint CheriOpnd:$cb)), (i64 simm11s4Pattern:$offset))))]> {
  let DecoderNamespace="CHERI";
}

def : Pat<(load GPR64Opnd:$rt),
          (LOADCAP GPR64Opnd:$rt, (i64 0), C0)>;
def : Pat<(load CheriOpnd:$cb),
          (LOADCAP ZERO_64, (i64 0), CheriOpnd:$cb)>;

// Load via a capability register
let canFoldAsLoad = 1, DecoderNamespace="CHERI" in
multiclass LoadViaCapScaled<bits<2> t, bit sExt, string instr_asm, RegisterOperand RC, PatFrag loadType, Operand simm, PatFrag immfrag> {
  def #NAME# : CheriFmtCLX<t, sExt,
                             (outs RC:$rd),
                             (ins GPR64Opnd:$rt, simm:$offset, CheriOpnd:$cb),
                             !strconcat(instr_asm, "\t$rd, ${rt}, ${offset}(${cb})"),
                           [(set RC:$rd, (loadType (iFATPTR (ptradd CheriOpnd:$cb, (add GPR64Opnd: $rt, (i64 immfrag:$offset))))))]>;
}
multiclass LoadViaCap<bits<2> t, bit e, string instr_asm, RegisterOperand RC, PatFrag storeType> :
  LoadViaCapScaled<t, e, instr_asm, RC, storeType, simm8, immSExt8>;

multiclass LoadViaCap0<Instruction Op, PatFrag loadType> {
  def : Pat<(loadType CheriOpnd:$cb),
            (Op ZERO_64, (i64 0), CheriOpnd:$cb)>;
}

// Store via a capability register
let DecoderNamespace="CHERI" in
multiclass StoreViaCapScaled<bits<2> t, bit e, string instr_asm, RegisterOperand RC, PatFrag storeType, Operand simm, PatFrag immfrag> {
  def #NAME# : CheriFmtCSX<t, e,
                             (outs),
                             (ins RC:$rs, GPR64Opnd:$rt, simm:$offset, CheriOpnd:$cb),
                             !strconcat(instr_asm, "\t$rs, ${rt}, ${offset}(${cb})"),
                           [(storeType (iFATPTR (ptradd CheriOpnd:$cb, (add GPR64Opnd: $rt, (i64 immfrag:$offset)))), RC:$rs)]>;
}
multiclass StoreViaCap<bits<2> t, bit e, string instr_asm, RegisterOperand RC, PatFrag storeType> :
  StoreViaCapScaled<t, e, instr_asm, RC, storeType, simm8, immSExt8>;
 
multiclass StoreViaCap0<Instruction Op, RegisterOperand RC, PatFrag storeType> {
  def : Pat<(storeType RC:$rd, CheriOpnd:$cb),
            (Op RC:$rd, ZERO_64, (i64 0), CheriOpnd:$cb)>;
  def : Pat<(storeType RC:$rd, (add (ptrtoint CheriOpnd:$cb), GPR64Opnd:$rt)),
            (Op RC:$rd, GPR64Opnd:$rt, (i64 0), CheriOpnd:$cb)>;
}

let isCodeGenOnly = 1 in {
defm CAPLOAD832    : LoadViaCap<0, 1, "clb", GPR32Opnd, sextloadi8>;
defm CAPLOADU832   : LoadViaCap<0, 0, "clbu", GPR32Opnd, extloadi8>;
defm CAPLOAD1632   : LoadViaCapScaled<1, 1, "clh", GPR32Opnd, sextloadi16, simm8s1, simm8s1Pattern>;
defm CAPLOADU1632  : LoadViaCapScaled<1, 0, "clhu", GPR32Opnd, extloadi16, simm8s1, simm8s1Pattern>;
}
defm : LoadViaCap0<CAPLOAD832, sextloadi8>;
defm : LoadViaCap0<CAPLOAD832, extloadi8>;
defm : LoadViaCap0<CAPLOADU832, zextloadi8>;
defm : LoadViaCap0<CAPLOAD1632, extloadi16>;
defm : LoadViaCap0<CAPLOAD1632, sextloadi16>;
defm : LoadViaCap0<CAPLOADU1632, zextloadi16>;

defm CAPLOAD8      : LoadViaCap<0, 1, "clb", GPR64Opnd, sextloadi8>;
defm CAPLOAD16     : LoadViaCapScaled<1, 1, "clh", GPR64Opnd, sextloadi16, simm8s1, simm8s1Pattern>;
defm CAPLOAD32     : LoadViaCapScaled<2, 1, "clw", GPR32Opnd, load, simm8s2, simm8s2Pattern>;
defm CAPLOAD64     : LoadViaCapScaled<3, 0, "cld", GPR64Opnd, load, simm8s3, simm8s3Pattern>;

defm CAPLOADU8     : LoadViaCap<0, 0, "clbu", GPR64Opnd, extloadi8>;
defm CAPLOADU16    : LoadViaCapScaled<1, 0, "clhu", GPR64Opnd, extloadi16, simm8s1, simm8s1Pattern>;
defm CAPLOADU32    : LoadViaCapScaled<2, 0, "clwu", GPR64Opnd, zextloadi32, simm8s2, simm8s2Pattern>;

let isCodeGenOnly=1 in {
defm CAPLOAD3264   : LoadViaCapScaled<2, 1, "clw", GPR64Opnd, extloadi32, simm8s2, simm8s2Pattern>;
}
defm : LoadViaCap0<CAPLOAD3264, extloadi32>;

defm CAP0LOADA8     : LoadViaCap0<CAPLOAD8, extloadi8>;
defm CAP0LOADA16    : LoadViaCap0<CAPLOAD16, extloadi16>;
defm CAP0LOADA32    : LoadViaCap0<CAPLOAD32, extloadi32>;

defm CAP0LOADU8     : LoadViaCap0<CAPLOADU8, zextloadi8>;
defm CAP0LOADU16    : LoadViaCap0<CAPLOADU16, zextloadi16>;
defm CAP0LOADU32    : LoadViaCap0<CAPLOADU32, zextloadi32>;

defm CAP0LOAD8     : LoadViaCap0<CAPLOAD8, sextloadi8>;
defm CAP0LOAD16    : LoadViaCap0<CAPLOAD16, sextloadi16>;
defm CAP0LOAD3264  : LoadViaCap0<CAPLOAD3264, sextloadi32>;
defm CAP0LOAD32    : LoadViaCap0<CAPLOAD32, load>;
defm CAP0LOAD64    : LoadViaCap0<CAPLOAD64, load>;

defm CAPSTORE8      : StoreViaCap<0, 0, "csb", GPR64Opnd, truncstorei8>;
defm CAPSTORE16     : StoreViaCapScaled<1, 0, "csh", GPR64Opnd, truncstorei16, simm8s1, simm8s1Pattern>;
defm CAPSTORE32     : StoreViaCapScaled<2, 0, "csw", GPR32Opnd, store, simm8s2, simm8s2Pattern>;
let isCodeGenOnly=1 in {
  defm CAPSTORE3264     : StoreViaCapScaled<2, 0, "csw", GPR64Opnd, truncstorei32, simm8s2, simm8s2Pattern>;
}
defm CAPSTORE64     : StoreViaCapScaled<3, 0, "csd", GPR64Opnd, store, simm8s3, simm8s3Pattern>;
          
def : Pat<(int_mips_c0_get), (CIncOffset C0, ZERO_64)>;
def : Pat<(int_mips_idc_get), (CIncOffset C26, ZERO_64)>;
def : Pat<(int_mips_kr1c_get), (CIncOffset C27, ZERO_64)>;
def : Pat<(int_mips_kr2c_get), (CIncOffset C28, ZERO_64)>;
def : Pat<(int_mips_kcc_get), (CIncOffset C29, ZERO_64)>;
def : Pat<(int_mips_kdc_get), (CIncOffset C30, ZERO_64)>;
def : Pat<(int_mips_epcc_get), (CIncOffset C31, ZERO_64)>;
def : Pat<(int_mips_stack_cap_get), (CIncOffset C11, ZERO_64)>;

def : Pat<(int_mips_stack_to_cap GPR64Opnd:$addr), (CSetOffset C11, $addr)>;
def : Pat<(MipsSTC i64:$addr), (CIncOffset C11, $addr)>;

defm CAP0STORE8     : StoreViaCap0<CAPSTORE8, GPR64Opnd, truncstorei8>;
defm CAP0STORE16    : StoreViaCap0<CAPSTORE16, GPR64Opnd, truncstorei16>;
defm CAP0STORE32    : StoreViaCap0<CAPSTORE32, GPR32Opnd, store>;
defm CAP0STORE3264  : StoreViaCap0<CAPSTORE3264, GPR64Opnd, truncstorei32>;
defm CAP0STORE64    : StoreViaCap0<CAPSTORE64, GPR64Opnd, store>;

def CLDC1 : CheriPseudo<(outs FGR64Opnd: $rd),
  (ins GPR64Opnd:$rt, simm8s3:$offset, CheriOpnd:$cb),
  "cldc1\t$rd,  ${rt}, ${offset}(${cb})",
  [(set FGR64Opnd:$rd, (load (add (add (ptrtoint CheriOpnd:$cb), GPR64Opnd:$rt), (i64 simm8s3Pattern:$offset))))]>;


def CLWC1 : CheriPseudo<(outs FGR32Opnd: $rd),
  (ins GPR64Opnd:$rt, simm8s2:$offset, CheriOpnd:$cb),
  "clwc1\t$rd,  ${rt}, ${offset}(${cb})",
  [(set FGR32Opnd:$rd, (load (add (add (ptrtoint CheriOpnd:$cb), GPR64Opnd:$rt), (i64 simm8s2Pattern:$offset))))]>;

def CSDC1 : CheriPseudo<(outs),
  (ins FGR64Opnd:$rs, GPR64Opnd:$rt, simm8s3:$offset, CheriOpnd:$cb),
  "csdc1\t$rs,  ${rt}, ${offset}(${cb})",
  [(store (add (add (ptrtoint CheriOpnd:$cb), GPR64Opnd:$rt), (i64 simm8s3Pattern:$offset)), FGR64Opnd: $rs)]>;

def CSWC1 : CheriPseudo<(outs),
  (ins FGR32Opnd:$rs, GPR64Opnd:$rt, simm8s2:$offset, CheriOpnd:$cb),
  "cswc1\t$rs,  ${rt}, ${offset}(${cb})",
  [(store (add (add (ptrtoint CheriOpnd:$cb), GPR64Opnd:$rt), (i64 simm8s2Pattern:$offset)), FGR32Opnd: $rs)]>;

defm : LoadViaCap0<CLWC1, load>;
defm : LoadViaCap0<CLDC1, load>;
defm : StoreViaCap0<CSWC1, FGR32Opnd, store>;
defm : StoreViaCap0<CSDC1, FGR64Opnd, store>;

////////////////////////////////////////////////////////////////////////////////
// Accessor pseudos
////////////////////////////////////////////////////////////////////////////////
def : InstAlias<"cgetdefault $dst",
                (CIncOffset CheriOpnd:$dst, C0, ZERO_64), 1>;
def : InstAlias<"csetdefault $src",
                (CIncOffset C0, CheriOpnd:$src, ZERO_64), 1>;

def : InstAlias<"cgetkcc $dst",
                (CIncOffset CheriOpnd:$dst, C29, ZERO_64), 1>;
def : InstAlias<"csetkcc $src",
                (CIncOffset C29, CheriOpnd:$src, ZERO_64), 1>;

def : InstAlias<"cgetepcc $dst",
                (CIncOffset CheriOpnd:$dst, C31, ZERO_64), 1>;
def : InstAlias<"csetepcc $src",
                (CIncOffset C31, CheriOpnd:$src, ZERO_64), 1>;


////////////////////////////////////////////////////////////////////////////////
// Atomic operations.
////////////////////////////////////////////////////////////////////////////////

class CapAtomic2Ops<PatFrag Op, RegisterClass DRC> :
  PseudoSE<(outs DRC:$dst), (ins CheriRegs:$ptr, DRC:$incr),
           [(set DRC:$dst, (Op CheriRegs:$ptr, DRC:$incr))]>;
class CapAtomicCmpSwap<PatFrag Op, RegisterClass DRC> :
  PseudoSE<(outs DRC:$dst), (ins CheriRegs:$ptr, DRC:$cmp, DRC:$swap),
           [(set DRC:$dst, (Op iFATPTR:$ptr, DRC:$cmp, DRC:$swap))]>;


let usesCustomInserter = 1 in {
  def CAP_ATOMIC_LOAD_ADD_I8   : CapAtomic2Ops<atomic_load_add_8, GPR32>;
  def CAP_ATOMIC_LOAD_ADD_I16  : CapAtomic2Ops<atomic_load_add_16, GPR32>;
  def CAP_ATOMIC_LOAD_ADD_I32  : CapAtomic2Ops<atomic_load_add_32, GPR32>;
  def CAP_ATOMIC_LOAD_SUB_I8   : CapAtomic2Ops<atomic_load_sub_8, GPR32>;
  def CAP_ATOMIC_LOAD_SUB_I16  : CapAtomic2Ops<atomic_load_sub_16, GPR32>;
  def CAP_ATOMIC_LOAD_SUB_I32  : CapAtomic2Ops<atomic_load_sub_32, GPR32>;
  def CAP_ATOMIC_LOAD_AND_I8   : CapAtomic2Ops<atomic_load_and_8, GPR32>;
  def CAP_ATOMIC_LOAD_AND_I16  : CapAtomic2Ops<atomic_load_and_16, GPR32>;
  def CAP_ATOMIC_LOAD_AND_I32  : CapAtomic2Ops<atomic_load_and_32, GPR32>;
  def CAP_ATOMIC_LOAD_OR_I8    : CapAtomic2Ops<atomic_load_or_8, GPR32>;
  def CAP_ATOMIC_LOAD_OR_I16   : CapAtomic2Ops<atomic_load_or_16, GPR32>;
  def CAP_ATOMIC_LOAD_OR_I32   : CapAtomic2Ops<atomic_load_or_32, GPR32>;
  def CAP_ATOMIC_LOAD_XOR_I8   : CapAtomic2Ops<atomic_load_xor_8, GPR32>;
  def CAP_ATOMIC_LOAD_XOR_I16  : CapAtomic2Ops<atomic_load_xor_16, GPR32>;
  def CAP_ATOMIC_LOAD_XOR_I32  : CapAtomic2Ops<atomic_load_xor_32, GPR32>;
  def CAP_ATOMIC_LOAD_NAND_I8  : CapAtomic2Ops<atomic_load_nand_8, GPR32>;
  def CAP_ATOMIC_LOAD_NAND_I16 : CapAtomic2Ops<atomic_load_nand_16, GPR32>;
  def CAP_ATOMIC_LOAD_NAND_I32 : CapAtomic2Ops<atomic_load_nand_32, GPR32>;

  def CAP_ATOMIC_SWAP_I8       : CapAtomic2Ops<atomic_swap_8, GPR32>;
  def CAP_ATOMIC_SWAP_I16      : CapAtomic2Ops<atomic_swap_16, GPR32>;
  def CAP_ATOMIC_SWAP_I32      : CapAtomic2Ops<atomic_swap_32, GPR32>;

  def CAP_ATOMIC_CMP_SWAP_I8   : CapAtomicCmpSwap<atomic_cmp_swap_8, GPR32>;
  def CAP_ATOMIC_CMP_SWAP_I16  : CapAtomicCmpSwap<atomic_cmp_swap_16, GPR32>;
  def CAP_ATOMIC_CMP_SWAP_I32  : CapAtomicCmpSwap<atomic_cmp_swap_32, GPR32>;

  def CAP_ATOMIC_LOAD_ADD_I64  : CapAtomic2Ops<atomic_load_add_64, GPR64>;
  def CAP_ATOMIC_LOAD_SUB_I64  : CapAtomic2Ops<atomic_load_sub_64, GPR64>;
  def CAP_ATOMIC_LOAD_AND_I64  : CapAtomic2Ops<atomic_load_and_64, GPR64>;
  def CAP_ATOMIC_LOAD_OR_I64   : CapAtomic2Ops<atomic_load_or_64, GPR64>;
  def CAP_ATOMIC_LOAD_XOR_I64  : CapAtomic2Ops<atomic_load_xor_64, GPR64>;
  def CAP_ATOMIC_LOAD_NAND_I64 : CapAtomic2Ops<atomic_load_nand_64, GPR64>;
  def CAP_ATOMIC_SWAP_I64      : CapAtomic2Ops<atomic_swap_64, GPR64>;
  def CAP_ATOMIC_CMP_SWAP_I64  : CapAtomicCmpSwap<atomic_cmp_swap_64, GPR64>;

}
}
////////////////////////////////////////////////////////////////////////////////
// Helpers for capability-using calls and returns
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// ClearRegs
////////////////////////////////////////////////////////////////////////////////

class ClearRegs<bits<5> regset, string instr_asm> :
              CheriFmtClearRegs<regset, (outs),
                                (ins regslist: $mask),
                                !strconcat(instr_asm, "\t${mask}"),
                                []>;

def ClearLo : ClearRegs<0, "clearlo">;
def ClearHi : ClearRegs<1, "clearhi">;
def CClearLo : ClearRegs<2, "cclearlo">;
def CClearHi : ClearRegs<3, "cclearhi">;


def  CCallPseudo : PseudoSE<(outs), (ins GPR64Opnd:$rs, uimm32: $mask),
                            [(CheriJmpLink GPR64Opnd:$rs, (i32 uimm32_pat: $mask))], NoItinerary>
{
  let isCall=1;
  let Defs = [RA];
}


def SDT_MipsCapJmpLink      : SDTypeProfile<0, 1, [SDTCisVT<0, iFATPTR>]>;
def CapJmpLink : SDNode<"MipsISD::CapJmpLink",SDT_MipsCapJmpLink,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;
let isCall=1, hasDelaySlot=1, Defs = [C17] in {
def CapJumpLinkPseudo :
  PseudoSE<(outs), (ins CheriOpnd: $rs), [(CapJmpLink CheriOpnd: $rs)], NoItinerary>,
  PseudoInstExpansion<(CJALR C17, CheriOpnd:$rs)>;
}
def CapRet : SDNode<"MipsISD::CapRet", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def PseudoReturnCap: PseudoReturnBase<CheriOpnd>;

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
def CapRetPseudo : PseudoSE<(outs), (ins), [(CapRet)]>;

////////////////////////////////////////////////////////////////////////////////
// Atomic loads and stores
////////////////////////////////////////////////////////////////////////////////

let DecoderNamespace="CHERI" in {
def CLLC  : CheriFmtCLL<1, 3, (outs CheriOpnd:$rd), (ins CheriOpnd:$cb),
                        "cllc\t$rd, $cb">;
def CLLD  : CheriFmtCLL<0, 3, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                        "clld\t$rd, $cb">;
def CLLW  : CheriFmtCLL<1, 2, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                        "cllw\t$rd, $cb">;
def CLLWU : CheriFmtCLL<0, 2, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                        "cllwu\t$rd, $cb">;
def CLLH  : CheriFmtCLL<1, 1, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                        "cllh\t$rd, $cb">;
def CLLHU : CheriFmtCLL<0, 1, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                        "cllhu\t$rd, $cb">;
def CLLB  : CheriFmtCLL<1, 0, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                        "cllb\t$rd, $cb">;
def CLLBU : CheriFmtCLL<0, 0, (outs GPR64Opnd:$rd), (ins CheriOpnd:$cb),
                        "cllbu\t$rd, $cb">;

def CSCC  : CheriFmtCSC<7, (outs GPR64Opnd:$rd), (ins CheriOpnd: $rs, CheriOpnd:$cb),
                        "cscc\t$rd, $rs, $cb">;
def CSCD  : CheriFmtCSC<3, (outs GPR64Opnd:$rd), (ins GPR64Opnd: $rs, CheriOpnd:$cb),
                        "cscd\t$rd, $rs, $cb">;
def CSCW  : CheriFmtCSC<2, (outs GPR64Opnd:$rd), (ins GPR64Opnd: $rs, CheriOpnd:$cb),
                        "cscw\t$rd, $rs, $cb">;
def CSCH  : CheriFmtCSC<1, (outs GPR64Opnd:$rd), (ins GPR64Opnd: $rs, CheriOpnd:$cb),
                        "csch\t$rd, $rs, $cb">;
def CSCB  : CheriFmtCSC<0, (outs GPR64Opnd:$rd), (ins GPR64Opnd: $rs, CheriOpnd:$cb),
                        "cscb\t$rd, $rs, $cb">;
}
