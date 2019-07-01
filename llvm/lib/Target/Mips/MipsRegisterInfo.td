//===-- MipsRegisterInfo.td - Mips Register defs -----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the MIPS register file
//===----------------------------------------------------------------------===//
let Namespace = "Mips" in {
def sub_32     : SubRegIndex<32>;
def sub_64     : SubRegIndex<64>;
def sub_lo     : SubRegIndex<32>;
def sub_hi     : SubRegIndex<32, 32>;
def sub_dsp16_19 : SubRegIndex<4, 16>;
def sub_dsp20    : SubRegIndex<1, 20>;
def sub_dsp21    : SubRegIndex<1, 21>;
def sub_dsp22    : SubRegIndex<1, 22>;
def sub_dsp23    : SubRegIndex<1, 23>;
}

class Unallocatable {
  bit isAllocatable = 0;
}

// We have banks of 32 registers each.
class MipsReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Mips";
}

class MipsRegWithSubRegs<bits<16> Enc, string n, list<Register> subregs>
  : RegisterWithSubRegs<n, subregs> {
  let HWEncoding = Enc;
  let Namespace = "Mips";
}

// Mips CPU Registers.
class MipsGPRReg<bits<16> Enc, string n> : MipsReg<Enc, n>;

// Mips 64-bit CPU Registers
class Mips64GPRReg<bits<16> Enc, string n, list<Register> subregs>
  : MipsRegWithSubRegs<Enc, n, subregs> {
  let SubRegIndices = [sub_32];
}

// Mips 32-bit FPU Registers
class FPR<bits<16> Enc, string n> : MipsReg<Enc, n>;

// Mips 64-bit (aliased) FPU Registers
class AFPR<bits<16> Enc, string n, list<Register> subregs>
  : MipsRegWithSubRegs<Enc, n, subregs> {
  let SubRegIndices = [sub_lo, sub_hi];
  let CoveredBySubRegs = 1;
}

class AFPR64<bits<16> Enc, string n, list<Register> subregs>
  : MipsRegWithSubRegs<Enc, n, subregs> {
  let SubRegIndices = [sub_lo, sub_hi];
  let CoveredBySubRegs = 1;
}

// Mips 128-bit (aliased) MSA Registers
class AFPR128<bits<16> Enc, string n, list<Register> subregs>
  : MipsRegWithSubRegs<Enc, n, subregs> {
  let SubRegIndices = [sub_64];
}

// Accumulator Registers
class ACCReg<bits<16> Enc, string n, list<Register> subregs>
  : MipsRegWithSubRegs<Enc, n, subregs> {
  let SubRegIndices = [sub_lo, sub_hi];
  let CoveredBySubRegs = 1;
}

// Mips Hardware Registers
class HWR<bits<16> Enc, string n> : MipsReg<Enc, n>;

// CHERI Hardware Registers
class CheriHWR<bits<16> Enc, string n> : MipsReg<Enc, n>;

// CHERI capability registers
// Uncomment the subregs lines to pretend we are super-registers of the MIPS
// integer registers. This is not true but could simulate a merged register file
class CAP<bits<16> Enc, string n, list<Register> subregs = []>
  // : MipsRegWithSubRegs<Enc, n, subregs> {
  : MipsRegWithSubRegs<Enc, n, []> {
  // Pretend that we have sub-registers for the purpose of register allocation
  // let SubRegIndices = [sub_64];
  let SubRegIndices = [];
}

// A capability register that "aliases" one of the MIPS registers
// We use this to simulate a merged register file for the purpose of regalloc.
// Note: This is a poor approximation, but if we mark the callee-saved registers
// as aliased we should no longer get large performance wins over MIPS
class AliasCAP<bits<16> Enc, string n, list<Register> aliases = []>
    : MipsRegWithSubRegs<Enc, n, []> {
  // Pretend that we have sub-registers for the purpose of register allocation
  // Unfortunately, this means that $s0 will also be saved if we write to $c18
  // There must be an easier way to tell regalloc to avoid allocating one
  // let Aliases = aliases;
}

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//

let Namespace = "Mips" in {
  // General Purpose Registers
  def ZERO : MipsGPRReg< 0, "zero">, DwarfRegNum<[0]>;
  def AT   : MipsGPRReg< 1, "1">,    DwarfRegNum<[1]>;
  def V0   : MipsGPRReg< 2, "2">,    DwarfRegNum<[2]>;
  def V1   : MipsGPRReg< 3, "3">,    DwarfRegNum<[3]>;
  def A0   : MipsGPRReg< 4, "4">,    DwarfRegNum<[4]>;
  def A1   : MipsGPRReg< 5, "5">,    DwarfRegNum<[5]>;
  def A2   : MipsGPRReg< 6, "6">,    DwarfRegNum<[6]>;
  def A3   : MipsGPRReg< 7, "7">,    DwarfRegNum<[7]>;
  def T0   : MipsGPRReg< 8, "8">,    DwarfRegNum<[8]>;
  def T1   : MipsGPRReg< 9, "9">,    DwarfRegNum<[9]>;
  def T2   : MipsGPRReg< 10, "10">,  DwarfRegNum<[10]>;
  def T3   : MipsGPRReg< 11, "11">,  DwarfRegNum<[11]>;
  def T4   : MipsGPRReg< 12, "12">,  DwarfRegNum<[12]>;
  def T5   : MipsGPRReg< 13, "13">,  DwarfRegNum<[13]>;
  def T6   : MipsGPRReg< 14, "14">,  DwarfRegNum<[14]>;
  def T7   : MipsGPRReg< 15, "15">,  DwarfRegNum<[15]>;
  def S0   : MipsGPRReg< 16, "16">,  DwarfRegNum<[16]>;
  def S1   : MipsGPRReg< 17, "17">,  DwarfRegNum<[17]>;
  def S2   : MipsGPRReg< 18, "18">,  DwarfRegNum<[18]>;
  def S3   : MipsGPRReg< 19, "19">,  DwarfRegNum<[19]>;
  def S4   : MipsGPRReg< 20, "20">,  DwarfRegNum<[20]>;
  def S5   : MipsGPRReg< 21, "21">,  DwarfRegNum<[21]>;
  def S6   : MipsGPRReg< 22, "22">,  DwarfRegNum<[22]>;
  def S7   : MipsGPRReg< 23, "23">,  DwarfRegNum<[23]>;
  def T8   : MipsGPRReg< 24, "24">,  DwarfRegNum<[24]>;
  def T9   : MipsGPRReg< 25, "25">,  DwarfRegNum<[25]>;
  def K0   : MipsGPRReg< 26, "26">,  DwarfRegNum<[26]>;
  def K1   : MipsGPRReg< 27, "27">,  DwarfRegNum<[27]>;
  def GP   : MipsGPRReg< 28, "gp">,  DwarfRegNum<[28]>;
  def SP   : MipsGPRReg< 29, "sp">,  DwarfRegNum<[29]>;
  def FP   : MipsGPRReg< 30, "fp">,  DwarfRegNum<[30]>;
  def RA   : MipsGPRReg< 31, "ra">,  DwarfRegNum<[31]>;

  // General Purpose 64-bit Registers
  def ZERO_64 : Mips64GPRReg< 0, "zero", [ZERO]>, DwarfRegNum<[0]>;
  def AT_64   : Mips64GPRReg< 1, "1",    [AT]>, DwarfRegNum<[1]>;
  def V0_64   : Mips64GPRReg< 2, "2",    [V0]>, DwarfRegNum<[2]>;
  def V1_64   : Mips64GPRReg< 3, "3",    [V1]>, DwarfRegNum<[3]>;
  def A0_64   : Mips64GPRReg< 4, "4",    [A0]>, DwarfRegNum<[4]>;
  def A1_64   : Mips64GPRReg< 5, "5",    [A1]>, DwarfRegNum<[5]>;
  def A2_64   : Mips64GPRReg< 6, "6",    [A2]>, DwarfRegNum<[6]>;
  def A3_64   : Mips64GPRReg< 7, "7",    [A3]>, DwarfRegNum<[7]>;
  def T0_64   : Mips64GPRReg< 8, "8",    [T0]>, DwarfRegNum<[8]>;
  def T1_64   : Mips64GPRReg< 9, "9",    [T1]>, DwarfRegNum<[9]>;
  def T2_64   : Mips64GPRReg< 10, "10",  [T2]>, DwarfRegNum<[10]>;
  def T3_64   : Mips64GPRReg< 11, "11",  [T3]>, DwarfRegNum<[11]>;
  def T4_64   : Mips64GPRReg< 12, "12",  [T4]>, DwarfRegNum<[12]>;
  def T5_64   : Mips64GPRReg< 13, "13",  [T5]>, DwarfRegNum<[13]>;
  def T6_64   : Mips64GPRReg< 14, "14",  [T6]>, DwarfRegNum<[14]>;
  def T7_64   : Mips64GPRReg< 15, "15",  [T7]>, DwarfRegNum<[15]>;
  def S0_64   : Mips64GPRReg< 16, "16",  [S0]>, DwarfRegNum<[16]>;
  def S1_64   : Mips64GPRReg< 17, "17",  [S1]>, DwarfRegNum<[17]>;
  def S2_64   : Mips64GPRReg< 18, "18",  [S2]>, DwarfRegNum<[18]>;
  def S3_64   : Mips64GPRReg< 19, "19",  [S3]>, DwarfRegNum<[19]>;
  def S4_64   : Mips64GPRReg< 20, "20",  [S4]>, DwarfRegNum<[20]>;
  def S5_64   : Mips64GPRReg< 21, "21",  [S5]>, DwarfRegNum<[21]>;
  def S6_64   : Mips64GPRReg< 22, "22",  [S6]>, DwarfRegNum<[22]>;
  def S7_64   : Mips64GPRReg< 23, "23",  [S7]>, DwarfRegNum<[23]>;
  def T8_64   : Mips64GPRReg< 24, "24",  [T8]>, DwarfRegNum<[24]>;
  def T9_64   : Mips64GPRReg< 25, "25",  [T9]>, DwarfRegNum<[25]>;
  def K0_64   : Mips64GPRReg< 26, "26",  [K0]>, DwarfRegNum<[26]>;
  def K1_64   : Mips64GPRReg< 27, "27",  [K1]>, DwarfRegNum<[27]>;
  def GP_64   : Mips64GPRReg< 28, "gp",  [GP]>, DwarfRegNum<[28]>;
  def SP_64   : Mips64GPRReg< 29, "sp",  [SP]>, DwarfRegNum<[29]>;
  def FP_64   : Mips64GPRReg< 30, "fp",  [FP]>, DwarfRegNum<[30]>;
  def RA_64   : Mips64GPRReg< 31, "ra",  [RA]>, DwarfRegNum<[31]>;

  /// Mips Single point precision FPU Registers
  foreach I = 0-31 in
  def F#I : FPR<I, "f"#I>, DwarfRegNum<[!add(I, 32)]>;

  // Higher half of 64-bit FP registers.
  foreach I = 0-31 in
  def F_HI#I : FPR<I, "f"#I>, DwarfRegNum<[!add(I, 32)]>;

  /// Mips Double point precision FPU Registers (aliased
  /// with the single precision to hold 64 bit values)
  foreach I = 0-15 in
  def D#I : AFPR<!shl(I, 1), "f"#!shl(I, 1),
                 [!cast<FPR>("F"#!shl(I, 1)),
                  !cast<FPR>("F"#!add(!shl(I, 1), 1))]>;

  /// Mips Double point precision FPU Registers in MFP64 mode.
  foreach I = 0-31 in
  def D#I#_64 : AFPR64<I, "f"#I, [!cast<FPR>("F"#I), !cast<FPR>("F_HI"#I)]>,
                DwarfRegNum<[!add(I, 32)]>;

  /// Mips MSA registers
  /// MSA and FPU cannot both be present unless the FPU has 64-bit registers
  foreach I = 0-31 in
  def W#I : AFPR128<I, "w"#I, [!cast<AFPR64>("D"#I#"_64")]>,
            DwarfRegNum<[!add(I, 32)]>;

  // Hi/Lo registers
  def HI0 : MipsReg<0, "ac0">, DwarfRegNum<[64]>;
  def HI1 : MipsReg<1, "ac1">, DwarfRegNum<[176]>;
  def HI2 : MipsReg<2, "ac2">, DwarfRegNum<[178]>;
  def HI3 : MipsReg<3, "ac3">, DwarfRegNum<[180]>;
  def LO0 : MipsReg<0, "ac0">, DwarfRegNum<[65]>;
  def LO1 : MipsReg<1, "ac1">, DwarfRegNum<[177]>;
  def LO2 : MipsReg<2, "ac2">, DwarfRegNum<[179]>;
  def LO3 : MipsReg<3, "ac3">, DwarfRegNum<[181]>;

  let SubRegIndices = [sub_32] in {
  def HI0_64  : RegisterWithSubRegs<"hi", [HI0]>;
  def LO0_64  : RegisterWithSubRegs<"lo", [LO0]>;
  }

  // FP control registers.
  foreach I = 0-31 in
  def FCR#I : MipsReg<#I, ""#I>;

  // FP condition code registers.
  foreach I = 0-7 in
  def FCC#I : MipsReg<#I, "fcc"#I>;

  // COP0 registers.
  foreach I = 0-31 in
  def COP0#I : MipsReg<#I, ""#I>;

  // COP2 registers.
  foreach I = 0-31 in
  def COP2#I : MipsReg<#I, ""#I>;

  // COP3 registers.
  foreach I = 0-31 in
  def COP3#I : MipsReg<#I, ""#I>;

  // PC register
  def PC : Register<"pc">;

  // Hardware registers
  def HWR0 : MipsReg<0, "hwr_cpunum">;
  def HWR1 : MipsReg<1, "hwr_synci_step">;
  def HWR2 : MipsReg<2, "hwr_cc">;
  def HWR3 : MipsReg<3, "hwr_ccres">;

  foreach I = 4-31 in
  def HWR#I : MipsReg<#I, ""#I>;

  // Accum registers
  foreach I = 0-3 in
  def AC#I : ACCReg<#I, "ac"#I,
                    [!cast<Register>("LO"#I), !cast<Register>("HI"#I)]>;

  def AC0_64 : ACCReg<0, "ac0", [LO0_64, HI0_64]>;

  // DSP-ASE control register fields.
  def DSPPos : Register<"">;
  def DSPSCount : Register<"">;
  def DSPCarry : Register<"">;
  def DSPEFI : Register<"">;
  def DSPOutFlag16_19 : Register<"">;
  def DSPOutFlag20 : Register<"">;
  def DSPOutFlag21 : Register<"">;
  def DSPOutFlag22 : Register<"">;
  def DSPOutFlag23 : Register<"">;
  def DSPCCond : Register<"">;

  let SubRegIndices = [sub_dsp16_19, sub_dsp20, sub_dsp21, sub_dsp22,
                       sub_dsp23] in
  def DSPOutFlag : RegisterWithSubRegs<"", [DSPOutFlag16_19, DSPOutFlag20,
                                            DSPOutFlag21, DSPOutFlag22,
                                            DSPOutFlag23]>;

  // MSA-ASE control registers.
  def MSAIR      : MipsReg<0, "0">;
  def MSACSR     : MipsReg<1, "1">;
  def MSAAccess  : MipsReg<2, "2">;
  def MSASave    : MipsReg<3, "3">;
  def MSAModify  : MipsReg<4, "4">;
  def MSARequest : MipsReg<5, "5">;
  def MSAMap     : MipsReg<6, "6">;
  def MSAUnmap   : MipsReg<7, "7">;
  // MSA-ASE fake control registers.
  // These registers do not exist, but instructions like `cfcmsa`
  // and `ctcmsa` allows to specify them.
  foreach I = 8-31 in
  def MSA#I : MipsReg<#I, ""#I>;

  // Capability registers

  def DDC : CAP<0, "DDC">, DwarfRegNum<[72]> { // TODO: this == CAPHWR0
    let SubRegIndices = [];
  }
  // CNULL and DDC share the same encoding register number since some instructions
  // use reg = 0 to mean $ddc (loads/stores/cfromptr/ctoptr/cbuildcap)
  // For compatibility with GDB keep the DwarfRegNum for DDC as 72 and assign
  // the next one to cnull (not very useful in GDB anyway since it will always
  // be null).
  def CNULL : CAP<0, "CNULL", []>, DwarfRegNum<[104]>;
  def C1  : CAP<1, "C1", []>, DwarfRegNum<[73]>;
  def C2  : CAP<2, "C2", []>, DwarfRegNum<[74]>;
  def C3  : CAP<3, "C3", []>, DwarfRegNum<[75]>;
  def C4  : CAP<4, "C4", []>, DwarfRegNum<[76]>;
  def C5  : CAP<5, "C5", []>, DwarfRegNum<[77]>;
  def C6  : CAP<6, "C6", []>, DwarfRegNum<[78]>;
  def C7  : CAP<7, "C7", []>, DwarfRegNum<[79]>;
  def C8  : CAP<8, "C8", []>, DwarfRegNum<[80]>;
  def C9  : CAP<9, "C9", []>, DwarfRegNum<[81]>;
  def C10 : CAP<10, "C10", []>, DwarfRegNum<[82]>;
  def C11 : CAP<11, "C11", []>, DwarfRegNum<[83]>;
  def C12 : CAP<12, "C12", []>, DwarfRegNum<[84]>;
  def C13 : CAP<13, "C13", []>, DwarfRegNum<[85]>;
  def C14 : CAP<14, "C14", []>, DwarfRegNum<[86]>;
  def C15 : CAP<15, "C15", []>, DwarfRegNum<[87]>;
  def C16 : CAP<16, "C16", []>, DwarfRegNum<[88]>;
  def C17 : AliasCAP<17, "C17", [RA_64]>, DwarfRegNum<[89]>;
  def C18 : AliasCAP<18, "C18", [S0_64]>, DwarfRegNum<[90]>;
  def C19 : AliasCAP<19, "C19", [S1_64]>, DwarfRegNum<[91]>;
  def C20 : AliasCAP<20, "C20", [S2_64]>, DwarfRegNum<[92]>;
  def C21 : AliasCAP<21, "C21", [S3_64]>, DwarfRegNum<[93]>;
  def C22 : AliasCAP<22, "C22", [S4_64]>, DwarfRegNum<[94]>;
  def C23 : AliasCAP<23, "C23", [S5_64]>, DwarfRegNum<[95]>;
  def C24 : AliasCAP<24, "C24", [FP_64]>, DwarfRegNum<[96]>;
  def C25 : CAP<25, "C25", []>, DwarfRegNum<[97]>;
  def C26 : AliasCAP<26, "C26", [GP_64]>, DwarfRegNum<[98]>;
  def C27 : AliasCAP<27, "C27", [K0_64]>, DwarfRegNum<[99]>;
  def C28 : AliasCAP<28, "C28", [K1_64]>, DwarfRegNum<[100]>;
  def C29 : CAP<29, "C29", []>, DwarfRegNum<[101]>;
  def C30 : CAP<30, "C30", []>, DwarfRegNum<[102]>;
  def C31 : CAP<31, "C31", []>, DwarfRegNum<[103]>;


  // TODO: assign DWARF register numbers to the special hwregs
  def CAPHWR0 : MipsReg<0, "chwr_ddc">;
  def CAPHWR1 : MipsReg<1, "chwr_userlocal">;
  foreach I = 2-7 in def CAPHWR#I : MipsReg<#I, ""#I>;
  def CAPHWR8 : MipsReg<8, "chwr_priv_userlocal">;
  foreach I = 9-21 in def CAPHWR#I : MipsReg<#I, ""#I>;
  def CAPHWR22 : MipsReg<22, "chwr_kr1c">;
  def CAPHWR23 : MipsReg<23, "chwr_kr2c">;
  foreach I = 24-28 in def CAPHWR#I : MipsReg<#I, ""#I>;
  def CAPHWR29 : MipsReg<29, "chwr_kcc">;
  def CAPHWR30 : MipsReg<30, "chwr_kdc">;
  def CAPHWR31 : MipsReg<31, "chwr_epcc">;

  // Octeon multiplier and product registers
  def MPL0 : MipsReg<0, "mpl0">;
  def MPL1 : MipsReg<1, "mpl1">;
  def MPL2 : MipsReg<2, "mpl2">;
  def P0 : MipsReg<0, "p0">;
  def P1 : MipsReg<1, "p1">;
  def P2 : MipsReg<2, "p2">;

}

//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

class GPR32Class<list<ValueType> regTypes> :
  RegisterClass<"Mips", regTypes, 32, (add
  // Reserved
  ZERO, AT,
  // Return Values and Arguments
  V0, V1, A0, A1, A2, A3,
  // Not preserved across procedure calls
  T0, T1, T2, T3, T4, T5, T6, T7,
  // Callee save
  S0, S1, S2, S3, S4, S5, S6, S7,
  // Not preserved across procedure calls
  T8, T9,
  // Reserved
  K0, K1, GP, SP, FP, RA)>;

def GPR32 : GPR32Class<[i32]>;

def GPR32ZERO : RegisterClass<"Mips", [i32], 32, (add
  // Reserved
  ZERO)>;

def GPR32NONZERO : RegisterClass<"Mips", [i32], 32, (add
  // Reserved
  AT,
  // Return Values and Arguments
  V0, V1, A0, A1, A2, A3,
  // Not preserved across procedure calls
  T0, T1, T2, T3, T4, T5, T6, T7,
  // Callee save
  S0, S1, S2, S3, S4, S5, S6, S7,
  // Not preserved across procedure calls
  T8, T9,
  // Reserved
  K0, K1, GP, SP, FP, RA)>;

def DSPR  : GPR32Class<[v4i8, v2i16]>;

def GPRMM16 : RegisterClass<"Mips", [i32], 32, (add
  // Callee save
  S0, S1,
  // Return Values and Arguments
  V0, V1, A0, A1, A2, A3)>;

def GPRMM16Zero : RegisterClass<"Mips", [i32], 32, (add
  // Reserved
  ZERO,
  // Callee save
  S1,
  // Return Values and Arguments
  V0, V1, A0, A1, A2, A3)>;

def GPRMM16MoveP : RegisterClass<"Mips", [i32], 32, (add
  // Reserved
  ZERO,
  // Callee save
  S1,
  // Return Values and Arguments
  V0, V1,
  // Callee save
  S0, S2, S3, S4)>;

def GPRMM16MovePPairFirst : RegisterClass<"Mips", [i32], 32, (add
  // Arguments
  A0, A1, A2)>;

def GPRMM16MovePPairSecond : RegisterClass<"Mips", [i32], 32, (add
  // Arguments
  A1, A2, A3,
  // Callee save
  S5, S6)>;

def GPR64 : RegisterClass<"Mips", [i64], 64, (add
  // Reserved
  ZERO_64, AT_64,
  // Return Values and Arguments
  V0_64, V1_64, A0_64, A1_64, A2_64, A3_64,
  // Not preserved across procedure calls
  T0_64, T1_64, T2_64, T3_64, T4_64, T5_64, T6_64, T7_64,
  // Callee save
  S0_64, S1_64, S2_64, S3_64, S4_64, S5_64, S6_64, S7_64,
  // Not preserved across procedure calls
  T8_64, T9_64,
  // Reserved
  K0_64, K1_64, GP_64, SP_64, FP_64, RA_64)>;

def CPU16Regs : RegisterClass<"Mips", [i32], 32, (add
  // Return Values and Arguments
  V0, V1, A0, A1, A2, A3,
  // Callee save
  S0, S1)>;

def CPU16RegsPlusSP : RegisterClass<"Mips", [i32], 32, (add
  // Return Values and Arguments
  V0, V1, A0, A1, A2, A3,
  // Callee save
  S0, S1,
  SP)>;

def CPURAReg : RegisterClass<"Mips", [i32], 32, (add RA)>, Unallocatable;

def CPUSPReg : RegisterClass<"Mips", [i32], 32, (add SP)>, Unallocatable;

def CapRegType : ValueTypeByHwMode<[Cap64, Cap128, Cap256, DefaultMode],
                                   [iFATPTR64, iFATPTR128, iFATPTR256, iFATPTR128]>;

def CheriGPR : RegisterClass<"Mips", [CapRegType], 256, (add
  C1, C2, C3, C4, C5, C6, C7, C8, C9,
  C10, C11, C12, C13, C14, C15, C16, C17, C18, C19,
  C20, C21, C22, C23, C24, C25, C26, C27, C28, C29,
  C30, C31)> {
  let RegInfos = RegInfoByHwMode<[Cap64, Cap128, Cap256, DefaultMode],
       [RegInfo<64,64,64>, RegInfo<128,128,128>, RegInfo<256,256,256>, RegInfo<128,128,128>]>;
}
def FakeCheriRegs : RegisterClass<"Mips", [iFATPTR256], 256, (add DDC)>, Unallocatable;

// Cap register class which includes both CNULL and DDC. This is not a
// constraint used by any instructions, it is used as a common super-class.
// This approach is copied from AArch64RegisterInfo.td
def CheriRegsAll : RegisterClass<"Mips", [CapRegType], 256, (add CheriGPR, CNULL, DDC)>, Unallocatable {
  let RegInfos = RegInfoByHwMode<[Cap64, Cap128, Cap256, DefaultMode],
       [RegInfo<64,64,64>, RegInfo<128,128,128>, RegInfo<256,256,256>, RegInfo<128,128,128>]>;
}

def CheriGPROrCNull : RegisterClass<"Mips", [CapRegType], 256, (add CNULL, CheriGPR)>, Unallocatable {
  let RegInfos = RegInfoByHwMode<[Cap64, Cap128, Cap256, DefaultMode],
       [RegInfo<64,64,64>, RegInfo<128,128,128>, RegInfo<256,256,256>, RegInfo<128,128,128>]>;
}
def CheriGPROrDDC : RegisterClass<"Mips", [CapRegType], 256, (add DDC, CheriGPR)>, Unallocatable {
  let RegInfos = RegInfoByHwMode<[Cap64, Cap128, Cap256, DefaultMode],
       [RegInfo<64,64,64>, RegInfo<128,128,128>, RegInfo<256,256,256>, RegInfo<128,128,128>]>;
}

def CheriHWRegs : RegisterClass<"Mips", [CapRegType], 256, (sequence "CAPHWR%u", 0, 31)>, Unallocatable {
  let RegInfos = RegInfoByHwMode<[Cap64, Cap128, Cap256, DefaultMode],
         [RegInfo<64,64,64>, RegInfo<128,128,128>, RegInfo<256,256,256>, RegInfo<128,128,128>]>;
}

// 64bit fp:
// * FGR64  - 32 64-bit registers
// * AFGR64 - 16 32-bit even registers (32-bit FP Mode)
//
// 32bit fp:
// * FGR32 - 16 32-bit even registers
// * FGR32 - 32 32-bit registers (single float only mode)
def FGR32 : RegisterClass<"Mips", [f32], 32, (sequence "F%u", 0, 31)> {
  // Do not allocate odd registers when given -mattr=+nooddspreg.
  let AltOrders = [(decimate FGR32, 2)];
  let AltOrderSelect = [{
    const auto & S = MF.getSubtarget<MipsSubtarget>();
    return S.isABI_O32() && !S.useOddSPReg();
  }];
}

def AFGR64 : RegisterClass<"Mips", [f64], 64, (add
  // Return Values and Arguments
  D0, D1,
  // Not preserved across procedure calls
  D2, D3, D4, D5,
  // Return Values and Arguments
  D6, D7,
  // Not preserved across procedure calls
  D8, D9,
  // Callee save
  D10, D11, D12, D13, D14, D15)>;

def FGR64 : RegisterClass<"Mips", [f64], 64, (sequence "D%u_64", 0, 31)> {
  // Do not allocate odd registers when given -mattr=+nooddspreg.
  let AltOrders = [(decimate FGR64, 2)];
  let AltOrderSelect = [{
    const auto & S = MF.getSubtarget<MipsSubtarget>();
    return S.isABI_O32() && !S.useOddSPReg();
  }];
}

// FP control registers.
def CCR : RegisterClass<"Mips", [i32], 32, (sequence "FCR%u", 0, 31)>,
          Unallocatable;

// FP condition code registers.
def FCC : RegisterClass<"Mips", [i32], 32, (sequence "FCC%u", 0, 7)>,
          Unallocatable;

// MIPS32r6/MIPS64r6 store FPU condition codes in normal FGR registers.
// This class allows us to represent this in codegen patterns.
def FGRCC : RegisterClass<"Mips", [i32], 32, (sequence "F%u", 0, 31)>;

def MSA128F16 : RegisterClass<"Mips", [f16], 128, (sequence "W%u", 0, 31)>;

def MSA128B: RegisterClass<"Mips", [v16i8], 128,
                           (sequence "W%u", 0, 31)>;
def MSA128H: RegisterClass<"Mips", [v8i16, v8f16], 128,
                           (sequence "W%u", 0, 31)>;
def MSA128W: RegisterClass<"Mips", [v4i32, v4f32], 128,
                           (sequence "W%u", 0, 31)>;
def MSA128D: RegisterClass<"Mips", [v2i64, v2f64], 128,
                           (sequence "W%u", 0, 31)>;
def MSA128WEvens: RegisterClass<"Mips", [v4i32, v4f32], 128,
                                (decimate (sequence "W%u", 0, 31), 2)>;

def MSACtrl: RegisterClass<"Mips", [i32], 32, (add
  MSAIR, MSACSR, MSAAccess, MSASave, MSAModify, MSARequest, MSAMap, MSAUnmap,
  (sequence "MSA%u", 8, 31))>, Unallocatable;

// Hi/Lo Registers
def LO32 : RegisterClass<"Mips", [i32], 32, (add LO0)>;
def HI32 : RegisterClass<"Mips", [i32], 32, (add HI0)>;
def LO32DSP : RegisterClass<"Mips", [i32], 32, (sequence "LO%u", 0, 3)>;
def HI32DSP : RegisterClass<"Mips", [i32], 32, (sequence "HI%u", 0, 3)>;
def LO64 : RegisterClass<"Mips", [i64], 64, (add LO0_64)>;
def HI64 : RegisterClass<"Mips", [i64], 64, (add HI0_64)>;

// Hardware registers
def HWRegs : RegisterClass<"Mips", [i32], 32, (sequence "HWR%u", 0, 31)>,
             Unallocatable;

// Accumulator Registers
def ACC64 : RegisterClass<"Mips", [untyped], 64, (add AC0)> {
  let Size = 64;
}

def ACC128 : RegisterClass<"Mips", [untyped], 128, (add AC0_64)> {
  let Size = 128;
}

def ACC64DSP : RegisterClass<"Mips", [untyped], 64, (sequence "AC%u", 0, 3)> {
  let Size = 64;
}

def DSPCC : RegisterClass<"Mips", [v4i8, v2i16], 32, (add DSPCCond)>;

// Coprocessor 0 registers.
def COP0 : RegisterClass<"Mips", [i32], 32, (sequence "COP0%u", 0, 31)>,
           Unallocatable;

// Coprocessor 2 registers.
def COP2 : RegisterClass<"Mips", [i32], 32, (sequence "COP2%u", 0, 31)>,
           Unallocatable;

// Coprocessor 3 registers.
def COP3 : RegisterClass<"Mips", [i32], 32, (sequence "COP3%u", 0, 31)>,
           Unallocatable;

// Stack pointer and global pointer classes for instructions that are limited
// to a single register such as lwgp/lwsp in microMIPS.
def SP32 : RegisterClass<"Mips", [i32], 32, (add SP)>, Unallocatable;
def SP64 : RegisterClass<"Mips", [i64], 64, (add SP_64)>, Unallocatable;
def GP32 : RegisterClass<"Mips", [i32], 32, (add GP)>, Unallocatable;
def GP64 : RegisterClass<"Mips", [i64], 64, (add GP_64)>, Unallocatable;

// Octeon multiplier and product registers
def OCTEON_MPL : RegisterClass<"Mips", [i64], 64, (add MPL0, MPL1, MPL2)>,
                 Unallocatable;
def OCTEON_P : RegisterClass<"Mips", [i64], 64, (add P0, P1, P2)>,
               Unallocatable;

// Register Operands.

class MipsAsmRegOperand : AsmOperandClass {
  let ParserMethod = "parseAnyRegister";
}

def GPR64AsmOperand : MipsAsmRegOperand {
  let Name = "GPR64AsmReg";
  let PredicateMethod = "isGPRAsmReg";
}

def CheriAsmOperand : MipsAsmRegOperand {
  let Name = "CheriAsmReg";
  let PredicateMethod = "isCheriAsmReg";
  let DiagnosticType = "CheriAsmReg";
}

def CheriAsmOperand0IsDDC : MipsAsmRegOperand {
  let Name = "CheriAsmReg0IsDDC";
  let PredicateMethod = "isCheriAsmReg0IsDDC";
  let DiagnosticType = "CheriAsmReg0IsDDC";
}

def GPR32ZeroAsmOperand : MipsAsmRegOperand {
  let Name = "GPR32ZeroAsmReg";
  let PredicateMethod = "isGPRZeroAsmReg";
}

def GPR32NonZeroAsmOperand : MipsAsmRegOperand {
  let Name = "GPR32NonZeroAsmReg";
  let PredicateMethod = "isGPRNonZeroAsmReg";
}

def GPR32AsmOperand : MipsAsmRegOperand {
  let Name = "GPR32AsmReg";
  let PredicateMethod = "isGPRAsmReg";
}

def GPRMM16AsmOperand : MipsAsmRegOperand {
  let Name = "GPRMM16AsmReg";
  let PredicateMethod = "isMM16AsmReg";
}

def GPRMM16AsmOperandZero : MipsAsmRegOperand {
  let Name = "GPRMM16AsmRegZero";
  let PredicateMethod = "isMM16AsmRegZero";
}

def GPRMM16AsmOperandMoveP : MipsAsmRegOperand {
  let Name = "GPRMM16AsmRegMoveP";
  let PredicateMethod = "isMM16AsmRegMoveP";
}

def GPRMM16AsmOperandMovePPairFirst : MipsAsmRegOperand {
  let Name = "GPRMM16AsmRegMovePPairFirst";
  let PredicateMethod = "isMM16AsmRegMovePPairFirst";
}

def GPRMM16AsmOperandMovePPairSecond : MipsAsmRegOperand {
  let Name = "GPRMM16AsmRegMovePPairSecond";
  let PredicateMethod = "isMM16AsmRegMovePPairSecond";
}

def ACC64DSPAsmOperand : MipsAsmRegOperand {
  let Name = "ACC64DSPAsmReg";
  let PredicateMethod = "isACCAsmReg";
}

def HI32DSPAsmOperand : MipsAsmRegOperand {
  let Name = "HI32DSPAsmReg";
  let PredicateMethod = "isACCAsmReg";
}

def LO32DSPAsmOperand : MipsAsmRegOperand {
  let Name = "LO32DSPAsmReg";
  let PredicateMethod = "isACCAsmReg";
}

def CCRAsmOperand : MipsAsmRegOperand {
  let Name = "CCRAsmReg";
}

def AFGR64AsmOperand : MipsAsmRegOperand {
  let Name = "AFGR64AsmReg";
  let PredicateMethod = "isFGRAsmReg";
}

def StrictlyAFGR64AsmOperand : MipsAsmRegOperand {
  let Name = "StrictlyAFGR64AsmReg";
  let PredicateMethod = "isStrictlyFGRAsmReg";
}

def FGR64AsmOperand : MipsAsmRegOperand {
  let Name = "FGR64AsmReg";
  let PredicateMethod = "isFGRAsmReg";
}

def StrictlyFGR64AsmOperand : MipsAsmRegOperand {
  let Name = "StrictlyFGR64AsmReg";
  let PredicateMethod = "isStrictlyFGRAsmReg";
}

def FGR32AsmOperand : MipsAsmRegOperand {
  let Name = "FGR32AsmReg";
  let PredicateMethod = "isFGRAsmReg";
}

def StrictlyFGR32AsmOperand : MipsAsmRegOperand {
  let Name = "StrictlyFGR32AsmReg";
  let PredicateMethod = "isStrictlyFGRAsmReg";
}

def FCCRegsAsmOperand : MipsAsmRegOperand {
  let Name = "FCCAsmReg";
}

def MSA128AsmOperand : MipsAsmRegOperand {
  let Name = "MSA128AsmReg";
}

def MSACtrlAsmOperand : MipsAsmRegOperand {
  let Name = "MSACtrlAsmReg";
}

def GPR32ZeroOpnd : RegisterOperand<GPR32ZERO> {
  let ParserMatchClass = GPR32ZeroAsmOperand;
}

def GPR32NonZeroOpnd : RegisterOperand<GPR32NONZERO> {
  let ParserMatchClass = GPR32NonZeroAsmOperand;
}

def GPR32Opnd : RegisterOperand<GPR32> {
  let ParserMatchClass = GPR32AsmOperand;
}

def GPRMM16Opnd : RegisterOperand<GPRMM16> {
  let ParserMatchClass = GPRMM16AsmOperand;
}

def GPRMM16OpndZero : RegisterOperand<GPRMM16Zero> {
  let ParserMatchClass = GPRMM16AsmOperandZero;
}

def GPRMM16OpndMoveP : RegisterOperand<GPRMM16MoveP> {
  let ParserMatchClass = GPRMM16AsmOperandMoveP;
  let EncoderMethod = "getMovePRegSingleOpValue";
}

def GPRMM16OpndMovePPairFirst : RegisterOperand<GPRMM16MovePPairFirst> {
  let ParserMatchClass = GPRMM16AsmOperandMovePPairFirst;
}

def GPRMM16OpndMovePPairSecond : RegisterOperand<GPRMM16MovePPairSecond> {
  let ParserMatchClass = GPRMM16AsmOperandMovePPairSecond;
}

def GPR64Opnd : RegisterOperand<GPR64> {
  let ParserMatchClass = GPR64AsmOperand;
}

def DSPROpnd : RegisterOperand<DSPR> {
  let ParserMatchClass = GPR32AsmOperand;
}

def CCROpnd : RegisterOperand<CCR> {
  let ParserMatchClass = CCRAsmOperand;
}

def HWRegsAsmOperand : MipsAsmRegOperand {
  let Name = "HWRegsAsmReg";
}

def CheriHWRegsAsmOperand : MipsAsmRegOperand {
  let Name = "CheriHWRegsAsmReg";
  let PredicateMethod = "isCheriHWAsmReg";
}

def COP0AsmOperand : MipsAsmRegOperand {
  let Name = "COP0AsmReg";
}

def COP2AsmOperand : MipsAsmRegOperand {
  let Name = "COP2AsmReg";
}

def CheriOpnd : RegisterOperand<CheriGPROrCNull> {
  let ParserMatchClass = CheriAsmOperand;
  let OperandNamespace = "Mips";
  let OperandType = "OPERAND_CHERI_GPR_OR_NULL";
}

def CheriOpnd0IsDDC : RegisterOperand<CheriGPROrDDC> {
  let ParserMatchClass = CheriAsmOperand0IsDDC;
  let OperandNamespace = "Mips";
  let OperandType = "OPERAND_CHERI_GPR_OR_DDC";
}

def COP3AsmOperand : MipsAsmRegOperand {
  let Name = "COP3AsmReg";
}

def HWRegsOpnd : RegisterOperand<HWRegs> {
  let ParserMatchClass = HWRegsAsmOperand;
}

def CheriHWRegsOpnd : RegisterOperand<CheriHWRegs> {
  let ParserMatchClass = CheriHWRegsAsmOperand;
}

def AFGR64Opnd : RegisterOperand<AFGR64> {
  let ParserMatchClass = AFGR64AsmOperand;
}

def StrictlyAFGR64Opnd : RegisterOperand<AFGR64> {
  let ParserMatchClass = StrictlyAFGR64AsmOperand;
}

def FGR64Opnd : RegisterOperand<FGR64> {
  let ParserMatchClass = FGR64AsmOperand;
}

def StrictlyFGR64Opnd : RegisterOperand<FGR64> {
  let ParserMatchClass = StrictlyFGR64AsmOperand;
}

def FGR32Opnd : RegisterOperand<FGR32> {
  let ParserMatchClass = FGR32AsmOperand;
}

def StrictlyFGR32Opnd : RegisterOperand<FGR32> {
  let ParserMatchClass = StrictlyFGR32AsmOperand;
}

def FGRCCOpnd : RegisterOperand<FGRCC> {
  // The assembler doesn't use register classes so we can re-use
  // FGR32AsmOperand.
  let ParserMatchClass = FGR32AsmOperand;
}

def FCCRegsOpnd : RegisterOperand<FCC> {
  let ParserMatchClass = FCCRegsAsmOperand;
}

def LO32DSPOpnd : RegisterOperand<LO32DSP> {
  let ParserMatchClass = LO32DSPAsmOperand;
}

def HI32DSPOpnd : RegisterOperand<HI32DSP> {
  let ParserMatchClass = HI32DSPAsmOperand;
}

def ACC64DSPOpnd : RegisterOperand<ACC64DSP> {
  let ParserMatchClass = ACC64DSPAsmOperand;
}

def COP0Opnd : RegisterOperand<COP0> {
  let ParserMatchClass = COP0AsmOperand;
}

def COP2Opnd : RegisterOperand<COP2> {
  let ParserMatchClass = COP2AsmOperand;
}

def COP3Opnd : RegisterOperand<COP3> {
  let ParserMatchClass = COP3AsmOperand;
}

def MSA128F16Opnd : RegisterOperand<MSA128F16> {
  let ParserMatchClass = MSA128AsmOperand;
}

def MSA128BOpnd : RegisterOperand<MSA128B> {
  let ParserMatchClass = MSA128AsmOperand;
}

def MSA128HOpnd : RegisterOperand<MSA128H> {
  let ParserMatchClass = MSA128AsmOperand;
}

def MSA128WOpnd : RegisterOperand<MSA128W> {
  let ParserMatchClass = MSA128AsmOperand;
}

def MSA128DOpnd : RegisterOperand<MSA128D> {
  let ParserMatchClass = MSA128AsmOperand;
}

def MSA128CROpnd : RegisterOperand<MSACtrl> {
  let ParserMatchClass = MSACtrlAsmOperand;
}
