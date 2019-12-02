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

def SDT_MipsCapThreadPointer : SDTypeProfile<1, 0, [SDTCisVT<0, CapRegType>]>;
def MipsCapThreadPointer : SDNode<"MipsISD::CapThreadPointer",
                                  SDT_MipsCapThreadPointer>;

def SDT_CheriBoolUnary : SDTypeProfile<1, 1, [
  SDTCisInt<0>, SDTCisVT<1, CapRegType>
]>;

def CapTagGet    : SDNode<"MipsISD::CapTagGet",    SDT_CheriBoolUnary>;
def CapSealedGet : SDNode<"MipsISD::CapSealedGet", SDT_CheriBoolUnary>;

def SDT_CheriBoolBinary : SDTypeProfile<1, 2, [
  SDTCisInt<0>, SDTCisVT<1, CapRegType>, SDTCisVT<2, CapRegType>
]>;

def CapSubsetTest : SDNode<"MipsISD::CapSubsetTest", SDT_CheriBoolBinary>;

def SDT_CheriCapModify : SDTypeProfile<1, 2, [
  SDTCisVT<0, CapRegType>, SDTCisVT<1, CapRegType>, SDTCisInt<2>
]>;

// Scaled immediate offsets

class SImmScaled<int Width, int Scale> : AsmOperandClass {
  let Name = "SImm" # Width # "s" # Scale;
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledSImm<" # Width # ", " # Scale # ">";
  let DiagnosticType = "SImm" # Width # "_Lsl" # Scale;
}

multiclass ShiftSImmPattern<int Width, int Shift> {
  def NAME # Pattern : ImmLeaf<i64,
    "return isShiftedInt<" # Width # ", " # Shift # ">(Imm);"
    >;
  def NAME : Operand<i64> {
    let ParserMatchClass = SImmScaled<Width, Shift>;
    let EncoderMethod = "getShiftedImmediate<" # Width # ", " # Shift # ">";
    let DecoderMethod = "DecodeSImmWithOffsetAndScale<" # Width # ", 0, (1 <<" # Shift # ")>";
    let OperandType = "OPERAND_IMMEDIATE";
  }
}

class UImmScaled<int Width, int Scale> : AsmOperandClass {
  let Name = "UImm" # Width # "s" # Scale;
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isScaledUImm<" # Width # ", " # Scale # ">";
  let DiagnosticType = "UImm" # Width # "_Lsl" # Scale;
}

multiclass ShiftUImmPattern<int Width, int Shift> {
  def NAME # Pattern : ImmLeaf<i64,
    "return isShiftedUInt<" # Width # ", " # Shift # ">(Imm);"
    >;
  def NAME : Operand<i64> {
    let ParserMatchClass = UImmScaled<Width, Shift>;
    let EncoderMethod = "getShiftedUnsignedImmediate<" # Width # ", " # Shift # ">";
    let DecoderMethod = "DecodeUImmWithOffsetAndScale<" # Width # ", 0, (1 <<" # Shift # ")>";
    let OperandType = "OPERAND_IMMEDIATE";
  }
}


// 11-bit immediate offset used by C[SL]CR
defm simm16s4  : ShiftSImmPattern<16, 4>;
defm simm11s4  : ShiftSImmPattern<11, 4>;
// 8-bit immediate offset used by C[SL][DWHB]
defm simm8s3  : ShiftSImmPattern<8, 3>;
defm simm8s2  : ShiftSImmPattern<8, 2>;
defm simm8s1  : ShiftSImmPattern<8, 1>;
defm simm8    : ShiftSImmPattern<8, 0>;
// 11-bit immediate offset used by immediate versions of cincoffset / csetbounds
defm simm11s0 : ShiftSImmPattern<11, 0>;
defm uimm11s0 : ShiftUImmPattern<11, 0>;

// Pattern for atomic loads on capabilities
// TODO: these should end up in TargetSelectionDAG.td
def SDTAtomicCapLoad : SDTypeProfile<1, 1, [
  SDTCisPtrTy<0>, SDTCisPtrTy<1>
]>;
def SDTAtomicCapStore : SDTypeProfile<0, 2, [
  SDTCisPtrTy<0>, SDTCisPtrTy<1>
]>;

def atomic_load_cap_node      : SDNode<"ISD::ATOMIC_LOAD", SDTAtomicCapLoad,
                    [SDNPHasChain, SDNPMayLoad, SDNPMemOperand]>;
def atomic_load_cap :
  PatFrag<(ops node:$ptr),
          (atomic_load_cap_node node:$ptr), [{
  return cast<AtomicSDNode>(N)->getMemoryVT().isFatPointer();
}]>;

def atomic_store_cap_node     : SDNode<"ISD::ATOMIC_STORE", SDTAtomicCapStore,
                    [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;

def atomic_store_cap : PatFrag<(ops node:$ptr, node:$val),
                   (atomic_store_cap_node node:$ptr, node:$val), [{
    return cast<AtomicSDNode>(N)->getMemoryVT().isFatPointer();
}]>;


def SDTAtomicCap3 : SDTypeProfile<1, 3, [
  SDTCisSameAs<0,2>,  SDTCisSameAs<0,3>, SDTCisPtrTy<0>, SDTCisPtrTy<1>
]>;
def SDTAtomicCap2 : SDTypeProfile<1, 2, [
  SDTCisSameAs<0,2>, SDTCisPtrTy<0>, SDTCisPtrTy<1>
]>;
def atomic_cmp_swap_cap_node : SDNode<"ISD::ATOMIC_CMP_SWAP" , SDTAtomicCap3,
                    [SDNPHasChain, SDNPMayStore, SDNPMayLoad, SDNPMemOperand]>;
def atomic_load_add_cap_node : SDNode<"ISD::ATOMIC_LOAD_ADD" , SDTAtomicCap2,
                    [SDNPHasChain, SDNPMayStore, SDNPMayLoad, SDNPMemOperand]>;
def atomic_swap_cap_node     : SDNode<"ISD::ATOMIC_SWAP", SDTAtomicCap2,
                    [SDNPHasChain, SDNPMayStore, SDNPMayLoad, SDNPMemOperand]>;
def atomic_load_sub_cap_node : SDNode<"ISD::ATOMIC_LOAD_SUB" , SDTAtomicCap2,
                    [SDNPHasChain, SDNPMayStore, SDNPMayLoad, SDNPMemOperand]>;


def atomic_swap_cap : PatFrag<(ops node:$ptr, node:$val),
                   (atomic_swap_cap_node node:$ptr, node:$val), [{
    return cast<AtomicSDNode>(N)->getMemoryVT().isFatPointer();
}]> {
    // We can't use let MemoryVT = iFATPTRAny here since by the time it runs
    // we have turned it into an IFATPTR128, etc -> use the C++ predicate above
}

def atomic_cmp_swap_cap : PatFrag<(ops node:$ptr, node:$cmp, node:$val),
                   (atomic_cmp_swap_cap_node node:$ptr, node:$cmp, node:$val), [{
    return cast<AtomicSDNode>(N)->getMemoryVT().isFatPointer();
}]> {
    // We can't use let MemoryVT = iFATPTRAny here since by the time it runs
    // we have turned it into an IFATPTR128, etc -> use the C++ predicate above
}

// ClearRegs values
// TODO: Add some nice parsing so that we can get handle register lists for
// these.
def regslist: Operand<i32>;
def reglist_pat : PatLeaf<(imm), [{ return isInt<16>(N->getZExtValue()); }]>;

def uimm32: Operand<i32>;
def uimm32_pat:  PatLeaf<(imm), [{ return isInt<32>(N->getZExtValue()); }]>;

////////////////////////////////////////////////////////////////////////////////
// Flow control
////////////////////////////////////////////////////////////////////////////////
let hasDelaySlot=1, isTerminator=1 in {
let isBranch=1, Defs = [AT] in {
def CBTS : CheriFmtCBT<0x0a, (outs), (ins CheriOpnd:$cb, brtarget:$offset),
                        "cbts\t$cb, $offset",
                        [(brcond (i32 (setne (assertzext (CapTagGet CheriOpnd:$cb)), (i64 0))), bb:$offset)]>;
def CBTU : CheriFmtCBT<0x09, (outs), (ins CheriOpnd:$cb, brtarget:$offset),
                        "cbtu\t$cb, $offset",
                        [(brcond (i32 (seteq (assertzext (CapTagGet CheriOpnd:$cb)), (i64 0))), bb:$offset)]>;
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

let isCall=1, hasDelaySlot=1 in {
def CJALR : CheriFmt2Op<0xc, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2),
                              "cjalr\t${r2}, ${r1}",
                              []>;
}
let isCall=1 in {
def CCall : CheriFmt3CCall<0x05, (outs), (ins CheriOpnd:$cs, CheriOpnd:$cb, uimm11s0: $selector),
                              "ccall\t${cs}, ${cb}, ${selector}",
                              []>;
}
def : InstAlias<"CCallTrap $cs, $cb",
                (CCall CheriOpnd: $cs, CheriOpnd:$cb, 0), 1>;
def : InstAlias<"CCallFast $cs, $cb",
                (CCall CheriOpnd: $cs, CheriOpnd:$cb, 1), 0>;
def CReturn : CheriFmt3CRet<0x06, (outs), (ins variable_ops),
                              "creturn",
                              []> {
  let isTerminator = 1;
  let isReturn = 1;
  let hasDelaySlot = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Capability-based pointer comparisons
////////////////////////////////////////////////////////////////////////////////
let isCompare=1, isReMaterializable = 1, isAsCheapAsAMove = 1 in {
def CEQ : CheriFmtPtrCmp<0x14, GPR64Opnd, "ceq", []>;
def CNE : CheriFmtPtrCmp<0x15, GPR64Opnd, "cne", []>;
def CLT : CheriFmtPtrCmp<0x16, GPR64Opnd, "clt", []>;
def CLE : CheriFmtPtrCmp<0x17, GPR64Opnd, "cle", []>;
def CLTU : CheriFmtPtrCmp<0x18, GPR64Opnd, "cltu", []>;
def CLEU : CheriFmtPtrCmp<0x19, GPR64Opnd, "cleu", []>;
def CEXEQ : CheriFmtPtrCmp<0x1a, GPR64Opnd, "cexeq", []>;
def CNEXEQ : CheriFmtPtrCmp<0x21, GPR64Opnd, "cnexeq", []>;
}
def : InstAlias<"cgt $rd, $cb, $ct",
                (CLE GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
def : InstAlias<"cge $rd, $cb, $ct",
                (CLT GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
def : InstAlias<"cgtu $rd, $cb, $ct",
                (CLEU GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
def : InstAlias<"cgeu $rd, $cb, $ct",
                (CLTU GPR64Opnd: $rd, CheriOpnd: $ct, CheriOpnd:$cb)>;
let isCodeGenOnly=1, isCompare=1, isReMaterializable = 1, isAsCheapAsAMove = 1 in {
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
let isAsCheapAsAMove = 1 in {
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

let isReMaterializable = 1, isAsCheapAsAMove = 1 in {
// Property accessors
def CGetPerms  : CheriFmtCGet<0, "perm", "perms">;
def CGetType   : CheriFmtCGet<1, "type">;
def CGetBase   : CheriFmtCGet<2, "base">;
def CGetLength : CheriFmtCGet<3, "len", "length">;
def CGetTag    : CheriFmt2Op<4, (outs GPR64Opnd:$r1), (ins CheriOpnd:$r2),
                             "cgettag\t$r1, $r2",
                             [(set GPR64Opnd:$r1, (CapTagGet CheriOpnd:$r2))]>;
def CGetSealed : CheriFmt2Op<5, (outs GPR64Opnd:$r1), (ins CheriOpnd:$r2),
                             "cgetsealed\t$r1, $r2",
                             [(set GPR64Opnd:$r1, (CapSealedGet CheriOpnd:$r2))]>;
def CGetOffset : CheriFmtCGet<6, "offset">;
def CGetAddr   : CheriFmtCGet<0xf, "addr", "address">;
def CGetFlags  : CheriFmtCGet<0x12, "flags">;
// Versions of property setters that take immediate operands
// We allow CIncOffset $c1, 10 as an alias for CIncOffset $c1, $c1, 10, and the same for CSetBounds
let TwoOperandAliasConstraint = "$cd = $cb" in {
  def CIncOffsetImm : CheriFmtCSetImm<0x13, simm11s0, "cincoffset",
    [(set CheriOpnd:$cd, (ptradd CheriOpnd:$cb, (i64 simm11s0Pattern: $imm)))]>;
  def CSetBoundsImm : CheriFmtCSetImm<0x14, uimm11s0, "csetbounds",
    [(set CheriOpnd:$cd, (int_cheri_cap_bounds_set CheriOpnd:$cb, (i64 uimm11s0Pattern: $imm)))]>;
}
} // end of isReMaterializable = 1, isAsCheapAsAMove = 1 block

// Accept CIncOffsetImm as an assembler mnemonic because ancient binutils is too stupid to have an opcode
// with differing encoding depending on the parameters
def : InstAlias<"csetboundsimm $dst, $src, $imm",
                (CSetBoundsImm CheriOpnd:$dst, CheriOpnd:$src, uimm11s0:$imm), 0>;
def : InstAlias<"cincoffsetimm $dst, $src, $imm",
                (CIncOffsetImm CheriOpnd:$dst, CheriOpnd:$src, simm11s0:$imm), 0>;
def : InstAlias<"csetboundsimm $dst, $imm",
                (CSetBoundsImm CheriOpnd:$dst, CheriOpnd:$dst, uimm11s0:$imm), 0>;
def : InstAlias<"cincoffsetimm $dst, $imm",
                (CIncOffsetImm CheriOpnd:$dst, CheriOpnd:$dst, simm11s0:$imm), 0>;
// CAssertInBounds pseudo
def : InstAlias<"cassertinbounds $dst, $imm",
                (CSetBoundsImm CNULL, CheriOpnd:$dst, uimm11s0:$imm), 1>;
def : InstAlias<"cassertinbounds $dst",
                (CSetBoundsImm CNULL, CheriOpnd:$dst, 1), 0>;

// Property setters
let isReMaterializable = 1, isAsCheapAsAMove = 1 in {
def CAndPerms  : CheriFmtCSet<0xd, "perm", "perms", "and">;
def CSetFlags  : CheriFmtCSet<0xe, "flags">;
def CSetOffset : CheriFmtCSet<0xf, "offset">;
def CSetBounds : CheriFmtCSet<0x8, "bounds">;
def CSetBoundsExact : CheriFmtCSet<0x9, "boundsexact", "bounds", "set", "set_exact">;
def CIncOffset : CheriFmt3Op<0x11, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, GPR64Opnd:$r3),
                            "cincoffset\t$r1, $r2, $r3",
                            [(set CheriOpnd:$r1, (ptradd CheriOpnd:$r2, GPR64Opnd:$r3))]>;
def CClearTag  : CheriFmt2Op<0xb, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2),
                              "ccleartag\t$r1, $r2",
                              [(set CheriOpnd:$r1, (int_cheri_cap_tag_clear CheriOpnd:$r2))]>;
def CSetAddr : CheriFmtCSet<0x22, "addr", "address">;

// CRoundRepresentableLength (a.k.a. CRoundArchitecturalPrecision)
def CRRL : CheriFmt2Op<0x10, (outs GPR64Opnd:$r1), (ins GPR64Opnd:$r2),
                       "croundrepresentablelength\t$r1, $r2",
                       [(set GPR64Opnd:$r1, (int_cheri_round_representable_length GPR64Opnd:$r2))]>;
// CRepresentableAlignmentMask
def CRAM : CheriFmt2Op<0x11, (outs GPR64Opnd:$r1), (ins GPR64Opnd:$r2),
                       "crepresentablealignmentmask\t$r1, $r2",
                       [(set GPR64Opnd:$r1, (int_cheri_representable_alignment_mask GPR64Opnd:$r2))]>;
}
// CRAM/CRRL (a.k.a. CRAP) aliases
def : InstAlias<"crap $dst, $len", (CRRL GPR64Opnd:$dst, GPR64Opnd:$len), 0>;
def : InstAlias<"crrl $dst, $len", (CRRL GPR64Opnd:$dst, GPR64Opnd:$len), 0>;
def : InstAlias<"cram $dst, $len", (CRAM GPR64Opnd:$dst, GPR64Opnd:$len), 0>;

// Convert CIncOffsets of the difference to csetaddr
def : Pat<(CapRegType (ptradd CheriOpnd:$src, (sub GPR64Opnd:$addr, (int_cheri_cap_address_get CheriOpnd:$src)))),
          (CSetAddr $src, $addr)>;

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

def CSealEntry : CheriFmt2Op<0x1d, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2),
    "csealentry\t$r1, $r2",
    [(set CheriOpnd:$r1, (int_cheri_cap_seal_entry CheriOpnd:$r2))]>;
def CLoadTags : CheriFmt2Op<0x1e, (outs GPR64Opnd:$r1), (ins CheriOpnd:$r2),
    "cloadtags\t$r1, $r2",
    [(set GPR64Opnd:$r1, (int_cheri_cap_load_tags CheriOpnd:$r2))]>;

// PCC access
def CGetPCC : CheriFmt1Op<0x0, (outs CheriOpnd:$r1), (ins),
                          "cgetpcc\t${r1}",
                          [(set CheriOpnd:$r1, (int_cheri_pcc_get))]>;

def CSetPCCOffset : CheriFmt2Op<7, (outs CheriOpnd:$r1), (ins GPR64Opnd:$r2),
                                "cgetpccsetoffset\t${r1}, $r2",
                                [(set CheriOpnd:$r1, (int_cheri_cap_offset_set (CapRegType (int_cheri_pcc_get)), GPR64Opnd: $r2))]>;
def CGetPccIncOffset : CheriFmt2Op<0x13, (outs CheriOpnd:$r1), (ins GPR64Opnd:$r2),
                                "cgetpccincoffset\t${r1}, $r2",
                                []>; // XXX: don't generate this pattern until the FPGA supports it.
                                // [(set CheriOpnd:$r1, (ptradd (CapRegType (int_cheri_pcc_get)), GPR64Opnd: $r2))]>;
def CGetPCCSetAddr : CheriFmt2Op<0x14, (outs CheriOpnd:$r1), (ins GPR64Opnd:$r2),
                                "cgetpccsetaddr\t${r1}, $r2",
                                []>; // XXX: don't generate this pattern until the FPGA supports it.
                                // [(set CheriOpnd:$r1, (int_cheri_cap_address_set (CapRegType (int_cheri_pcc_get)), GPR64Opnd: $r2))]>;

// Sealing and unsealing
def CSeal   : CheriFmt3Op<0xb, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, CheriOpnd:$r3),
                               "cseal\t$r1, $r2, $r3",
                               [(set CheriOpnd:$r1, (int_cheri_cap_seal CheriOpnd:$r2, CheriOpnd:$r3))]>;
def CUnSeal : CheriFmt3Op<0xc, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, CheriOpnd:$r3),
                               "cunseal\t$r1, $r2, $r3",
                               [(set CheriOpnd:$r1, (int_cheri_cap_unseal CheriOpnd:$r2, CheriOpnd:$r3))]>;

// Pointer arithmetic
let isReMaterializable = 1, isAsCheapAsAMove = 1 in {
def CFromPtr : CheriFmt3Op<0x13, (outs CheriOpnd:$r1), (ins CheriOpnd0IsDDC:$r2, GPR64Opnd:$r3),
  "cfromptr\t$r1, $r2, $r3", [(set CheriOpnd: $r1, (int_cheri_cap_from_pointer CheriOpnd0IsDDC: $r2, GPR64Opnd: $r3))]>;
def CToPtr : CheriFmt3Op<0x12, (outs GPR64Opnd:$r1), (ins CheriOpnd0IsDDC:$r3, CheriOpnd:$r2),
  "ctoptr\t$r1, $r2, $r3", [(set GPR64Opnd: $r1, (int_cheri_cap_to_pointer CheriOpnd0IsDDC: $r3, CheriOpnd: $r2))]>;
let isCodeGenOnly = 1 in {
def CFromPtr32 : CheriFmt3Op<0x13, (outs CheriOpnd:$r1), (ins CheriOpnd0IsDDC:$r2, GPR32Opnd:$r3),
  "cfromptr\t$r1, $r2, $r3", []>;
}
def CSub : CheriFmt3Op<0xa, (outs GPR64Opnd:$r1), (ins CheriOpnd:$r2, CheriOpnd:$r3),
  "csub\t$r1, $r2, $r3", [(set GPR64Opnd:$r1, (int_cheri_cap_diff CheriOpnd:$r2, CheriOpnd:$r3))]>;

def CMove : CheriFmt2Op<0xa, (outs CheriOpnd: $r1), (ins CheriOpnd:$r2), "cmove\t$r1, $r2", []> {
  let isMoveReg = 1;
}
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
  "ctestsubset\t$r1, $r2, $r3", [(set GPR64Opnd: $r1, (CapSubsetTest CheriOpnd0IsDDC: $r2, CheriOpnd: $r3))]>;
def CGetAndAddr : CheriFmt3Op<0x23, (outs GPR64Opnd:$r1), (ins CheriOpnd:$r2, GPR64Opnd:$r3),
  "cgetandaddr\t$r1, $r2, $r3", [(set GPR64Opnd: $r1, (and (int_cheri_cap_address_get CheriOpnd: $r2), GPR64Opnd: $r3))]>;
// CAndAddr is the same as csetaddr(getaddr(x) & mask):
def CAndAddr : CheriFmt3Op<0x24, (outs CheriOpnd:$r1), (ins CheriOpnd:$r2, GPR64Opnd:$r3),
  "candaddr\t$r1, $r2, $r3",
  [(set CheriOpnd: $r1,
        (CapRegType (int_cheri_cap_address_set CheriOpnd:$r2,
                     (and (int_cheri_cap_address_get CheriOpnd:$r2), GPR64Opnd:$r3))))]>;


// Note: we need to duplicate these patterns for iFATPTR128,256, etc. for some reason
multiclass UseCNullPatterns<PatFrag NullPat> {
  def : Pat<(ptradd NullPat, GPR64Opnd:$addr), (CIncOffset CNULL, $addr)>;
  def : Pat<(int_cheri_cap_offset_set NullPat, GPR64Opnd:$addr), (CIncOffset CNULL, $addr)>;
  // The immediate versions should be used first!
  let AddedComplexity = 10 in {
    def : Pat<(int_cheri_cap_offset_set NullPat, simm11s0Pattern:$imm), (CIncOffsetImm CNULL, simm11s0Pattern:$imm)>;
    def : Pat<(int_cheri_cap_address_set NullPat, simm11s0Pattern:$imm), (CIncOffsetImm CNULL, simm11s0Pattern:$imm)>;
  }
}
defm : UseCNullPatterns<(iFATPTR64 (inttoptr (i64 0)))>;
defm : UseCNullPatterns<(iFATPTR128 (inttoptr (i64 0)))>;
defm : UseCNullPatterns<(iFATPTR256 (inttoptr (i64 0)))>;


// Cursors
let AddedComplexity = 10 in {
  def : Pat<(ptradd CheriOpnd:$r2, simm11s0Pattern:$imm),
          (CIncOffsetImm $r2, simm11s0Pattern: $imm)>;
}
def : Pat<(ptradd CheriOpnd:$r2, GPR64Opnd:$r3),
          (CIncOffset $r2, $r3)>;
// Int to pointer in address space 0
def : Pat<(inttoptr (i64 0)),
          (CMove CNULL)>;
let Predicates=[IsCHERI,IsCHERIABI] in {
def : Pat<(inttoptr simm11s0Pattern:$imm),
          (CIncOffsetImm CNULL, simm11s0Pattern: $imm)>;
def : Pat<(inttoptr GPR64Opnd:$rt),
          (CIncOffset CNULL, $rt)>;
def : Pat<(ptrtoint CheriOpnd:$rt),
          (CGetAddr $rt)>;
def : Pat<(i32 (ptrtoint CheriOpnd:$rt)),
          (SLL (EXTRACT_SUBREG (CGetAddr $rt), sub_32), 0)>;
}
let Predicates=[NotCHERIABI] in {
def : Pat<(inttoptr GPR64Opnd:$rt),
          (CFromPtr DDC, $rt)>;
def : Pat<(inttoptr GPR32:$rt),
          (CFromPtr32 DDC, $rt)>;
def : Pat<(ptrtoint CheriOpnd:$rt),
          (CToPtr DDC, $rt)>;
def : Pat<(i32 (ptrtoint CheriOpnd:$rt)),
          (SLL (EXTRACT_SUBREG (CToPtr DDC, $rt), sub_32), 0)>;
}
def : Pat<(int_cheri_cap_to_pointer (CapRegType (int_cheri_ddc_get)), CheriOpnd:$rt),
          (CToPtr DDC, $rt)>;


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
let Predicates=[IsCHERI,IsCHERIABI] in {
  // C11 is only defined in the purecap ABI
  def : Pat<(int_cheri_stack_cap_get), (CMove C11)>;
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
def : InstAlias<"cgetnull $dst", (CMove CheriOpnd:$dst, CNULL), 1>;
def : InstAlias<"cgetnull_old $dst", (CFromPtr CheriOpnd:$dst, DDC, ZERO_64), 1>;
// def : InstAlias<"cfromint $dst, $src", (CIncOffset CheriOpnd:$dst, CNULL, GPR64Opnd:$src), 1>;

// So that we can use ptr -> int without a warning that $c0 access is deprecated in .s files
def : Pat<(int_cheri_cap_from_ddc GPR64Opnd:$addr), (CFromPtr DDC, $addr)>;
def : InstAlias<"cfromddc $dst, $src", (CFromPtr CheriOpnd:$dst, DDC, GPR64Opnd:$src), 1>;

////////////////////////////////////////////////////////////////////////////////
// Atomic operations.
////////////////////////////////////////////////////////////////////////////////

class CapAtomic2Ops<PatFrag Op, RegisterOperand ValOpnd> :
  PseudoSE<(outs ValOpnd:$dst), (ins CheriOpnd0IsDDC:$ptr, ValOpnd:$incr),
           [(set ValOpnd:$dst, (Op CheriOpnd0IsDDC:$ptr, ValOpnd:$incr))]>;
class CapAtomicCmpSwap<PatFrag Op, RegisterOperand ValOpnd> :
  PseudoSE<(outs ValOpnd:$dst), (ins CheriOpnd0IsDDC:$ptr, ValOpnd:$cmp, ValOpnd:$swap),
           [(set ValOpnd:$dst, (Op CheriOpnd0IsDDC:$ptr, ValOpnd:$cmp, ValOpnd:$swap))]>;


let usesCustomInserter = 1 in {
  def CAP_ATOMIC_LOAD_ADD_I8   : CapAtomic2Ops<atomic_load_add_8, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_ADD_I16  : CapAtomic2Ops<atomic_load_add_16, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_ADD_I32  : CapAtomic2Ops<atomic_load_add_32, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_SUB_I8   : CapAtomic2Ops<atomic_load_sub_8, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_SUB_I16  : CapAtomic2Ops<atomic_load_sub_16, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_SUB_I32  : CapAtomic2Ops<atomic_load_sub_32, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_AND_I8   : CapAtomic2Ops<atomic_load_and_8, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_AND_I16  : CapAtomic2Ops<atomic_load_and_16, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_AND_I32  : CapAtomic2Ops<atomic_load_and_32, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_OR_I8    : CapAtomic2Ops<atomic_load_or_8, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_OR_I16   : CapAtomic2Ops<atomic_load_or_16, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_OR_I32   : CapAtomic2Ops<atomic_load_or_32, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_XOR_I8   : CapAtomic2Ops<atomic_load_xor_8, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_XOR_I16  : CapAtomic2Ops<atomic_load_xor_16, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_XOR_I32  : CapAtomic2Ops<atomic_load_xor_32, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_NAND_I8  : CapAtomic2Ops<atomic_load_nand_8, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_NAND_I16 : CapAtomic2Ops<atomic_load_nand_16, GPR32Opnd>;
  def CAP_ATOMIC_LOAD_NAND_I32 : CapAtomic2Ops<atomic_load_nand_32, GPR32Opnd>;

  def CAP_ATOMIC_SWAP_I8       : CapAtomic2Ops<atomic_swap_8, GPR32Opnd>;
  def CAP_ATOMIC_SWAP_I16      : CapAtomic2Ops<atomic_swap_16, GPR32Opnd>;
  def CAP_ATOMIC_SWAP_I32      : CapAtomic2Ops<atomic_swap_32, GPR32Opnd>;
  def CAP_ATOMIC_CMP_SWAP_I8   : CapAtomicCmpSwap<atomic_cmp_swap_8, GPR32Opnd>;
  def CAP_ATOMIC_CMP_SWAP_I16  : CapAtomicCmpSwap<atomic_cmp_swap_16, GPR32Opnd>;
  def CAP_ATOMIC_CMP_SWAP_I32  : CapAtomicCmpSwap<atomic_cmp_swap_32, GPR32Opnd>;

  def CAP_ATOMIC_LOAD_ADD_I64  : CapAtomic2Ops<atomic_load_add_64, GPR64Opnd>;
  def CAP_ATOMIC_LOAD_SUB_I64  : CapAtomic2Ops<atomic_load_sub_64, GPR64Opnd>;
  def CAP_ATOMIC_LOAD_AND_I64  : CapAtomic2Ops<atomic_load_and_64, GPR64Opnd>;
  def CAP_ATOMIC_LOAD_OR_I64   : CapAtomic2Ops<atomic_load_or_64, GPR64Opnd>;
  def CAP_ATOMIC_LOAD_XOR_I64  : CapAtomic2Ops<atomic_load_xor_64, GPR64Opnd>;
  def CAP_ATOMIC_LOAD_NAND_I64 : CapAtomic2Ops<atomic_load_nand_64, GPR64Opnd>;
  def CAP_ATOMIC_SWAP_I64      : CapAtomic2Ops<atomic_swap_64, GPR64Opnd>;
  def CAP_ATOMIC_CMP_SWAP_I64  : CapAtomicCmpSwap<atomic_cmp_swap_64, GPR64Opnd>;

  // Capability atomics:
  // FIXME: this seems wrong it should be CheriGPROrCNULL
  def CAP_ATOMIC_SWAP_CAP      : CapAtomic2Ops<atomic_swap_cap, CheriOpnd>;
  def CAP_ATOMIC_CMP_SWAP_CAP  : CapAtomicCmpSwap<atomic_cmp_swap_cap, CheriOpnd>;

  // TODO: implement these:
  // def ATOMIC_LOAD_ADD_CAP  : Atomic2Ops<atomic_load_add_cap, CheriOpnd>;
  // def ATOMIC_LOAD_SUB_CAP  : Atomic2Ops<atomic_load_sub_cap, CheriOpnd>;
  // Not sure about the bitwise ops though, they are probably useless on caps
  // def ATOMIC_LOAD_AND_CAP  : Atomic2Ops<atomic_load_and_cap, CheriOpnd>;
  // def ATOMIC_LOAD_OR_CAP   : Atomic2Ops<atomic_load_or_cap, CheriOpnd>;
  // def ATOMIC_LOAD_XOR_CAP  : Atomic2Ops<atomic_load_xor_cap, CheriOpnd>;
  // def ATOMIC_LOAD_NAND_CAP : Atomic2Ops<atomic_load_nand_cap, CheriOpnd>;
}

class CapAtomic2OpsPostRA<RegisterOperand ValOpnd> :
  PseudoSE<(outs ValOpnd:$dst), (ins CheriGPROrCNull:$ptr, ValOpnd:$incr), []> {
  let mayLoad = 1;
  let mayStore = 1;
}

class CapAtomicCmpSwapPostRA<RegisterOperand ValOpnd> :
  PseudoSE<(outs ValOpnd:$dst), (ins CheriGPROrCNull:$ptr, ValOpnd:$cmp, ValOpnd:$swap), []> {
  let mayLoad = 1;
  let mayStore = 1;
}

multiclass CapAtomicPostRA2Ops {
  def NAME # _I8_POSTRA : CapAtomic2OpsPostRA<GPR32Opnd>;
  def NAME # _I16_POSTRA : CapAtomic2OpsPostRA<GPR32Opnd>;
  def NAME # _I32_POSTRA : CapAtomic2OpsPostRA<GPR32Opnd>;
  def NAME # _I64_POSTRA : CapAtomic2OpsPostRA<GPR64Opnd>;
  // def NAME # _CAP_POSTRA : CapAtomic2OpsPostRA<CheriOpnd>;
}

defm CAP_ATOMIC_LOAD_ADD  : CapAtomicPostRA2Ops;
defm CAP_ATOMIC_LOAD_SUB  : CapAtomicPostRA2Ops;
defm CAP_ATOMIC_LOAD_AND  : CapAtomicPostRA2Ops;
defm CAP_ATOMIC_LOAD_OR   : CapAtomicPostRA2Ops;
defm CAP_ATOMIC_LOAD_XOR  : CapAtomicPostRA2Ops;
defm CAP_ATOMIC_LOAD_NAND : CapAtomicPostRA2Ops;
defm CAP_ATOMIC_SWAP      : CapAtomicPostRA2Ops;

def CAP_ATOMIC_CMP_SWAP_I8_POSTRA   : CapAtomicCmpSwapPostRA<GPR32Opnd>;
def CAP_ATOMIC_CMP_SWAP_I16_POSTRA  : CapAtomicCmpSwapPostRA<GPR32Opnd>;
def CAP_ATOMIC_CMP_SWAP_I32_POSTRA  : CapAtomicCmpSwapPostRA<GPR32Opnd>;
def CAP_ATOMIC_CMP_SWAP_I64_POSTRA  : CapAtomicCmpSwapPostRA<GPR64Opnd>;

// Capability postra atomics:
// TODO: do we want add/sub/or/xor/nand/and for capabilities?
// I guess add/sub makes sense but the others don't
def CAP_ATOMIC_SWAP_CAP_POSTRA      : CapAtomic2OpsPostRA<CheriOpnd>;
def CAP_ATOMIC_CMP_SWAP_CAP_POSTRA  : CapAtomicCmpSwapPostRA<CheriOpnd>;
// TODO:
// def CAP_ATOMIC_LOAD_ADD_CAP_POSTRA      : CapAtomic2OpsPostRA<CheriOpnd>;
// def CAP_ATOMIC_LOAD_SUB_CAP_POSTRA      : CapAtomic2OpsPostRA<CheriOpnd>;

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
let isCall=1, hasDelaySlot=1, Defs = [C17, C26], hasNoSchedulingInfo = 1 in {
def CapJumpLinkPseudo :
  PseudoSE<(outs), (ins CheriOpnd: $rs), [(CapJmpLink CheriOpnd: $rs)], NoItinerary>,
  PseudoInstExpansion<(CJALR C17, CheriOpnd:$rs)>;
}
def CapRet : SDNode<"MipsISD::CapRet", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

let Predicates=[IsCHERI], hasNoSchedulingInfo=1 in
def PseudoReturnCap: PseudoReturnBase<CheriOpnd>;

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1,
    Predicates=[IsCHERI], hasNoSchedulingInfo=1 in
def CapRetPseudo : PseudoSE<(outs), (ins), [(CapRet)]>;

////////////////////////////////////////////////////////////////////////////////
// Atomic loads and stores
////////////////////////////////////////////////////////////////////////////////

let DecoderNamespace="CHERI" in {
def CLLC  : CheriFmtCLL<1, 3, (outs CheriOpnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllc\t$rd, $cb">;
def CLLD  : CheriFmtCLL<0, 3, (outs GPR64Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "clld\t$rd, $cb">;
def CLLW  : CheriFmtCLL<1, 2, (outs GPR32Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllw\t$rd, $cb">;
def CLLWU : CheriFmtCLL<0, 2, (outs GPR32Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllwu\t$rd, $cb">;
def CLLH  : CheriFmtCLL<1, 1, (outs GPR32Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllh\t$rd, $cb">;
def CLLHU : CheriFmtCLL<0, 1, (outs GPR32Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllhu\t$rd, $cb">;
def CLLB  : CheriFmtCLL<1, 0, (outs GPR32Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllb\t$rd, $cb">;
def CLLBU : CheriFmtCLL<0, 0, (outs GPR32Opnd:$rd), (ins CheriOpnd0IsDDC:$cb),
                        "cllbu\t$rd, $cb">;

def CSCC  : CheriFmtCSC<7, (outs GPR64Opnd:$rd), (ins CheriOpnd: $rs, CheriOpnd0IsDDC:$cb),
                        "cscc\t$rd, $rs, $cb">;
def CSCD  : CheriFmtCSC<3, (outs GPR64Opnd:$rd), (ins GPR64Opnd: $rs, CheriOpnd0IsDDC:$cb),
                        "cscd\t$rd, $rs, $cb">;
// FIXME: these are all GPR32 so that the MIPS atomics expansion is happy but I don't think they should be
def CSCW  : CheriFmtCSC<2, (outs GPR32Opnd:$rd), (ins GPR32Opnd: $rs, CheriOpnd0IsDDC:$cb),
                        "cscw\t$rd, $rs, $cb">;
def CSCH  : CheriFmtCSC<1, (outs GPR32Opnd:$rd), (ins GPR32Opnd: $rs, CheriOpnd0IsDDC:$cb),
                        "csch\t$rd, $rs, $cb">;
def CSCB  : CheriFmtCSC<0, (outs GPR32Opnd:$rd), (ins GPR32Opnd: $rs, CheriOpnd0IsDDC:$cb),
                        "cscb\t$rd, $rs, $cb">;
}

def AddrFI64  : ComplexPattern<iFATPTR64, 1, "selectAddrFI", [frameindex], []>;
def AddrFI128 : ComplexPattern<iFATPTR128, 1, "selectAddrFI", [frameindex], []>;
def AddrFI256 : ComplexPattern<iFATPTR256, 1, "selectAddrFI", [frameindex], []>;
// Load and store patterns for frame indices
// FIXME: truncstore/extload nodes are not being used?
// These patterns need duplicating for each of the capability sizes.  It's not
// clear why: we should be able to pattern match a ComplexPattern with an
// overloaded return type, but that appears to be broken.

multiclass FrameIndexStorePatterns<PatFrag storeOp, ValueType iFatptrTy, ValueType intTy, ComplexPattern Addr, Instruction OutOp, Operand simm, PatFrag immfrag> {
  // TODO: for some reason enabling this pattern makes QtWebkit run slower
  // def : Pat<(storeOp intTy:$rs, (iFatptrTy (ptradd Addr:$addr, immfrag:$offset))), (OutOp $rs, ZERO_64, imm:$offset, $addr)>;
  def : Pat<(storeOp intTy:$rs, Addr:$addr), (OutOp $rs, ZERO_64, (i64 0), $addr)>;
}
multiclass FrameIndexLoadPatterns<PatFrag loadOp, ValueType iFatptrTy, ValueType intTy, ComplexPattern Addr, Instruction OutOp, Operand simm, PatFrag immfrag> {
  // TODO: for some reason enabling this pattern makes QtWebkit run slower
  // def : Pat<(intTy (loadOp (iFatptrTy (ptradd Addr:$addr, immfrag:$offset)))), (OutOp ZERO_64, imm:$offset, $addr)>;
  def : Pat<(intTy (loadOp Addr:$addr)), (OutOp ZERO_64, (i64 0), $addr)>;
}

multiclass FrameIndexPatterns<ValueType iFatptrTy, ComplexPattern Addr> {
  // FIXME: avoid this pattern for i8/i16 stack loads
  def : Pat<(store CheriOpnd:$cs, (iFatptrTy (ptradd Addr:$addr, simm11s4Pattern:$offset))), (STORECAP $cs, ZERO_64, imm:$offset, $addr)>;
  def : Pat<(store CheriOpnd:$cs, Addr:$addr), (STORECAP $cs, ZERO_64, (i64 0), $addr)>;

  def : Pat<(load (iFatptrTy (ptradd Addr:$addr, simm11s4Pattern:$offset))), (LOADCAP ZERO_64, imm:$offset, $addr)>;
  def : Pat<(load Addr:$addr), (LOADCAP ZERO_64, (i64 0), $addr)>;

  // i64 load
  defm: FrameIndexLoadPatterns<load, iFATPTR128, i64, Addr, CAPLOAD64, simm8s3, simm8s3Pattern>;

  // i32 load
  defm: FrameIndexLoadPatterns<sextloadi32, iFATPTR128, i64, Addr, CAPLOAD3264, simm8s2, simm8s2Pattern>;
  defm: FrameIndexLoadPatterns<extloadi32, iFATPTR128, i64, Addr, CAPLOAD3264, simm8s2, simm8s2Pattern>;
  defm: FrameIndexLoadPatterns<zextloadi32, iFATPTR128, i64, Addr, CAPLOADU32, simm8s2, simm8s2Pattern>;
  defm: FrameIndexLoadPatterns<load, iFATPTR128, i32, Addr, CAPLOAD32, simm8s2, simm8s2Pattern>;

  // i16 load
  defm: FrameIndexLoadPatterns<zextloadi16, iFATPTR128, i32, Addr, CAPLOADU1632, simm8s1, simm8s1Pattern>;
  defm: FrameIndexLoadPatterns<zextloadi16, iFATPTR128, i64, Addr, CAPLOADU16, simm8s1, simm8s1Pattern>;
  defm: FrameIndexLoadPatterns<sextloadi16, iFATPTR128, i32, Addr, CAPLOAD1632, simm8s1, simm8s1Pattern>;
  defm: FrameIndexLoadPatterns<sextloadi16, iFATPTR128, i64, Addr, CAPLOAD16, simm8s1, simm8s1Pattern>;
  // Use clhu instead of clh for anyext to match MIPS
  defm: FrameIndexLoadPatterns<extloadi16, iFATPTR128, i32, Addr, CAPLOADU1632, simm8s1, simm8s1Pattern>;
  defm: FrameIndexLoadPatterns<extloadi16, iFATPTR128, i64, Addr, CAPLOADU16, simm8s1, simm8s1Pattern>;

  // i8 load
  defm: FrameIndexLoadPatterns<zextloadi8, iFATPTR128, i32, Addr, CAPLOADU832, simm8, simm8Pattern>;
  defm: FrameIndexLoadPatterns<zextloadi8, iFATPTR128, i64, Addr, CAPLOADU8, simm8, simm8Pattern>;
  defm: FrameIndexLoadPatterns<sextloadi8, iFATPTR128, i32, Addr, CAPLOAD832, simm8, simm8Pattern>;
  defm: FrameIndexLoadPatterns<sextloadi8, iFATPTR128, i64, Addr, CAPLOAD8, simm8, simm8Pattern>;
  // Use clbu instead of clb for anyext to match MIPS
  defm: FrameIndexLoadPatterns<extloadi8, iFATPTR128, i32, Addr, CAPLOADU832, simm8, simm8Pattern>;
  defm: FrameIndexLoadPatterns<extloadi8, iFATPTR128, i64, Addr, CAPLOADU8, simm8, simm8Pattern>;

  // i1 load
  defm: FrameIndexLoadPatterns<zextloadi1, iFATPTR128, i32, Addr, CAPLOADU832, simm8, simm8Pattern>;
  defm: FrameIndexLoadPatterns<zextloadi1, iFATPTR128, i64, Addr, CAPLOADU8, simm8, simm8Pattern>;
  defm: FrameIndexLoadPatterns<sextloadi1, iFATPTR128, i32, Addr, CAPLOAD832, simm8, simm8Pattern>;
  defm: FrameIndexLoadPatterns<sextloadi1, iFATPTR128, i64, Addr, CAPLOAD8, simm8, simm8Pattern>;
  // Use clbu instead of clb for anyext to match MIPS
  defm: FrameIndexLoadPatterns<extloadi1, iFATPTR128, i32, Addr, CAPLOADU832, simm8, simm8Pattern>;
  defm: FrameIndexLoadPatterns<extloadi1, iFATPTR128, i64, Addr, CAPLOADU8, simm8, simm8Pattern>;

  // stores

  // This pattern allows omitting lots of unnncessary sll instructions
  def : Pat<(store (i32 (trunc GPR64:$rs)), Addr:$addr), (CAPSTORE3264 $rs, ZERO_64, (i64 0), $addr)>;

  defm: FrameIndexStorePatterns<truncstorei32, iFatptrTy, i64, Addr, CAPSTORE3264, simm8s2, simm8s2Pattern>;
  defm: FrameIndexStorePatterns<truncstorei16, iFatptrTy, i64, Addr, CAPSTORE16, simm8s1, simm8s1Pattern>;
  defm: FrameIndexStorePatterns<truncstorei8, iFatptrTy, i64, Addr, CAPSTORE8, simm8, simm8Pattern>;
  // GPR32 versions
  defm: FrameIndexStorePatterns<truncstorei16, iFatptrTy, i32, Addr, CAPSTORE1632, simm8s1, simm8s1Pattern>;
  defm: FrameIndexStorePatterns<truncstorei8, iFatptrTy, i32, Addr, CAPSTORE832, simm8, simm8Pattern>;

  defm: FrameIndexStorePatterns<store, iFatptrTy, i64, Addr, CAPSTORE64, simm8s3, simm8s3Pattern>;
  defm: FrameIndexStorePatterns<store, iFatptrTy, i32, Addr, CAPSTORE32, simm8s2, simm8s2Pattern>;
}

defm : FrameIndexPatterns<iFATPTR64, AddrFI64>;
defm : FrameIndexPatterns<iFATPTR128, AddrFI128>;
defm : FrameIndexPatterns<iFATPTR256, AddrFI256>;


// Add a pseudo instruction to pretend this is only one instruction to avoid register spills
def CheriBoundedStackPseudo : CheriPseudo<(outs CheriOpnd: $cd), (ins CheriOpnd:$fi, GPR64Opnd:$frameoffs, GPR64Opnd:$size),
                                      "#CapGlobalAddrPseudo $cd, $fi, $frameoffs, $size", []> {
  let hasSideEffects = 0; // could actually trap if we are out of stack space but I don't think that matters
  let isReMaterializable = 1;
  // Actually two instructions but for now pretend that it is only one since otherwise
  // the regalloc passes will not attempt to rematerialize this which results in
  // stack spills and more saved registers.
  let isAsCheapAsAMove = 1;  // TODO: this should be false
  let usesCustomInserter = 0;
}

// TODO: for some reason all these need to be duplicated for the AddrFI patterns again
multiclass StackCapPatterns<ValueType iFatptrTy, ComplexPattern Addr> {
  def : Pat<(iFatptrTy (int_cheri_bounded_stack_cap Addr:$fi, simm11s0Pattern:$off)),
            (CheriBoundedStackPseudo (Addr:$fi), (i64 0), imm:$off)>;
  def : Pat<(iFatptrTy (int_cheri_bounded_stack_cap Addr:$fi, GPR64Opnd:$off)),
            (CheriBoundedStackPseudo (Addr:$fi), (i64 0), $off)>;

  // These  should not be needed since they would mask bugs in CheriBoundAllocas.cpp.
  // (but keep them here just in case):
  // let AddedComplexity = -120 in {
  //   // If the incoffset was placed in a different basic block we
  //   // might have to turn the bounded stack cap into a plain setbounds:
  //   def : Pat<(iFatptrTy (int_cheri_bounded_stack_cap CheriOpnd:$reg, uimm11s0Pattern:$size)),
  //             (iFatptrTy (CSetBounds CheriOpnd:$reg, imm:$size))>;
  //   def : Pat<(iFatptrTy (int_cheri_bounded_stack_cap CheriOpnd:$reg, GPR64Opnd:$size)),
  //             (iFatptrTy (CSetBounds CheriOpnd:$reg, GPR64Opnd:$size))>;
  // }

  // NOTE: removing the parentheses around (AddrFI128:$fi) in the output pattern will result in
  // an invalid CIncOffset: %2:cherigpr = CIncOffset %2:cherigpr, $zero_64
  def : Pat<(iFatptrTy Addr:$fi), (CIncOffset (Addr:$fi), ZERO_64)>;
  let AddedComplexity = 5 in {
    // Add complexity to select this before the generic CIncOffsetImm pattern
    def : Pat<(iFatptrTy (ptradd Addr:$fi, simm11s0Pattern:$offset)), (CIncOffsetImm (Addr:$fi), imm:$offset)>;
  }
}
defm : StackCapPatterns<iFATPTR64, AddrFI64>;
defm : StackCapPatterns<iFATPTR128, AddrFI128>;
defm : StackCapPatterns<iFATPTR256, AddrFI256>;

// This would also work:
// def frameindex_to_targetframeindex : SDNodeXForm<frameindex, [{
//   auto FI = cast<FrameIndexSDNode>(N);
//   return CurDAG->getTargetFrameIndex(FI->getIndex(), MVT::i32);
// }]>;
// def : Pat<(iFATPTR128 frameindex:$fi), (CIncOffset (frameindex_to_targetframeindex $fi), ZERO_64)>;
//
// Using a Pseudo instruction also seems to work fine:
// def StackToCapPseudo64 : CheriPseudo<(outs CheriOpnd: $cd), (ins CheriOpnd:$fi), "#StackToCapPseudo $cd, $fi", [(set CheriOpnd:$cd, AddrFI64:$fi)]>;
// def StackToCapPseudo128 : CheriPseudo<(outs CheriOpnd: $cd), (ins CheriOpnd:$fi), "#StackToCapPseudo $cd, $fi", [(set CheriOpnd:$cd, AddrFI128:$fi)]>;
// def StackToCapPseudo256 : CheriPseudo<(outs CheriOpnd: $cd), (ins CheriOpnd:$fi), "#StackToCapPseudo $cd, $fi", [(set CheriOpnd:$cd, AddrFI256:$fi)]>;

// TODO: maybe this will allow folding more offsets into the CIncOffset/clc/csc, etc.
// def addrRegImm64 : ComplexPattern<iFATPTR64, 2, "selectAddrRegImm", [frameindex]>;
// def addrRegImm128 : ComplexPattern<iFATPTR128, 2, "selectAddrRegImm", [frameindex]>;
// def addrRegImm256 : ComplexPattern<iFATPTR256, 2, "selectAddrRegImm", [frameindex]>;


let Defs = [C11], Uses = [C11], hasSideEffects = 1, hasNoSchedulingInfo=1, Predicates=[IsCHERI,IsCHERIABI] in {
def ADJCALLSTACKCAPDOWN : MipsPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                  [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKCAPUP   : MipsPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                  [(callseq_end timm:$amt1, timm:$amt2)]>;
}

let Predicates=[IsCHERI], hasNoSchedulingInfo=1 in
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

def : Pat<(load GPR64Opnd:$rt),
          (LOADCAP GPR64Opnd:$rt, (i64 0), DDC)>;
def : Pat<(load CheriOpnd:$cb),
          (LOADCAP ZERO_64, (i64 0), CheriOpnd:$cb)>;


def PseudoPccRelativeAddress : CheriPseudo<(outs CheriOpnd: $cd), (ins CheriOpnd: $addr),
                                           "#PseudoPccRelBlockAddress $cd, $addr", []>;
let Predicates=[IsCHERI], hasNoSchedulingInfo=1 in
def PseudoPccRelativeAddressPostRA : PseudoSE<(outs CheriOpnd:$dst), (ins CheriOpnd:$addr), []>;
