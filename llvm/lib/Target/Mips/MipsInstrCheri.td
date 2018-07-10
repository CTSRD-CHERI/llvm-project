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


def SDT_MipsCBT      : SDTypeProfile<0, 2, [SDTCisVT<0, CapRegType>]>;
def SDT_MipsPtrToCap     : SDTypeProfile<1, 1, [SDTCisVT<0, CapRegType>,
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
  let PredicateMethod = "isScaledSImm<" # Width # ", " # Scale # ">";
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
defm simm16s4  : ShiftImmPattern<16, 4>;
defm simm11s4  : ShiftImmPattern<11, 4>;
// 8-bit immediate offset used by C[SL][DWHB]
defm simm8s3  : ShiftImmPattern<8, 3>;
defm simm8s2  : ShiftImmPattern<8, 2>;
defm simm8s1  : ShiftImmPattern<8, 1>;
defm simm8    : ShiftImmPattern<8, 0>;
// 11-bit immediate offset used by immediate versions of cincoffset / csetbounds
defm simm11s0 : ShiftImmPattern<11, 0>;

// Pattern for atomic loads on capabilities
def SDTAtomicCapLoad : SDTypeProfile<1, 1, [
  SDTCisPtrTy<0>, SDTCisPtrTy<1>
]>;
def SDTAtomicCapStore : SDTypeProfile<0, 2, [
  SDTCisPtrTy<0>, SDTCisPtrTy<1>
]>;

def atomic_load_node      : SDNode<"ISD::ATOMIC_LOAD", SDTAtomicCapLoad,
                    [SDNPHasChain, SDNPMayLoad, SDNPMemOperand]>;
def atomic_load_cap :
  PatFrag<(ops node:$ptr),
          (atomic_load_node node:$ptr), [{
  return cast<AtomicSDNode>(N)->getMemoryVT().isFatPointer();
}]>;

def atomic_store_node     : SDNode<"ISD::ATOMIC_STORE", SDTAtomicCapStore,
                    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;

def atomic_store_cap : PatFrag<(ops node:$ptr, node:$val),
                   (atomic_store_node node:$ptr, node:$val), [{
    return cast<AtomicSDNode>(N)->getMemoryVT().isFatPointer();
}]>;


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
let hasDelaySlot=1, isTerminator=1 in {
let isBranch=1, Defs = [AT] in {
def CBTS : CheriFmtCBT<0x0a, (outs), (ins CheriOpnd:$cb, brtarget:$offset),
                        "cbts\t$cb, $offset",
                        [(brcond (i32 (setne (int_cheri_cap_tag_get CheriOpnd:$cb), 0)), bb:$offset)]>;
def CBTU : CheriFmtCBT<0x09, (outs), (ins CheriOpnd:$cb, brtarget:$offset),
                        "cbtu\t$cb, $offset",
                        [(brcond (i32 (seteq (int_cheri_cap_tag_get CheriOpnd:$cb), 0)), bb:$offset)]>;
def CBEZ : CheriFmtCBT<0x11, (outs), (ins CheriOpnd:$cb, brtarget:$offset),
                        "cbez\t$cb, $offset",
                        [(brcond (i32 (seteq CheriOpnd:$cb, (inttoptr (i64 0)))), bb:$offset)]>;
def CBNZ : CheriFmtCBT<0x12, (outs), (ins CheriOpnd:$cb, brtarget:$offset),
                        "cbnz\t$cb, $offset",
                        [(brcond (i32 (setne CheriOpnd:$cb, (inttoptr (i64 0)))), bb:$offset)]>;
}
def CJR : CheriFmt1Op<0x3, (outs), (ins CheriOpnd:$r1),
                              "cjr\t${r1}",
                              []>;
}

// SelectionDAG doesn't know that int_cheri_cap_tag_get returns 0 or 1.
def : Pat<(brcond (i32 (seteq (int_cheri_cap_tag_get CheriOpnd:$cb), 1)), bb:$offset),
          (CBTS CheriOpnd: $cb, bb:$offset)>;
def : Pat<(brcond (i32 (setne (int_cheri_cap_tag_get CheriOpnd:$cb), 1)), bb:$offset),
          (CBTU CheriOpnd: $cb, bb:$offset)>;

let isCall=1, hasDelaySlot=1 in {
def CJALR : CheriFmt2Op<0xc, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2),
                              "cjalr\t${r2}, ${r1}",
                              []>;
def CCall : CheriFmt3CCall<0x05, (outs), (ins CheriOpnd:$cs, CheriOpnd:$cb, uimm11: $selector),
                              "ccall\t${cs}, ${cb}, ${selector}",
                              []>;
}

let isCall=1, hasDelaySlot=0 in {
def CJALRSlotless : CheriFmt2Op<0xc, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2),
                              "cjalr\t${r2}, ${r1}",
                              []>;
def CCallSlotless : CheriFmt3CCall<0x05, (outs), (ins CheriOpnd:$cs, CheriOpnd:$cb, uimm11: $selector),
                              "ccall\t${cs}, ${cb}, ${selector}",
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
def CEQ : CheriFmtPtrCmp<0x14, GPR64Opnd, "ceq", []>;
def CNE : CheriFmtPtrCmp<0x15, GPR64Opnd, "cne", []>;
def CLT : CheriFmtPtrCmp<0x16, GPR64Opnd, "clt", []>;
def CLE : CheriFmtPtrCmp<0x17, GPR64Opnd, "cle", []>;
def CLTU : CheriFmtPtrCmp<0x18, GPR64Opnd, "cltu", []>;
def CLEU : CheriFmtPtrCmp<0x19, GPR64Opnd, "cleu", []>;
def CEXEQ : CheriFmtPtrCmp<0x1a, GPR64Opnd, "cexeq", []>;
def CNEXEQ : CheriFmtPtrCmp<0x21, GPR64Opnd, "cnexeq", []>;
def : InstAlias<"cgt $rd, $cb, $ct",
                (CLE GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
def : InstAlias<"cge $rd, $cb, $ct",
                (CLT GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
def : InstAlias<"cgtu $rd, $cb, $ct",
                (CLEU GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
def : InstAlias<"cgeu $rd, $cb, $ct",
                (CLTU GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
let isCodeGenOnly=1 in {
  def CEQ32 : CheriFmtPtrCmp<0x14, GPR32Opnd, "ceq", []>;
  def CEXEQ32 : CheriFmtPtrCmp<0x1a, GPR32Opnd, "cexeq", []>;
  def CNEXEQ32 : CheriFmtPtrCmp<0x21, GPR32Opnd, "cnexeq", []>;
  def CNE32 : CheriFmtPtrCmp<0x15, GPR32Opnd, "cne", []>;
  def CLT32 : CheriFmtPtrCmp<0x16, GPR32Opnd, "clt", [(set GPR32Opnd: $rd, (setlt CheriOpnd: $cb, CheriOpnd:$ct))]>;
  def CLE32 : CheriFmtPtrCmp<0x17, GPR32Opnd, "cle", [(set GPR32Opnd: $rd, (setle CheriOpnd: $cb, CheriOpnd:$ct))]>;
  def CLTU32 : CheriFmtPtrCmp<0x18, GPR32Opnd, "cltu", [(set GPR32Opnd: $rd, (setult CheriOpnd: $cb, CheriOpnd:$ct))]>;
  def CLEU32 : CheriFmtPtrCmp<0x19, GPR32Opnd, "cleu", [(set GPR32Opnd: $rd, (setule CheriOpnd: $cb, CheriOpnd:$ct))]>;
}
def : Pat<(setugt CheriOpnd: $cb, CheriOpnd:$ct), (CLTU32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setuge CheriOpnd: $cb, CheriOpnd:$ct), (CLEU32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setgt CheriOpnd: $cb, CheriOpnd:$ct), (CLT32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setge CheriOpnd: $cb, CheriOpnd:$ct), (CLE32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setogt CheriOpnd: $cb, CheriOpnd:$ct), (CLT32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setoge CheriOpnd: $cb, CheriOpnd:$ct), (CLE32 CheriOpnd: $ct, CheriOpnd:$cb)>;
def : Pat<(setolt CheriOpnd: $cb, CheriOpnd:$ct), (CLT32 CheriOpnd: $cb, CheriOpnd:$ct)>;
def : Pat<(setole CheriOpnd: $cb, CheriOpnd:$ct), (CLE32 CheriOpnd: $cb, CheriOpnd:$ct)>;
def CMOVZ : CheriFmt3Op<0x1b, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, GPR64Opnd:$r3, CheriOpnd:$S),
                         "cmovz\t$r1, $r2, $r3",
                         []> {
  let Constraints = "$S = $r1";
}
def CMOVN : CheriFmt3Op<0x1c, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, GPR64Opnd:$r3, CheriOpnd:$S),
                         "cmovn\t$r1, $r2, $r3",
                         []> {
  let Constraints = "$S = $r1";
}
let isCodeGenOnly=1 in {
def CMOVZ32 : CheriFmt3Op<0x1b, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, GPR32Opnd:$r3, CheriOpnd:$S),
                           "cmovz\t$r1, $r2, $r3",
                           []> {
  let Constraints = "$S = $r1";
}
def CMOVN32 : CheriFmt3Op<0x1c, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, GPR32Opnd:$r3, CheriOpnd:$S),
                           "cmovn\t$r1, $r2, $r3",
                           []> {
  let Constraints = "$S = $r1";
}
}
defm : MovnPats<GPR64, CheriGPROrCNull, CMOVN, XOR64>;
defm : MovnPats<GPR32, CheriGPROrCNull, CMOVN32, XOR>;
defm : MovzPats1<GPR64, CheriGPROrCNull, CMOVZ, XOR64>;
defm : MovzPats2<GPR64, CheriGPROrCNull, CMOVZ, XORi64>;
defm : MovzPats1<GPR32, CheriGPROrCNull, CMOVZ32, XOR>;
defm : MovzPats2<GPR32, CheriGPROrCNull, CMOVZ32, XORi>;

class CheriPseudo<dag outs, dag ins, string asmstr,
                list<dag> pattern> :
  MipsInst<outs, ins, asmstr, pattern, NoItinerary, Pseudo>
{
  let isPseudo = 1;
  let usesCustomInserter = 1;
  let hasNoSchedulingInfo = 1;
}

def CEQPseudo : CheriPseudo<(outs GPR64Opnd: $rd), (ins CheriOpnd: $cb, CheriOpnd: $ct),
                            "#CEQPsuedo $rd, $cb, $ct",
                            [(set GPR64Opnd: $rd, (seteq CheriOpnd: $cb, CheriOpnd:$ct))]>;
def CEQPseudo32 : CheriPseudo<(outs GPR32Opnd: $rd), (ins CheriOpnd: $cb, CheriOpnd: $ct),
                            "#CEQPsuedo32 $rd, $cb, $ct",
                            [(set GPR32Opnd: $rd, (seteq CheriOpnd: $cb, CheriOpnd:$ct))]>;
def CNEPseudo : CheriPseudo<(outs GPR64Opnd: $rd), (ins CheriOpnd: $cb, CheriOpnd: $ct),
                            "#CEQPsuedo $rd, $cb, $ct",
                            [(set GPR64Opnd: $rd, (setne CheriOpnd: $cb, CheriOpnd:$ct))]>;
def CNEPseudo32 : CheriPseudo<(outs GPR32Opnd: $rd), (ins CheriOpnd: $cb, CheriOpnd: $ct),
                              "#CEQPsuedo32 $rd, $cb, $ct",
                              [(set GPR32Opnd: $rd, (setne CheriOpnd: $cb, CheriOpnd:$ct))]>;

////////////////////////////////////////////////////////////////////////////////
// Capability manipulation
////////////////////////////////////////////////////////////////////////////////

// Property accessors
def CGetPerms  : CheriFmtCGet<0, "perm", "perms">;
def CGetType   : CheriFmtCGet<1, "type">;
def CGetBase   : CheriFmtCGet<2, "base">;
def CGetLength : CheriFmtCGet<3, "len", "length">;
def CGetTag    : CheriFmtCGet<4, "tag">;
def CGetSealed : CheriFmtCGet<5, "sealed">;
def CGetOffset : CheriFmtCGet<6, "offset">;
def CGetAddr   : CheriFmtCGet<0xf, "addr", "address">;
// Versions of property setters that take immediate operands
def CSetBoundsImm : CheriFmtCSetImm<0x14, simm11s0, simm11s0Pattern, "bounds">;
def CIncOffsetImm : CheriFmtCSetImm<0x13, simm11s0, simm11s0Pattern, "offset", "offset", "inc", "increment">;

// Accept CIncOffsetImm as an assembler mnemonic because ancient binutils is too stupid to have an opcode
// with differing encoding depending on the parameters
def : InstAlias<"csetboundsimm $dst, $src, $imm",
                (CSetBoundsImm CheriOpnd:$dst, CheriOpnd:$src, simm11s0:$imm), 0>;
def : InstAlias<"cincoffsetimm $dst, $src, $imm",
                (CIncOffsetImm CheriOpnd:$dst, CheriOpnd:$src, simm11s0:$imm), 0>;

// Property setters
def CAndPerms  : CheriFmtCSet<0xd, "perm", "perms", "and">;
def CSetOffset : CheriFmtCSet<0xf, "offset">;
def CSetBounds : CheriFmtCSet<0x8, "bounds">;
// TODO: do we want an intrinsic for this as well?
def CSetBoundsExact : CheriFmt3Op<0x9, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, GPR64Opnd:$r3),
                              !strconcat("csetboundsexact", "\t$r1, $r2, $r3"),
                              []>;
def CIncOffset : CheriFmtCSet<0x11, "offset", "offset", "inc", "increment">;
def CClearTag  : CheriFmt2Op<0xb, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2), 
                              "ccleartag\t$r1, $r2",
                              [(set CheriOpnd:$r1, (int_cheri_cap_tag_clear CheriOpnd:$r2))]>;


let isMoveReg = 1 in {
def CReadHwr : CheriFmt2Op<0xd, (outs CheriOpnd:$r1), (ins CheriHWRegsOpnd:$r2),
                              "creadhwr\t$r1, $r2",
                              []>;
def CWriteHwr : CheriFmt2Op<0xe, (outs CheriHWRegsOpnd:$r2), (ins CheriOpnd:$r1),
                              "cwritehwr\t$r1, $r2",
                              []> {
  // CWriteHwr can change $ddc so we should probably not be reordering it
  let hasSideEffects = 1;
}
} // isMoveReg = 1

// PCC access
def CGetPCC : CheriFmt1Op<0x0, (outs CheriOpnd:$r1), (ins),
                          "cgetpcc\t${r1}",
                          [(set CheriOpnd:$r1, (int_cheri_pcc_get))]>;

def CSetPCCOffset : CheriFmt2Op<7, (outs CheriOpnd:$r1), (ins GPR64Opnd:$r2),
                                "cgetpccsetoffset\t${r1}, $r2",
                                [(set CheriOpnd:$r1, (int_cheri_cap_offset_set (CapRegType (int_cheri_pcc_get)), GPR64Opnd: $r2))]>;

// Sealing and unsealing
def CSeal   : CheriFmt3Op<0xb, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, CheriOpnd:$r3), 
                               "cseal\t$r1, $r2, $r3",
                               [(set CheriOpnd:$r1, (int_cheri_cap_seal CheriOpnd:$r2, CheriOpnd:$r3))]>;
def CUnSeal : CheriFmt3Op<0xc, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, CheriOpnd:$r3), 
                               "cunseal\t$r1, $r2, $r3",
                               [(set CheriOpnd:$r1, (int_cheri_cap_unseal CheriOpnd:$r2, CheriOpnd:$r3))]>;

// Pointer arithmetic

def CFromPtr : CheriFmt3Op<0x13, (outs CheriOpnd:$r1), (ins CheriOpnd0IsDDC:$r2, GPR64Opnd:$r3),
  "cfromptr\t$r1, $r2, $r3", [(set CheriOpnd: $r1, (int_cheri_cap_from_pointer CheriOpnd0IsDDC: $r2, GPR64Opnd: $r3))]>;
def CToPtr : CheriFmt3Op<0x12, (outs GPR64Opnd:$r1), (ins CheriOpnd0IsDDC:$r3, CheriOpnd:$r2),
  "ctoptr\t$r1, $r2, $r3", [(set GPR64Opnd: $r1, (int_cheri_cap_to_pointer CheriOpnd0IsDDC: $r3, CheriOpnd: $r2))]>;
let isCodeGenOnly = 1 in {
def CFromPtr32 : CheriFmt3Op<0x13, (outs CheriOpnd:$r1), (ins CheriOpnd0IsDDC:$r2, GPR32Opnd:$r3),
  "cfromptr\t$r1, $r2, $r3", []>;
def CToPtr32 : CheriFmt3Op<0x12, (outs GPR32Opnd:$r1), (ins CheriOpnd0IsDDC:$r3, CheriOpnd:$r2),
  "ctoptr\t$r1, $r2, $r3", []>;
}
def CSub : CheriFmt3Op<0xa, (outs GPR64Opnd:$r1), (ins CheriOpnd:$r2, CheriOpnd:$r3),
  "csub\t$r1, $r2, $r3", [(set GPR64Opnd:$r1, (int_cheri_cap_diff CheriOpnd:$r2, CheriOpnd:$r3))]>;

def CMove : CheriFmt2Op<0xa, (outs CheriOpnd: $r1), (ins CheriOpnd:$r2), "cmove\t$r1,  $r2", []> {
  let isMoveReg = 1;
}



// Cause register (privileged mode only)
let hasSideEffects = 1 in {
def CGetCause : CheriFmt1Op<1, (outs GPR64Opnd:$r1), (ins),
                          "cgetcause\t${r1}",
                          [(set GPR64Opnd:$r1, (int_mips_cap_cause_get))]>;
def CSetCause : CheriFmt1Op<2, (outs), (ins GPR64Opnd: $r1),
                          "csetcause\t${r1}",
                          [(int_mips_cap_cause_set GPR64Opnd: $r1)]>;

// Assertion instructions
def CCheckPerm : CheriFmt2Op<0x8, (outs), (ins CheriOpnd:$r1, GPR64Opnd: $r2),
                              "ccheckperm\t${r1}, ${r2}",
                              [(int_cheri_cap_perms_check CheriOpnd:$r1,  GPR64Opnd: $r2)]>;
def CCheckType : CheriFmt2Op<0x9, (outs), (ins CheriOpnd:$r1, CheriOpnd:$r2),
                              "cchecktype\t${r1}, ${r2}",
                              [(int_cheri_cap_type_check CheriOpnd:$r1,  CheriOpnd:$r2)]>;
}

def CBuildCap : CheriFmt3Op<0x1d, (outs CheriOpnd:$r1), (ins CheriOpnd0IsDDC:$r2, CheriOpnd:$r3),
  "cbuildcap\t$r1, $r2, $r3", [(set CheriOpnd: $r1, (int_cheri_cap_build CheriOpnd0IsDDC: $r2, CheriOpnd: $r3))]>;
def CCopyType : CheriFmt3Op<0x1e, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, CheriOpnd:$r3),
  "ccopytype\t$r1, $r2, $r3", [(set CheriOpnd: $r1, (int_cheri_cap_type_copy CheriOpnd: $r2, CheriOpnd: $r3))]>;
def CCSeal : CheriFmt3Op<0x1f, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, CheriOpnd:$r3),
  "ccseal\t$r1, $r2, $r3", [(set CheriOpnd: $r1, (int_cheri_cap_conditional_seal CheriOpnd: $r2, CheriOpnd: $r3))]>;
def CTestSubset : CheriFmt3Op<0x20, (outs GPR64Opnd:$r1), (ins CheriOpnd0IsDDC:$r2, CheriOpnd:$r3),
  "ctestsubset\t$r1, $r2, $r3", [(set GPR64Opnd: $r1, (int_cheri_cap_subset_test CheriOpnd0IsDDC: $r2, CheriOpnd: $r3))]>;

// Cursors
def : Pat<(ptradd CheriOpnd:$r2, simm11s0Pattern:$imm),
          (CIncOffsetImm $r2, simm11s0Pattern: $imm)>;
def : Pat<(ptradd CheriOpnd:$r2, GPR64Opnd:$r3),
          (CIncOffset $r2, $r3)>;
// Int to pointer in address space 0
def : Pat<(inttoptr GPR64Opnd:$rt),
          (CFromPtr DDC, $rt)>;
def : Pat<(inttoptr GPR32:$rt),
          (CFromPtr32 DDC, $rt)>;
def : Pat<(ptrtoint CheriOpnd:$rt),
          (CToPtr DDC, $rt)>;
def : Pat<(ptrtoint CheriOpnd:$rt),
          (CToPtr32 DDC, $rt)>;


let Predicates = [IsCHERI], DecoderNamespace="CHERI" in {
def STORECAP : CheriFmtCLS<0x3e, (outs), (ins CheriOpnd:$cs, GPR64Opnd:$rt, simm11s4:$offset, CheriOpnd0IsDDC:$cb),
                         "csc\t$cs, ${rt}, ${offset}(${cb})",
                         [(store CheriOpnd:$cs, (add (add GPR64Opnd:$rt, (ptrtoint CheriOpnd0IsDDC:$cb)), (i64 simm11s4Pattern:$offset)))]>;
// new experimental 16bit immediate CLC (not sure if needed but added for symmetry with clc)
// Reuses the MDMX/MSA opcodes
def STORECAP_BigImm : CheriFmtCLS_BigImm<0x1E, (outs), (ins CheriOpnd:$cs, simm16s4:$offset, CheriOpnd0IsDDC:$cb),
                         "cscbi\t$cs, ${offset}(${cb})",
                         [(store CheriOpnd:$cs, (add (ptrtoint CheriOpnd0IsDDC:$cb), (i64 simm16s4Pattern:$offset)))]
                         >;

}
def : Pat<(store CheriOpnd:$cs, CheriOpnd:$cb),
          (STORECAP CheriOpnd:$cs, ZERO_64, (i64 0), $cb)>;
def : Pat<(store CheriOpnd:$cs, GPR64Opnd:$rt),
          (STORECAP CheriOpnd:$cs, GPR64Opnd:$rt, (i64 0), DDC)>;

// Load a capability
let Predicates = [IsCHERI] in {
def LOADCAP : CheriFmtCLS<0x36, (outs CheriOpnd:$cs), (ins GPR64Opnd:$rt, simm11s4:$offset, CheriOpnd0IsDDC:$cb),
                        "clc\t$cs, ${rt}, ${offset}(${cb})",
                        [(set CheriOpnd:$cs, (load (add (add GPR64Opnd:$rt, (ptrtoint CheriOpnd0IsDDC:$cb)), (i64 simm11s4Pattern:$offset))))]> {
  let DecoderNamespace="CHERI";
}
// Experimental new CLC instruction using a 16 bit immediate and for now the JALX toplevel opcode
def LOADCAP_BigImm : CheriFmtCLS_BigImm<0x1D, (outs CheriOpnd:$cs), (ins simm16s4:$offset, CheriOpnd0IsDDC:$cb),
                        "clcbi\t$cs, ${offset}(${cb})", // different mnemonic for now to simplify testing
                        [(set CheriOpnd:$cs, (load (add (ptrtoint CheriOpnd0IsDDC:$cb), (i64 simm16s4Pattern:$offset))))]
                        > {
  let DecoderNamespace="CHERI";
}

// Load via a capability register
let canFoldAsLoad = 1, DecoderNamespace="CHERI" in
multiclass LoadViaCapScaled<bits<2> t, bit sExt, string instr_asm, RegisterOperand RC, PatFrag loadType, Operand simm, PatFrag immfrag> {
  def #NAME# : CheriFmtCLX<t, sExt,
                             (outs RC:$rd),
                             (ins GPR64Opnd:$rt, simm:$offset, CheriOpnd0IsDDC:$cb),
                             !strconcat(instr_asm, "\t$rd, ${rt}, ${offset}(${cb})"),
                           [(set RC:$rd, (loadType (CapRegType (ptradd CheriOpnd0IsDDC:$cb, (add GPR64Opnd: $rt, (i64 immfrag:$offset))))))]>;
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
                             (ins RC:$rs, GPR64Opnd:$rt, simm:$offset, CheriOpnd0IsDDC:$cb),
                             !strconcat(instr_asm, "\t$rs, ${rt}, ${offset}(${cb})"),
                           [(storeType (CapRegType (ptradd CheriOpnd0IsDDC:$cb, (add GPR64Opnd: $rt, (i64 immfrag:$offset)))), RC:$rs)]>;
}
multiclass StoreViaCap<bits<2> t, bit e, string instr_asm, RegisterOperand RC, PatFrag storeType> :
  StoreViaCapScaled<t, e, instr_asm, RC, storeType, simm8, immSExt8>;
 
multiclass StoreViaCap0<Instruction Op, RegisterOperand RC, PatFrag storeType> {
  def : Pat<(storeType RC:$rd, CheriOpnd0IsDDC:$cb),
            (Op RC:$rd, ZERO_64, (i64 0), CheriOpnd0IsDDC:$cb)>;
  def : Pat<(storeType RC:$rd, (add (ptrtoint CheriOpnd0IsDDC:$cb), GPR64Opnd:$rt)),
            (Op RC:$rd, GPR64Opnd:$rt, (i64 0), CheriOpnd0IsDDC:$cb)>;
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
  defm CAPSTORE832      : StoreViaCap<0, 0, "csb", GPR32Opnd, truncstorei8>;
  defm CAPSTORE1632     : StoreViaCapScaled<1, 0, "csh", GPR32Opnd, truncstorei16, simm8s1, simm8s1Pattern>;
  defm CAPSTORE3264     : StoreViaCapScaled<2, 0, "csw", GPR64Opnd, truncstorei32, simm8s2, simm8s2Pattern>;
}
defm CAPSTORE64     : StoreViaCapScaled<3, 0, "csd", GPR64Opnd, store, simm8s3, simm8s3Pattern>;

// TODO: replace these by CReadHwr
def : Pat<(int_cheri_ddc_get), (CReadHwr CAPHWR0)>;
def : Pat<(int_mips_idc_get),  (CIncOffset C26, ZERO_64)>;
def : Pat<(int_mips_kr1c_get), (CReadHwr CAPHWR22)>;
def : Pat<(int_mips_kr2c_get), (CReadHwr CAPHWR23)>;
def : Pat<(int_mips_kcc_get),  (CReadHwr CAPHWR29)>;
def : Pat<(int_mips_kdc_get),  (CReadHwr CAPHWR30)>;
def : Pat<(int_mips_epcc_get), (CReadHwr CAPHWR31)>;
let Predicates=[IsCHERIABI] in {
  // C11 is only defined in the purecap ABI
  def : Pat<(int_cheri_stack_cap_get), (CIncOffset C11, ZERO_64)>;
}


defm CAP0STORE8     : StoreViaCap0<CAPSTORE8, GPR64Opnd, truncstorei8>;
defm CAP0STORE16    : StoreViaCap0<CAPSTORE16, GPR64Opnd, truncstorei16>;
defm CAP0STORE32    : StoreViaCap0<CAPSTORE32, GPR32Opnd, store>;
defm CAP0STORE3264  : StoreViaCap0<CAPSTORE3264, GPR64Opnd, truncstorei32>;
defm CAP0STORE64    : StoreViaCap0<CAPSTORE64, GPR64Opnd, store>;

def CLDC1 : CheriPseudo<(outs FGR64Opnd: $rd),
  (ins GPR64Opnd:$rt, simm8s3:$offset, CheriOpnd0IsDDC:$cb),
  "cldc1\t$rd,  ${rt}, ${offset}(${cb})",
  [(set FGR64Opnd:$rd, (load (add (add (ptrtoint CheriOpnd0IsDDC:$cb), GPR64Opnd:$rt), (i64 simm8s3Pattern:$offset))))]>;


def CLWC1 : CheriPseudo<(outs FGR32Opnd: $rd),
  (ins GPR64Opnd:$rt, simm8s2:$offset, CheriOpnd0IsDDC:$cb),
  "clwc1\t$rd,  ${rt}, ${offset}(${cb})",
  [(set FGR32Opnd:$rd, (load (add (add (ptrtoint CheriOpnd0IsDDC:$cb), GPR64Opnd:$rt), (i64 simm8s2Pattern:$offset))))]>;

def CSDC1 : CheriPseudo<(outs),
  (ins FGR64Opnd:$rs, GPR64Opnd:$rt, simm8s3:$offset, CheriOpnd0IsDDC:$cb),
  "csdc1\t$rs,  ${rt}, ${offset}(${cb})",
  [(store (add (add (ptrtoint CheriOpnd0IsDDC:$cb), GPR64Opnd:$rt), (i64 simm8s3Pattern:$offset)), FGR64Opnd: $rs)]>;

def CSWC1 : CheriPseudo<(outs),
  (ins FGR32Opnd:$rs, GPR64Opnd:$rt, simm8s2:$offset, CheriOpnd0IsDDC:$cb),
  "cswc1\t$rs,  ${rt}, ${offset}(${cb})",
  [(store (add (add (ptrtoint CheriOpnd0IsDDC:$cb), GPR64Opnd:$rt), (i64 simm8s2Pattern:$offset)), FGR32Opnd: $rs)]>;

defm : LoadViaCap0<CLWC1, load>;
defm : LoadViaCap0<CLDC1, load>;
defm : StoreViaCap0<CSWC1, FGR32Opnd, store>;
defm : StoreViaCap0<CSDC1, FGR64Opnd, store>;

////////////////////////////////////////////////////////////////////////////////
// Accessor pseudos
////////////////////////////////////////////////////////////////////////////////

// Set Emit = 0 here to show that creadhwr is used in the disasm
// TODO: change this to 1?
def : InstAlias<"cgetdefault $dst", (CReadHwr CheriOpnd:$dst, CAPHWR0), 0>;
def : InstAlias<"csetdefault $src", (CWriteHwr CAPHWR0, CheriOpnd:$src), 0>;
def : InstAlias<"cgetkcc $dst",     (CReadHwr CheriOpnd:$dst, CAPHWR29), 0>;
def : InstAlias<"csetkcc $src",     (CWriteHwr CAPHWR29, CheriOpnd:$src), 0>;
def : InstAlias<"cgetkdc $dst",     (CReadHwr CheriOpnd:$dst, CAPHWR30), 0>;
def : InstAlias<"csetkdc $src",     (CWriteHwr CAPHWR30, CheriOpnd:$src), 0>;
def : InstAlias<"cgetepcc $dst",    (CReadHwr CheriOpnd:$dst, CAPHWR31), 0>;
def : InstAlias<"csetepcc $src",    (CWriteHwr CAPHWR31, CheriOpnd:$src), 0>;
// Access to the two scratch hardware registers:
def : InstAlias<"cgetkr1c $dst",    (CReadHwr CheriOpnd:$dst, CAPHWR22), 0>;
def : InstAlias<"csetkr1c $src",    (CWriteHwr CAPHWR22, CheriOpnd:$src), 0>;
def : InstAlias<"cgetkr2c $dst",    (CReadHwr CheriOpnd:$dst, CAPHWR23), 0>;
def : InstAlias<"csetkr2c $src",    (CWriteHwr CAPHWR23, CheriOpnd:$src), 0>;


// Must come before cfromddc to have a higher priority when disassembling
// TODO: once $c0 is a null register change this
def : InstAlias<"cgetnull $dst", (CFromPtr CheriOpnd:$dst, DDC, ZERO_64), 1>;
// TODO: enable these once we have a null register:
// def : InstAlias<"cgetnull $dst", (CMove CheriOpnd:$dst, CNULL), 1>;
// def : InstAlias<"cfromint $dst, $src", (CIncOffset CheriOpnd:$dst, CNULL, GPR64Opnd:$src), 1>;

// So that we can use ptr -> int without a warning that $c0 access is deprecated in .s files
def : InstAlias<"cfromddc $dst, $src", (CFromPtr CheriOpnd:$dst, DDC, GPR64Opnd:$src), 1>;

////////////////////////////////////////////////////////////////////////////////
// Atomic operations.
////////////////////////////////////////////////////////////////////////////////

class CapAtomic2Ops<PatFrag Op, RegisterClass DRC> :
  PseudoSE<(outs DRC:$dst), (ins CheriOpnd0IsDDC:$ptr, DRC:$incr),
           [(set DRC:$dst, (Op CheriOpnd0IsDDC:$ptr, DRC:$incr))]>;
class CapAtomicCmpSwap<PatFrag Op, RegisterClass DRC> :
  PseudoSE<(outs DRC:$dst), (ins CheriOpnd0IsDDC:$ptr, DRC:$cmp, DRC:$swap),
           [(set DRC:$dst, (Op CheriOpnd0IsDDC:$ptr, DRC:$cmp, DRC:$swap))]>;


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

// Atomic load patterns.  These (and the store patterns) expand to simple loads
// because an IR pass adds the required barriers.
def : MipsPat<(atomic_load_8 CheriOpnd0IsDDC:$a), (CAPLOAD832 ZERO_64, (i64 0), CheriOpnd0IsDDC:$a)>;
def : MipsPat<(atomic_load_16 CheriOpnd0IsDDC:$a), (CAPLOAD1632 ZERO_64, (i64 0), CheriOpnd0IsDDC:$a)>;
def : MipsPat<(atomic_load_32 CheriOpnd0IsDDC:$a), (CAPLOAD32 ZERO_64, (i64 0), CheriOpnd0IsDDC:$a)>;
def : MipsPat<(atomic_load_64 CheriOpnd0IsDDC:$a), (CAPLOAD64 ZERO_64, (i64 0), CheriOpnd0IsDDC:$a)>;
def : MipsPat<(atomic_load_cap CheriOpnd0IsDDC:$a), (LOADCAP ZERO_64, (i64 0), CheriOpnd0IsDDC:$a)>;

// Atomic store patterns.
def : MipsPat<(atomic_store_8 CheriOpnd0IsDDC:$a, GPR64:$v),
              (CAPSTORE8 $v, ZERO_64, (i64 0), $a)>;
def : MipsPat<(atomic_store_8 CheriOpnd0IsDDC:$a, GPR32:$v),
              (CAPSTORE832 $v, ZERO_64, (i64 0), $a)>;
def : MipsPat<(atomic_store_16 CheriOpnd0IsDDC:$a, GPR64:$v),
              (CAPSTORE16 $v, ZERO_64, (i64 0), $a)>;
def : MipsPat<(atomic_store_16 CheriOpnd0IsDDC:$a, GPR32:$v),
              (CAPSTORE1632 $v, ZERO_64, (i64 0), $a)>;
def : MipsPat<(atomic_store_32 CheriOpnd0IsDDC:$a, GPR64:$v),
              (CAPSTORE3264 $v, ZERO_64, (i64 0), $a)>;
def : MipsPat<(atomic_store_32 CheriOpnd0IsDDC:$a, GPR32:$v),
              (CAPSTORE32 $v, ZERO_64, (i64 0), $a)>;
def : MipsPat<(atomic_store_64 CheriOpnd0IsDDC:$a, GPR64:$v),
              (CAPSTORE64 $v, ZERO_64, (i64 0), $a)>;
def : MipsPat<(atomic_store_cap CheriOpnd0IsDDC:$a, CheriOpnd0IsDDC:$v),
              (STORECAP $v, ZERO_64, (i64 0), $a)>;


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
  let hasNoSchedulingInfo = 1;
}


def SDT_MipsCapJmpLink      : SDTypeProfile<0, 1, [SDTCisVT<0, CapRegType>]>;
def CapJmpLink : SDNode<"MipsISD::CapJmpLink",SDT_MipsCapJmpLink,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;
let isCall=1, hasDelaySlot=1, Defs = [C17], hasNoSchedulingInfo = 1 in {
def CapJumpLinkPseudo :
  PseudoSE<(outs), (ins CheriOpnd: $rs), [(CapJmpLink CheriOpnd: $rs)], NoItinerary>,
  PseudoInstExpansion<(CJALR C17, CheriOpnd:$rs)>;
}
def CapRet : SDNode<"MipsISD::CapRet", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// def PseudoReturnCap: PseudoReturnBase<CheriOpnd>;

def PseudoReturnCap : MipsPseudo<(outs), (ins CheriOpnd:$cs, CheriOpnd:$cb, uimm11: $selector),
                      [], II_ReturnPseudo> {
  let isTerminator = 1;
  let isBarrier = 1;
  let hasDelaySlot = 1;
  let isReturn = 1;
  let isCodeGenOnly = 1;
  let hasCtrlDep = 1;
  let hasExtraSrcRegAllocReq = 1;
  bit isCTI = 1;
}

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
def CapRetPseudo : PseudoSE<(outs), (ins), [(CapRet)]>;

////////////////////////////////////////////////////////////////////////////////
// Atomic loads and stores
////////////////////////////////////////////////////////////////////////////////

let DecoderNamespace="CHERI" in {
def CLLC  : CheriFmtCLL<1, 3, (outs CheriOpnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllc\t$rd, $cb">;
def CLLD  : CheriFmtCLL<0, 3, (outs GPR64Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "clld\t$rd, $cb">;
def CLLW  : CheriFmtCLL<1, 2, (outs GPR64Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllw\t$rd, $cb">;
def CLLWU : CheriFmtCLL<0, 2, (outs GPR64Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllwu\t$rd, $cb">;
def CLLH  : CheriFmtCLL<1, 1, (outs GPR64Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllh\t$rd, $cb">;
def CLLHU : CheriFmtCLL<0, 1, (outs GPR64Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllhu\t$rd, $cb">;
def CLLB  : CheriFmtCLL<1, 0, (outs GPR64Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllb\t$rd, $cb">;
def CLLBU : CheriFmtCLL<0, 0, (outs GPR64Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllbu\t$rd, $cb">;

def CSCC  : CheriFmtCSC<7, (outs GPR64Opnd:$rd), (ins CheriOpnd: $rs, CheriOpnd0IsDDC:$cb),
                        "cscc\t$rd, $rs, $cb">;
def CSCD  : CheriFmtCSC<3, (outs GPR64Opnd:$rd), (ins GPR64Opnd: $rs, CheriOpnd0IsDDC:$cb),
                        "cscd\t$rd, $rs, $cb">;
def CSCW  : CheriFmtCSC<2, (outs GPR64Opnd:$rd), (ins GPR64Opnd: $rs, CheriOpnd0IsDDC:$cb),
                        "cscw\t$rd, $rs, $cb">;
def CSCH  : CheriFmtCSC<1, (outs GPR64Opnd:$rd), (ins GPR64Opnd: $rs, CheriOpnd0IsDDC:$cb),
                        "csch\t$rd, $rs, $cb">;
def CSCB  : CheriFmtCSC<0, (outs GPR64Opnd:$rd), (ins GPR64Opnd: $rs, CheriOpnd0IsDDC:$cb),
                        "cscb\t$rd, $rs, $cb">;
}

def AddrFI64  : ComplexPattern<iFATPTR64, 1, "selectAddrFI", [frameindex], []>;
def AddrFI128 : ComplexPattern<iFATPTR128, 1, "selectAddrFI", [frameindex], []>;
def AddrFI256 : ComplexPattern<iFATPTR256, 1, "selectAddrFI", [frameindex], []>;

def StackToCapPseudo : CheriPseudo<(outs CheriOpnd: $cd), (ins CheriOpnd: $fi),
                            "#StackToCapPseudo $cd, $fi",
                            []>;

def : Pat<(int_mips_stack_to_cap CheriOpnd:$addr), (CIncOffset $addr, ZERO_64)>;

// These patterns need duplicating for each of the capability sizes.  It's not
// clear why: we should be able to pattern match a ComplexPattern with an
// overloaded return type, but that appears to be broken.
def : Pat<(int_mips_stack_to_cap AddrFI64:$addr), (CIncOffset $addr, ZERO_64)>;
def : Pat<(load (CapRegType (AddrFI64:$addr))),
              (CAPLOAD64 ZERO_64, (i64 0), $addr)>;
def : Pat<(load (CapRegType (AddrFI64:$addr))),
              (LOADCAP ZERO_64, (i64 0), $addr)>;
def : Pat<(int_mips_stack_to_cap AddrFI128:$addr), (CIncOffset $addr, ZERO_64)>;
def : Pat<(load (CapRegType (AddrFI128:$addr))),
              (CAPLOAD64 ZERO_64, (i64 0), $addr)>;
def : Pat<(load (CapRegType (AddrFI128:$addr))),
              (LOADCAP ZERO_64, (i64 0), $addr)>;
def : Pat<(int_mips_stack_to_cap AddrFI256:$addr), (CIncOffset $addr, ZERO_64)>;
def : Pat<(load (CapRegType (AddrFI256:$addr))),
              (CAPLOAD64 ZERO_64, (i64 0), $addr)>;
def : Pat<(load (CapRegType (AddrFI256:$addr))),
              (LOADCAP ZERO_64, (i64 0), $addr)>;

let Defs = [C11], Uses = [C11], hasSideEffects = 1, Predicates=[IsCHERIABI] in {
def ADJCALLSTACKCAPDOWN : MipsPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                  [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKCAPUP   : MipsPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                  [(callseq_end timm:$amt1, timm:$amt2)]>;
}

def PseudoIndirectBranchCap : PseudoIndirectBranchBase<CJR, CheriOpnd>;



// This node is a hack to allow `clc	$c1, $zero, %captab(global)($c1)`
def CapGlobalAddrPseudo : CheriPseudo<(outs CheriOpnd: $cd), (ins CheriOpnd:$cgp, GPR64Opnd:$offs),
                                      "#CapGlobalAddrPseudo $cd, $cgp, $offs",
                                      []>;
def SDTCheriGlobal  : SDTypeProfile<1, 2, [SDTCisVT<0, CapRegType>,
                                           SDTCisVT<1, CapRegType>,
                                           SDTCisVT<2, i64>]>;
def WrapperCapOp : SDNode<"MipsISD::WrapperCapOp", SDTCheriGlobal>;
class CapTableWrapperPat<SDNode node>:
      Pat<(load (WrapperCapOp CheriOpnd:$cgp, node:$in)),
                (LOADCAP_BigImm node:$in, CheriOpnd:$cgp)>;

// def : Pat<(WrapperCapOp CheriOpnd:$cgp, GPR64Opnd:$in),
//           (CapGlobalAddrPseudo CheriOpnd:$cgp, GPR64Opnd:$in)>;

// Can't use ptradd/shl in output patters
// def : Pat<(WrapperCapOp CheriOpnd:$cgp, GPR64Opnd:$in),
//          (ptradd $cgp, (shl imm:$in, 4))>;


def shift_imm_by_4 : OutPatFrag<(ops node:$in), (DSLL (DADDiu ZERO_64, $in), 4)>;

//def : Pat<(select GPR64Opnd:$cond,
//                  (WrapperCapOp CheriOpnd:$cgp, imm:$in),
//                  (WrapperCapOp CheriOpnd:$cgp2, imm:$in2)),
//          // We can assume that $cgp is the same register here (but tablegen won't let me ignore it)
//          // TODO: How to add an assertion for this?
//         (CIncOffset (CMOVN $cgp, $cgp2, $cond),
//          (MOVN_I64_I64 (shift_imm_by_4 imm:$in), GPR64Opnd:$cond,
//                                          (shift_imm_by_4 imm:$in2)))>;



// This one seems to work but is really inefficient because stuff can't be folded
def : Pat<(WrapperCapOp CheriOpnd:$cgp, GPR64Opnd:$in),
          (CIncOffset $cgp, (shift_imm_by_4 imm:$in))>;

def : CapTableWrapperPat<tglobaladdr>;
def : CapTableWrapperPat<tjumptable>;
// TODO: add test cases for these:
def : CapTableWrapperPat<tconstpool>;
def : CapTableWrapperPat<texternalsym>;
def : CapTableWrapperPat<tblockaddress>;
def : CapTableWrapperPat<tglobaltlsaddr>;

def : Pat<(load GPR64Opnd:$rt),
          (LOADCAP GPR64Opnd:$rt, (i64 0), DDC)>;
def : Pat<(load CheriOpnd:$cb),
          (LOADCAP ZERO_64, (i64 0), CheriOpnd:$cb)>;
