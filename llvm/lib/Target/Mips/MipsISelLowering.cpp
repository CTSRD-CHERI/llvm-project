//===- MipsISelLowering.cpp - Mips DAG Lowering Implementation ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Mips uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "MipsISelLowering.h"
#include "MCTargetDesc/MipsBaseInfo.h"
#include "MCTargetDesc/MipsInstPrinter.h"
#include "MCTargetDesc/MipsMCTargetDesc.h"
#include "MipsCCState.h"
#include "MipsInstrInfo.h"
#include "MipsMachineFunction.h"
#include "MipsRegisterInfo.h"
#include "MipsSubtarget.h"
#include "MipsTargetMachine.h"
#include "MipsTargetObjectFile.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CHERI/cheri-compressed-cap/cheri_compressed_cap.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RuntimeLibcalls.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Cheri.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MachineValueType.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Utils/CheriSetBounds.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <deque>
#include <iterator>
#include <utility>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "mips-lower"

STATISTIC(NumTailCalls, "Number of tail calls");

static cl::opt<bool>
HugeGOT("mxmxgot", cl::Hidden,
         cl::desc("MIPS: Use large GOT relocations even for local symbols."), cl::init(false),
         cl::ZeroOrMore);

bool llvm::LargeCapTable = true;

static cl::opt<bool, true> LargeCapTableOption(
    "mxcaptable", cl::Hidden, cl::location(llvm::LargeCapTable),
    cl::desc("CHERI: Enable capability immediates larget than 11 bits"),
    cl::init(false));

static cl::opt<bool>
NoZeroDivCheck("mno-check-zero-division", cl::Hidden,
               cl::desc("MIPS: Don't trap on integer division by zero."),
               cl::init(false));

extern cl::opt<bool> EmitJalrReloc;

cl::opt<bool>
UseClearRegs("cheri-use-clearregs",
  cl::desc("Zero registers using the ClearRegs instructions"),
  cl::init(false));

cl::opt<bool>
SkipGlobalBounds("cheri-no-global-bounds",
  cl::desc("Skip bounds checks on globals"),
  cl::init(false));

static const MCPhysReg Mips64DPRegs[8] = {
  Mips::D12_64, Mips::D13_64, Mips::D14_64, Mips::D15_64,
  Mips::D16_64, Mips::D17_64, Mips::D18_64, Mips::D19_64
};

// If I is a shifted mask, set the size (Size) and the first bit of the
// mask (Pos), and return true.
// For example, if I is 0x003ff800, (Pos, Size) = (11, 11).
static bool isShiftedMask(uint64_t I, uint64_t &Pos, uint64_t &Size) {
  if (!isShiftedMask_64(I))
    return false;

  Size = countPopulation(I);
  Pos = countTrailingZeros(I);
  return true;
}

// The MIPS MSA ABI passes vector arguments in the integer register set.
// The number of integer registers used is dependant on the ABI used.
MVT MipsTargetLowering::getRegisterTypeForCallingConv(LLVMContext &Context,
                                                      CallingConv::ID CC,
                                                      EVT VT) const {
  if (!VT.isVector())
    return getRegisterType(Context, VT);

  return Subtarget.isABI_O32() || VT.getSizeInBits() == 32 ? MVT::i32
                                                           : MVT::i64;
}

unsigned MipsTargetLowering::getNumRegistersForCallingConv(LLVMContext &Context,
                                                           CallingConv::ID CC,
                                                           EVT VT) const {
  if (VT.isVector())
    return divideCeil(VT.getSizeInBits(), Subtarget.isABI_O32() ? 32 : 64);
  return MipsTargetLowering::getNumRegisters(Context, VT);
}

unsigned MipsTargetLowering::getVectorTypeBreakdownForCallingConv(
    LLVMContext &Context, CallingConv::ID CC, EVT VT, EVT &IntermediateVT,
    unsigned &NumIntermediates, MVT &RegisterVT) const {
  // Break down vector types to either 2 i64s or 4 i32s.
  RegisterVT = getRegisterTypeForCallingConv(Context, CC, VT);
  IntermediateVT = RegisterVT;
  NumIntermediates =
      VT.getFixedSizeInBits() < RegisterVT.getFixedSizeInBits()
          ? VT.getVectorNumElements()
          : divideCeil(VT.getSizeInBits(), RegisterVT.getSizeInBits());
  return NumIntermediates;
}

SDValue MipsTargetLowering::getGlobalReg(SelectionDAG &DAG, EVT Ty,
                                         bool IsForTls) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MipsFunctionInfo *FI = MF.getInfo<MipsFunctionInfo>();
  return DAG.getRegister(FI->getGlobalBaseReg(MF, IsForTls), Ty);
}

SDValue MipsTargetLowering::getCapGlobalReg(SelectionDAG &DAG, EVT Ty) const {
  assert(Ty.isFatPointer());
  assert(ABI.IsCheriPureCap());
  MachineFunction &MF = DAG.getMachineFunction();
  MipsFunctionInfo *FI = MF.getInfo<MipsFunctionInfo>();
  return DAG.getRegister(FI->getCapGlobalBaseRegForGlobalISel(MF), Ty);
}

SDValue MipsTargetLowering::getTargetNode(GlobalAddressSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
  return DAG.getTargetGlobalAddress(N->getGlobal(), SDLoc(N), Ty, 0, Flag);
}

SDValue MipsTargetLowering::getTargetNode(ExternalSymbolSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
  return DAG.getTargetExternalSymbol(N->getSymbol(), Ty, Flag);
}

SDValue MipsTargetLowering::getTargetNode(BlockAddressSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
  return DAG.getTargetBlockAddress(N->getBlockAddress(), Ty, 0, Flag);
}

SDValue MipsTargetLowering::getTargetNode(JumpTableSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
  return DAG.getTargetJumpTable(N->getIndex(), Ty, Flag);
}

SDValue MipsTargetLowering::getTargetNode(ConstantPoolSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
  return DAG.getTargetConstantPool(N->getConstVal(), Ty, N->getAlign(),
                                   N->getOffset(), Flag);
}

const char *MipsTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((MipsISD::NodeType)Opcode) {
  case MipsISD::FIRST_NUMBER:      break;
  case MipsISD::JmpLink:           return "MipsISD::JmpLink";
  case MipsISD::TailCall:          return "MipsISD::TailCall";
  case MipsISD::Highest:           return "MipsISD::Highest";
  case MipsISD::Higher:            return "MipsISD::Higher";
  case MipsISD::Hi:                return "MipsISD::Hi";
  case MipsISD::Lo:                return "MipsISD::Lo";
  case MipsISD::GotHi:             return "MipsISD::GotHi";
  case MipsISD::TlsHi:             return "MipsISD::TlsHi";
  case MipsISD::GPRel:             return "MipsISD::GPRel";
  case MipsISD::ThreadPointer:     return "MipsISD::ThreadPointer";
  case MipsISD::CapThreadPointer:  return "MipsISD::CapThreadPointer";
  case MipsISD::Ret:               return "MipsISD::Ret";
  case MipsISD::ERet:              return "MipsISD::ERet";
  case MipsISD::EH_RETURN:         return "MipsISD::EH_RETURN";
  case MipsISD::FMS:               return "MipsISD::FMS";
  case MipsISD::FPBrcond:          return "MipsISD::FPBrcond";
  case MipsISD::FPCmp:             return "MipsISD::FPCmp";
  case MipsISD::FSELECT:           return "MipsISD::FSELECT";
  case MipsISD::MTC1_D64:          return "MipsISD::MTC1_D64";
  case MipsISD::CMovFP_T:          return "MipsISD::CMovFP_T";
  case MipsISD::CMovFP_F:          return "MipsISD::CMovFP_F";
  case MipsISD::TruncIntFP:        return "MipsISD::TruncIntFP";
  case MipsISD::MFHI:              return "MipsISD::MFHI";
  case MipsISD::MFLO:              return "MipsISD::MFLO";
  case MipsISD::MTLOHI:            return "MipsISD::MTLOHI";
  case MipsISD::Mult:              return "MipsISD::Mult";
  case MipsISD::Multu:             return "MipsISD::Multu";
  case MipsISD::MAdd:              return "MipsISD::MAdd";
  case MipsISD::MAddu:             return "MipsISD::MAddu";
  case MipsISD::MSub:              return "MipsISD::MSub";
  case MipsISD::MSubu:             return "MipsISD::MSubu";
  case MipsISD::DivRem:            return "MipsISD::DivRem";
  case MipsISD::DivRemU:           return "MipsISD::DivRemU";
  case MipsISD::DivRem16:          return "MipsISD::DivRem16";
  case MipsISD::DivRemU16:         return "MipsISD::DivRemU16";
  case MipsISD::BuildPairF64:      return "MipsISD::BuildPairF64";
  case MipsISD::ExtractElementF64: return "MipsISD::ExtractElementF64";
  case MipsISD::Wrapper:           return "MipsISD::Wrapper";
  case MipsISD::WrapperCapOp:      return "MipsISD::WrapperCapOp";
  case MipsISD::DynAlloc:          return "MipsISD::DynAlloc";
  case MipsISD::Sync:              return "MipsISD::Sync";
  case MipsISD::Ext:               return "MipsISD::Ext";
  case MipsISD::Ins:               return "MipsISD::Ins";
  case MipsISD::CIns:              return "MipsISD::CIns";
  case MipsISD::LWL:               return "MipsISD::LWL";
  case MipsISD::LWR:               return "MipsISD::LWR";
  case MipsISD::SWL:               return "MipsISD::SWL";
  case MipsISD::SWR:               return "MipsISD::SWR";
  case MipsISD::LDL:               return "MipsISD::LDL";
  case MipsISD::LDR:               return "MipsISD::LDR";
  case MipsISD::SDL:               return "MipsISD::SDL";
  case MipsISD::SDR:               return "MipsISD::SDR";
  case MipsISD::EXTP:              return "MipsISD::EXTP";
  case MipsISD::EXTPDP:            return "MipsISD::EXTPDP";
  case MipsISD::EXTR_S_H:          return "MipsISD::EXTR_S_H";
  case MipsISD::EXTR_W:            return "MipsISD::EXTR_W";
  case MipsISD::EXTR_R_W:          return "MipsISD::EXTR_R_W";
  case MipsISD::EXTR_RS_W:         return "MipsISD::EXTR_RS_W";
  case MipsISD::SHILO:             return "MipsISD::SHILO";
  case MipsISD::MTHLIP:            return "MipsISD::MTHLIP";
  case MipsISD::MULSAQ_S_W_PH:     return "MipsISD::MULSAQ_S_W_PH";
  case MipsISD::MAQ_S_W_PHL:       return "MipsISD::MAQ_S_W_PHL";
  case MipsISD::MAQ_S_W_PHR:       return "MipsISD::MAQ_S_W_PHR";
  case MipsISD::MAQ_SA_W_PHL:      return "MipsISD::MAQ_SA_W_PHL";
  case MipsISD::MAQ_SA_W_PHR:      return "MipsISD::MAQ_SA_W_PHR";
  case MipsISD::DPAU_H_QBL:        return "MipsISD::DPAU_H_QBL";
  case MipsISD::DPAU_H_QBR:        return "MipsISD::DPAU_H_QBR";
  case MipsISD::DPSU_H_QBL:        return "MipsISD::DPSU_H_QBL";
  case MipsISD::DPSU_H_QBR:        return "MipsISD::DPSU_H_QBR";
  case MipsISD::DPAQ_S_W_PH:       return "MipsISD::DPAQ_S_W_PH";
  case MipsISD::DPSQ_S_W_PH:       return "MipsISD::DPSQ_S_W_PH";
  case MipsISD::DPAQ_SA_L_W:       return "MipsISD::DPAQ_SA_L_W";
  case MipsISD::DPSQ_SA_L_W:       return "MipsISD::DPSQ_SA_L_W";
  case MipsISD::DPA_W_PH:          return "MipsISD::DPA_W_PH";
  case MipsISD::DPS_W_PH:          return "MipsISD::DPS_W_PH";
  case MipsISD::DPAQX_S_W_PH:      return "MipsISD::DPAQX_S_W_PH";
  case MipsISD::DPAQX_SA_W_PH:     return "MipsISD::DPAQX_SA_W_PH";
  case MipsISD::DPAX_W_PH:         return "MipsISD::DPAX_W_PH";
  case MipsISD::DPSX_W_PH:         return "MipsISD::DPSX_W_PH";
  case MipsISD::DPSQX_S_W_PH:      return "MipsISD::DPSQX_S_W_PH";
  case MipsISD::DPSQX_SA_W_PH:     return "MipsISD::DPSQX_SA_W_PH";
  case MipsISD::MULSA_W_PH:        return "MipsISD::MULSA_W_PH";
  case MipsISD::MULT:              return "MipsISD::MULT";
  case MipsISD::MULTU:             return "MipsISD::MULTU";
  case MipsISD::MADD_DSP:          return "MipsISD::MADD_DSP";
  case MipsISD::MADDU_DSP:         return "MipsISD::MADDU_DSP";
  case MipsISD::MSUB_DSP:          return "MipsISD::MSUB_DSP";
  case MipsISD::MSUBU_DSP:         return "MipsISD::MSUBU_DSP";
  case MipsISD::SHLL_DSP:          return "MipsISD::SHLL_DSP";
  case MipsISD::SHRA_DSP:          return "MipsISD::SHRA_DSP";
  case MipsISD::SHRL_DSP:          return "MipsISD::SHRL_DSP";
  case MipsISD::SETCC_DSP:         return "MipsISD::SETCC_DSP";
  case MipsISD::SELECT_CC_DSP:     return "MipsISD::SELECT_CC_DSP";
  case MipsISD::VALL_ZERO:         return "MipsISD::VALL_ZERO";
  case MipsISD::VANY_ZERO:         return "MipsISD::VANY_ZERO";
  case MipsISD::VALL_NONZERO:      return "MipsISD::VALL_NONZERO";
  case MipsISD::VANY_NONZERO:      return "MipsISD::VANY_NONZERO";
  case MipsISD::VCEQ:              return "MipsISD::VCEQ";
  case MipsISD::VCLE_S:            return "MipsISD::VCLE_S";
  case MipsISD::VCLE_U:            return "MipsISD::VCLE_U";
  case MipsISD::VCLT_S:            return "MipsISD::VCLT_S";
  case MipsISD::VCLT_U:            return "MipsISD::VCLT_U";
  case MipsISD::VEXTRACT_SEXT_ELT: return "MipsISD::VEXTRACT_SEXT_ELT";
  case MipsISD::VEXTRACT_ZEXT_ELT: return "MipsISD::VEXTRACT_ZEXT_ELT";
  case MipsISD::VNOR:              return "MipsISD::VNOR";
  case MipsISD::VSHF:              return "MipsISD::VSHF";
  case MipsISD::SHF:               return "MipsISD::SHF";
  case MipsISD::ILVEV:             return "MipsISD::ILVEV";
  case MipsISD::ILVOD:             return "MipsISD::ILVOD";
  case MipsISD::ILVL:              return "MipsISD::ILVL";
  case MipsISD::ILVR:              return "MipsISD::ILVR";
  case MipsISD::PCKEV:             return "MipsISD::PCKEV";
  case MipsISD::PCKOD:             return "MipsISD::PCKOD";
  case MipsISD::INSVE:             return "MipsISD::INSVE";
  case MipsISD::CBTS:              return "MipsISD::CBTS";
  case MipsISD::CBTU:              return "MipsISD::CBTU";
  case MipsISD::STACKTOCAP:        return "MipsISD::STACKTOCAP";
  case MipsISD::CheriJmpLink:      return "MipsISD::CheriJmpLink";
  case MipsISD::CapJmpLink:        return "MipsISD::CapJmpLink";
  case MipsISD::CapRet:            return "MipsISD::CapRet";
  case MipsISD::CapTagGet:         return "MipsISD::CapTagGet";
  case MipsISD::CapSealedGet:      return "MipsISD::CapSealedGet";
  case MipsISD::CapSubsetTest:     return "MipsISD::CapSubsetTest";
  case MipsISD::CapEqualExact:     return "MipsISD::CapEqualExact";
  }
  return nullptr;
}

MipsTargetLowering::MipsTargetLowering(const MipsTargetMachine &TM,
                                       const MipsSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI), ABI(TM.getABI()) {
  CapType = STI.typeForCapabilities();
  CapTypeHasPreciseBounds = STI.isCheri256();
  assert(cheriCapabilityType().isFatPointer());

  // Mips does not have i1 type, so use i32 for
  // setcc operations results (slt, sgt, ...).
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrNegativeOneBooleanContent);
  // The cmp.cond.fmt instruction in MIPS32r6/MIPS64r6 uses 0 and -1 like MSA
  // does. Integer booleans still use 0 and 1.
  if (Subtarget.hasMips32r6())
    setBooleanContents(ZeroOrOneBooleanContent,
                       ZeroOrNegativeOneBooleanContent);

  // Load extented operations for i1 types must be promoted
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD,  VT, MVT::i1,  Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1,  Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1,  Promote);
  }

  // MIPS doesn't have extending float->double load/store.  Set LoadExtAction
  // for f32, f16
  for (MVT VT : MVT::fp_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::f32, Expand);
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::f16, Expand);
  }

  // Set LoadExtAction for f16 vectors to Expand
  for (MVT VT : MVT::fp_fixedlen_vector_valuetypes()) {
    MVT F16VT = MVT::getVectorVT(MVT::f16, VT.getVectorNumElements());
    if (F16VT.isValid())
      setLoadExtAction(ISD::EXTLOAD, VT, F16VT, Expand);
  }

  setTruncStoreAction(MVT::f32, MVT::f16, Expand);
  setTruncStoreAction(MVT::f64, MVT::f16, Expand);

  setTruncStoreAction(MVT::f64, MVT::f32, Expand);

  // Used by legalize types to correctly generate the setcc result.
  // Without this, every float setcc comes with a AND/OR with the result,
  // we don't want this, since the fpcmp result goes to a flag register,
  // which is used implicitly by brcond and select operations.
  AddPromotedToType(ISD::SETCC, MVT::i1, MVT::i32);

  // Trap is an invalid instruction sequence
  setOperationAction(ISD::TRAP,               MVT::Other, Legal);
  if (Subtarget.isCheri()) {
    setOperationAction(ISD::ADDRSPACECAST,    CapType, Custom);
    setOperationAction(ISD::ADDRSPACECAST,    MVT::i64, Custom);
    setOperationAction(ISD::ConstantPool, CapType, Custom);
  }

  // Mips Custom Operations
  setOperationAction(ISD::GlobalAddress,      CapType,    Custom);
  if (ABI.IsCheriPureCap())
    setOperationAction(ISD::GlobalTLSAddress, CapType,    Custom);
  if (ABI.IsCheriPureCap())
    setOperationAction(ISD::BR_JT,            MVT::Other, Custom);
  else
    setOperationAction(ISD::BR_JT,            MVT::Other, Expand);
  setOperationAction(ISD::GlobalAddress,      MVT::i32,   Custom);
  setOperationAction(ISD::BlockAddress,       MVT::i32,   Custom);
  setOperationAction(ISD::GlobalTLSAddress,   MVT::i32,   Custom);
  setOperationAction(ISD::JumpTable,          MVT::i32,   Custom);
  setOperationAction(ISD::ConstantPool,       MVT::i32,   Custom);
  setOperationAction(ISD::SELECT,             MVT::f32,   Custom);
  setOperationAction(ISD::SELECT,             MVT::f64,   Custom);
  setOperationAction(ISD::SELECT,             MVT::i32,   Custom);
  setOperationAction(ISD::SETCC,              MVT::f32,   Custom);
  setOperationAction(ISD::SETCC,              MVT::f64,   Custom);
  setOperationAction(ISD::BRCOND,             MVT::Other, Custom);
  setOperationAction(ISD::FCOPYSIGN,          MVT::f32,   Custom);
  setOperationAction(ISD::FCOPYSIGN,          MVT::f64,   Custom);
  setOperationAction(ISD::FP_TO_SINT,         MVT::i32,   Custom);

  if (!(TM.Options.NoNaNsFPMath || Subtarget.inAbs2008Mode())) {
    setOperationAction(ISD::FABS, MVT::f32, Custom);
    setOperationAction(ISD::FABS, MVT::f64, Custom);
  }

  if (Subtarget.isGP64bit()) {
    setOperationAction(ISD::GlobalAddress,      MVT::i64,   Custom);
    setOperationAction(ISD::BlockAddress,       MVT::i64,   Custom);
    // Handle blockaddress in AS200:
    if (ABI.IsCheriPureCap())
      setOperationAction(ISD::BlockAddress, CapType, Custom);

    setOperationAction(ISD::GlobalTLSAddress,   MVT::i64,   Custom);
    setOperationAction(ISD::JumpTable,          MVT::i64,   Custom);
    setOperationAction(ISD::ConstantPool,       MVT::i64,   Custom);
    //setOperationAction(ISD::SELECT,             MVT::i64,   Custom);
    setOperationAction(ISD::LOAD,               MVT::i64,   Custom);
    setOperationAction(ISD::STORE,              MVT::i64,   Custom);
    setOperationAction(ISD::FP_TO_SINT,         MVT::i64,   Custom);
    setOperationAction(ISD::SHL_PARTS,          MVT::i64,   Custom);
    setOperationAction(ISD::SRA_PARTS,          MVT::i64,   Custom);
    setOperationAction(ISD::SRL_PARTS,          MVT::i64,   Custom);
  }

  if (!Subtarget.isGP64bit()) {
    setOperationAction(ISD::SHL_PARTS,          MVT::i32,   Custom);
    setOperationAction(ISD::SRA_PARTS,          MVT::i32,   Custom);
    setOperationAction(ISD::SRL_PARTS,          MVT::i32,   Custom);
  }

  setOperationAction(ISD::EH_DWARF_CFA,         MVT::i32,   Custom);
  if (Subtarget.isGP64bit())
    setOperationAction(ISD::EH_DWARF_CFA,       MVT::i64,   Custom);

  setOperationAction(ISD::SDIV, MVT::i32, Expand);
  setOperationAction(ISD::SREM, MVT::i32, Expand);
  setOperationAction(ISD::UDIV, MVT::i32, Expand);
  setOperationAction(ISD::UREM, MVT::i32, Expand);
  setOperationAction(ISD::SDIV, MVT::i64, Expand);
  setOperationAction(ISD::SREM, MVT::i64, Expand);
  setOperationAction(ISD::UDIV, MVT::i64, Expand);
  setOperationAction(ISD::UREM, MVT::i64, Expand);

  // Operations not directly supported by Mips.
  setOperationAction(ISD::BR_CC,             MVT::f32,   Expand);
  setOperationAction(ISD::BR_CC,             MVT::f64,   Expand);
  setOperationAction(ISD::BR_CC,             MVT::i32,   Expand);
  setOperationAction(ISD::BR_CC,             MVT::i64,   Expand);
  setOperationAction(ISD::SELECT_CC,         MVT::i32,   Expand);
  setOperationAction(ISD::SELECT_CC,         MVT::i64,   Expand);
  setOperationAction(ISD::SELECT_CC,         MVT::f32,   Expand);
  setOperationAction(ISD::SELECT_CC,         MVT::f64,   Expand);
  setOperationAction(ISD::UINT_TO_FP,        MVT::i32,   Expand);
  setOperationAction(ISD::UINT_TO_FP,        MVT::i64,   Expand);
  setOperationAction(ISD::FP_TO_UINT,        MVT::i32,   Expand);
  setOperationAction(ISD::FP_TO_UINT,        MVT::i64,   Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1,    Expand);
  if (Subtarget.hasCnMips()) {
    setOperationAction(ISD::CTPOP,           MVT::i32,   Legal);
    setOperationAction(ISD::CTPOP,           MVT::i64,   Legal);
  } else {
    setOperationAction(ISD::CTPOP,           MVT::i32,   Expand);
    setOperationAction(ISD::CTPOP,           MVT::i64,   Expand);
  }
  setOperationAction(ISD::CTTZ,              MVT::i32,   Expand);
  setOperationAction(ISD::CTTZ,              MVT::i64,   Expand);
  setOperationAction(ISD::ROTL,              MVT::i32,   Expand);
  setOperationAction(ISD::ROTL,              MVT::i64,   Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32,  Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i64,  Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, CapType,  Expand);

  if (!Subtarget.hasMips32r2())
    setOperationAction(ISD::ROTR, MVT::i32,   Expand);

  if (!Subtarget.hasMips64r2())
    setOperationAction(ISD::ROTR, MVT::i64,   Expand);

  setOperationAction(ISD::FSIN,              MVT::f32,   Expand);
  setOperationAction(ISD::FSIN,              MVT::f64,   Expand);
  setOperationAction(ISD::FCOS,              MVT::f32,   Expand);
  setOperationAction(ISD::FCOS,              MVT::f64,   Expand);
  setOperationAction(ISD::FSINCOS,           MVT::f32,   Expand);
  setOperationAction(ISD::FSINCOS,           MVT::f64,   Expand);
  setOperationAction(ISD::FPOW,              MVT::f32,   Expand);
  setOperationAction(ISD::FPOW,              MVT::f64,   Expand);
  setOperationAction(ISD::FLOG,              MVT::f32,   Expand);
  setOperationAction(ISD::FLOG2,             MVT::f32,   Expand);
  setOperationAction(ISD::FLOG10,            MVT::f32,   Expand);
  setOperationAction(ISD::FEXP,              MVT::f32,   Expand);
  setOperationAction(ISD::FMA,               MVT::f32,   Expand);
  setOperationAction(ISD::FMA,               MVT::f64,   Expand);
  setOperationAction(ISD::FREM,              MVT::f32,   Expand);
  setOperationAction(ISD::FREM,              MVT::f64,   Expand);

  // Lower f16 conversion operations into library calls
  setOperationAction(ISD::FP16_TO_FP,        MVT::f32,   Expand);
  setOperationAction(ISD::FP_TO_FP16,        MVT::f32,   Expand);
  setOperationAction(ISD::FP16_TO_FP,        MVT::f64,   Expand);
  setOperationAction(ISD::FP_TO_FP16,        MVT::f64,   Expand);

  setOperationAction(ISD::EH_RETURN, MVT::Other, Custom);

  setOperationAction(ISD::VASTART,           MVT::Other, Custom);
  setOperationAction(ISD::VAARG,             MVT::Other, Custom);
  if (ABI.IsCheriPureCap())
    setOperationAction(ISD::VACOPY,          MVT::Other, Custom);
  else
    setOperationAction(ISD::VACOPY,          MVT::Other, Expand);
  setOperationAction(ISD::VAEND,             MVT::Other, Expand);

  // Use the default for now
  setOperationAction(ISD::STACKSAVE,         MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE,      MVT::Other, Expand);

  if (!Subtarget.isGP64bit()) {
    setOperationAction(ISD::ATOMIC_LOAD,     MVT::i64,   Expand);
    setOperationAction(ISD::ATOMIC_STORE,    MVT::i64,   Expand);
  }

  if (!Subtarget.hasMips32r2()) {
    setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8,  Expand);
    setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  }

  // MIPS16 lacks MIPS32's clz and clo instructions.
  if (!Subtarget.hasMips32() || Subtarget.inMips16Mode())
    setOperationAction(ISD::CTLZ, MVT::i32, Expand);
  if (!Subtarget.hasMips64())
    setOperationAction(ISD::CTLZ, MVT::i64, Expand);

  if (!Subtarget.hasMips32r2())
    setOperationAction(ISD::BSWAP, MVT::i32, Expand);
  if (!Subtarget.hasMips64r2())
    setOperationAction(ISD::BSWAP, MVT::i64, Expand);

  if (Subtarget.isGP64bit()) {
    setLoadExtAction(ISD::SEXTLOAD, MVT::i64, MVT::i32, Custom);
    setLoadExtAction(ISD::ZEXTLOAD, MVT::i64, MVT::i32, Custom);
    setLoadExtAction(ISD::EXTLOAD, MVT::i64, MVT::i32, Custom);
    setTruncStoreAction(MVT::i64, MVT::i32, Custom);
  }

  setOperationAction(ISD::TRAP, MVT::Other, Legal);

  setTargetDAGCombine(ISD::SDIVREM);
  setTargetDAGCombine(ISD::UDIVREM);
  setTargetDAGCombine(ISD::SELECT);
  setTargetDAGCombine(ISD::AND);
  setTargetDAGCombine(ISD::OR);
  setTargetDAGCombine(ISD::ADD);
  setTargetDAGCombine(ISD::SUB);
  setTargetDAGCombine(ISD::AssertZext);
  setTargetDAGCombine(ISD::SHL);

  // Some CHERI intrinsics return i1, which isn't legal, so we have to custom
  // lower them in the DAG combine phase before the first type legalization
  // pass.
  if (Subtarget.isCheri()) {
    // We also fold Incoffsets to andaddr if possible
    setTargetDAGCombine(ISD::INTRINSIC_WO_CHAIN);
    setTargetDAGCombine(ISD::PTRADD);
  }

  if (ABI.IsO32()) {
    // These libcalls are not available in 32-bit.
    setLibcallName(RTLIB::SHL_I128, nullptr);
    setLibcallName(RTLIB::SRL_I128, nullptr);
    setLibcallName(RTLIB::SRA_I128, nullptr);
  }

  setMinFunctionAlignment(Subtarget.isGP64bit() ? Align(8) : Align(4));

  // The arguments on the stack are defined in terms of 4-byte slots on O32
  // and 8-byte slots on N32/N64.
  setMinStackArgumentAlignment((ABI.IsN32() || ABI.IsN64()) ? Align(8)
                                                            : Align(4));

  setStackPointerRegisterToSaveRestore(ABI.GetStackPtr());

  MaxStoresPerMemcpy = 16;

  // Set these thresholds low enough for CHERI that we will never inline a
  // memcpy that can contain a capability.
  if (Subtarget.isCheri())
    MaxStoresPerMemcpy = MaxStoresPerMemmove = 15;

  if (Subtarget.isCheri()) {
    SupportsAtomicCapabilityOperations = true;
    NullCapabilityRegister = Mips::CNULL;
  }

  isMicroMips = Subtarget.inMicroMipsMode();
}

const MipsTargetLowering *
MipsTargetLowering::create(const MipsTargetMachine &TM,
                           const MipsSubtarget &STI) {
  if (STI.inMips16Mode())
    return createMips16TargetLowering(TM, STI);

  return createMipsSETargetLowering(TM, STI);
}

// Create a fast isel object.
FastISel *
MipsTargetLowering::createFastISel(FunctionLoweringInfo &funcInfo,
                                  const TargetLibraryInfo *libInfo) const {
  const MipsTargetMachine &TM =
      static_cast<const MipsTargetMachine &>(funcInfo.MF->getTarget());

  // We support only the standard encoding [MIPS32,MIPS32R5] ISAs.
  bool UseFastISel = TM.Options.EnableFastISel && Subtarget.hasMips32() &&
                     !Subtarget.hasMips32r6() && !Subtarget.inMips16Mode() &&
                     !Subtarget.inMicroMipsMode();

  // Disable if either of the following is true:
  // We do not generate PIC, the ABI is not O32, XGOT is being used.
  if (!TM.isPositionIndependent() || !TM.getABI().IsO32() ||
      Subtarget.useXGOT())
    UseFastISel = false;

  return UseFastISel ? Mips::createFastISel(funcInfo, libInfo) : nullptr;
}

EVT MipsTargetLowering::getSetCCResultType(const DataLayout &, LLVMContext &,
                                           EVT VT) const {
  if (!VT.isVector())
    return MVT::i32;
  return VT.changeVectorElementTypeToInteger();
}

static SDValue performDivRemCombine(SDNode *N, SelectionDAG &DAG,
                                    TargetLowering::DAGCombinerInfo &DCI,
                                    const MipsSubtarget &Subtarget) {
  if (DCI.isBeforeLegalizeOps())
    return SDValue();

  EVT Ty = N->getValueType(0);
  unsigned LO = (Ty == MVT::i32) ? Mips::LO0 : Mips::LO0_64;
  unsigned HI = (Ty == MVT::i32) ? Mips::HI0 : Mips::HI0_64;
  unsigned Opc = N->getOpcode() == ISD::SDIVREM ? MipsISD::DivRem16 :
                                                  MipsISD::DivRemU16;
  SDLoc DL(N);

  SDValue DivRem = DAG.getNode(Opc, DL, MVT::Glue,
                               N->getOperand(0), N->getOperand(1));
  SDValue InChain = DAG.getEntryNode();
  SDValue InGlue = DivRem;

  // insert MFLO
  if (N->hasAnyUseOfValue(0)) {
    SDValue CopyFromLo = DAG.getCopyFromReg(InChain, DL, LO, Ty,
                                            InGlue);
    DAG.ReplaceAllUsesOfValueWith(SDValue(N, 0), CopyFromLo);
    InChain = CopyFromLo.getValue(1);
    InGlue = CopyFromLo.getValue(2);
  }

  // insert MFHI
  if (N->hasAnyUseOfValue(1)) {
    SDValue CopyFromHi = DAG.getCopyFromReg(InChain, DL,
                                            HI, Ty, InGlue);
    DAG.ReplaceAllUsesOfValueWith(SDValue(N, 1), CopyFromHi);
  }

  return SDValue();
}

static Mips::CondCode condCodeToFCC(ISD::CondCode CC) {
  switch (CC) {
  default: llvm_unreachable("Unknown fp condition code!");
  case ISD::SETEQ:
  case ISD::SETOEQ: return Mips::FCOND_OEQ;
  case ISD::SETUNE: return Mips::FCOND_UNE;
  case ISD::SETLT:
  case ISD::SETOLT: return Mips::FCOND_OLT;
  case ISD::SETGT:
  case ISD::SETOGT: return Mips::FCOND_OGT;
  case ISD::SETLE:
  case ISD::SETOLE: return Mips::FCOND_OLE;
  case ISD::SETGE:
  case ISD::SETOGE: return Mips::FCOND_OGE;
  case ISD::SETULT: return Mips::FCOND_ULT;
  case ISD::SETULE: return Mips::FCOND_ULE;
  case ISD::SETUGT: return Mips::FCOND_UGT;
  case ISD::SETUGE: return Mips::FCOND_UGE;
  case ISD::SETUO:  return Mips::FCOND_UN;
  case ISD::SETO:   return Mips::FCOND_OR;
  case ISD::SETNE:
  case ISD::SETONE: return Mips::FCOND_ONE;
  case ISD::SETUEQ: return Mips::FCOND_UEQ;
  }
}

/// This function returns true if the floating point conditional branches and
/// conditional moves which use condition code CC should be inverted.
static bool invertFPCondCodeUser(Mips::CondCode CC) {
  if (CC >= Mips::FCOND_F && CC <= Mips::FCOND_NGT)
    return false;

  assert((CC >= Mips::FCOND_T && CC <= Mips::FCOND_GT) &&
         "Illegal Condition Code");

  return true;
}

// Creates and returns an FPCmp node from a setcc node.
// Returns Op if setcc is not a floating point comparison.
static SDValue createFPCmp(SelectionDAG &DAG, const SDValue &Op) {
  // must be a SETCC node
  if (Op.getOpcode() != ISD::SETCC)
    return Op;

  SDValue LHS = Op.getOperand(0);

  if (!LHS.getValueType().isFloatingPoint())
    return Op;

  SDValue RHS = Op.getOperand(1);
  SDLoc DL(Op);

  // Assume the 3rd operand is a CondCodeSDNode. Add code to check the type of
  // node if necessary.
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();

  return DAG.getNode(MipsISD::FPCmp, DL, MVT::Glue, LHS, RHS,
                     DAG.getConstant(condCodeToFCC(CC), DL, MVT::i32));
}

// Creates and returns a CMovFPT/F node.
static SDValue createCMovFP(SelectionDAG &DAG, SDValue Cond, SDValue True,
                            SDValue False, const SDLoc &DL) {
  ConstantSDNode *CC = cast<ConstantSDNode>(Cond.getOperand(2));
  bool invert = invertFPCondCodeUser((Mips::CondCode)CC->getSExtValue());
  SDValue FCC0 = DAG.getRegister(Mips::FCC0, MVT::i32);

  return DAG.getNode((invert ? MipsISD::CMovFP_F : MipsISD::CMovFP_T), DL,
                     True.getValueType(), True, FCC0, False, Cond);
}

static SDValue performSELECTCombine(SDNode *N, SelectionDAG &DAG,
                                    TargetLowering::DAGCombinerInfo &DCI,
                                    const MipsSubtarget &Subtarget) {
  if (DCI.isBeforeLegalizeOps())
    return SDValue();

  SDValue SetCC = N->getOperand(0);

  if ((SetCC.getOpcode() != ISD::SETCC) ||
      !SetCC.getOperand(0).getValueType().isInteger())
    return SDValue();

  SDValue False = N->getOperand(2);
  EVT FalseTy = False.getValueType();

  if (!FalseTy.isInteger())
    return SDValue();

  ConstantSDNode *FalseC = dyn_cast<ConstantSDNode>(False);

  // If the RHS (False) is 0, we swap the order of the operands
  // of ISD::SELECT (obviously also inverting the condition) so that we can
  // take advantage of conditional moves using the $0 register.
  // Example:
  //   return (a != 0) ? x : 0;
  //     load $reg, x
  //     movz $reg, $0, a
  if (!FalseC)
    return SDValue();

  const SDLoc DL(N);

  if (!FalseC->getZExtValue()) {
    ISD::CondCode CC = cast<CondCodeSDNode>(SetCC.getOperand(2))->get();
    SDValue True = N->getOperand(1);

    SetCC = DAG.getSetCC(DL, SetCC.getValueType(), SetCC.getOperand(0),
                         SetCC.getOperand(1),
                         ISD::getSetCCInverse(CC, SetCC.getValueType()));

    return DAG.getNode(ISD::SELECT, DL, FalseTy, SetCC, False, True);
  }

  // If both operands are integer constants there's a possibility that we
  // can do some interesting optimizations.
  SDValue True = N->getOperand(1);
  ConstantSDNode *TrueC = dyn_cast<ConstantSDNode>(True);

  if (!TrueC || !True.getValueType().isInteger())
    return SDValue();

  // We'll also ignore MVT::i64 operands as this optimizations proves
  // to be ineffective because of the required sign extensions as the result
  // of a SETCC operator is always MVT::i32 for non-vector types.
  if (True.getValueType() == MVT::i64)
    return SDValue();

  int64_t Diff = TrueC->getSExtValue() - FalseC->getSExtValue();

  // 1)  (a < x) ? y : y-1
  //  slti $reg1, a, x
  //  addiu $reg2, $reg1, y-1
  if (Diff == 1)
    return DAG.getNode(ISD::ADD, DL, SetCC.getValueType(), SetCC, False);

  // 2)  (a < x) ? y-1 : y
  //  slti $reg1, a, x
  //  xor $reg1, $reg1, 1
  //  addiu $reg2, $reg1, y-1
  if (Diff == -1) {
    ISD::CondCode CC = cast<CondCodeSDNode>(SetCC.getOperand(2))->get();
    assert(True.getValueType().isInteger());
    SetCC = DAG.getSetCC(DL, SetCC.getValueType(), SetCC.getOperand(0),
                         SetCC.getOperand(1),
                         ISD::getSetCCInverse(CC, SetCC.getValueType()));
    return DAG.getNode(ISD::ADD, DL, SetCC.getValueType(), SetCC, True);
  }

  // Could not optimize.
  return SDValue();
}

static SDValue performCMovFPCombine(SDNode *N, SelectionDAG &DAG,
                                    TargetLowering::DAGCombinerInfo &DCI,
                                    const MipsSubtarget &Subtarget) {
  if (DCI.isBeforeLegalizeOps())
    return SDValue();

  SDValue ValueIfTrue = N->getOperand(0), ValueIfFalse = N->getOperand(2);

  ConstantSDNode *FalseC = dyn_cast<ConstantSDNode>(ValueIfFalse);
  if (!FalseC || FalseC->getZExtValue())
    return SDValue();

  // Since RHS (False) is 0, we swap the order of the True/False operands
  // (obviously also inverting the condition) so that we can
  // take advantage of conditional moves using the $0 register.
  // Example:
  //   return (a != 0) ? x : 0;
  //     load $reg, x
  //     movz $reg, $0, a
  unsigned Opc = (N->getOpcode() == MipsISD::CMovFP_T) ? MipsISD::CMovFP_F :
                                                         MipsISD::CMovFP_T;

  SDValue FCC = N->getOperand(1), Glue = N->getOperand(3);
  return DAG.getNode(Opc, SDLoc(N), ValueIfFalse.getValueType(),
                     ValueIfFalse, FCC, ValueIfTrue, Glue);
}

static SDValue performANDCombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const MipsSubtarget &Subtarget) {
  if (DCI.isBeforeLegalizeOps() || !Subtarget.hasExtractInsert())
    return SDValue();

  SDValue FirstOperand = N->getOperand(0);
  unsigned FirstOperandOpc = FirstOperand.getOpcode();
  SDValue Mask = N->getOperand(1);
  EVT ValTy = N->getValueType(0);
  SDLoc DL(N);

  uint64_t Pos = 0, SMPos, SMSize;
  ConstantSDNode *CN;
  SDValue NewOperand;
  unsigned Opc;

  // Op's second operand must be a shifted mask.
  if (!(CN = dyn_cast<ConstantSDNode>(Mask)) ||
      !isShiftedMask(CN->getZExtValue(), SMPos, SMSize))
    return SDValue();

  if (FirstOperandOpc == ISD::SRA || FirstOperandOpc == ISD::SRL) {
    // Pattern match EXT.
    //  $dst = and ((sra or srl) $src , pos), (2**size - 1)
    //  => ext $dst, $src, pos, size

    // The second operand of the shift must be an immediate.
    if (!(CN = dyn_cast<ConstantSDNode>(FirstOperand.getOperand(1))))
      return SDValue();

    Pos = CN->getZExtValue();

    // Return if the shifted mask does not start at bit 0 or the sum of its size
    // and Pos exceeds the word's size.
    if (SMPos != 0 || Pos + SMSize > ValTy.getSizeInBits())
      return SDValue();

    Opc = MipsISD::Ext;
    NewOperand = FirstOperand.getOperand(0);
  } else if (FirstOperandOpc == ISD::SHL && Subtarget.hasCnMips()) {
    // Pattern match CINS.
    //  $dst = and (shl $src , pos), mask
    //  => cins $dst, $src, pos, size
    // mask is a shifted mask with consecutive 1's, pos = shift amount,
    // size = population count.

    // The second operand of the shift must be an immediate.
    if (!(CN = dyn_cast<ConstantSDNode>(FirstOperand.getOperand(1))))
      return SDValue();

    Pos = CN->getZExtValue();

    if (SMPos != Pos || Pos >= ValTy.getSizeInBits() || SMSize >= 32 ||
        Pos + SMSize > ValTy.getSizeInBits())
      return SDValue();

    NewOperand = FirstOperand.getOperand(0);
    // SMSize is 'location' (position) in this case, not size.
    SMSize--;
    Opc = MipsISD::CIns;
  } else {
    // Pattern match EXT.
    //  $dst = and $src, (2**size - 1) , if size > 16
    //  => ext $dst, $src, pos, size , pos = 0

    // If the mask is <= 0xffff, andi can be used instead.
    if (CN->getZExtValue() <= 0xffff)
      return SDValue();

    // Return if the mask doesn't start at position 0.
    if (SMPos)
      return SDValue();

    Opc = MipsISD::Ext;
    NewOperand = FirstOperand;
  }
  return DAG.getNode(Opc, DL, ValTy, NewOperand,
                     DAG.getConstant(Pos, DL, MVT::i32),
                     DAG.getConstant(SMSize, DL, MVT::i32));
}

static SDValue performORCombine(SDNode *N, SelectionDAG &DAG,
                                TargetLowering::DAGCombinerInfo &DCI,
                                const MipsSubtarget &Subtarget) {
  // Pattern match INS.
  //  $dst = or (and $src1 , mask0), (and (shl $src, pos), mask1),
  //  where mask1 = (2**size - 1) << pos, mask0 = ~mask1
  //  => ins $dst, $src, size, pos, $src1
  if (DCI.isBeforeLegalizeOps() || !Subtarget.hasExtractInsert())
    return SDValue();

  SDValue And0 = N->getOperand(0), And1 = N->getOperand(1);
  uint64_t SMPos0, SMSize0, SMPos1, SMSize1;
  ConstantSDNode *CN, *CN1;

  // See if Op's first operand matches (and $src1 , mask0).
  if (And0.getOpcode() != ISD::AND)
    return SDValue();

  if (!(CN = dyn_cast<ConstantSDNode>(And0.getOperand(1))) ||
      !isShiftedMask(~CN->getSExtValue(), SMPos0, SMSize0))
    return SDValue();

  // See if Op's second operand matches (and (shl $src, pos), mask1).
  if (And1.getOpcode() == ISD::AND &&
      And1.getOperand(0).getOpcode() == ISD::SHL) {

    if (!(CN = dyn_cast<ConstantSDNode>(And1.getOperand(1))) ||
        !isShiftedMask(CN->getZExtValue(), SMPos1, SMSize1))
      return SDValue();

    // The shift masks must have the same position and size.
    if (SMPos0 != SMPos1 || SMSize0 != SMSize1)
      return SDValue();

    SDValue Shl = And1.getOperand(0);

    if (!(CN = dyn_cast<ConstantSDNode>(Shl.getOperand(1))))
      return SDValue();

    unsigned Shamt = CN->getZExtValue();

    // Return if the shift amount and the first bit position of mask are not the
    // same.
    EVT ValTy = N->getValueType(0);
    if ((Shamt != SMPos0) || (SMPos0 + SMSize0 > ValTy.getSizeInBits()))
      return SDValue();

    SDLoc DL(N);
    return DAG.getNode(MipsISD::Ins, DL, ValTy, Shl.getOperand(0),
                       DAG.getConstant(SMPos0, DL, MVT::i32),
                       DAG.getConstant(SMSize0, DL, MVT::i32),
                       And0.getOperand(0));
  } else {
    // Pattern match DINS.
    //  $dst = or (and $src, mask0), mask1
    //  where mask0 = ((1 << SMSize0) -1) << SMPos0
    //  => dins $dst, $src, pos, size
    if (~CN->getSExtValue() == ((((int64_t)1 << SMSize0) - 1) << SMPos0) &&
        ((SMSize0 + SMPos0 <= 64 && Subtarget.hasMips64r2()) ||
         (SMSize0 + SMPos0 <= 32))) {
      // Check if AND instruction has constant as argument
      bool isConstCase = And1.getOpcode() != ISD::AND;
      if (And1.getOpcode() == ISD::AND) {
        if (!(CN1 = dyn_cast<ConstantSDNode>(And1->getOperand(1))))
          return SDValue();
      } else {
        if (!(CN1 = dyn_cast<ConstantSDNode>(N->getOperand(1))))
          return SDValue();
      }
      // Don't generate INS if constant OR operand doesn't fit into bits
      // cleared by constant AND operand.
      if (CN->getSExtValue() & CN1->getSExtValue())
        return SDValue();

      SDLoc DL(N);
      EVT ValTy = N->getOperand(0)->getValueType(0);
      SDValue Const1;
      SDValue SrlX;
      if (!isConstCase) {
        Const1 = DAG.getConstant(SMPos0, DL, MVT::i32);
        SrlX = DAG.getNode(ISD::SRL, DL, And1->getValueType(0), And1, Const1);
      }
      return DAG.getNode(
          MipsISD::Ins, DL, N->getValueType(0),
          isConstCase
              ? DAG.getConstant(CN1->getSExtValue() >> SMPos0, DL, ValTy)
              : SrlX,
          DAG.getConstant(SMPos0, DL, MVT::i32),
          DAG.getConstant(ValTy.getSizeInBits() / 8 < 8 ? SMSize0 & 31
                                                        : SMSize0,
                          DL, MVT::i32),
          And0->getOperand(0));

    }
    return SDValue();
  }
}

static SDValue performMADD_MSUBCombine(SDNode *ROOTNode, SelectionDAG &CurDAG,
                                       const MipsSubtarget &Subtarget) {
  // ROOTNode must have a multiplication as an operand for the match to be
  // successful.
  if (ROOTNode->getOperand(0).getOpcode() != ISD::MUL &&
      ROOTNode->getOperand(1).getOpcode() != ISD::MUL)
    return SDValue();

  // We don't handle vector types here.
  if (ROOTNode->getValueType(0).isVector())
    return SDValue();

  // For MIPS64, madd / msub instructions are inefficent to use with 64 bit
  // arithmetic. E.g.
  // (add (mul a b) c) =>
  //   let res = (madd (mthi (drotr c 32))x(mtlo c) a b) in
  //   MIPS64:   (or (dsll (mfhi res) 32) (dsrl (dsll (mflo res) 32) 32)
  //   or
  //   MIPS64R2: (dins (mflo res) (mfhi res) 32 32)
  //
  // The overhead of setting up the Hi/Lo registers and reassembling the
  // result makes this a dubious optimzation for MIPS64. The core of the
  // problem is that Hi/Lo contain the upper and lower 32 bits of the
  // operand and result.
  //
  // It requires a chain of 4 add/mul for MIPS64R2 to get better code
  // density than doing it naively, 5 for MIPS64. Additionally, using
  // madd/msub on MIPS64 requires the operands actually be 32 bit sign
  // extended operands, not true 64 bit values.
  //
  // FIXME: For the moment, disable this completely for MIPS64.
  if (Subtarget.hasMips64())
    return SDValue();

  SDValue Mult = ROOTNode->getOperand(0).getOpcode() == ISD::MUL
                     ? ROOTNode->getOperand(0)
                     : ROOTNode->getOperand(1);

  SDValue AddOperand = ROOTNode->getOperand(0).getOpcode() == ISD::MUL
                     ? ROOTNode->getOperand(1)
                     : ROOTNode->getOperand(0);

  // Transform this to a MADD only if the user of this node is the add.
  // If there are other users of the mul, this function returns here.
  if (!Mult.hasOneUse())
    return SDValue();

  // maddu and madd are unusual instructions in that on MIPS64 bits 63..31
  // must be in canonical form, i.e. sign extended. For MIPS32, the operands
  // of the multiply must have 32 or more sign bits, otherwise we cannot
  // perform this optimization. We have to check this here as we're performing
  // this optimization pre-legalization.
  SDValue MultLHS = Mult->getOperand(0);
  SDValue MultRHS = Mult->getOperand(1);

  bool IsSigned = MultLHS->getOpcode() == ISD::SIGN_EXTEND &&
                  MultRHS->getOpcode() == ISD::SIGN_EXTEND;
  bool IsUnsigned = MultLHS->getOpcode() == ISD::ZERO_EXTEND &&
                    MultRHS->getOpcode() == ISD::ZERO_EXTEND;

  if (!IsSigned && !IsUnsigned)
    return SDValue();

  // Initialize accumulator.
  SDLoc DL(ROOTNode);
  SDValue TopHalf;
  SDValue BottomHalf;
  BottomHalf = CurDAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i32, AddOperand,
                              CurDAG.getIntPtrConstant(0, DL));

  TopHalf = CurDAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i32, AddOperand,
                           CurDAG.getIntPtrConstant(1, DL));
  SDValue ACCIn = CurDAG.getNode(MipsISD::MTLOHI, DL, MVT::Untyped,
                                  BottomHalf,
                                  TopHalf);

  // Create MipsMAdd(u) / MipsMSub(u) node.
  bool IsAdd = ROOTNode->getOpcode() == ISD::ADD;
  unsigned Opcode = IsAdd ? (IsUnsigned ? MipsISD::MAddu : MipsISD::MAdd)
                          : (IsUnsigned ? MipsISD::MSubu : MipsISD::MSub);
  SDValue MAddOps[3] = {
      CurDAG.getNode(ISD::TRUNCATE, DL, MVT::i32, Mult->getOperand(0)),
      CurDAG.getNode(ISD::TRUNCATE, DL, MVT::i32, Mult->getOperand(1)), ACCIn};
  EVT VTs[2] = {MVT::i32, MVT::i32};
  SDValue MAdd = CurDAG.getNode(Opcode, DL, VTs, MAddOps);

  SDValue ResLo = CurDAG.getNode(MipsISD::MFLO, DL, MVT::i32, MAdd);
  SDValue ResHi = CurDAG.getNode(MipsISD::MFHI, DL, MVT::i32, MAdd);
  SDValue Combined =
      CurDAG.getNode(ISD::BUILD_PAIR, DL, MVT::i64, ResLo, ResHi);
  return Combined;
}

static SDValue performSUBCombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const MipsSubtarget &Subtarget) {
  // (sub v0 (mul v1, v2)) => (msub v1, v2, v0)
  if (DCI.isBeforeLegalizeOps()) {
    if (Subtarget.hasMips32() && !Subtarget.hasMips32r6() &&
        !Subtarget.inMips16Mode() && N->getValueType(0) == MVT::i64)
      return performMADD_MSUBCombine(N, DAG, Subtarget);

    return SDValue();
  }

  return SDValue();
}

static SDValue performADDCombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const MipsSubtarget &Subtarget) {
  // (add v0 (mul v1, v2)) => (madd v1, v2, v0)
  if (DCI.isBeforeLegalizeOps()) {
    if (Subtarget.hasMips32() && !Subtarget.hasMips32r6() &&
        !Subtarget.inMips16Mode() && N->getValueType(0) == MVT::i64)
      return performMADD_MSUBCombine(N, DAG, Subtarget);

    return SDValue();
  }

  // (add v0, (add v1, abs_lo(tjt))) => (add (add v0, v1), abs_lo(tjt))
  SDValue Add = N->getOperand(1);

  if (Add.getOpcode() != ISD::ADD)
    return SDValue();

  SDValue Lo = Add.getOperand(1);

  if ((Lo.getOpcode() != MipsISD::Lo) ||
      (Lo.getOperand(0).getOpcode() != ISD::TargetJumpTable))
    return SDValue();

  EVT ValTy = N->getValueType(0);
  SDLoc DL(N);

  SDValue Add1 = DAG.getNode(ISD::ADD, DL, ValTy, N->getOperand(0),
                             Add.getOperand(0));
  return DAG.getNode(ISD::ADD, DL, ValTy, Add1, Lo);
}

static SDValue performSHLCombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const MipsSubtarget &Subtarget) {
  // Pattern match CINS.
  //  $dst = shl (and $src , imm), pos
  //  => cins $dst, $src, pos, size

  if (DCI.isBeforeLegalizeOps() || !Subtarget.hasCnMips())
    return SDValue();

  SDValue FirstOperand = N->getOperand(0);
  unsigned FirstOperandOpc = FirstOperand.getOpcode();
  SDValue SecondOperand = N->getOperand(1);
  EVT ValTy = N->getValueType(0);
  SDLoc DL(N);

  uint64_t Pos = 0, SMPos, SMSize;
  ConstantSDNode *CN;
  SDValue NewOperand;

  // The second operand of the shift must be an immediate.
  if (!(CN = dyn_cast<ConstantSDNode>(SecondOperand)))
    return SDValue();

  Pos = CN->getZExtValue();

  if (Pos >= ValTy.getSizeInBits())
    return SDValue();

  if (FirstOperandOpc != ISD::AND)
    return SDValue();

  // AND's second operand must be a shifted mask.
  if (!(CN = dyn_cast<ConstantSDNode>(FirstOperand.getOperand(1))) ||
      !isShiftedMask(CN->getZExtValue(), SMPos, SMSize))
    return SDValue();

  // Return if the shifted mask does not start at bit 0 or the sum of its size
  // and Pos exceeds the word's size.
  if (SMPos != 0 || SMSize > 32 || Pos + SMSize > ValTy.getSizeInBits())
    return SDValue();

  NewOperand = FirstOperand.getOperand(0);
  // SMSize is 'location' (position) in this case, not size.
  SMSize--;

  return DAG.getNode(MipsISD::CIns, DL, ValTy, NewOperand,
                     DAG.getConstant(Pos, DL, MVT::i32),
                     DAG.getConstant(SMSize, DL, MVT::i32));
}

static inline bool isOptNone(const MachineFunction &MF) {
    return MF.getFunction().hasFnAttribute(Attribute::OptimizeNone) ||
           MF.getTarget().getOptLevel() == CodeGenOpt::None;
}

static SDValue
performCIncOffsetToCandAddrCombine(SDNode *N, SelectionDAG &DAG,
                                   TargetLowering::DAGCombinerInfo &DCI,
                                   const MipsSubtarget &Subtarget) {
  // Don't do this combine for optnone functions
  if (isOptNone(DAG.getMachineFunction()))
    return SDValue();

  // We could do this in tablegen too, but doing it here might enable
  // reuse of constants instead of always emitting the NOR64. It also means
  // we don't have to duplicate the pattern for ptraddr and the incoffset intrinsic
  // Tablegen pattern:
  // def : Pat<(ptradd CheriOpnd: $r1, (sub (i64 0), (and (int_cheri_cap_address_get CheriOpnd: $r1), GPR64Opnd: $mask))),
  //           (CAndAddr $r1, (XORi64 (i64 -1), GPR64Opnd:$mask))>;

  // Could be ptraddr or incoffset
  SDValue FirstOperand;
  SDValue SecondOperand;
  if (N->getOpcode() == ISD::INTRINSIC_WO_CHAIN) {
    FirstOperand = N->getOperand(1);
    SecondOperand = N->getOperand(2);
  } else {
    assert(N->getOpcode() == ISD::PTRADD);
    FirstOperand = N->getOperand(0);
    SecondOperand = N->getOperand(1);
  }

  // Old pattern for __builtin_align() and C code that does the same thing (and
  // is also generated by cap_covers_pages() in CheriBSD a -O2)
  //          t6: i64 = llvm.cheri.cap.address.get TargetConstant:i64<9>, t4
  //        t8: i64 = and t6, Constant:i64<16383>
  //      t10: i64 = sub nsw Constant:i64<0>, t8
  //    t11: iFATPTR128 = PTRADD t4, t10
  // This can be folded to a CAndAddr with the inverted mask (i.e. XOR -1,
  // mask).

  if (SecondOperand->getOpcode() != ISD::SUB)
    return SDValue();
  if (auto SubConst = dyn_cast<ConstantSDNode>(SecondOperand->getOperand(0))) {
    if (!SubConst->isNullValue())
      return SDValue();
  } else {
    return SDValue();
  }
  // Second operand is a negate
  SDValue NegatedValue = SecondOperand->getOperand(1);
  if (NegatedValue->getOpcode() != ISD::AND)
    return SDValue();

  SDValue AndSrc = NegatedValue->getOperand(0);
  SDValue Mask = NegatedValue->getOperand(1);

  if (AndSrc->getOpcode() != ISD::INTRINSIC_WO_CHAIN)
    return SDValue(); // Not a getaddr node
  unsigned IID = cast<ConstantSDNode>(AndSrc->getOperand(0))->getZExtValue();
  if (IID == Intrinsic::cheri_cap_address_get) {
    auto SrcCap = AndSrc->getOperand(1);
    if (SrcCap == FirstOperand) {
      // Same operand -> for getaddr and ptraddr -> can combine into the
      // pattern for a CAndAddr.
      //
      //   (ptradd CheriOpnd:$r1,
      //           (sub (i64 0),
      //                (and (int_cheri_cap_address_get CheriOpnd:$r1),
      //                     GPR64Opnd:$mask)))
      //
      // becomes
      //
      //   (int_cheri_cap_address_set
      //       CheriOpnd:$rs1
      //       (and (int_cheri_cap_address_get CheriOpnd:$rs1),
      //            (xor (i64 -1), GPR64Opnd:$mask)))
      SDLoc DL(N);
      EVT IndexVT = Mask->getValueType(0);
      auto InvertedMask = DAG.getNode(ISD::XOR, DL, IndexVT, DAG.getAllOnesConstant(DL, IndexVT), Mask);
      auto NewAddr = DAG.getNode(ISD::AND, DL, IndexVT, AndSrc, InvertedMask);
      return DAG.getNode(
          ISD::INTRINSIC_WO_CHAIN, DL, N->getValueType(0),
          DAG.getConstant(Intrinsic::cheri_cap_address_set, DL, MVT::i64),
          SrcCap, NewAddr);
    }
  }

  // TODO: for __builtin_align_up() we could do a cincoffset and then candaddr
  // (or will that cause representability issues for values close to the end?)
  return SDValue();
}

static SDValue performINTRINSIC_WO_CHAINCombine(
    SDNode *N, SelectionDAG &DAG, TargetLowering::DAGCombinerInfo &DCI,
    const MipsSubtarget &Subtarget) {
  SDLoc DL(N);
  unsigned IID = cast<ConstantSDNode>(N->getOperand(0))->getZExtValue();
  EVT VT = Subtarget.isGP64bit() ? MVT::i64 : MVT::i32;

  // Lower to our custom node, but with a truncate back to i1 so we can
  // replace its uses.
  switch (IID) {
  case Intrinsic::cheri_cap_tag_get: {
    SDValue IntRes = DAG.getNode(MipsISD::CapTagGet, DL, VT,
                                 N->getOperand(1));
    IntRes = DAG.getNode(ISD::AssertZext, DL, VT, IntRes,
                         DAG.getValueType(MVT::i1));
    return DAG.getSetCC(DL, MVT::i1, IntRes,
                        DAG.getConstant(0, DL, VT), ISD::SETNE);
  }
  case Intrinsic::cheri_cap_sealed_get: {
    SDValue IntRes = DAG.getNode(MipsISD::CapSealedGet, DL, VT,
                                 N->getOperand(1));
    IntRes = DAG.getNode(ISD::AssertZext, DL, VT, IntRes,
                         DAG.getValueType(MVT::i1));
    return DAG.getSetCC(DL, MVT::i1, IntRes,
                        DAG.getConstant(0, DL, VT), ISD::SETNE);
  }
  case Intrinsic::cheri_cap_subset_test: {
    SDValue IntRes = DAG.getNode(MipsISD::CapSubsetTest, DL, VT,
                                 N->getOperand(1), N->getOperand(2));
    IntRes = DAG.getNode(ISD::AssertZext, DL, VT, IntRes,
                         DAG.getValueType(MVT::i1));
    return DAG.getSetCC(DL, MVT::i1, IntRes,
                        DAG.getConstant(0, DL, VT), ISD::SETNE);
  }
  case Intrinsic::cheri_cap_equal_exact: {
    SDValue IntRes = DAG.getNode(MipsISD::CapEqualExact, DL, VT,
                                 N->getOperand(1), N->getOperand(2));
    IntRes = DAG.getNode(ISD::AssertZext, DL, VT, IntRes,
                         DAG.getValueType(MVT::i1));
    return DAG.getSetCC(DL, MVT::i1, IntRes,
                        DAG.getConstant(0, DL, VT), ISD::SETNE);
  }
  case Intrinsic::cheri_cap_bounds_set:
  case Intrinsic::cheri_cap_bounds_set_exact:
  case Intrinsic::cheri_bounded_stack_cap:
  case Intrinsic::cheri_bounded_stack_cap_dynamic: {
    // FIXME: this should be target-independent
    auto SrcOperand = N->getOperand(1);
    auto SizeOperand = N->getOperand(2);
    if (SrcOperand->getOpcode() != ISD::INTRINSIC_WO_CHAIN)
      break; // Not an intrinisic  node
    unsigned SrcIID =
        cast<ConstantSDNode>(SrcOperand->getOperand(0))->getZExtValue();
    if (SrcIID != Intrinsic::cheri_cap_bounds_set &&
        SrcIID != Intrinsic::cheri_cap_bounds_set_exact &&
        SrcIID != Intrinsic::cheri_bounded_stack_cap &&
        SrcIID != Intrinsic::cheri_bounded_stack_cap_dynamic) {
      break; // not a setbounds node
    }
    if (!DAG.isEqualTo(SizeOperand, SrcOperand->getOperand(2)))
      break; // not the same size
    if (SrcIID == IID) {
      // We can always replace chains of the same intrinsic
      LLVM_DEBUG(dbgs() << "  replacing setbounds with same size intrinsic: ";
                 SrcOperand->print(dbgs(), &DAG));
      return SrcOperand;
    } else if (SrcIID == Intrinsic::cheri_cap_bounds_set_exact) {
      // upgrading imprecise -> precise is safe since it will  lways return same
      // value csetbounds(csetboundsexact(x, len), len) -> csetboundsexact(x,
      // len)
      LLVM_DEBUG(dbgs() << "  replacing imprecise setbounds with same size "
                           "precise setbounds: ";
                 SrcOperand->print(dbgs(), &DAG));
      return SrcOperand;
    } else if (IID != Intrinsic::cheri_cap_bounds_set_exact) {
      assert(SrcIID == Intrinsic::cheri_cap_bounds_set ||
             SrcIID == Intrinsic::cheri_bounded_stack_cap ||
             SrcIID == Intrinsic::cheri_bounded_stack_cap_dynamic);
      LLVM_DEBUG(dbgs() << "  replacing imprecise setbounds with same size "
                           "imprecise setbounds: ";
                 SrcOperand->print(dbgs(), &DAG));
      return SrcOperand;
    } else {
      assert(IID == Intrinsic::cheri_cap_bounds_set_exact);
      // However, this is not true for exact setbounds on imprecise setbounds
      // TODO handle this case:
      LLVM_DEBUG(dbgs() << "  using source operand of imprecise setbounds for "
                           "same size precise setbounds: ";
                 SrcOperand->print(dbgs(), &DAG));
      return DAG.getNode(
          ISD::INTRINSIC_WO_CHAIN, DL, N->getValueType(0),
          DAG.getConstant(Intrinsic::cheri_cap_bounds_set_exact, DL, MVT::i64),
          SrcOperand, SizeOperand);
    }
    break;
  }
  case Intrinsic::cheri_round_representable_length: {
    if (Subtarget.isCheri256())
      return N->getOperand(1);

    KnownBits Known = DAG.computeKnownBits(SDValue(N, 0));
    if (Known.isConstant())
      return DAG.getConstant(Known.One, DL, N->getValueType(0));
    break;
  }
  case Intrinsic::cheri_representable_alignment_mask: {
    if (Subtarget.isCheri256())
      return DAG.getAllOnesConstant(DL, N->getValueType(0));

    KnownBits Known = DAG.computeKnownBits(SDValue(N, 0));
    if (Known.isConstant())
      return DAG.getConstant(Known.One, DL, N->getValueType(0));
    break;
  }
  }

  return SDValue();
}

SDValue  MipsTargetLowering::PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI)
  const {
  SelectionDAG &DAG = DCI.DAG;
  unsigned Opc = N->getOpcode();

  switch (Opc) {
  default: break;
  case ISD::SDIVREM:
  case ISD::UDIVREM:
    return performDivRemCombine(N, DAG, DCI, Subtarget);
  case ISD::SELECT:
    return performSELECTCombine(N, DAG, DCI, Subtarget);
  case MipsISD::CMovFP_F:
  case MipsISD::CMovFP_T:
    return performCMovFPCombine(N, DAG, DCI, Subtarget);
  case ISD::AND:
    return performANDCombine(N, DAG, DCI, Subtarget);
  case ISD::OR:
    return performORCombine(N, DAG, DCI, Subtarget);
  case ISD::ADD:
    return performADDCombine(N, DAG, DCI, Subtarget);
  case ISD::SHL:
    return performSHLCombine(N, DAG, DCI, Subtarget);
  case ISD::SUB:
    return performSUBCombine(N, DAG, DCI, Subtarget);
  case ISD::PTRADD:
    return performCIncOffsetToCandAddrCombine(N, DAG, DCI, Subtarget);
  case ISD::INTRINSIC_WO_CHAIN:
    return performINTRINSIC_WO_CHAINCombine(N, DAG, DCI, Subtarget);
  }

  return SDValue();
}

bool MipsTargetLowering::isCheapToSpeculateCttz() const {
  return Subtarget.hasMips32();
}

bool MipsTargetLowering::isCheapToSpeculateCtlz() const {
  return Subtarget.hasMips32();
}

bool MipsTargetLowering::shouldFoldConstantShiftPairToMask(
    const SDNode *N, CombineLevel Level) const {
  if (N->getOperand(0).getValueType().isVector())
    return false;
  return true;
}

void
MipsTargetLowering::ReplaceNodeResults(SDNode *N,
                                       SmallVectorImpl<SDValue> &Results,
                                       SelectionDAG &DAG) const {
  return LowerOperationWrapper(N, Results, DAG);
}

SDValue MipsTargetLowering::
LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
  switch (Op.getOpcode())
  {
  case ISD::ADDRSPACECAST:      return lowerADDRSPACECAST(Op, DAG);
  case ISD::BRCOND:             return lowerBRCOND(Op, DAG);
  case ISD::ConstantPool:       return lowerConstantPool(Op, DAG);
  case ISD::GlobalAddress:      return lowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:       return lowerBlockAddress(Op, DAG);
  case ISD::GlobalTLSAddress:   return lowerGlobalTLSAddress(Op, DAG);
  case ISD::JumpTable:          return lowerJumpTable(Op, DAG);
  case ISD::SELECT:             return lowerSELECT(Op, DAG);
  case ISD::SETCC:              return lowerSETCC(Op, DAG);
  case ISD::VASTART:            return lowerVASTART(Op, DAG);
  case ISD::VAARG:              return lowerVAARG(Op, DAG);
  case ISD::VACOPY:             return lowerVACOPY(Op, DAG);
  case ISD::FCOPYSIGN:          return lowerFCOPYSIGN(Op, DAG);
  case ISD::FABS:               return lowerFABS(Op, DAG);
  case ISD::FRAMEADDR:          return lowerFRAMEADDR(Op, DAG);
  case ISD::RETURNADDR:         return lowerRETURNADDR(Op, DAG);
  case ISD::EH_RETURN:          return lowerEH_RETURN(Op, DAG);
  case ISD::ATOMIC_FENCE:       return lowerATOMIC_FENCE(Op, DAG);
  case ISD::SHL_PARTS:          return lowerShiftLeftParts(Op, DAG);
  case ISD::SRA_PARTS:          return lowerShiftRightParts(Op, DAG, true);
  case ISD::SRL_PARTS:          return lowerShiftRightParts(Op, DAG, false);
  case ISD::LOAD:               return lowerLOAD(Op, DAG);
  case ISD::STORE:              return lowerSTORE(Op, DAG);
  case ISD::EH_DWARF_CFA:       return lowerEH_DWARF_CFA(Op, DAG);
  case ISD::FP_TO_SINT:         return lowerFP_TO_SINT(Op, DAG);
  case ISD::BR_JT:              return lowerBR_JT(Op, DAG);
  }
  return SDValue();
}

void MipsTargetLowering::computeKnownBitsForTargetNode(
    const SDValue Op, KnownBits &Known, const APInt &DemandedElts,
    const SelectionDAG &DAG, unsigned Depth) const {
  Known.resetAll();
  switch (Op.getOpcode()) {
  default: break;
  case ISD::INTRINSIC_WO_CHAIN: {
    switch (cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue()) {
    default: break;
    case Intrinsic::cheri_round_representable_length:
      if (Subtarget.isCheri256()) {
        Known = DAG.computeKnownBits(Op.getOperand(1));
      } else if (Subtarget.isCheri128()) {
        KnownBits KnownLengthBits = DAG.computeKnownBits(Op.getOperand(1));
        uint64_t MinLength = KnownLengthBits.One.getZExtValue();
        uint64_t MaxLength = (~KnownLengthBits.Zero).getZExtValue();
        uint64_t MinMask = cc128_get_alignment_mask(MinLength);
        uint64_t MaxMask = cc128_get_alignment_mask(MaxLength);
        uint64_t MinRoundedLength = (MinLength + ~MinMask) & MinMask;
        uint64_t MaxRoundedLength = (MaxLength + ~MaxMask) & MaxMask;
        bool MinRoundedOverflow = MinRoundedLength < MinLength;
        bool MaxRoundedOverflow = MaxRoundedLength < MaxLength;

        // A bit is known if the two different output bits are the same and:
        //
        //  (1) All more-significant bits are known. This is regardless of
        //      whether the corresponding input bits were known, since rounding
        //      is monotonic.
        //
        //  OR
        //
        //  (2) All less-significant bits are known and the corresponding input
        //      bit is known.
        //
        // If the two rounded values are the same, repeated application of (1)
        // yields the expected result that all bits are known.
        //
        // Note that the properties as described above are in terms of the
        // (N+1)-bit outputs, not their truncated forms, with the (N+1)th bit
        // being the overflow bit, and so we must take that into account.
        //
        // This can be improved upon to consider inner and trailing bits that
        // are still known regardless of the input bits (such as because they
        // are 1 in the input and the bounds are not rounded up too much to
        // lose them), but this is a good first start.

        uint64_t MinMaxRoundedAgreeMask = MinRoundedLength ^ ~MaxRoundedLength;
        uint64_t InputKnownMask =
          (KnownLengthBits.Zero | KnownLengthBits.One).getZExtValue();

        // Calculate bits for property (1)
        uint64_t LeadingKnownBits = countLeadingOnes(MinMaxRoundedAgreeMask);
        uint64_t LeadingKnownMask =
          MinRoundedOverflow == MaxRoundedOverflow
            ? maskLeadingOnes<uint64_t>(LeadingKnownBits) : 0;

        // Calculate bits for property (2)
        uint64_t TrailingKnownBits = countTrailingOnes(MinMaxRoundedAgreeMask);
        uint64_t TrailingKnownMask =
            maskTrailingOnes<uint64_t>(TrailingKnownBits) & InputKnownMask;

        // Combine properties
        uint64_t KnownMask = LeadingKnownMask | TrailingKnownMask;

        Known.Zero |= ~MinRoundedLength & KnownMask;
        Known.One |= MinRoundedLength & KnownMask;
      }
      break;
    case Intrinsic::cheri_representable_alignment_mask:
      if (Subtarget.isCheri256()) {
        Known.setAllOnes();
      } else if (Subtarget.isCheri128()) {
        KnownBits KnownLengthBits = DAG.computeKnownBits(Op.getOperand(1));
        uint64_t MinLength = KnownLengthBits.One.getZExtValue();
        uint64_t MaxLength = (~KnownLengthBits.Zero).getZExtValue();

        Known.Zero |= ~cc128_get_alignment_mask(MinLength);
        Known.One |= cc128_get_alignment_mask(MaxLength);
      }
      break;
    }
  }
  }
}

TailPaddingAmount
MipsTargetLowering::getTailPaddingForPreciseBounds(uint64_t Size) const {
  if (!Subtarget.isCheri())
    return TailPaddingAmount::None;
  if (Subtarget.isCheri128()) {
    return static_cast<TailPaddingAmount>(
        llvm::alignTo(Size, cc128_get_required_alignment(Size)) - Size);
  }
  assert(Subtarget.isCheri256());
  // No padding required for CHERI256
  return TailPaddingAmount::None;
}

Align
MipsTargetLowering::getAlignmentForPreciseBounds(uint64_t Size) const {
  if (!Subtarget.isCheri())
    return Align();
  if (Subtarget.isCheri128()) {
    return Align(cc128_get_required_alignment(Size));
  }
  assert(Subtarget.isCheri256());
  // No alignment required for CHERI256
  return Align();
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//

// addLiveIn - This helper function adds the specified physical register to the
// MachineFunction as a live in value.  It also creates a corresponding
// virtual register for it.
static unsigned
addLiveIn(MachineFunction &MF, unsigned PReg, const TargetRegisterClass *RC)
{
  Register VReg = MF.getRegInfo().createVirtualRegister(RC);
  MF.getRegInfo().addLiveIn(PReg, VReg);
  return VReg;
}

static MachineBasicBlock *insertDivByZeroTrap(MachineInstr &MI,
                                              MachineBasicBlock &MBB,
                                              const TargetInstrInfo &TII,
                                              bool Is64Bit, bool IsMicroMips) {
  if (NoZeroDivCheck)
    return &MBB;

  // Insert instruction "teq $divisor_reg, $zero, 7".
  MachineBasicBlock::iterator I(MI);
  MachineInstrBuilder MIB;
  MachineOperand &Divisor = MI.getOperand(2);
  MIB = BuildMI(MBB, std::next(I), MI.getDebugLoc(),
                TII.get(IsMicroMips ? Mips::TEQ_MM : Mips::TEQ))
            .addReg(Divisor.getReg(), getKillRegState(Divisor.isKill()))
            .addReg(Mips::ZERO)
            .addImm(7);

  // Use the 32-bit sub-register if this is a 64-bit division.
  if (Is64Bit)
    MIB->getOperand(0).setSubReg(Mips::sub_32);

  // Clear Divisor's kill flag.
  Divisor.setIsKill(false);

  // We would normally delete the original instruction here but in this case
  // we only needed to inject an additional instruction rather than replace it.

  return &MBB;
}

MachineBasicBlock *
MipsTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                MachineBasicBlock *BB) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("Unexpected instr type to insert");
  case Mips::ATOMIC_LOAD_ADD_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_ADD_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_ADD_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_ADD_I64:
    return emitAtomicBinary(MI, BB);
  case Mips::CAP_ATOMIC_LOAD_ADD_I8:
  case Mips::CAP_ATOMIC_LOAD_ADD_I16:
  case Mips::CAP_ATOMIC_LOAD_ADD_I32:
  case Mips::CAP_ATOMIC_LOAD_ADD_I64:
  // TODO: Mips::CAP_ATOMIC_LOAD_ADD_CAP:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_LOAD_AND_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_AND_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_AND_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_AND_I64:
    return emitAtomicBinary(MI, BB);
  case Mips::CAP_ATOMIC_LOAD_AND_I8:
  case Mips::CAP_ATOMIC_LOAD_AND_I16:
  case Mips::CAP_ATOMIC_LOAD_AND_I32:
  case Mips::CAP_ATOMIC_LOAD_AND_I64:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_LOAD_OR_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_OR_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_OR_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_OR_I64:
    return emitAtomicBinary(MI, BB);
  case Mips::CAP_ATOMIC_LOAD_OR_I8:
  case Mips::CAP_ATOMIC_LOAD_OR_I16:
  case Mips::CAP_ATOMIC_LOAD_OR_I32:
  case Mips::CAP_ATOMIC_LOAD_OR_I64:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_LOAD_XOR_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_XOR_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_XOR_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_XOR_I64:
    return emitAtomicBinary(MI, BB);
  case Mips::CAP_ATOMIC_LOAD_XOR_I8:
  case Mips::CAP_ATOMIC_LOAD_XOR_I16:
  case Mips::CAP_ATOMIC_LOAD_XOR_I32:
  case Mips::CAP_ATOMIC_LOAD_XOR_I64:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_LOAD_NAND_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_NAND_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_NAND_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_NAND_I64:
    return emitAtomicBinary(MI, BB);
  case Mips::CAP_ATOMIC_LOAD_NAND_I8:
  case Mips::CAP_ATOMIC_LOAD_NAND_I16:
  case Mips::CAP_ATOMIC_LOAD_NAND_I32:
  case Mips::CAP_ATOMIC_LOAD_NAND_I64:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_LOAD_SUB_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_SUB_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_SUB_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_SUB_I64:
    return emitAtomicBinary(MI, BB);
  case Mips::CAP_ATOMIC_LOAD_SUB_I8:
  case Mips::CAP_ATOMIC_LOAD_SUB_I16:
  case Mips::CAP_ATOMIC_LOAD_SUB_I32:
  case Mips::CAP_ATOMIC_LOAD_SUB_I64:
  // TODO: Mips::CAP_ATOMIC_LOAD_SUB_CAP:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_SWAP_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_SWAP_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_SWAP_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_SWAP_I64:
    return emitAtomicBinary(MI, BB);
  case Mips::CAP_ATOMIC_SWAP_I8:
  case Mips::CAP_ATOMIC_SWAP_I16:
  case Mips::CAP_ATOMIC_SWAP_I32:
  case Mips::CAP_ATOMIC_SWAP_I64:
  case Mips::CAP_ATOMIC_SWAP_CAP:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_CMP_SWAP_I8:
    return emitAtomicCmpSwapPartword(MI, BB, 1);
  case Mips::ATOMIC_CMP_SWAP_I16:
    return emitAtomicCmpSwapPartword(MI, BB, 2);
  case Mips::ATOMIC_CMP_SWAP_I32:
    return emitAtomicCmpSwap(MI, BB);
  case Mips::ATOMIC_CMP_SWAP_I64:
    return emitAtomicCmpSwap(MI, BB);
  case Mips::CAP_ATOMIC_CMP_SWAP_I8:
  case Mips::CAP_ATOMIC_CMP_SWAP_I16:
  case Mips::CAP_ATOMIC_CMP_SWAP_I32:
  case Mips::CAP_ATOMIC_CMP_SWAP_I64:
  case Mips::CAP_ATOMIC_CMP_SWAP_CAP:
    return emitAtomicCmpSwap(MI, BB);


  case Mips::ATOMIC_LOAD_MIN_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_MIN_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_MIN_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_MIN_I64:
  case Mips::CAP_ATOMIC_LOAD_MIN_I8:
  case Mips::CAP_ATOMIC_LOAD_MIN_I16:
  case Mips::CAP_ATOMIC_LOAD_MIN_I32:
  case Mips::CAP_ATOMIC_LOAD_MIN_I64:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_LOAD_MAX_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_MAX_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_MAX_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_MAX_I64:
  case Mips::CAP_ATOMIC_LOAD_MAX_I8:
  case Mips::CAP_ATOMIC_LOAD_MAX_I16:
  case Mips::CAP_ATOMIC_LOAD_MAX_I32:
  case Mips::CAP_ATOMIC_LOAD_MAX_I64:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_LOAD_UMIN_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_UMIN_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_UMIN_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_UMIN_I64:
  case Mips::CAP_ATOMIC_LOAD_UMIN_I8:
  case Mips::CAP_ATOMIC_LOAD_UMIN_I16:
  case Mips::CAP_ATOMIC_LOAD_UMIN_I32:
  case Mips::CAP_ATOMIC_LOAD_UMIN_I64:
    return emitAtomicBinary(MI, BB);

  case Mips::ATOMIC_LOAD_UMAX_I8:
    return emitAtomicBinaryPartword(MI, BB, 1);
  case Mips::ATOMIC_LOAD_UMAX_I16:
    return emitAtomicBinaryPartword(MI, BB, 2);
  case Mips::ATOMIC_LOAD_UMAX_I32:
    return emitAtomicBinary(MI, BB);
  case Mips::ATOMIC_LOAD_UMAX_I64:
  case Mips::CAP_ATOMIC_LOAD_UMAX_I8:
  case Mips::CAP_ATOMIC_LOAD_UMAX_I16:
  case Mips::CAP_ATOMIC_LOAD_UMAX_I32:
  case Mips::CAP_ATOMIC_LOAD_UMAX_I64:
    return emitAtomicBinary(MI, BB);

  case Mips::PseudoSDIV:
  case Mips::PseudoUDIV:
  case Mips::DIV:
  case Mips::DIVU:
  case Mips::MOD:
  case Mips::MODU:
    return insertDivByZeroTrap(MI, *BB, *Subtarget.getInstrInfo(), false,
                               false);
  case Mips::SDIV_MM_Pseudo:
  case Mips::UDIV_MM_Pseudo:
  case Mips::SDIV_MM:
  case Mips::UDIV_MM:
  case Mips::DIV_MMR6:
  case Mips::DIVU_MMR6:
  case Mips::MOD_MMR6:
  case Mips::MODU_MMR6:
    return insertDivByZeroTrap(MI, *BB, *Subtarget.getInstrInfo(), false, true);
  case Mips::PseudoDSDIV:
  case Mips::PseudoDUDIV:
  case Mips::DDIV:
  case Mips::DDIVU:
  case Mips::DMOD:
  case Mips::DMODU:
    return insertDivByZeroTrap(MI, *BB, *Subtarget.getInstrInfo(), true, false);

  case Mips::PseudoSELECT_I:
  case Mips::PseudoSELECT_I64:
  case Mips::PseudoSELECT_S:
  case Mips::PseudoSELECT_D32:
  case Mips::PseudoSELECT_D64:
    return emitPseudoSELECT(MI, BB, false, Mips::BNE);
  case Mips::PseudoSELECTFP_F_I:
  case Mips::PseudoSELECTFP_F_I64:
  case Mips::PseudoSELECTFP_F_S:
  case Mips::PseudoSELECTFP_F_D32:
  case Mips::PseudoSELECTFP_F_D64:
    return emitPseudoSELECT(MI, BB, true, Mips::BC1F);
  case Mips::PseudoSELECTFP_T_I:
  case Mips::PseudoSELECTFP_T_I64:
  case Mips::PseudoSELECTFP_T_S:
  case Mips::PseudoSELECTFP_T_D32:
  case Mips::PseudoSELECTFP_T_D64:
    return emitPseudoSELECT(MI, BB, true, Mips::BC1T);
  case Mips::PseudoD_SELECT_I:
  case Mips::PseudoD_SELECT_I64:
    return emitPseudoD_SELECT(MI, BB);
  case Mips::LDR_W:
    return emitLDR_W(MI, BB);
  case Mips::LDR_D:
    return emitLDR_D(MI, BB);
  case Mips::STR_W:
    return emitSTR_W(MI, BB);
  case Mips::STR_D:
    return emitSTR_D(MI, BB);
  case Mips::PseudoPccRelativeAddress: {
    // Since we need to ensure that these instrs are written together we need to
    // bundle them. However, this is only possible after register allocation so
    // we convert this to another pseudo instruction first.
    MachineFunction *MF = BB->getParent();
    MachineRegisterInfo &RegInfo = MF->getRegInfo();
    const TargetInstrInfo *TII = Subtarget.getInstrInfo();
    DebugLoc DL = MI.getDebugLoc();
    Register Scratch = RegInfo.createVirtualRegister(&Mips::GPR64RegClass);
    BuildMI(*BB, &MI, DL, TII->get(Mips::PseudoPccRelativeAddressPostRA))
        .add(MI.getOperand(0))
        .add(MI.getOperand(1))
        .addReg(Scratch, RegState::Define | RegState::EarlyClobber |
                             RegState::Implicit | RegState::Dead);
    MI.eraseFromParent();
    return BB;
  }
  }
}

static unsigned getCheriPostRAAtomicOp(const MachineInstr &MI,
                                       bool *IsCapabilityArg,
                                       bool *NeedsAdditionalReg) {
#define CAP_ATOMIC_POSTRA_CASES(op, extrareg)                                  \
  case Mips::CAP_ATOMIC_##op##_I8:                                             \
    *NeedsAdditionalReg = extrareg;                                            \
    return Mips::CAP_ATOMIC_##op##_I8_POSTRA;                                  \
  case Mips::CAP_ATOMIC_##op##_I16:                                            \
    *NeedsAdditionalReg = extrareg;                                            \
    return Mips::CAP_ATOMIC_##op##_I16_POSTRA;                                 \
  case Mips::CAP_ATOMIC_##op##_I32:                                            \
    *NeedsAdditionalReg = extrareg;                                            \
    return Mips::CAP_ATOMIC_##op##_I32_POSTRA;                                 \
  case Mips::CAP_ATOMIC_##op##_I64:                                            \
    *NeedsAdditionalReg = extrareg;                                            \
    return Mips::CAP_ATOMIC_##op##_I64_POSTRA;

  // TODO: case Mips::CAP_ATOMIC_##op##_CAP: return
  // Mips::CAP_ATOMIC_##op##_CAP_POSTRA;

  // clang-format off
  switch (MI.getOpcode()) {
  CAP_ATOMIC_POSTRA_CASES(LOAD_ADD, false)
  CAP_ATOMIC_POSTRA_CASES(LOAD_SUB, false)
  CAP_ATOMIC_POSTRA_CASES(LOAD_AND, false)
  CAP_ATOMIC_POSTRA_CASES(LOAD_OR, false)
  CAP_ATOMIC_POSTRA_CASES(LOAD_XOR, false)
  CAP_ATOMIC_POSTRA_CASES(LOAD_NAND, false)
  CAP_ATOMIC_POSTRA_CASES(LOAD_MIN, true)
  CAP_ATOMIC_POSTRA_CASES(LOAD_MAX, true)
  CAP_ATOMIC_POSTRA_CASES(LOAD_UMIN, true)
  CAP_ATOMIC_POSTRA_CASES(LOAD_UMAX, true)
  CAP_ATOMIC_POSTRA_CASES(SWAP, false)
  // clang-format on
  case Mips::CAP_ATOMIC_SWAP_CAP:
    *IsCapabilityArg = true;
    return Mips::CAP_ATOMIC_SWAP_CAP_POSTRA;
  default:
    llvm_unreachable("Unknown pseudo atomic for replacement!");
  }
}

// This function also handles Mips::ATOMIC_SWAP_I32 (when BinOpcode == 0), and
// Mips::ATOMIC_LOAD_NAND_I32 (when Nand == true)
MachineBasicBlock *
MipsTargetLowering::emitAtomicBinary(MachineInstr &MI,
                                     MachineBasicBlock *BB) const {

  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  unsigned AtomicOp;
  bool NeedsAdditionalReg = false;
  bool IsCapabilityArg = false;
  switch (MI.getOpcode()) {
  case Mips::ATOMIC_LOAD_ADD_I32:
    AtomicOp = Mips::ATOMIC_LOAD_ADD_I32_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_SUB_I32:
    AtomicOp = Mips::ATOMIC_LOAD_SUB_I32_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_AND_I32:
    AtomicOp = Mips::ATOMIC_LOAD_AND_I32_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_OR_I32:
    AtomicOp = Mips::ATOMIC_LOAD_OR_I32_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_XOR_I32:
    AtomicOp = Mips::ATOMIC_LOAD_XOR_I32_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_NAND_I32:
    AtomicOp = Mips::ATOMIC_LOAD_NAND_I32_POSTRA;
    break;
  case Mips::ATOMIC_SWAP_I32:
    AtomicOp = Mips::ATOMIC_SWAP_I32_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_ADD_I64:
    AtomicOp = Mips::ATOMIC_LOAD_ADD_I64_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_SUB_I64:
    AtomicOp = Mips::ATOMIC_LOAD_SUB_I64_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_AND_I64:
    AtomicOp = Mips::ATOMIC_LOAD_AND_I64_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_OR_I64:
    AtomicOp = Mips::ATOMIC_LOAD_OR_I64_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_XOR_I64:
    AtomicOp = Mips::ATOMIC_LOAD_XOR_I64_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_NAND_I64:
    AtomicOp = Mips::ATOMIC_LOAD_NAND_I64_POSTRA;
    break;
  case Mips::ATOMIC_SWAP_I64:
    AtomicOp = Mips::ATOMIC_SWAP_I64_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_MIN_I32:
    AtomicOp = Mips::ATOMIC_LOAD_MIN_I32_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_MAX_I32:
    AtomicOp = Mips::ATOMIC_LOAD_MAX_I32_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_UMIN_I32:
    AtomicOp = Mips::ATOMIC_LOAD_UMIN_I32_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_UMAX_I32:
    AtomicOp = Mips::ATOMIC_LOAD_UMAX_I32_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_MIN_I64:
    AtomicOp = Mips::ATOMIC_LOAD_MIN_I64_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_MAX_I64:
    AtomicOp = Mips::ATOMIC_LOAD_MAX_I64_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_UMIN_I64:
    AtomicOp = Mips::ATOMIC_LOAD_UMIN_I64_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_UMAX_I64:
    AtomicOp = Mips::ATOMIC_LOAD_UMAX_I64_POSTRA;
    NeedsAdditionalReg = true;
    break;
  default:
    AtomicOp =
        getCheriPostRAAtomicOp(MI, &IsCapabilityArg, &NeedsAdditionalReg);
    break;
  }

  Register OldVal = MI.getOperand(0).getReg();
  auto RC = RegInfo.getRegClass(OldVal);
  // For capability cscc we want a GPR64 scratch register and not CheriGPR
  if (IsCapabilityArg)
    RC = &Mips::GPR64RegClass;
  Register Ptr = MI.getOperand(1).getReg();
  Register Incr = MI.getOperand(2).getReg();
  Register Scratch = RegInfo.createVirtualRegister(RC);

  MachineBasicBlock::iterator II(MI);

  // The scratch registers here with the EarlyClobber | Define | Implicit
  // flags is used to persuade the register allocator and the machine
  // verifier to accept the usage of this register. This has to be a real
  // register which has an UNDEF value but is dead after the instruction which
  // is unique among the registers chosen for the instruction.

  // The EarlyClobber flag has the semantic properties that the operand it is
  // attached to is clobbered before the rest of the inputs are read. Hence it
  // must be unique among the operands to the instruction.
  // The Define flag is needed to coerce the machine verifier that an Undef
  // value isn't a problem.
  // The Dead flag is needed as the value in scratch isn't used by any other
  // instruction. Kill isn't used as Dead is more precise.
  // The implicit flag is here due to the interaction between the other flags
  // and the machine verifier.

  // For correctness purpose, a new pseudo is introduced here. We need this
  // new pseudo, so that FastRegisterAllocator does not see an ll/sc sequence
  // that is spread over >1 basic blocks. A register allocator which
  // introduces (or any codegen infact) a store, can violate the expectations
  // of the hardware.
  //
  // An atomic read-modify-write sequence starts with a linked load
  // instruction and ends with a store conditional instruction. The atomic
  // read-modify-write sequence fails if any of the following conditions
  // occur between the execution of ll and sc:
  //   * A coherent store is completed by another process or coherent I/O
  //     module into the block of synchronizable physical memory containing
  //     the word. The size and alignment of the block is
  //     implementation-dependent.
  //   * A coherent store is executed between an LL and SC sequence on the
  //     same processor to the block of synchornizable physical memory
  //     containing the word.
  //

  Register PtrCopy = RegInfo.createVirtualRegister(RegInfo.getRegClass(Ptr));
  // If the increment is NULL/ZERO we might not have to make a copy since it
  // will be a constant physical register
  unsigned IncrCopy = Incr;
  if (Register::isVirtualRegister(Incr)) {
    IncrCopy = RegInfo.createVirtualRegister(RegInfo.getRegClass(Incr));
    BuildMI(*BB, II, DL, TII->get(Mips::COPY), IncrCopy).addReg(Incr);
  } else {
    assert(RegInfo.isConstantPhysReg(Incr));
  }
  BuildMI(*BB, II, DL, TII->get(Mips::COPY), PtrCopy).addReg(Ptr);

  MachineInstrBuilder MIB =
      BuildMI(*BB, II, DL, TII->get(AtomicOp))
          .addReg(OldVal, RegState::Define | RegState::EarlyClobber)
          .addReg(PtrCopy)
          .addReg(IncrCopy)
          .addReg(Scratch, RegState::Define | RegState::EarlyClobber |
                               RegState::Implicit | RegState::Dead);
  if (NeedsAdditionalReg) {
    Register Scratch2 =
        RegInfo.createVirtualRegister(RegInfo.getRegClass(OldVal));
    MIB.addReg(Scratch2, RegState::Define | RegState::EarlyClobber |
                             RegState::Implicit | RegState::Dead);
  }

  MI.eraseFromParent();

  return BB;
}

bool MipsTargetLowering::supportsAtomicOperation(const DataLayout &DL,
                                                 const Instruction *AI,
                                                 Type *ValueTy, Type *PointerTy,
                                                 Align Alignment) const {
  // FIXME: We should have a proper implementation of RMW atomics on capability
  // values but CHERI-MIPS is EOL.
  if (DL.isFatPointer(ValueTy) && isa<AtomicRMWInst>(AI))
    if (cast<AtomicRMWInst>(AI)->getOperation() != AtomicRMWInst::Xchg)
      return false;
  return TargetLowering::supportsAtomicOperation(DL, AI, ValueTy, PointerTy,
                                                 Alignment);
}

MachineBasicBlock *MipsTargetLowering::emitSignExtendToI32InReg(
    MachineInstr &MI, MachineBasicBlock *BB, unsigned Size, unsigned DstReg,
    unsigned SrcReg) const {
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  const DebugLoc &DL = MI.getDebugLoc();

  if (Subtarget.hasMips32r2() && Size == 1) {
    BuildMI(BB, DL, TII->get(Mips::SEB), DstReg).addReg(SrcReg);
    return BB;
  }

  if (Subtarget.hasMips32r2() && Size == 2) {
    BuildMI(BB, DL, TII->get(Mips::SEH), DstReg).addReg(SrcReg);
    return BB;
  }

  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetRegisterClass *RC = getRegClassFor(MVT::i32);
  Register ScrReg = RegInfo.createVirtualRegister(RC);

  assert(Size < 32);
  int64_t ShiftImm = 32 - (Size * 8);

  BuildMI(BB, DL, TII->get(Mips::SLL), ScrReg).addReg(SrcReg).addImm(ShiftImm);
  BuildMI(BB, DL, TII->get(Mips::SRA), DstReg).addReg(ScrReg).addImm(ShiftImm);

  return BB;
}

MachineBasicBlock *MipsTargetLowering::emitAtomicBinaryPartword(
    MachineInstr &MI, MachineBasicBlock *BB, unsigned Size) const {
  assert((Size == 1 || Size == 2) &&
         "Unsupported size for EmitAtomicBinaryPartial.");
  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetRegisterClass *RC = getRegClassFor(MVT::i32);
  const bool ArePtrs64bit = ABI.ArePtrs64bit();
  const TargetRegisterClass *RCp =
    getRegClassFor(ArePtrs64bit ? MVT::i64 : MVT::i32);
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  Register Dest = MI.getOperand(0).getReg();
  Register Ptr = MI.getOperand(1).getReg();
  Register Incr = MI.getOperand(2).getReg();

  Register AlignedAddr = RegInfo.createVirtualRegister(RCp);
  Register ShiftAmt = RegInfo.createVirtualRegister(RC);
  Register Mask = RegInfo.createVirtualRegister(RC);
  Register Mask2 = RegInfo.createVirtualRegister(RC);
  Register Incr2 = RegInfo.createVirtualRegister(RC);
  Register MaskLSB2 = RegInfo.createVirtualRegister(RCp);
  Register PtrLSB2 = RegInfo.createVirtualRegister(RC);
  Register MaskUpper = RegInfo.createVirtualRegister(RC);
  Register Scratch = RegInfo.createVirtualRegister(RC);
  Register Scratch2 = RegInfo.createVirtualRegister(RC);
  Register Scratch3 = RegInfo.createVirtualRegister(RC);

  unsigned AtomicOp = 0;
  bool NeedsAdditionalReg = false;
  switch (MI.getOpcode()) {
  case Mips::ATOMIC_LOAD_NAND_I8:
    AtomicOp = Mips::ATOMIC_LOAD_NAND_I8_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_NAND_I16:
    AtomicOp = Mips::ATOMIC_LOAD_NAND_I16_POSTRA;
    break;
  case Mips::ATOMIC_SWAP_I8:
    AtomicOp = Mips::ATOMIC_SWAP_I8_POSTRA;
    break;
  case Mips::ATOMIC_SWAP_I16:
    AtomicOp = Mips::ATOMIC_SWAP_I16_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_ADD_I8:
    AtomicOp = Mips::ATOMIC_LOAD_ADD_I8_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_ADD_I16:
    AtomicOp = Mips::ATOMIC_LOAD_ADD_I16_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_SUB_I8:
    AtomicOp = Mips::ATOMIC_LOAD_SUB_I8_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_SUB_I16:
    AtomicOp = Mips::ATOMIC_LOAD_SUB_I16_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_AND_I8:
    AtomicOp = Mips::ATOMIC_LOAD_AND_I8_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_AND_I16:
    AtomicOp = Mips::ATOMIC_LOAD_AND_I16_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_OR_I8:
    AtomicOp = Mips::ATOMIC_LOAD_OR_I8_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_OR_I16:
    AtomicOp = Mips::ATOMIC_LOAD_OR_I16_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_XOR_I8:
    AtomicOp = Mips::ATOMIC_LOAD_XOR_I8_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_XOR_I16:
    AtomicOp = Mips::ATOMIC_LOAD_XOR_I16_POSTRA;
    break;
  case Mips::ATOMIC_LOAD_MIN_I8:
    AtomicOp = Mips::ATOMIC_LOAD_MIN_I8_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_MIN_I16:
    AtomicOp = Mips::ATOMIC_LOAD_MIN_I16_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_MAX_I8:
    AtomicOp = Mips::ATOMIC_LOAD_MAX_I8_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_MAX_I16:
    AtomicOp = Mips::ATOMIC_LOAD_MAX_I16_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_UMIN_I8:
    AtomicOp = Mips::ATOMIC_LOAD_UMIN_I8_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_UMIN_I16:
    AtomicOp = Mips::ATOMIC_LOAD_UMIN_I16_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_UMAX_I8:
    AtomicOp = Mips::ATOMIC_LOAD_UMAX_I8_POSTRA;
    NeedsAdditionalReg = true;
    break;
  case Mips::ATOMIC_LOAD_UMAX_I16:
    AtomicOp = Mips::ATOMIC_LOAD_UMAX_I16_POSTRA;
    NeedsAdditionalReg = true;
    break;
  default:
    llvm_unreachable("Unknown subword atomic pseudo for expansion!");
  }

  // insert new blocks after the current block
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineBasicBlock *exitMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineFunction::iterator It = ++BB->getIterator();
  MF->insert(It, exitMBB);

  // Transfer the remainder of BB and its successor edges to exitMBB.
  exitMBB->splice(exitMBB->begin(), BB,
                  std::next(MachineBasicBlock::iterator(MI)), BB->end());
  exitMBB->transferSuccessorsAndUpdatePHIs(BB);

  BB->addSuccessor(exitMBB, BranchProbability::getOne());

  //  thisMBB:
  //    addiu   masklsb2,$0,-4                # 0xfffffffc
  //    and     alignedaddr,ptr,masklsb2
  //    andi    ptrlsb2,ptr,3
  //    sll     shiftamt,ptrlsb2,3
  //    ori     maskupper,$0,255               # 0xff
  //    sll     mask,maskupper,shiftamt
  //    nor     mask2,$0,mask
  //    sll     incr2,incr,shiftamt

  int64_t MaskImm = (Size == 1) ? 255 : 65535;
  BuildMI(BB, DL, TII->get(ABI.GetPtrAddiuOp()), MaskLSB2)
    .addReg(ABI.GetNullPtr()).addImm(-4);
  BuildMI(BB, DL, TII->get(ABI.GetPtrAndOp()), AlignedAddr)
    .addReg(Ptr).addReg(MaskLSB2);
  BuildMI(BB, DL, TII->get(Mips::ANDi), PtrLSB2)
      .addReg(Ptr, 0, ArePtrs64bit ? Mips::sub_32 : 0).addImm(3);
  if (Subtarget.isLittle()) {
    BuildMI(BB, DL, TII->get(Mips::SLL), ShiftAmt).addReg(PtrLSB2).addImm(3);
  } else {
    Register Off = RegInfo.createVirtualRegister(RC);
    BuildMI(BB, DL, TII->get(Mips::XORi), Off)
      .addReg(PtrLSB2).addImm((Size == 1) ? 3 : 2);
    BuildMI(BB, DL, TII->get(Mips::SLL), ShiftAmt).addReg(Off).addImm(3);
  }
  BuildMI(BB, DL, TII->get(Mips::ORi), MaskUpper)
    .addReg(Mips::ZERO).addImm(MaskImm);
  BuildMI(BB, DL, TII->get(Mips::SLLV), Mask)
    .addReg(MaskUpper).addReg(ShiftAmt);
  BuildMI(BB, DL, TII->get(Mips::NOR), Mask2).addReg(Mips::ZERO).addReg(Mask);
  BuildMI(BB, DL, TII->get(Mips::SLLV), Incr2).addReg(Incr).addReg(ShiftAmt);


  // The purposes of the flags on the scratch registers is explained in
  // emitAtomicBinary. In summary, we need a scratch register which is going to
  // be undef, that is unique among registers chosen for the instruction.

  MachineInstrBuilder MIB =
      BuildMI(BB, DL, TII->get(AtomicOp))
          .addReg(Dest, RegState::Define | RegState::EarlyClobber)
          .addReg(AlignedAddr)
          .addReg(Incr2)
          .addReg(Mask)
          .addReg(Mask2)
          .addReg(ShiftAmt)
          .addReg(Scratch, RegState::EarlyClobber | RegState::Define |
                               RegState::Dead | RegState::Implicit)
          .addReg(Scratch2, RegState::EarlyClobber | RegState::Define |
                                RegState::Dead | RegState::Implicit)
          .addReg(Scratch3, RegState::EarlyClobber | RegState::Define |
                                RegState::Dead | RegState::Implicit);
  if (NeedsAdditionalReg) {
    Register Scratch4 = RegInfo.createVirtualRegister(RC);
    MIB.addReg(Scratch4, RegState::EarlyClobber | RegState::Define |
                             RegState::Dead | RegState::Implicit);
  }

  MI.eraseFromParent(); // The instruction is gone now.

  return exitMBB;
}

// Lower atomic compare and swap to a pseudo instruction, taking care to
// define a scratch register for the pseudo instruction's expansion. The
// instruction is expanded after the register allocator as to prevent
// the insertion of stores between the linked load and the store conditional.

MachineBasicBlock *
MipsTargetLowering::emitAtomicCmpSwap(MachineInstr &MI,
                                      MachineBasicBlock *BB) const {

  unsigned AtomicOp = -1;
  MVT ScratchTy;
  switch (MI.getOpcode()) {
  case Mips::ATOMIC_CMP_SWAP_I32:
    AtomicOp = Mips::ATOMIC_CMP_SWAP_I32_POSTRA;
    ScratchTy = MVT::i32;
    break;
  case Mips::ATOMIC_CMP_SWAP_I64:
    AtomicOp = Mips::ATOMIC_CMP_SWAP_I64_POSTRA;
    ScratchTy = MVT::i64;
    break;
  case Mips::CAP_ATOMIC_CMP_SWAP_I8:
    AtomicOp = Mips::CAP_ATOMIC_CMP_SWAP_I8_POSTRA;
    ScratchTy = MVT::i32;
    break;
  case Mips::CAP_ATOMIC_CMP_SWAP_I16:
    AtomicOp = Mips::CAP_ATOMIC_CMP_SWAP_I16_POSTRA;
    ScratchTy = MVT::i32;
    break;
  case Mips::CAP_ATOMIC_CMP_SWAP_I32:
    AtomicOp = Mips::CAP_ATOMIC_CMP_SWAP_I32_POSTRA;
    ScratchTy = MVT::i32;
    break;
  case Mips::CAP_ATOMIC_CMP_SWAP_I64:
    AtomicOp = Mips::CAP_ATOMIC_CMP_SWAP_I64_POSTRA;
    ScratchTy = MVT::i64;
    break;
  case Mips::CAP_ATOMIC_CMP_SWAP_CAP:
    AtomicOp = Mips::CAP_ATOMIC_CMP_SWAP_CAP_POSTRA;
    ScratchTy = MVT::i64;
    break;
  default:
    llvm_unreachable("Unsupported atomic pseudo for EmitAtomicCmpSwap.");
  }

  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  const TargetRegisterClass *RC = getRegClassFor(ScratchTy);
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  Register Dest = MI.getOperand(0).getReg();
  Register Ptr = MI.getOperand(1).getReg();
  Register OldVal = MI.getOperand(2).getReg();
  Register NewVal = MI.getOperand(3).getReg();

  Register Scratch = MRI.createVirtualRegister(RC);
  MachineBasicBlock::iterator II(MI);

  // We need to create copies of the various registers and kill them at the
  // atomic pseudo. If the copies are not made, when the atomic is expanded
  // after fast register allocation, the spills will end up outside of the
  // blocks that their values are defined in, causing livein errors.

  Register PtrCopy = MRI.createVirtualRegister(MRI.getRegClass(Ptr));

  BuildMI(*BB, II, DL, TII->get(Mips::COPY), PtrCopy).addReg(Ptr);

  // If one of the operands is NULL it was replaced with $CNULL/ZERO so we don't
  // need to create a copy in that case.
  unsigned OldValCopy = OldVal;
  unsigned NewValCopy = NewVal;
  if (Register::isVirtualRegister(OldValCopy)) {
    OldValCopy = MRI.createVirtualRegister(MRI.getRegClass(OldVal));
    BuildMI(*BB, II, DL, TII->get(Mips::COPY), OldValCopy).addReg(OldVal);
  } else {
    assert(MRI.isConstantPhysReg(OldValCopy));
  }
  if (Register::isVirtualRegister(NewValCopy)) {
    NewValCopy = MRI.createVirtualRegister(MRI.getRegClass(NewVal));
    BuildMI(*BB, II, DL, TII->get(Mips::COPY), NewValCopy).addReg(NewVal);
  } else {
    assert(MRI.isConstantPhysReg(NewVal));
  }

  // The purposes of the flags on the scratch registers is explained in
  // emitAtomicBinary. In summary, we need a scratch register which is going to
  // be undef, that is unique among registers chosen for the instruction.

  BuildMI(*BB, II, DL, TII->get(AtomicOp))
      .addReg(Dest, RegState::Define | RegState::EarlyClobber)
      .addReg(PtrCopy, RegState::Kill)
      .addReg(OldValCopy, RegState::Kill)
      .addReg(NewValCopy, RegState::Kill)
      .addReg(Scratch, RegState::EarlyClobber | RegState::Define |
                           RegState::Dead | RegState::Implicit);

  MI.eraseFromParent(); // The instruction is gone now.

  return BB;
}

MachineBasicBlock *MipsTargetLowering::emitAtomicCmpSwapPartword(
    MachineInstr &MI, MachineBasicBlock *BB, unsigned Size) const {
  assert((Size == 1 || Size == 2) &&
      "Unsupported size for EmitAtomicCmpSwapPartial.");

  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  const TargetRegisterClass *RC = getRegClassFor(MVT::i32);
  const bool ArePtrs64bit = ABI.ArePtrs64bit();
  const TargetRegisterClass *RCp =
    getRegClassFor(ArePtrs64bit ? MVT::i64 : MVT::i32);
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  Register Dest = MI.getOperand(0).getReg();
  Register Ptr = MI.getOperand(1).getReg();
  Register CmpVal = MI.getOperand(2).getReg();
  Register NewVal = MI.getOperand(3).getReg();

  Register AlignedAddr = RegInfo.createVirtualRegister(RCp);
  Register ShiftAmt = RegInfo.createVirtualRegister(RC);
  Register Mask = RegInfo.createVirtualRegister(RC);
  Register Mask2 = RegInfo.createVirtualRegister(RC);
  Register ShiftedCmpVal = RegInfo.createVirtualRegister(RC);
  Register ShiftedNewVal = RegInfo.createVirtualRegister(RC);
  Register MaskLSB2 = RegInfo.createVirtualRegister(RCp);
  Register PtrLSB2 = RegInfo.createVirtualRegister(RC);
  Register MaskUpper = RegInfo.createVirtualRegister(RC);
  Register MaskedCmpVal = RegInfo.createVirtualRegister(RC);
  Register MaskedNewVal = RegInfo.createVirtualRegister(RC);
  unsigned AtomicOp = MI.getOpcode() == Mips::ATOMIC_CMP_SWAP_I8
                          ? Mips::ATOMIC_CMP_SWAP_I8_POSTRA
                          : Mips::ATOMIC_CMP_SWAP_I16_POSTRA;

  // The scratch registers here with the EarlyClobber | Define | Dead | Implicit
  // flags are used to coerce the register allocator and the machine verifier to
  // accept the usage of these registers.
  // The EarlyClobber flag has the semantic properties that the operand it is
  // attached to is clobbered before the rest of the inputs are read. Hence it
  // must be unique among the operands to the instruction.
  // The Define flag is needed to coerce the machine verifier that an Undef
  // value isn't a problem.
  // The Dead flag is needed as the value in scratch isn't used by any other
  // instruction. Kill isn't used as Dead is more precise.
  Register Scratch = RegInfo.createVirtualRegister(RC);
  Register Scratch2 = RegInfo.createVirtualRegister(RC);

  // insert new blocks after the current block
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineBasicBlock *exitMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineFunction::iterator It = ++BB->getIterator();
  MF->insert(It, exitMBB);

  // Transfer the remainder of BB and its successor edges to exitMBB.
  exitMBB->splice(exitMBB->begin(), BB,
                  std::next(MachineBasicBlock::iterator(MI)), BB->end());
  exitMBB->transferSuccessorsAndUpdatePHIs(BB);

  BB->addSuccessor(exitMBB, BranchProbability::getOne());

  //  thisMBB:
  //    addiu   masklsb2,$0,-4                # 0xfffffffc
  //    and     alignedaddr,ptr,masklsb2
  //    andi    ptrlsb2,ptr,3
  //    xori    ptrlsb2,ptrlsb2,3              # Only for BE
  //    sll     shiftamt,ptrlsb2,3
  //    ori     maskupper,$0,255               # 0xff
  //    sll     mask,maskupper,shiftamt
  //    nor     mask2,$0,mask
  //    andi    maskedcmpval,cmpval,255
  //    sll     shiftedcmpval,maskedcmpval,shiftamt
  //    andi    maskednewval,newval,255
  //    sll     shiftednewval,maskednewval,shiftamt
  int64_t MaskImm = (Size == 1) ? 255 : 65535;
  BuildMI(BB, DL, TII->get(ArePtrs64bit ? Mips::DADDiu : Mips::ADDiu), MaskLSB2)
    .addReg(ABI.GetNullPtr()).addImm(-4);
  BuildMI(BB, DL, TII->get(ArePtrs64bit ? Mips::AND64 : Mips::AND), AlignedAddr)
    .addReg(Ptr).addReg(MaskLSB2);
  BuildMI(BB, DL, TII->get(Mips::ANDi), PtrLSB2)
      .addReg(Ptr, 0, ArePtrs64bit ? Mips::sub_32 : 0).addImm(3);
  if (Subtarget.isLittle()) {
    BuildMI(BB, DL, TII->get(Mips::SLL), ShiftAmt).addReg(PtrLSB2).addImm(3);
  } else {
    Register Off = RegInfo.createVirtualRegister(RC);
    BuildMI(BB, DL, TII->get(Mips::XORi), Off)
      .addReg(PtrLSB2).addImm((Size == 1) ? 3 : 2);
    BuildMI(BB, DL, TII->get(Mips::SLL), ShiftAmt).addReg(Off).addImm(3);
  }
  BuildMI(BB, DL, TII->get(Mips::ORi), MaskUpper)
    .addReg(Mips::ZERO).addImm(MaskImm);
  BuildMI(BB, DL, TII->get(Mips::SLLV), Mask)
    .addReg(MaskUpper).addReg(ShiftAmt);
  BuildMI(BB, DL, TII->get(Mips::NOR), Mask2).addReg(Mips::ZERO).addReg(Mask);
  BuildMI(BB, DL, TII->get(Mips::ANDi), MaskedCmpVal)
    .addReg(CmpVal).addImm(MaskImm);
  BuildMI(BB, DL, TII->get(Mips::SLLV), ShiftedCmpVal)
    .addReg(MaskedCmpVal).addReg(ShiftAmt);
  BuildMI(BB, DL, TII->get(Mips::ANDi), MaskedNewVal)
    .addReg(NewVal).addImm(MaskImm);
  BuildMI(BB, DL, TII->get(Mips::SLLV), ShiftedNewVal)
    .addReg(MaskedNewVal).addReg(ShiftAmt);

  // The purposes of the flags on the scratch registers are explained in
  // emitAtomicBinary. In summary, we need a scratch register which is going to
  // be undef, that is unique among the register chosen for the instruction.

  BuildMI(BB, DL, TII->get(AtomicOp))
      .addReg(Dest, RegState::Define | RegState::EarlyClobber)
      .addReg(AlignedAddr)
      .addReg(Mask)
      .addReg(ShiftedCmpVal)
      .addReg(Mask2)
      .addReg(ShiftedNewVal)
      .addReg(ShiftAmt)
      .addReg(Scratch, RegState::EarlyClobber | RegState::Define |
                           RegState::Dead | RegState::Implicit)
      .addReg(Scratch2, RegState::EarlyClobber | RegState::Define |
                            RegState::Dead | RegState::Implicit);

  MI.eraseFromParent(); // The instruction is gone now.

  return exitMBB;
}

static SDValue setBounds(SelectionDAG &DAG, SDValue Val, SDValue Length,
                         bool CSetBoundsStatsLogged) {
  assert(CSetBoundsStatsLogged);
  (void)CSetBoundsStatsLogged;
  SDLoc DL(Val);
  Intrinsic::ID SetBounds = Intrinsic::cheri_cap_bounds_set;
  return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, Val.getValueType(),
        DAG.getConstant(SetBounds, DL, MVT::i64), Val,
        Length);
}

static SDValue setBounds(SelectionDAG &DAG, SDValue Val, uint64_t Length,
                         bool CSetBoundsStatsLogged) {
  return setBounds(DAG, Val, DAG.getIntPtrConstant(Length, SDLoc(Val)),
                   CSetBoundsStatsLogged);
}

static void addGlobalsCSetBoundsStats(const GlobalValue *GV, SelectionDAG &DAG,
                                      StringRef Pass, const DebugLoc &DL) {
  int64_t AllocSize = -1;
  Optional<uint64_t> Size = None;
  if (GV->getValueType()->isSized()) {
    Size = DAG.getDataLayout().getTypeStoreSize(GV->getValueType());
    AllocSize = DAG.getDataLayout().getTypeAllocSize(GV->getValueType());
  }
  cheri::CSetBoundsStats->add(
      GV->getPointerAlignment(DAG.getDataLayout()), Size, Pass,
      cheri::SetBoundsPointerSource::GlobalVar,
      "load of global " + GV->getName() + " (alloc size=" + Twine(AllocSize) +
          ")",
      cheri::inferSourceLocation(DL, DAG.getMachineFunction().getName()));
}

SDValue MipsTargetLowering::lowerADDRSPACECAST(SDValue Op, SelectionDAG &DAG)
  const {
  SDLoc DL(Op);
  SDValue Src = Op.getOperand(0);
  EVT DstTy = Op.getValueType();
  // A noop addrspacecast can happen when using the cap table because functions
  // can not yet be in AS200
  if (Src.getValueType() == Op.getValueType())
    return Src;
  if (ABI.IsCheriPureCap()) {
    // XXXAR: HACK for the capability table (we turn this into an iFATPTR later
    // so we can just return the value
    if (isa<GlobalAddressSDNode>(Src)) {
      return Op;
    }
  }
  if (Src.getValueType() == MVT::i64) {
    assert(Op.getValueType() == CapType);
    bool IsFunction = false;
    if (GlobalAddressSDNode *GN = dyn_cast<GlobalAddressSDNode>(Src)) {
      IsFunction = isa<Function>(GN->getGlobal()->getBaseObject());
    }
    LLVM_DEBUG(dbgs() << "Lowering addrspacecast: "; Op.dump(&DAG));
    // INTTOPTR will lower to a cincoffset on null in the purecap ABI, so we
    // need to use cfromddc here to keep the legacy ABI working:
    SDValue Ptr = IsFunction ? setPccOffset(DAG, DL, Src) : cFromDDC(DAG, DL, Src);
    if (auto *N = dyn_cast<GlobalAddressSDNode>(Src)) {
      const GlobalValue *GV = N->getGlobal();
      auto *Ty = GV->getValueType();
      if (Ty->isSized() && GV->isDefinitionExact() && !SkipGlobalBounds){
        uint64_t SizeBytes = DAG.getDataLayout().getTypeAllocSize(Ty);
        if (cheri::ShouldCollectCSetBoundsStats) {
          addGlobalsCSetBoundsStats(GV, DAG, "MipsTargetLowering::lowerADDRSPACECAST",
                                    N->getDebugLoc());
        }
        Ptr = setBounds(DAG, Ptr, SizeBytes, /*CSetBoundsStatsLogged=*/true);
      }
    }
    LLVM_DEBUG(dbgs() << "Lowering addrspacecast result: "; Ptr.dump(&DAG));
    return Ptr;
  }
  assert(Src.getValueType() == CapType);
  assert(Op.getValueType() == MVT::i64);
  assert(!ABI.IsCheriPureCap() && "Should not generate ptrtoint in captable");
  return DAG.getNode(ISD::PTRTOINT, DL, DstTy, Src);
}

SDValue MipsTargetLowering::lowerBRCOND(SDValue Op, SelectionDAG &DAG) const {
  // The first operand is the chain, the second is the condition, the third is
  // the block to branch to if the condition is true.
  SDValue Chain = Op.getOperand(0);
  SDValue Dest = Op.getOperand(2);
  SDLoc DL(Op);

  assert(!Subtarget.hasMips32r6() && !Subtarget.hasMips64r6());
  SDValue CondRes = createFPCmp(DAG, Op.getOperand(1));

  // Return if flag is not set by a floating point comparison.
  if (CondRes.getOpcode() != MipsISD::FPCmp)
    return Op;

  SDValue CCNode  = CondRes.getOperand(2);
  Mips::CondCode CC =
    (Mips::CondCode)cast<ConstantSDNode>(CCNode)->getZExtValue();
  unsigned Opc = invertFPCondCodeUser(CC) ? Mips::BRANCH_F : Mips::BRANCH_T;
  SDValue BrCode = DAG.getConstant(Opc, DL, MVT::i32);
  SDValue FCC0 = DAG.getRegister(Mips::FCC0, MVT::i32);
  return DAG.getNode(MipsISD::FPBrcond, DL, Op.getValueType(), Chain, BrCode,
                     FCC0, Dest, CondRes);
}

SDValue MipsTargetLowering::
lowerSELECT(SDValue Op, SelectionDAG &DAG) const
{
  assert(!Subtarget.hasMips32r6() && !Subtarget.hasMips64r6());
  SDValue Cond = createFPCmp(DAG, Op.getOperand(0));

  // Return if flag is not set by a floating point comparison.
  if (Cond.getOpcode() != MipsISD::FPCmp)
    return Op;

  return createCMovFP(DAG, Cond, Op.getOperand(1), Op.getOperand(2),
                      SDLoc(Op));
}

SDValue MipsTargetLowering::lowerSETCC(SDValue Op, SelectionDAG &DAG) const {
  assert(!Subtarget.hasMips32r6() && !Subtarget.hasMips64r6());
  SDValue Cond = createFPCmp(DAG, Op);

  assert(Cond.getOpcode() == MipsISD::FPCmp &&
         "Floating point operand expected.");

  SDLoc DL(Op);
  SDValue True  = DAG.getConstant(1, DL, MVT::i32);
  SDValue False = DAG.getConstant(0, DL, MVT::i32);

  return createCMovFP(DAG, Cond, True, False, DL);
}

SDValue MipsTargetLowering::lowerGlobalAddress(SDValue Op,
                                               SelectionDAG &DAG) const {
  EVT Ty = Op.getValueType();
  GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
  const GlobalValue *GV = N->getGlobal();
  const Type *GVTy = GV->getType();
  if (Ty.isFatPointer() && cheri::ShouldCollectCSetBoundsStats) {
    addGlobalsCSetBoundsStats(GV, DAG, "MipsTargetLowering::lowerGlobalAddress",
                              N->getDebugLoc());
  }

  if (ABI.IsCheriPureCap()) {
    if (GVTy->getPointerAddressSpace() == 0) {
      // We used to support a legacy ABI with globals in AS0, but this is now
      // invalid IR. Report a useful error message and exit instead of crashing.
      report_fatal_error("Found global " + GV->getName() +
                         "in address space 0. Please fix the input IR");
    }
    // FIXME: shouldn't functions have a R_MIPS_CHERI_CAPCALL relocation?
    bool CanUseCapTable = GVTy->isFunctionTy() || DAG.getDataLayout().isFatPointer(GVTy);
    EVT GlobalTy = Ty.isFatPointer() ? Ty : CapType;
    bool IsFnPtr =
      GVTy->isPointerTy() && GVTy->getPointerElementType()->isFunctionTy();

    if (IsFnPtr) {
      // FIXME: in the future it would be good to inline local function pointers
      // into the capability table directly (right now the value is a
      // void () addrspace(200)* addrspace(200)* instead of a
      // void () addrspace(200)*
      // llvm::errs() << "is fn ptr: " << IsFnPtr << "\n";

      if (!CanUseCapTable) {
        // Functions didn't have address spaces yet which is why this hack was
        // needed. See https://reviews.llvm.org/D37054
        // Should no longer be the case now so lets report and error
        report_fatal_error("Found function " + GV->getName() +
                           " not in AS200. Compiling old IR?");
        CanUseCapTable = true;
      }
    }
    if (CanUseCapTable) {
      // FIXME: or should this be something else? like in lowerCall?
      auto PtrInfo = MachinePointerInfo::getCapTable(DAG.getMachineFunction());
      auto Addr = getDataFromCapTable(N, SDLoc(N), GlobalTy, DAG,
                                      DAG.getEntryNode(), PtrInfo);
      // Also handle the case where a ptrtoint is used on a global:
      if (Ty.isFatPointer())
        return Addr;
      assert(GlobalTy == CapType && Ty == MVT::i64 && "Should only have a ptrtoint node here");
      return DAG.getNode(ISD::PTRTOINT, SDLoc(N), Ty, Addr);
    } else {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      GV->dump();
#endif
      llvm_unreachable("SHOULD HAVE USED CAP TABLE");
    }
  }

  assert(!ABI.IsCheriPureCap());
  if (!isPositionIndependent()) {
    const MipsTargetObjectFile *TLOF =
        static_cast<const MipsTargetObjectFile *>(
            getTargetMachine().getObjFileLowering());
    const GlobalObject *GO = GV->getBaseObject();
    if (GO && TLOF->IsGlobalInSmallSection(GO, getTargetMachine()))
      // %gp_rel relocation
      return getAddrGPRel(N, SDLoc(N), Ty, DAG, ABI.IsN64());

                                // %hi/%lo relocation
    return Subtarget.hasSym32() ? getAddrNonPIC(N, SDLoc(N), Ty, DAG)
                                // %highest/%higher/%hi/%lo relocation
                                : getAddrNonPICSym64(N, SDLoc(N), Ty, DAG);
  }

  // Every other architecture would use shouldAssumeDSOLocal in here, but
  // mips is special.
  // * In PIC code mips requires got loads even for local statics!
  // * To save on got entries, for local statics the got entry contains the
  //   page and an additional add instruction takes care of the low bits.
  // * It is legal to access a hidden symbol with a non hidden undefined,
  //   so one cannot guarantee that all access to a hidden symbol will know
  //   it is hidden.
  // * Mips linkers don't support creating a page and a full got entry for
  //   the same symbol.
  // * Given all that, we have to use a full got entry for hidden symbols :-(
  if (GV->hasLocalLinkage())
    return getAddrLocal(N, SDLoc(N), Ty, DAG, ABI.IsN32() || ABI.IsN64(),
                        GV->isThreadLocal());

  if (Subtarget.useXGOT() || HugeGOT)
    return getAddrGlobalLargeGOT(
        N, SDLoc(N), Ty, DAG, MipsII::MO_GOT_HI16, MipsII::MO_GOT_LO16,
        DAG.getEntryNode(),
        MachinePointerInfo::getGOT(DAG.getMachineFunction()),
        GV->isThreadLocal());

  return getAddrGlobal(
      N, SDLoc(N), Ty, DAG,
      (ABI.IsN32() || ABI.IsN64()) ? MipsII::MO_GOT_DISP : MipsII::MO_GOT,
      DAG.getEntryNode(), MachinePointerInfo::getGOT(DAG.getMachineFunction()),
      GV->isThreadLocal());
}

SDValue MipsTargetLowering::lowerBlockAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  BlockAddressSDNode *N = cast<BlockAddressSDNode>(Op);
  EVT Ty = Op.getValueType();
  MachineFunction &MF = DAG.getMachineFunction();

  if (!isPositionIndependent())
    return Subtarget.hasSym32() ? getAddrNonPIC(N, SDLoc(N), Ty, DAG)
                                : getAddrNonPICSym64(N, SDLoc(N), Ty, DAG);

  if (ABI.IsCheriPureCap()) {
    if (N->getBlockAddress()->getFunction() != &MF.getFunction())
      report_fatal_error(
          "Should only get a blockaddress for the current function");
    auto BANode = DAG.getTargetBlockAddress(N->getBlockAddress(), MVT::i64,
                                            N->getOffset());
    // TODO: add the offset as another node?
    assert(N->getOffset() == 0 && "nonzero offset is not supported");
    // auto Offset = DAG.getConstant(N->getOffset(), SDLoc(N), MVT::i64);
    return SDValue(DAG.getMachineNode(Mips::PseudoPccRelativeAddress, SDLoc(N),
                                      CapType, BANode),
                   0);
  } else {
    return getAddrLocal(N, SDLoc(N), Ty, DAG, ABI.IsN32() || ABI.IsN64(),
                        /*IsForTls=*/false);
  }
}

SDValue MipsTargetLowering::
lowerGlobalTLSAddress(SDValue Op, SelectionDAG &DAG) const
{
  // If the relocation model is PIC, use the General Dynamic TLS Model or
  // Local Dynamic TLS model, otherwise use the Initial Exec or
  // Local Exec TLS Model.

  GlobalAddressSDNode *GA = cast<GlobalAddressSDNode>(Op);
  if (DAG.getTarget().useEmulatedTLS())
    return LowerToTLSEmulatedModel(GA, DAG);

  SDLoc DL(GA);
  const GlobalValue *GV = GA->getGlobal();
  TLSModel::Model model = getTargetMachine().getTLSModel(GV);

  if (Subtarget.getABI().IsCheriPureCap()) {
    const Type *GVTy = GV->getType();
    if (DAG.getDataLayout().isFatPointer(GVTy)) {
      EVT OffsetVT = getPointerRangeTy(DAG.getDataLayout(),
                                       GVTy->getPointerAddressSpace());
      EVT CapVT = CapType;

      if (model == TLSModel::GeneralDynamic || model == TLSModel::LocalDynamic) {
        // General Dynamic and Local Dynamic TLS Model.
        unsigned HiFlag, LoFlag;
        if (model == TLSModel::GeneralDynamic) {
          HiFlag = MipsII::MO_CAPTAB_TLSGD_HI16;
          LoFlag = MipsII::MO_CAPTAB_TLSGD_LO16;
        } else {
          HiFlag = MipsII::MO_CAPTAB_TLSLDM_HI16;
          LoFlag = MipsII::MO_CAPTAB_TLSLDM_LO16;
        }

        SDValue Argument = getCapToCapTable(GA, DL, DAG, HiFlag, LoFlag, 0,
                                            MipsISD::TlsHi);
        // Set bounds to sizeof(tls_index)
        if (cheri::ShouldCollectCSetBoundsStats)
          addGlobalsCSetBoundsStats(GV, DAG, "set bounds on tls_index",
                                    DL.getDebugLoc());
        Argument = setBounds(DAG, Argument, 16, /*CSetBoundsStatsLogged=*/true);
        Type *CapTy = Type::getInt8PtrTy(*DAG.getContext(),
                                         GVTy->getPointerAddressSpace());

        SDValue TlsGetAddr = DAG.getExternalFunctionSymbol("__tls_get_addr");

        ArgListTy Args;
        ArgListEntry Entry;
        Entry.Node = Argument;
        Entry.Ty = CapTy;
        Args.push_back(Entry);

        TargetLowering::CallLoweringInfo CLI(DAG);
        CLI.setDebugLoc(DL)
            .setChain(DAG.getEntryNode())
            .setLibCallee(CallingConv::C, CapTy, TlsGetAddr, std::move(Args));
        std::pair<SDValue, SDValue> CallResult = LowerCallTo(CLI);

        SDValue Ret = CallResult.first;

        if (model != TLSModel::LocalDynamic)
          return Ret;

        SDValue TGAHi = DAG.getTargetGlobalAddress(GV, DL, OffsetVT, 0,
                                                   MipsII::MO_DTPREL_HI);
        SDValue Hi = DAG.getNode(MipsISD::TlsHi, DL, OffsetVT, TGAHi);
        SDValue TGALo = DAG.getTargetGlobalAddress(GV, DL, OffsetVT, 0,
                                                   MipsII::MO_DTPREL_LO);
        SDValue Lo = DAG.getNode(MipsISD::Lo, DL, OffsetVT, TGALo);
        SDValue Add = DAG.getNode(ISD::ADD, DL, OffsetVT, Hi, Lo);
        return DAG.getNode(ISD::PTRADD, DL, CapVT, Ret, Add);
      }

      SDValue Offset;
      if (model == TLSModel::InitialExec) {
        // Initial Exec TLS Model
        auto PtrInfo =
          MachinePointerInfo::getCapTable(DAG.getMachineFunction());
        Offset =
          getFromCapTable(GA, DL, OffsetVT, DAG, DAG.getEntryNode(), PtrInfo,
                          MipsII::MO_CAPTAB_TPREL_HI16,
                          MipsII::MO_CAPTAB_TPREL_LO16, 0, MipsISD::TlsHi);
      } else {
        // Local Exec TLS Model
        assert(model == TLSModel::LocalExec);
        SDValue TGAHi = DAG.getTargetGlobalAddress(GV, DL, OffsetVT, 0,
                                                   MipsII::MO_TPREL_HI);
        SDValue TGALo = DAG.getTargetGlobalAddress(GV, DL, OffsetVT, 0,
                                                   MipsII::MO_TPREL_LO);
        SDValue Hi = DAG.getNode(MipsISD::TlsHi, DL, OffsetVT, TGAHi);
        SDValue Lo = DAG.getNode(MipsISD::Lo, DL, OffsetVT, TGALo);
        Offset = DAG.getNode(ISD::ADD, DL, OffsetVT, Hi, Lo);
      }

      SDValue ThreadPointer = DAG.getNode(MipsISD::CapThreadPointer,
                                          DL, CapVT);
      return DAG.getNode(ISD::PTRADD, DL, CapVT, ThreadPointer, Offset);
    } else {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      GV->dump();
#endif
      report_fatal_error("SHOULD HAVE USED CAP TLS for " + GV->getName());
    }
  }

  // NOTE: AS0 is okay since we are using the legacy ABI
  assert(!ABI.IsCheriPureCap());
  EVT PtrVT = getPointerTy(DAG.getDataLayout(), 0);

  if (model == TLSModel::GeneralDynamic || model == TLSModel::LocalDynamic) {
    // General Dynamic and Local Dynamic TLS Model.
    unsigned Flag = (model == TLSModel::LocalDynamic) ? MipsII::MO_TLSLDM
                                                      : MipsII::MO_TLSGD;

    SDValue TGA = DAG.getTargetGlobalAddress(GV, DL, PtrVT, 0, Flag);
    SDValue Argument = DAG.getNode(MipsISD::Wrapper, DL, PtrVT,
                                   getGlobalReg(DAG, PtrVT, /*IsForTls=*/true), TGA);
    unsigned PtrSize = PtrVT.getSizeInBits();
    Type *TlsGetAddrArgAndRetTy = Type::getIntNTy(*DAG.getContext(), PtrSize);

    SDValue TlsGetAddr = DAG.getExternalFunctionSymbol("__tls_get_addr");

    ArgListTy Args;
    ArgListEntry Entry;
    Entry.Node = Argument;
    Entry.Ty = TlsGetAddrArgAndRetTy;
    Args.push_back(Entry);

    TargetLowering::CallLoweringInfo CLI(DAG);
    CLI.setDebugLoc(DL)
        .setChain(DAG.getEntryNode())
        .setLibCallee(CallingConv::C, TlsGetAddrArgAndRetTy, TlsGetAddr, std::move(Args));
    std::pair<SDValue, SDValue> CallResult = LowerCallTo(CLI);

    SDValue Ret = CallResult.first;

    if (model != TLSModel::LocalDynamic)
      return Ret;

    SDValue TGAHi = DAG.getTargetGlobalAddress(GV, DL, PtrVT, 0,
                                               MipsII::MO_DTPREL_HI);
    SDValue Hi = DAG.getNode(MipsISD::TlsHi, DL, PtrVT, TGAHi);
    SDValue TGALo = DAG.getTargetGlobalAddress(GV, DL, PtrVT, 0,
                                               MipsII::MO_DTPREL_LO);
    SDValue Lo = DAG.getNode(MipsISD::Lo, DL, PtrVT, TGALo);
    SDValue Add = DAG.getNode(ISD::ADD, DL, PtrVT, Hi, Ret);
    return DAG.getNode(ISD::ADD, DL, PtrVT, Add, Lo);
  }

  SDValue Offset;
  if (model == TLSModel::InitialExec) {
    // Initial Exec TLS Model
    SDValue TGA = DAG.getTargetGlobalAddress(GV, DL, PtrVT, 0,
                                             MipsII::MO_GOTTPREL);
    TGA = DAG.getNode(MipsISD::Wrapper, DL, PtrVT, getGlobalReg(DAG, PtrVT, true),
                      TGA);
    Offset =
        DAG.getLoad(PtrVT, DL, DAG.getEntryNode(), TGA, MachinePointerInfo());
  } else {
    // Local Exec TLS Model
    assert(model == TLSModel::LocalExec);
    SDValue TGAHi = DAG.getTargetGlobalAddress(GV, DL, PtrVT, 0,
                                               MipsII::MO_TPREL_HI);
    SDValue TGALo = DAG.getTargetGlobalAddress(GV, DL, PtrVT, 0,
                                               MipsII::MO_TPREL_LO);
    SDValue Hi = DAG.getNode(MipsISD::TlsHi, DL, PtrVT, TGAHi);
    SDValue Lo = DAG.getNode(MipsISD::Lo, DL, PtrVT, TGALo);
    Offset = DAG.getNode(ISD::ADD, DL, PtrVT, Hi, Lo);
  }

  SDValue ThreadPointer = DAG.getNode(MipsISD::ThreadPointer, DL, PtrVT);
  return DAG.getNode(ISD::ADD, DL, PtrVT, ThreadPointer, Offset);
}

SDValue MipsTargetLowering::
lowerJumpTable(SDValue Op, SelectionDAG &DAG) const
{
  JumpTableSDNode *N = cast<JumpTableSDNode>(Op);
  EVT Ty = Op.getValueType();

  if (ABI.IsCheriPureCap()) {
    auto PtrInfo = MachinePointerInfo::getCapTable(DAG.getMachineFunction());
    return getDataFromCapTable(N, SDLoc(N), CapType, DAG, DAG.getEntryNode(),
                               PtrInfo);
  }

  if (!isPositionIndependent())
    return Subtarget.hasSym32() ? getAddrNonPIC(N, SDLoc(N), Ty, DAG)
                                : getAddrNonPICSym64(N, SDLoc(N), Ty, DAG);

  if (Subtarget.useXGOT())
    return getAddrGlobalLargeGOT(N, SDLoc(N), Ty, DAG, MipsII::MO_GOT_HI16,
                                 MipsII::MO_GOT_LO16, DAG.getEntryNode(),
                                 MachinePointerInfo::getGOT(DAG.getMachineFunction()), /*IsForTls=*/false);

  return getAddrLocal(N, SDLoc(N), Ty, DAG, ABI.IsN32() || ABI.IsN64(), /*IsForTls=*/false);
}

SDValue MipsTargetLowering::
lowerConstantPool(SDValue Op, SelectionDAG &DAG) const
{
  ConstantPoolSDNode *N = cast<ConstantPoolSDNode>(Op);
  EVT Ty = Op.getValueType();

  if (ABI.IsCheriPureCap()) {
    auto PtrInfo = MachinePointerInfo::getCapTable(DAG.getMachineFunction());
    assert(Ty.isFatPointer());
    auto Result = getDataFromCapTable(N, SDLoc(N), Ty, DAG, DAG.getEntryNode(), PtrInfo);
    assert(Result.getValueType() == Ty);
    return Result;
  }

  if (!isPositionIndependent()) {
    const MipsTargetObjectFile *TLOF =
        static_cast<const MipsTargetObjectFile *>(
            getTargetMachine().getObjFileLowering());

    if (TLOF->IsConstantInSmallSection(DAG.getDataLayout(), N->getConstVal(),
                                       getTargetMachine()))
      // %gp_rel relocation
      return getAddrGPRel(N, SDLoc(N), Ty, DAG, ABI.IsN64());

    return Subtarget.hasSym32() ? getAddrNonPIC(N, SDLoc(N), Ty, DAG)
                                : getAddrNonPICSym64(N, SDLoc(N), Ty, DAG);
  }

  return getAddrLocal(N, SDLoc(N), Ty, DAG, ABI.IsN32() || ABI.IsN64(),
                          /*IsForTls=*/false);
}

SDValue MipsTargetLowering::lowerVASTART(SDValue Op, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MipsFunctionInfo *FuncInfo = MF.getInfo<MipsFunctionInfo>();

  SDLoc DL(Op);
  SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                 getPointerTy(MF.getDataLayout(),
                                     ABI.StackAddrSpace()));

  if (ABI.IsCheriPureCap()) {
    unsigned Reg = MF.addLiveIn(Mips::C13, getRegClassFor(CapType));
    // In the sandbox ABI, the va_start intrinsic will (to work around LLVM's
    // assumption that allocas are all in AS0) be an address space cast of the
    // alloca to AS 0.  We need to extract the original operands.
    const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
    SV = SV->stripPointerCasts();
    assert(isCheriPointer(SV->getType(), &DAG.getDataLayout()));
    SDValue Chain = Op.getOperand(0);
    SDValue CapAddr = Op.getOperand(1);
    FI = DAG.getCopyFromReg(DAG.getEntryNode(), DL, Reg, CapType);
    return DAG.getStore(Chain, DL, FI, CapAddr, MachinePointerInfo(SV));
  }

  // vastart just stores the address of the VarArgsFrameIndex slot into the
  // memory location argument.
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), DL, FI, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

SDValue MipsTargetLowering::lowerVACOPY(SDValue Op, SelectionDAG &DAG) const {
  // VACOPY lowering should only be custom with the sandbox ABI.
  assert(ABI.IsCheriPureCap());

  SDValue Chain = Op->getOperand(0);
  SDValue Dest = Op->getOperand(1);
  SDValue Src = Op->getOperand(2);
  SDLoc DL(Op);
  // The source is a pointer to the existing va_list
  Src = DAG.getLoad(CapType, DL, Chain, Src, MachinePointerInfo());
  return DAG.getStore(Chain, DL, Src, Dest, MachinePointerInfo());
}

SDValue MipsTargetLowering::lowerVAARG(SDValue Op, SelectionDAG &DAG) const {
  SDNode *Node = Op.getNode();
  EVT VT = Node->getValueType(0);
  SDValue Chain = Node->getOperand(0);
  SDValue VAListPtr = Node->getOperand(1);
  const Align Align =
      llvm::MaybeAlign(Node->getConstantOperandVal(3)).valueOrOne();
  const Value *SV = cast<SrcValueSDNode>(Node->getOperand(2))->getValue();
  SDLoc DL(Node);
  unsigned ArgSlotSizeInBytes = (ABI.IsN32() || ABI.IsN64()) ? 8 : 4;

  SDValue VAListLoad =
      DAG.getLoad(getPointerTy(DAG.getDataLayout(),
                               DAG.getDataLayout().getAllocaAddrSpace()),
                  DL, Chain, VAListPtr, MachinePointerInfo(SV));
  SDValue VAList = VAListLoad;

  // Re-align the pointer if necessary.
  // It should only ever be necessary for 64-bit types on O32 since the minimum
  // argument alignment is the same as the maximum type alignment for N32/N64.
  //
  // FIXME: We currently align too often. The code generator doesn't notice
  //        when the pointer is still aligned from the last va_arg (or pair of
  //        va_args for the i64 on O32 case).
  if (Align > getMinStackArgumentAlignment()) {
    VAList = DAG.getNode(
        ISD::ADD, DL, VAList.getValueType(), VAList,
        DAG.getConstant(Align.value() - 1, DL, VAList.getValueType()));

    VAList = DAG.getNode(
        ISD::AND, DL, VAList.getValueType(), VAList,
        DAG.getConstant(-(int64_t)Align.value(), DL, VAList.getValueType()));
  }

  // Increment the pointer, VAList, to the next vaarg.
  auto &TD = DAG.getDataLayout();
  unsigned ArgSizeInBytes =
      TD.getTypeAllocSize(VT.getTypeForEVT(*DAG.getContext()));
  SDValue Tmp3 =
      DAG.getNode(ISD::ADD, DL, VAList.getValueType(), VAList,
                  DAG.getConstant(alignTo(ArgSizeInBytes, ArgSlotSizeInBytes),
                                  DL, VAList.getValueType()));
  // Store the incremented VAList to the legalized pointer
  Chain = DAG.getStore(VAListLoad.getValue(1), DL, Tmp3, VAListPtr,
                       MachinePointerInfo(SV));

  // In big-endian mode we must adjust the pointer when the load size is smaller
  // than the argument slot size. We must also reduce the known alignment to
  // match. For example in the N64 ABI, we must add 4 bytes to the offset to get
  // the correct half of the slot, and reduce the alignment from 8 (slot
  // alignment) down to 4 (type alignment).
  if (!Subtarget.isLittle() && ArgSizeInBytes < ArgSlotSizeInBytes) {
    unsigned Adjustment = ArgSlotSizeInBytes - ArgSizeInBytes;
    VAList = DAG.getNode(ISD::ADD, DL, VAListPtr.getValueType(), VAList,
                         DAG.getIntPtrConstant(Adjustment, DL));
  }
  // Load the actual argument out of the pointer VAList
  return DAG.getLoad(VT, DL, Chain, VAList, MachinePointerInfo());
}

static SDValue lowerFCOPYSIGN32(SDValue Op, SelectionDAG &DAG,
                                bool HasExtractInsert) {
  EVT TyX = Op.getOperand(0).getValueType();
  EVT TyY = Op.getOperand(1).getValueType();
  SDLoc DL(Op);
  SDValue Const1 = DAG.getConstant(1, DL, MVT::i32);
  SDValue Const31 = DAG.getConstant(31, DL, MVT::i32);
  SDValue Res;

  // If operand is of type f64, extract the upper 32-bit. Otherwise, bitcast it
  // to i32.
  SDValue X = (TyX == MVT::f32) ?
    DAG.getNode(ISD::BITCAST, DL, MVT::i32, Op.getOperand(0)) :
    DAG.getNode(MipsISD::ExtractElementF64, DL, MVT::i32, Op.getOperand(0),
                Const1);
  SDValue Y = (TyY == MVT::f32) ?
    DAG.getNode(ISD::BITCAST, DL, MVT::i32, Op.getOperand(1)) :
    DAG.getNode(MipsISD::ExtractElementF64, DL, MVT::i32, Op.getOperand(1),
                Const1);

  if (HasExtractInsert) {
    // ext  E, Y, 31, 1  ; extract bit31 of Y
    // ins  X, E, 31, 1  ; insert extracted bit at bit31 of X
    SDValue E = DAG.getNode(MipsISD::Ext, DL, MVT::i32, Y, Const31, Const1);
    Res = DAG.getNode(MipsISD::Ins, DL, MVT::i32, E, Const31, Const1, X);
  } else {
    // sll SllX, X, 1
    // srl SrlX, SllX, 1
    // srl SrlY, Y, 31
    // sll SllY, SrlX, 31
    // or  Or, SrlX, SllY
    SDValue SllX = DAG.getNode(ISD::SHL, DL, MVT::i32, X, Const1);
    SDValue SrlX = DAG.getNode(ISD::SRL, DL, MVT::i32, SllX, Const1);
    SDValue SrlY = DAG.getNode(ISD::SRL, DL, MVT::i32, Y, Const31);
    SDValue SllY = DAG.getNode(ISD::SHL, DL, MVT::i32, SrlY, Const31);
    Res = DAG.getNode(ISD::OR, DL, MVT::i32, SrlX, SllY);
  }

  if (TyX == MVT::f32)
    return DAG.getNode(ISD::BITCAST, DL, Op.getOperand(0).getValueType(), Res);

  SDValue LowX = DAG.getNode(MipsISD::ExtractElementF64, DL, MVT::i32,
                             Op.getOperand(0),
                             DAG.getConstant(0, DL, MVT::i32));
  return DAG.getNode(MipsISD::BuildPairF64, DL, MVT::f64, LowX, Res);
}

static SDValue lowerFCOPYSIGN64(SDValue Op, SelectionDAG &DAG,
                                bool HasExtractInsert) {
  unsigned WidthX = Op.getOperand(0).getValueSizeInBits();
  unsigned WidthY = Op.getOperand(1).getValueSizeInBits();
  EVT TyX = MVT::getIntegerVT(WidthX), TyY = MVT::getIntegerVT(WidthY);
  SDLoc DL(Op);
  SDValue Const1 = DAG.getConstant(1, DL, MVT::i32);

  // Bitcast to integer nodes.
  SDValue X = DAG.getNode(ISD::BITCAST, DL, TyX, Op.getOperand(0));
  SDValue Y = DAG.getNode(ISD::BITCAST, DL, TyY, Op.getOperand(1));

  if (HasExtractInsert) {
    // ext  E, Y, width(Y) - 1, 1  ; extract bit width(Y)-1 of Y
    // ins  X, E, width(X) - 1, 1  ; insert extracted bit at bit width(X)-1 of X
    SDValue E = DAG.getNode(MipsISD::Ext, DL, TyY, Y,
                            DAG.getConstant(WidthY - 1, DL, MVT::i32), Const1);

    if (WidthX > WidthY)
      E = DAG.getNode(ISD::ZERO_EXTEND, DL, TyX, E);
    else if (WidthY > WidthX)
      E = DAG.getNode(ISD::TRUNCATE, DL, TyX, E);

    SDValue I = DAG.getNode(MipsISD::Ins, DL, TyX, E,
                            DAG.getConstant(WidthX - 1, DL, MVT::i32), Const1,
                            X);
    return DAG.getNode(ISD::BITCAST, DL, Op.getOperand(0).getValueType(), I);
  }

  // (d)sll SllX, X, 1
  // (d)srl SrlX, SllX, 1
  // (d)srl SrlY, Y, width(Y)-1
  // (d)sll SllY, SrlX, width(Y)-1
  // or     Or, SrlX, SllY
  SDValue SllX = DAG.getNode(ISD::SHL, DL, TyX, X, Const1);
  SDValue SrlX = DAG.getNode(ISD::SRL, DL, TyX, SllX, Const1);
  SDValue SrlY = DAG.getNode(ISD::SRL, DL, TyY, Y,
                             DAG.getConstant(WidthY - 1, DL, MVT::i32));

  if (WidthX > WidthY)
    SrlY = DAG.getNode(ISD::ZERO_EXTEND, DL, TyX, SrlY);
  else if (WidthY > WidthX)
    SrlY = DAG.getNode(ISD::TRUNCATE, DL, TyX, SrlY);

  SDValue SllY = DAG.getNode(ISD::SHL, DL, TyX, SrlY,
                             DAG.getConstant(WidthX - 1, DL, MVT::i32));
  SDValue Or = DAG.getNode(ISD::OR, DL, TyX, SrlX, SllY);
  return DAG.getNode(ISD::BITCAST, DL, Op.getOperand(0).getValueType(), Or);
}

SDValue
MipsTargetLowering::lowerFCOPYSIGN(SDValue Op, SelectionDAG &DAG) const {
  if (Subtarget.isGP64bit())
    return lowerFCOPYSIGN64(Op, DAG, Subtarget.hasExtractInsert());

  return lowerFCOPYSIGN32(Op, DAG, Subtarget.hasExtractInsert());
}

static SDValue lowerFABS32(SDValue Op, SelectionDAG &DAG,
                           bool HasExtractInsert) {
  SDLoc DL(Op);
  SDValue Res, Const1 = DAG.getConstant(1, DL, MVT::i32);

  // If operand is of type f64, extract the upper 32-bit. Otherwise, bitcast it
  // to i32.
  SDValue X = (Op.getValueType() == MVT::f32)
                  ? DAG.getNode(ISD::BITCAST, DL, MVT::i32, Op.getOperand(0))
                  : DAG.getNode(MipsISD::ExtractElementF64, DL, MVT::i32,
                                Op.getOperand(0), Const1);

  // Clear MSB.
  if (HasExtractInsert)
    Res = DAG.getNode(MipsISD::Ins, DL, MVT::i32,
                      DAG.getRegister(Mips::ZERO, MVT::i32),
                      DAG.getConstant(31, DL, MVT::i32), Const1, X);
  else {
    // TODO: Provide DAG patterns which transform (and x, cst)
    // back to a (shl (srl x (clz cst)) (clz cst)) sequence.
    SDValue SllX = DAG.getNode(ISD::SHL, DL, MVT::i32, X, Const1);
    Res = DAG.getNode(ISD::SRL, DL, MVT::i32, SllX, Const1);
  }

  if (Op.getValueType() == MVT::f32)
    return DAG.getNode(ISD::BITCAST, DL, MVT::f32, Res);

  // FIXME: For mips32r2, the sequence of (BuildPairF64 (ins (ExtractElementF64
  // Op 1), $zero, 31 1) (ExtractElementF64 Op 0)) and the Op has one use, we
  // should be able to drop the usage of mfc1/mtc1 and rewrite the register in
  // place.
  SDValue LowX =
      DAG.getNode(MipsISD::ExtractElementF64, DL, MVT::i32, Op.getOperand(0),
                  DAG.getConstant(0, DL, MVT::i32));
  return DAG.getNode(MipsISD::BuildPairF64, DL, MVT::f64, LowX, Res);
}

static SDValue lowerFABS64(SDValue Op, SelectionDAG &DAG,
                           bool HasExtractInsert) {
  SDLoc DL(Op);
  SDValue Res, Const1 = DAG.getConstant(1, DL, MVT::i32);

  // Bitcast to integer node.
  SDValue X = DAG.getNode(ISD::BITCAST, DL, MVT::i64, Op.getOperand(0));

  // Clear MSB.
  if (HasExtractInsert)
    Res = DAG.getNode(MipsISD::Ins, DL, MVT::i64,
                      DAG.getRegister(Mips::ZERO_64, MVT::i64),
                      DAG.getConstant(63, DL, MVT::i32), Const1, X);
  else {
    SDValue SllX = DAG.getNode(ISD::SHL, DL, MVT::i64, X, Const1);
    Res = DAG.getNode(ISD::SRL, DL, MVT::i64, SllX, Const1);
  }

  return DAG.getNode(ISD::BITCAST, DL, MVT::f64, Res);
}

SDValue MipsTargetLowering::lowerFABS(SDValue Op, SelectionDAG &DAG) const {
  if ((ABI.IsN32() || ABI.IsN64()) && (Op.getValueType() == MVT::f64))
    return lowerFABS64(Op, DAG, Subtarget.hasExtractInsert());

  return lowerFABS32(Op, DAG, Subtarget.hasExtractInsert());
}

SDValue MipsTargetLowering::
lowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const {
  // check the depth
  if (cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue() != 0) {
    DAG.getContext()->emitError(
        "return address can be determined only for current frame");
    return SDValue();
  }

  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  MFI.setFrameAddressIsTaken(true);
  EVT VT = Op.getValueType();
  SDLoc DL(Op);
  SDValue FrameAddr =
      DAG.getCopyFromReg(DAG.getEntryNode(), DL, ABI.GetFramePtr(), VT);
  if (ABI.IsCheriPureCap()) {
    assert(VT == CapType);
  }
  return FrameAddr;
}

SDValue MipsTargetLowering::lowerRETURNADDR(SDValue Op,
                                            SelectionDAG &DAG) const {
  if (verifyReturnAddressArgumentIsConstant(Op, DAG))
    return SDValue();

  // check the depth
  if (cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue() != 0) {
    DAG.getContext()->emitError(
        "return address can be determined only for current frame");
    return SDValue();
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MVT VT = Op.getSimpleValueType();
  unsigned RA = ABI.GetReturnAddress();
  if (ABI.IsCheriPureCap()) {
     assert(VT == CapType);
  }
  MFI.setReturnAddressIsTaken(true);

  // Return RA, which contains the return address. Mark it an implicit live-in.
  unsigned Reg = MF.addLiveIn(RA, getRegClassFor(VT));
  return DAG.getCopyFromReg(DAG.getEntryNode(), SDLoc(Op), Reg, VT);
}

// An EH_RETURN is the result of lowering llvm.eh.return which in turn is
// generated from __builtin_eh_return (offset, handler)
// The effect of this is to adjust the stack pointer by "offset"
// and then branch to "handler".
SDValue MipsTargetLowering::lowerEH_RETURN(SDValue Op, SelectionDAG &DAG)
                                                                     const {
  MachineFunction &MF = DAG.getMachineFunction();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();

  if (ABI.IsCheriPureCap())
    report_fatal_error("MipsTargetLowering::lowerEH_RETURN needs fixing!");
  unsigned AS = getExceptionPointerAS();

  MipsFI->setCallsEhReturn();
  SDValue Chain     = Op.getOperand(0);
  SDValue Offset    = Op.getOperand(1);
  SDValue Handler   = Op.getOperand(2);
  SDLoc DL(Op);
  EVT Ty = ABI.IsN64() ? MVT::i64 : MVT::i32;

  // Store stack offset in V1, store jump target in V0. Glue CopyToReg and
  // EH_RETURN nodes, so that instructions are emitted back-to-back.
  unsigned OffsetReg = ABI.IsN64() ? Mips::V1_64 : Mips::V1;
  unsigned AddrReg = ABI.IsN64() ? Mips::V0_64 : Mips::V0;
  Chain = DAG.getCopyToReg(Chain, DL, OffsetReg, Offset, SDValue());
  Chain = DAG.getCopyToReg(Chain, DL, AddrReg, Handler, Chain.getValue(1));
  return DAG.getNode(
      MipsISD::EH_RETURN, DL, MVT::Other, Chain, DAG.getRegister(OffsetReg, Ty),
      DAG.getRegister(AddrReg, getPointerTy(MF.getDataLayout(), AS)),
      Chain.getValue(1));
}

SDValue MipsTargetLowering::lowerATOMIC_FENCE(SDValue Op,
                                              SelectionDAG &DAG) const {
  // FIXME: Need pseudo-fence for 'singlethread' fences
  // FIXME: Set SType for weaker fences where supported/appropriate.
  unsigned SType = 0;
  SDLoc DL(Op);
  return DAG.getNode(MipsISD::Sync, DL, MVT::Other, Op.getOperand(0),
                     DAG.getConstant(SType, DL, MVT::i32));
}

SDValue MipsTargetLowering::lowerShiftLeftParts(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDLoc DL(Op);
  MVT VT = Subtarget.isGP64bit() ? MVT::i64 : MVT::i32;

  SDValue Lo = Op.getOperand(0), Hi = Op.getOperand(1);
  SDValue Shamt = Op.getOperand(2);
  // if shamt < (VT.bits):
  //  lo = (shl lo, shamt)
  //  hi = (or (shl hi, shamt) (srl (srl lo, 1), ~shamt))
  // else:
  //  lo = 0
  //  hi = (shl lo, shamt[4:0])
  SDValue Not = DAG.getNode(ISD::XOR, DL, MVT::i32, Shamt,
                            DAG.getConstant(-1, DL, MVT::i32));
  SDValue ShiftRight1Lo = DAG.getNode(ISD::SRL, DL, VT, Lo,
                                      DAG.getConstant(1, DL, VT));
  SDValue ShiftRightLo = DAG.getNode(ISD::SRL, DL, VT, ShiftRight1Lo, Not);
  SDValue ShiftLeftHi = DAG.getNode(ISD::SHL, DL, VT, Hi, Shamt);
  SDValue Or = DAG.getNode(ISD::OR, DL, VT, ShiftLeftHi, ShiftRightLo);
  SDValue ShiftLeftLo = DAG.getNode(ISD::SHL, DL, VT, Lo, Shamt);
  SDValue Cond = DAG.getNode(ISD::AND, DL, MVT::i32, Shamt,
                             DAG.getConstant(VT.getSizeInBits(), DL, MVT::i32));
  Lo = DAG.getNode(ISD::SELECT, DL, VT, Cond,
                   DAG.getConstant(0, DL, VT), ShiftLeftLo);
  Hi = DAG.getNode(ISD::SELECT, DL, VT, Cond, ShiftLeftLo, Or);

  SDValue Ops[2] = {Lo, Hi};
  return DAG.getMergeValues(Ops, DL);
}

SDValue MipsTargetLowering::lowerShiftRightParts(SDValue Op, SelectionDAG &DAG,
                                                 bool IsSRA) const {
  SDLoc DL(Op);
  SDValue Lo = Op.getOperand(0), Hi = Op.getOperand(1);
  SDValue Shamt = Op.getOperand(2);
  MVT VT = Subtarget.isGP64bit() ? MVT::i64 : MVT::i32;

  // if shamt < (VT.bits):
  //  lo = (or (shl (shl hi, 1), ~shamt) (srl lo, shamt))
  //  if isSRA:
  //    hi = (sra hi, shamt)
  //  else:
  //    hi = (srl hi, shamt)
  // else:
  //  if isSRA:
  //   lo = (sra hi, shamt[4:0])
  //   hi = (sra hi, 31)
  //  else:
  //   lo = (srl hi, shamt[4:0])
  //   hi = 0
  SDValue Not = DAG.getNode(ISD::XOR, DL, MVT::i32, Shamt,
                            DAG.getConstant(-1, DL, MVT::i32));
  SDValue ShiftLeft1Hi = DAG.getNode(ISD::SHL, DL, VT, Hi,
                                     DAG.getConstant(1, DL, VT));
  SDValue ShiftLeftHi = DAG.getNode(ISD::SHL, DL, VT, ShiftLeft1Hi, Not);
  SDValue ShiftRightLo = DAG.getNode(ISD::SRL, DL, VT, Lo, Shamt);
  SDValue Or = DAG.getNode(ISD::OR, DL, VT, ShiftLeftHi, ShiftRightLo);
  SDValue ShiftRightHi = DAG.getNode(IsSRA ? ISD::SRA : ISD::SRL,
                                     DL, VT, Hi, Shamt);
  SDValue Cond = DAG.getNode(ISD::AND, DL, MVT::i32, Shamt,
                             DAG.getConstant(VT.getSizeInBits(), DL, MVT::i32));
  SDValue Ext = DAG.getNode(ISD::SRA, DL, VT, Hi,
                            DAG.getConstant(VT.getSizeInBits() - 1, DL, VT));

  if (!(Subtarget.hasMips4() || Subtarget.hasMips32())) {
    SDVTList VTList = DAG.getVTList(VT, VT);
    return DAG.getNode(Subtarget.isGP64bit() ? Mips::PseudoD_SELECT_I64
                                             : Mips::PseudoD_SELECT_I,
                       DL, VTList, Cond, ShiftRightHi,
                       IsSRA ? Ext : DAG.getConstant(0, DL, VT), Or,
                       ShiftRightHi);
  }

  Lo = DAG.getNode(ISD::SELECT, DL, VT, Cond, ShiftRightHi, Or);
  Hi = DAG.getNode(ISD::SELECT, DL, VT, Cond,
                   IsSRA ? Ext : DAG.getConstant(0, DL, VT), ShiftRightHi);

  SDValue Ops[2] = {Lo, Hi};
  return DAG.getMergeValues(Ops, DL);
}

static SDValue createLoadLR(unsigned Opc, SelectionDAG &DAG, LoadSDNode *LD,
                            SDValue Chain, SDValue Src, unsigned Offset) {
  SDValue Ptr = LD->getBasePtr();
  EVT VT = LD->getValueType(0), MemVT = LD->getMemoryVT();
  SDLoc DL(LD);
  SDVTList VTList = DAG.getVTList(VT, MVT::Other);

  if (Offset)
    Ptr = DAG.getPointerAdd(DL, Ptr, Offset);

  SDValue Ops[] = { Chain, Ptr, Src };
  return DAG.getMemIntrinsicNode(Opc, DL, VTList, Ops, MemVT,
                                 LD->getMemOperand());
}
// Expand an unaligned 32 or 64-bit integer load node.
SDValue MipsTargetLowering::lowerLOAD(SDValue Op, SelectionDAG &DAG) const {
  LoadSDNode *LD = cast<LoadSDNode>(Op);
  EVT MemVT = LD->getMemoryVT();

  if (Subtarget.systemSupportsUnalignedAccess(LD->getAddressSpace()))
    return Op;

  // We don't handle capability-releative loads here, so make sure that we
  // don't encounter them!
  assert(!MemVT.isFatPointer());

  // Return if load is aligned or if MemVT is neither i32 nor i64.
  if ((LD->getAlignment() >= MemVT.getSizeInBits() / 8)) {
    return SDValue();
  } else if (LD->getBasePtr()->getValueType(0).isFatPointer()) {
    // No capability version of LWL/LWR -> fall back to generic code
    std::pair<SDValue, SDValue> P = expandUnalignedLoad(LD, DAG);
    return DAG.getMergeValues({P.first, P.second}, SDLoc(LD));
  } else if (((MemVT != MVT::i32) && (MemVT != MVT::i64))) {
    return SDValue();
  }

  bool IsLittle = Subtarget.isLittle();
  EVT VT = Op.getValueType();
  ISD::LoadExtType ExtType = LD->getExtensionType();
  SDValue Chain = LD->getChain(), Undef = DAG.getUNDEF(VT);

  assert((VT == MVT::i32) || (VT == MVT::i64));

  assert(!Subtarget.isABI_CheriPureCap() && "LWL/LWR not supported in purecap");

  // Expand
  //  (set dst, (i64 (load baseptr)))
  // to
  //  (set tmp, (ldl (add baseptr, 7), undef))
  //  (set dst, (ldr baseptr, tmp))
  if ((VT == MVT::i64) && (ExtType == ISD::NON_EXTLOAD)) {
    SDValue LDL = createLoadLR(MipsISD::LDL, DAG, LD, Chain, Undef,
                               IsLittle ? 7 : 0);
    return createLoadLR(MipsISD::LDR, DAG, LD, LDL.getValue(1), LDL,
                        IsLittle ? 0 : 7);
  }

  SDValue LWL = createLoadLR(MipsISD::LWL, DAG, LD, Chain, Undef,
                             IsLittle ? 3 : 0);
  SDValue LWR = createLoadLR(MipsISD::LWR, DAG, LD, LWL.getValue(1), LWL,
                             IsLittle ? 0 : 3);

  // Expand
  //  (set dst, (i32 (load baseptr))) or
  //  (set dst, (i64 (sextload baseptr))) or
  //  (set dst, (i64 (extload baseptr)))
  // to
  //  (set tmp, (lwl (add baseptr, 3), undef))
  //  (set dst, (lwr baseptr, tmp))
  if ((VT == MVT::i32) || (ExtType == ISD::SEXTLOAD) ||
      (ExtType == ISD::EXTLOAD))
    return LWR;

  assert((VT == MVT::i64) && (ExtType == ISD::ZEXTLOAD));

  // Expand
  //  (set dst, (i64 (zextload baseptr)))
  // to
  //  (set tmp0, (lwl (add baseptr, 3), undef))
  //  (set tmp1, (lwr baseptr, tmp0))
  //  (set tmp2, (shl tmp1, 32))
  //  (set dst, (srl tmp2, 32))
  SDLoc DL(LD);
  SDValue Const32 = DAG.getConstant(32, DL, MVT::i32);
  SDValue SLL = DAG.getNode(ISD::SHL, DL, MVT::i64, LWR, Const32);
  SDValue SRL = DAG.getNode(ISD::SRL, DL, MVT::i64, SLL, Const32);
  SDValue Ops[] = { SRL, LWR.getValue(1) };
  return DAG.getMergeValues(Ops, DL);
}

static SDValue createStoreLR(unsigned Opc, SelectionDAG &DAG, StoreSDNode *SD,
                             SDValue Chain, unsigned Offset) {
  SDValue Ptr = SD->getBasePtr(), Value = SD->getValue();
  EVT MemVT = SD->getMemoryVT();
  SDLoc DL(SD);
  SDVTList VTList = DAG.getVTList(MVT::Other);

  if (Offset)
    Ptr = DAG.getPointerAdd(DL, Ptr, Offset);

  SDValue Ops[] = { Chain, Value, Ptr };
  return DAG.getMemIntrinsicNode(Opc, DL, VTList, Ops, MemVT,
                                 SD->getMemOperand());
}

// Expand an unaligned 32 or 64-bit integer store node.
static SDValue lowerUnalignedIntStore(StoreSDNode *SD, SelectionDAG &DAG,
                                      bool IsLittle,
                                      const MipsTargetLowering &TLI) {
  SDValue Value = SD->getValue(), Chain = SD->getChain();
  EVT VT = Value.getValueType();
  SDValue BasePtr = SD->getBasePtr();

  // CHERI doesn't have a capability version of SDR / SDL, so we fall back to
  // generic code.
  if (BasePtr->getValueType(0).isFatPointer()) {
    return TLI.expandUnalignedStore(SD, DAG);
  }

  // Expand
  //  (store val, baseptr) or
  //  (truncstore val, baseptr)
  // to
  //  (swl val, (add baseptr, 3))
  //  (swr val, baseptr)
  if ((VT == MVT::i32) || SD->isTruncatingStore()) {
    SDValue SWL = createStoreLR(MipsISD::SWL, DAG, SD, Chain,
                                IsLittle ? 3 : 0);
    return createStoreLR(MipsISD::SWR, DAG, SD, SWL, IsLittle ? 0 : 3);
  }

  assert(VT == MVT::i64);

  // Expand
  //  (store val, baseptr)
  // to
  //  (sdl val, (add baseptr, 7))
  //  (sdr val, baseptr)
  SDValue SDL = createStoreLR(MipsISD::SDL, DAG, SD, Chain, IsLittle ? 7 : 0);
  return createStoreLR(MipsISD::SDR, DAG, SD, SDL, IsLittle ? 0 : 7);
}

// Lower (store (fp_to_sint $fp) $ptr) to (store (TruncIntFP $fp), $ptr).
static SDValue lowerFP_TO_SINT_STORE(StoreSDNode *SD, SelectionDAG &DAG,
                                     bool SingleFloat) {
  SDValue Val = SD->getValue();

  if (Val.getOpcode() != ISD::FP_TO_SINT ||
      (Val.getValueSizeInBits() > 32 && SingleFloat))
    return SDValue();

  EVT FPTy = EVT::getFloatingPointVT(Val.getValueSizeInBits());
  SDValue Tr = DAG.getNode(MipsISD::TruncIntFP, SDLoc(Val), FPTy,
                           Val.getOperand(0));
  return DAG.getStore(SD->getChain(), SDLoc(SD), Tr, SD->getBasePtr(),
                      SD->getPointerInfo(), SD->getAlignment(),
                      SD->getMemOperand()->getFlags());
}

SDValue MipsTargetLowering::lowerSTORE(SDValue Op, SelectionDAG &DAG) const {
  StoreSDNode *SD = cast<StoreSDNode>(Op);
  EVT MemVT = SD->getMemoryVT();
  const SDValue Val = SD->getValue();

  // If we're doing a capability-relative load or store of something smaller
  // than 64 bits, then we need to insert an anyext node to make the input
  // value an i64
  if (Subtarget.isCheri() && SD->isTruncatingStore() && Val.getValueType() == MVT::i32) {
    SDValue Chain = SD->getChain();
    SDValue BasePtr = SD->getBasePtr();
    SDLoc DL(Op);
    const SDValue ExtNode = DAG.getAnyExtOrTrunc(Val, DL, MVT::i64);
    return DAG.getTruncStore(Chain, DL, ExtNode, BasePtr, MemVT,
        SD->getMemOperand());
  }

  // Lower unaligned integer stores.
  if (!Subtarget.systemSupportsUnalignedAccess(SD->getAddressSpace()) &&
      (SD->getAlignment() < MemVT.getSizeInBits() / 8) && MemVT.isInteger())
    return lowerUnalignedIntStore(SD, DAG, Subtarget.isLittle(), *this);

  return lowerFP_TO_SINT_STORE(SD, DAG, Subtarget.isSingleFloat());
}

SDValue MipsTargetLowering::lowerEH_DWARF_CFA(SDValue Op,
                                              SelectionDAG &DAG) const {

  // Return a fixed StackObject with offset 0 which points to the old stack
  // pointer.
  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  EVT ValTy = Op->getValueType(0);
  int FI = MFI.CreateFixedObject(Op.getValueSizeInBits() / 8, 0, false);
  return DAG.getFrameIndex(FI, ValTy);
}

SDValue MipsTargetLowering::lowerFP_TO_SINT(SDValue Op,
                                            SelectionDAG &DAG) const {
  if (Op.getValueSizeInBits() > 32 && Subtarget.isSingleFloat())
    return SDValue();

  EVT FPTy = EVT::getFloatingPointVT(Op.getValueSizeInBits());
  SDValue Trunc = DAG.getNode(MipsISD::TruncIntFP, SDLoc(Op), FPTy,
                              Op.getOperand(0));
  return DAG.getNode(ISD::BITCAST, SDLoc(Op), Op.getValueType(), Trunc);
}

SDValue
MipsTargetLowering::convertToPCCDerivedCap(SDValue TargetAddr, const SDLoc &dl,
                                           SelectionDAG &DAG,
                                           SDValue AdditionalOffset) const {
  // To get a pcc derived capability we do a SetAddr on PCC.
  // The getpcc+setaddr intrinsics will be folded into a single cgetpccsetaddr
  // instruction due to the MipsInstrCheri.td patterns.
  auto GetPCC = DAG.getConstant(Intrinsic::cheri_pcc_get, dl, MVT::i64);
  auto PCC = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, CapType, GetPCC);
  if (TargetAddr.getValueType() == CapType) {
    // If addr is a capability: CSetAddr($pcc, CGetAddr($tgt))
    auto GetAddr =
        DAG.getConstant(Intrinsic::cheri_cap_address_get, dl, MVT::i64);
    TargetAddr =
        DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, MVT::i64, GetAddr, TargetAddr);
  } else {
    // Otherwise we should already have an i64: CSetAddr($pcc, $tgt)
    assert(TargetAddr.getValueType() == MVT::i64);
  }
  // Potentially add an additional offset:
  if (AdditionalOffset) {
    TargetAddr =
        DAG.getNode(ISD::ADD, dl, MVT::i64, TargetAddr, AdditionalOffset);
  }
  auto SetAddr =
      DAG.getConstant(Intrinsic::cheri_cap_address_set, dl, MVT::i64);
  return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, CapType, SetAddr, PCC,
                     TargetAddr);
}

SDValue MipsTargetLowering::lowerBR_JT(SDValue Op,
                                       SelectionDAG &DAG) const {
  // FIXME: this is almost the same as the code in LegalizeDAG.cpp, can
  // I just adjust that to work for capabilities too?

  assert(ABI.IsCheriPureCap());

  SDLoc dl(Op);
  SDValue Chain = Op->getOperand(0);
  SDValue Table = Op->getOperand(1);
  SDValue Index = Op->getOperand(2);

  const DataLayout &TD = DAG.getDataLayout();
  EVT PTy = getPointerTy(TD, 200);
  assert(PTy == CapType);
  unsigned EntrySize =
    DAG.getMachineFunction().getJumpTableInfo()->getEntrySize(TD);
  assert(EntrySize == 4);
  assert(isPowerOf2_64(EntrySize));
  // FIXME: or should this be something else? like in lowerCall?
  auto PtrInfo = MachinePointerInfo::getCapTable(DAG.getMachineFunction());
  SDValue Jumptable = getDataFromCapTable(
      cast<JumpTableSDNode>(Table.getNode()), dl, PTy, DAG, Chain, PtrInfo);
  // TODO: use a mul here and let the code later on convert it to a shift?
  Index = DAG.getNode(ISD::SHL, dl, MVT::i64, Index,
                      DAG.getConstant(Log2_32(EntrySize), dl, MVT::i64));
  EVT MemVT = EVT::getIntegerVT(*DAG.getContext(), EntrySize * 8);
  auto LoadAddr = DAG.getPointerAdd(dl, Jumptable, Index);
  SDValue JTEntryValue = DAG.getExtLoad(
      ISD::SEXTLOAD, dl, MVT::i64, Chain, LoadAddr,
      MachinePointerInfo::getJumpTable(DAG.getMachineFunction()), MemVT);
  Chain = JTEntryValue.getValue(1);
  assert(isJumpTableRelative());
  // Each jump table entry contains .4byte	.LBB0_N - .LJTI0_0.
  // Since the jump table comes before .text this will generally be a positive
  // number that we can add to the address of the jump-table to get the
  // address of the basic block:

  // TODO: use a smarter model for jump tables, this is just used because
  // there is existing infrastucture. Maybe use offset from beginning of func
  // instead?

  // Addr is a data capability so won't have Permit_Execute so we have to derive
  // a new capability from PCC:
  // We use the jump table capability as the target address capability and
  // then add on the value loaded from the jump table to get the actual basic
  // block address:
  // addrof(BB) = addrof(JT) + *(JT[INDEX])
  // New PPC = CIncOffset, PCC, (addrof(BB) - addrof(PCC))
  // This can be done slightly more efficiently as:
  // New PPC = CIncOffset, PCC, (CSub(JT, PCC) + *(JT[INDEX]))
  // But not the following since it might cause JT to become unrepresentable:
  // New PPC = CIncOffset, PCC, (CSub(JT + *(JT[INDEX]), PCC))
  JTEntryValue = convertToPCCDerivedCap(Jumptable, dl, DAG, JTEntryValue);
  auto SealEntry =
      DAG.getConstant(Intrinsic::cheri_cap_seal_entry, dl, MVT::i64);
  JTEntryValue = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, dl, CapType, SealEntry, JTEntryValue);
  return DAG.getNode(ISD::BRIND, dl, MVT::Other, Chain, JTEntryValue);
}

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// TODO: Implement a generic logic using tblgen that can support this.
// Mips O32 ABI rules:
// ---
// i32 - Passed in A0, A1, A2, A3 and stack
// f32 - Only passed in f32 registers if no int reg has been used yet to hold
//       an argument. Otherwise, passed in A1, A2, A3 and stack.
// f64 - Only passed in two aliased f32 registers if no int reg has been used
//       yet to hold an argument. Otherwise, use A2, A3 and stack. If A1 is
//       not used, it must be shadowed. If only A3 is available, shadow it and
//       go to stack.
// vXiX - Received as scalarized i32s, passed in A0 - A3 and the stack.
// vXf32 - Passed in either a pair of registers {A0, A1}, {A2, A3} or {A0 - A3}
//         with the remainder spilled to the stack.
// vXf64 - Passed in either {A0, A1, A2, A3} or {A2, A3} and in both cases
//         spilling the remainder to the stack.
//
//  For vararg functions, all arguments are passed in A0, A1, A2, A3 and stack.
//===----------------------------------------------------------------------===//

static bool CC_MipsO32(unsigned ValNo, MVT ValVT, MVT LocVT,
                       CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                       CCState &State, ArrayRef<MCPhysReg> F64Regs) {
  const MipsSubtarget &Subtarget = static_cast<const MipsSubtarget &>(
      State.getMachineFunction().getSubtarget());

  static const MCPhysReg IntRegs[] = { Mips::A0, Mips::A1, Mips::A2, Mips::A3 };

  const MipsCCState * MipsState = static_cast<MipsCCState *>(&State);

  static const MCPhysReg F32Regs[] = { Mips::F12, Mips::F14 };

  static const MCPhysReg FloatVectorIntRegs[] = { Mips::A0, Mips::A2 };

  // Do not process byval args here.
  if (ArgFlags.isByVal())
    return true;

  // Promote i8 and i16
  if (ArgFlags.isInReg() && !Subtarget.isLittle()) {
    if (LocVT == MVT::i8 || LocVT == MVT::i16 || LocVT == MVT::i32) {
      LocVT = MVT::i32;
      if (ArgFlags.isSExt())
        LocInfo = CCValAssign::SExtUpper;
      else if (ArgFlags.isZExt())
        LocInfo = CCValAssign::ZExtUpper;
      else
        LocInfo = CCValAssign::AExtUpper;
    }
  }

  // Promote i8 and i16
  if (LocVT == MVT::i8 || LocVT == MVT::i16) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  unsigned Reg;

  // f32 and f64 are allocated in A0, A1, A2, A3 when either of the following
  // is true: function is vararg, argument is 3rd or higher, there is previous
  // argument which is not f32 or f64.
  bool AllocateFloatsInIntReg = State.isVarArg() || ValNo > 1 ||
                                State.getFirstUnallocated(F32Regs) != ValNo;
  Align OrigAlign = ArgFlags.getNonZeroOrigAlign();
  bool isI64 = (ValVT == MVT::i32 && OrigAlign == Align(8));
  bool isVectorFloat = MipsState->WasOriginalArgVectorFloat(ValNo);

  // The MIPS vector ABI for floats passes them in a pair of registers
  if (ValVT == MVT::i32 && isVectorFloat) {
    // This is the start of an vector that was scalarized into an unknown number
    // of components. It doesn't matter how many there are. Allocate one of the
    // notional 8 byte aligned registers which map onto the argument stack, and
    // shadow the register lost to alignment requirements.
    if (ArgFlags.isSplit()) {
      Reg = State.AllocateReg(FloatVectorIntRegs);
      if (Reg == Mips::A2)
        State.AllocateReg(Mips::A1);
      else if (Reg == 0)
        State.AllocateReg(Mips::A3);
    } else {
      // If we're an intermediate component of the split, we can just attempt to
      // allocate a register directly.
      Reg = State.AllocateReg(IntRegs);
    }
  } else if (ValVT == MVT::i32 ||
             (ValVT == MVT::f32 && AllocateFloatsInIntReg)) {
    Reg = State.AllocateReg(IntRegs);
    // If this is the first part of an i64 arg,
    // the allocated register must be either A0 or A2.
    if (isI64 && (Reg == Mips::A1 || Reg == Mips::A3))
      Reg = State.AllocateReg(IntRegs);
    LocVT = MVT::i32;
  } else if (ValVT == MVT::f64 && AllocateFloatsInIntReg) {
    LocVT = MVT::i32;

    // Allocate int register and shadow next int register. If first
    // available register is Mips::A1 or Mips::A3, shadow it too.
    Reg = State.AllocateReg(IntRegs);
    if (Reg == Mips::A1 || Reg == Mips::A3)
      Reg = State.AllocateReg(IntRegs);

    if (Reg) {
      State.addLoc(
          CCValAssign::getCustomReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      MCRegister HiReg = State.AllocateReg(IntRegs);
      assert(HiReg);
      State.addLoc(
          CCValAssign::getCustomReg(ValNo, ValVT, HiReg, LocVT, LocInfo));
      return false;
    }
  } else if (ValVT.isFloatingPoint() && !AllocateFloatsInIntReg) {
    // we are guaranteed to find an available float register
    if (ValVT == MVT::f32) {
      Reg = State.AllocateReg(F32Regs);
      // Shadow int register
      State.AllocateReg(IntRegs);
    } else {
      Reg = State.AllocateReg(F64Regs);
      // Shadow int registers
      unsigned Reg2 = State.AllocateReg(IntRegs);
      if (Reg2 == Mips::A1 || Reg2 == Mips::A3)
        State.AllocateReg(IntRegs);
      State.AllocateReg(IntRegs);
    }
  } else
    llvm_unreachable("Cannot handle this ValVT.");

  if (!Reg) {
    unsigned Offset = State.AllocateStack(ValVT.getStoreSize(), OrigAlign);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
  } else
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));

  return false;
}

static bool CC_MipsO32_FP32(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {
  static const MCPhysReg F64Regs[] = { Mips::D6, Mips::D7 };

  return CC_MipsO32(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State, F64Regs);
}

static bool CC_MipsO32_FP64(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {
  static const MCPhysReg F64Regs[] = { Mips::D12_64, Mips::D14_64 };

  return CC_MipsO32(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State, F64Regs);
}

static bool CC_MipsO32(unsigned ValNo, MVT ValVT, MVT LocVT,
                       CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                       CCState &State) LLVM_ATTRIBUTE_UNUSED;

#include "MipsGenCallingConv.inc"

 CCAssignFn *MipsTargetLowering::CCAssignFnForCall() const{
   return CC_Mips_FixedArg;
 }

 CCAssignFn *MipsTargetLowering::CCAssignFnForReturn() const{
   return RetCC_Mips;
 }
//===----------------------------------------------------------------------===//
//                  Call Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue MipsTargetLowering::passArgOnStack(SDValue StackPtr, unsigned Offset,
                                           SDValue Chain, SDValue Arg,
                                           const SDLoc &DL, bool IsTailCall,
                                           SelectionDAG &DAG) const {
  if (!IsTailCall) {
    SDValue PtrOff = DAG.getPointerAdd(DL, StackPtr, Offset);
    return DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo());
  }

  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  int FI = MFI.CreateFixedObject(Arg.getValueSizeInBits() / 8, Offset, false);
  SDValue FIN = DAG.getFrameIndex(
      FI, getPointerTy(DAG.getDataLayout(), ABI.StackAddrSpace()));
  return DAG.getStore(Chain, DL, Arg, FIN, MachinePointerInfo(), MaybeAlign(),
                      MachineMemOperand::MOVolatile);
}

void MipsTargetLowering::
getOpndList(SmallVectorImpl<SDValue> &Ops,
            std::deque<std::pair<unsigned, SDValue>> &RegsToPass,
            bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
            bool IsCallReloc, CallLoweringInfo &CLI, SDValue Callee,
            SDValue Chain) const {
  // Insert node "GP copy globalreg" before call to function.
  //
  // R_MIPS_CALL* operators (emitted when non-internal functions are called
  // in PIC mode) allow symbols to be resolved via lazy binding.
  // The lazy binding stub requires GP to point to the GOT.
  // Note that we don't need GP to point to the GOT for indirect calls
  // (when R_MIPS_CALL* is not used for the call) because Mips linker generates
  // lazy binding stub for a function only when R_MIPS_CALL* are the only relocs
  // used for the function (that is, Mips linker doesn't generate lazy binding
  // stub for a function whose address is taken in the program).
  //
  // When using the capability table we don't need $GP, but use $cgp instead.
  //
  if (IsPICCall && !InternalLinkage && IsCallReloc && !ABI.IsCheriPureCap()) {
    unsigned GPReg = ABI.IsN64() ? Mips::GP_64 : Mips::GP;
    EVT Ty = ABI.IsN64() ? MVT::i64 : MVT::i32;
    RegsToPass.push_back(
        std::make_pair(GPReg, getGlobalReg(CLI.DAG, Ty, /*IsForTls=*/false)));
  }

  // Build a sequence of copy-to-reg nodes chained together with token
  // chain and flag operands which copy the outgoing args into registers.
  // The InFlag in necessary since all emitted instructions must be
  // stuck together.
  SDValue InFlag;

  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = CLI.DAG.getCopyToReg(Chain, CLI.DL, RegsToPass[i].first,
                                 RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(CLI.DAG.getRegister(RegsToPass[i].first,
                                      RegsToPass[i].second.getValueType()));

  // Add a register mask operand representing the call-preserved registers.
  const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask =
      TRI->getCallPreservedMask(CLI.DAG.getMachineFunction(), CLI.CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");
  if (Subtarget.inMips16HardFloat()) {
    if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(CLI.Callee)) {
      StringRef Sym = G->getGlobal()->getName();
      Function *F = G->getGlobal()->getParent()->getFunction(Sym);
      if (F && F->hasFnAttribute("__Mips16RetHelper")) {
        Mask = MipsRegisterInfo::getMips16RetHelperMask();
      }
    }
  }
  Ops.push_back(CLI.DAG.getRegisterMask(Mask));

  if (InFlag.getNode())
    Ops.push_back(InFlag);
}

void MipsTargetLowering::AdjustInstrPostInstrSelection(MachineInstr &MI,
                                                       SDNode *Node) const {
  switch (MI.getOpcode()) {
    default:
      return;
    case Mips::JALR:
    case Mips::JALRPseudo:
    case Mips::JALR64:
    case Mips::JALR64Pseudo:
    case Mips::JALR16_MM:
    case Mips::JALRC16_MMR6:
    case Mips::TAILCALLREG:
    case Mips::TAILCALLREG64:
    case Mips::TAILCALLR6REG:
    case Mips::TAILCALL64R6REG:
    case Mips::TAILCALLREG_MM:
    case Mips::TAILCALLREG_MMR6: {
      if (!EmitJalrReloc ||
          Subtarget.inMips16Mode() ||
          !isPositionIndependent() ||
          Node->getNumOperands() < 1 ||
          Node->getOperand(0).getNumOperands() < 2) {
        return;
      }
      // We are after the callee address, set by LowerCall().
      // If added to MI, asm printer will emit .reloc R_MIPS_JALR for the
      // symbol.
      const SDValue TargetAddr = Node->getOperand(0).getOperand(1);
      StringRef Sym;
      if (const GlobalAddressSDNode *G =
              dyn_cast_or_null<const GlobalAddressSDNode>(TargetAddr)) {
        // We must not emit the R_MIPS_JALR relocation against data symbols
        // since this will cause run-time crashes if the linker replaces the
        // call instruction with a relative branch to the data symbol.
        if (!isa<Function>(G->getGlobal())) {
          LLVM_DEBUG(dbgs() << "Not adding R_MIPS_JALR against data symbol "
                            << G->getGlobal()->getName() << "\n");
          return;
        }
        Sym = G->getGlobal()->getName();
      }
      else if (const ExternalSymbolSDNode *ES =
                   dyn_cast_or_null<const ExternalSymbolSDNode>(TargetAddr)) {
        Sym = ES->getSymbol();
      }

      if (Sym.empty())
        return;

      MachineFunction *MF = MI.getParent()->getParent();
      MCSymbol *S = MF->getContext().getOrCreateSymbol(Sym);
      LLVM_DEBUG(dbgs() << "Adding R_MIPS_JALR against " << Sym << "\n");
      MI.addOperand(MachineOperand::CreateMCSymbol(S, MipsII::MO_JALR));
    }
  }
}

/// LowerCall - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
SDValue
MipsTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                              SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG                     = CLI.DAG;
  SDLoc DL                              = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals     = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &IsTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool IsVarArg                         = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFL = Subtarget.getFrameLowering();
  MipsFunctionInfo *FuncInfo = MF.getInfo<MipsFunctionInfo>();
  bool IsPIC = isPositionIndependent();

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  MipsCCState CCInfo(
      CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext(),
      MipsCCState::getSpecialCallingConvForCallee(Callee.getNode(), Subtarget));

  const ExternalSymbolSDNode *ES =
      dyn_cast_or_null<const ExternalSymbolSDNode>(Callee.getNode());

  // There is one case where CALLSEQ_START..CALLSEQ_END can be nested, which
  // is during the lowering of a call with a byval argument which produces
  // a call to memcpy. For the O32 case, this causes the caller to allocate
  // stack space for the reserved argument area for the callee, then recursively
  // again for the memcpy call. In the NEWABI case, this doesn't occur as those
  // ABIs mandate that the callee allocates the reserved argument area. We do
  // still produce nested CALLSEQ_START..CALLSEQ_END with zero space though.
  //
  // If the callee has a byval argument and memcpy is used, we are mandated
  // to already have produced a reserved argument area for the callee for O32.
  // Therefore, the reserved argument area can be reused for both calls.
  //
  // Other cases of calling memcpy cannot have a chain with a CALLSEQ_START
  // present, as we have yet to hook that node onto the chain.
  //
  // Hence, the CALLSEQ_START and CALLSEQ_END nodes can be eliminated in this
  // case. GCC does a similar trick, in that wherever possible, it calculates
  // the maximum out going argument area (including the reserved area), and
  // preallocates the stack space on entrance to the caller.
  //
  // FIXME: We should do the same for efficiency and space.

  // Note: The check on the calling convention below must match
  //       MipsABIInfo::GetCalleeAllocdArgSizeInBytes().
  bool MemcpyInByVal = ES &&
                       StringRef(ES->getSymbol()) == StringRef("memcpy") &&
                       CallConv != CallingConv::Fast &&
                       Chain.getOpcode() == ISD::CALLSEQ_START;

  // Allocate the reserved argument area. It seems strange to do this from the
  // caller side but removing it breaks the frame size calculation.
  unsigned ReservedArgArea =
      MemcpyInByVal ? 0 : ABI.GetCalleeAllocdArgSizeInBytes(CallConv);
  CCInfo.AllocateStack(ReservedArgArea, Align(1));

  CCInfo.AnalyzeCallOperands(Outs, CC_Mips, CLI.getArgs(),
                             ES ? ES->getSymbol() : nullptr);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NextStackOffset = CCInfo.getNextStackOffset();

  // Call site info for function parameters tracking.
  MachineFunction::CallSiteInfo CSInfo;

  // Check if it's really possible to do a tail call. Restrict it to functions
  // that are part of this compilation unit.
  bool InternalLinkage = false;
  if (IsTailCall) {
    IsTailCall = isEligibleForTailCallOptimization(
        CCInfo, NextStackOffset, *MF.getInfo<MipsFunctionInfo>());
     if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
      InternalLinkage = G->getGlobal()->hasInternalLinkage();
      IsTailCall &= (InternalLinkage || G->getGlobal()->hasLocalLinkage() ||
                     G->getGlobal()->hasPrivateLinkage() ||
                     G->getGlobal()->hasHiddenVisibility() ||
                     G->getGlobal()->hasProtectedVisibility());
     }
  }
  if (!IsTailCall && CLI.CB && CLI.CB->isMustTailCall())
    report_fatal_error("failed to perform tail call elimination on a call "
                       "site marked musttail");

  if (IsTailCall)
    ++NumTailCalls;

  // Chain is the output chain of the last Load/Store or CopyToReg node.
  // ByValChain is the output chain of the last Memcpy node created for copying
  // byval arguments to the stack.
  unsigned StackAlignment = TFL->getStackAlignment();
  NextStackOffset = alignTo(NextStackOffset, StackAlignment);
  SDValue NextStackOffsetVal = DAG.getIntPtrConstant(NextStackOffset, DL, true);

  if (!(IsTailCall || MemcpyInByVal))
    Chain = DAG.getCALLSEQ_START(Chain, NextStackOffset, 0, DL);

  SDValue StackPtr =
      DAG.getCopyFromReg(Chain, DL, ABI.GetStackPtr(),
                         getPointerTy(DAG.getDataLayout(),
                           ABI.StackAddrSpace()));

  std::deque<std::pair<unsigned, SDValue>> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  CCInfo.rewindByValRegsInfo();

  unsigned CapArgs = 0;
  unsigned IntArgs = 0;
  int FirstOffset = -1;
  int LastOffset;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(), OutIdx = 0; i != e; ++i, ++OutIdx) {
    SDValue Arg = OutVals[OutIdx];
    CCValAssign &VA = ArgLocs[i];
    MVT ValVT = VA.getValVT(), LocVT = VA.getLocVT();
    ISD::ArgFlagsTy Flags = Outs[OutIdx].Flags;
    bool UseUpperBits = false;

    // ByVal Arg.
    if (Flags.isByVal()) {
      unsigned FirstByValReg = ~0u, LastByValReg = ~0u;
      assert(Flags.getByValSize() &&
             "ByVal args of size 0 should have been ignored by front-end.");
      if (CCInfo.getInRegsParamsCount() > 0) {
        unsigned ByValIdx = CCInfo.getInRegsParamsProcessed();
        CCInfo.getInRegsParamInfo(ByValIdx, FirstByValReg, LastByValReg);
        assert(ByValIdx < CCInfo.getInRegsParamsCount());
      }
      assert(!IsTailCall &&
             "Do not tail-call optimize if there is a byval argument.");
      passByValArg(Chain, DL, RegsToPass, MemOpChains, StackPtr, MFI, DAG, Arg,
                   FirstByValReg, LastByValReg, Flags, Subtarget.isLittle(),
                   VA);
      CCInfo.nextInRegsParam();
      if (FirstOffset == -1)
        FirstOffset = VA.getLocMemOffset();
      LastOffset = VA.getLocMemOffset() + Flags.getByValSize();
      continue;
    }

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      if (VA.isRegLoc()) {
        if ((ValVT == MVT::f32 && LocVT == MVT::i32) ||
            (ValVT == MVT::f64 && LocVT == MVT::i64) ||
            (ValVT == MVT::i64 && LocVT == MVT::f64))
          Arg = DAG.getNode(ISD::BITCAST, DL, LocVT, Arg);
        else if (ValVT == MVT::f64 && LocVT == MVT::i32) {
          SDValue Lo = DAG.getNode(MipsISD::ExtractElementF64, DL, MVT::i32,
                                   Arg, DAG.getConstant(0, DL, MVT::i32));
          SDValue Hi = DAG.getNode(MipsISD::ExtractElementF64, DL, MVT::i32,
                                   Arg, DAG.getConstant(1, DL, MVT::i32));
          if (!Subtarget.isLittle())
            std::swap(Lo, Hi);

          assert(VA.needsCustom());

          Register LocRegLo = VA.getLocReg();
          Register LocRegHigh = ArgLocs[++i].getLocReg();
          RegsToPass.push_back(std::make_pair(LocRegLo, Lo));
          RegsToPass.push_back(std::make_pair(LocRegHigh, Hi));
          continue;
        }
      }
      break;
    case CCValAssign::BCvt:
      Arg = DAG.getNode(ISD::BITCAST, DL, LocVT, Arg);
      break;
    case CCValAssign::SExtUpper:
      UseUpperBits = true;
      LLVM_FALLTHROUGH;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, DL, LocVT, Arg);
      break;
    case CCValAssign::ZExtUpper:
      UseUpperBits = true;
      LLVM_FALLTHROUGH;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, DL, LocVT, Arg);
      break;
    case CCValAssign::AExtUpper:
      UseUpperBits = true;
      LLVM_FALLTHROUGH;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, DL, LocVT, Arg);
      break;
    }

    if (UseUpperBits) {
      unsigned ValSizeInBits = Outs[OutIdx].ArgVT.getSizeInBits();
      unsigned LocSizeInBits = VA.getLocVT().getSizeInBits();
      Arg = DAG.getNode(
          ISD::SHL, DL, VA.getLocVT(), Arg,
          DAG.getConstant(LocSizeInBits - ValSizeInBits, DL, VA.getLocVT()));
    }

    // Arguments that can be passed on register must be kept at
    // RegsToPass vector
    if (VA.isRegLoc()) {
      if (ValVT.isFatPointer())
        CapArgs++;
      else if ((VA.getLocReg() >= Mips::A0_64) &&
               (VA.getLocReg() <= Mips::T3_64))
        IntArgs++;
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));

      // If the parameter is passed through reg $D, which splits into
      // two physical registers, avoid creating call site info.
      if (Mips::AFGR64RegClass.contains(VA.getLocReg()))
        continue;

      // Collect CSInfo about which register passes which parameter.
      const TargetOptions &Options = DAG.getTarget().Options;
      if (Options.SupportsDebugEntryValues)
        CSInfo.emplace_back(VA.getLocReg(), i);

      continue;
    }

    // Register can't get to this point...
    assert(VA.isMemLoc());

    // Ignore leading undef args (inserted by clang for alignment), as they
    // break varargs
    if (FirstOffset == -1 && Arg.isUndef() && ABI.IsCheriPureCap())
      continue;

    if (FirstOffset == -1)
      FirstOffset = VA.getLocMemOffset();
    LastOffset = VA.getLocMemOffset() + (Arg.getValueSizeInBits() / 8);
    // emit ISD::STORE whichs stores the
    // parameter value to a stack Location
    MemOpChains.push_back(passArgOnStack(StackPtr, VA.getLocMemOffset(),
                                         Chain, Arg, DL, IsTailCall, DAG));
  }
  if (ABI.IsCheriPureCap()) {
    if (FirstOffset != -1) {
      SDValue PtrOff = DAG.getPointerAdd(DL, StackPtr, FirstOffset);
      std::string BoundsDetails;
      if (cheri::ShouldCollectCSetBoundsStats) {
        StringRef Name = "<unknown function>";
        if (ES) {
          Name = ES->getSymbol();
        } else if (GlobalAddressSDNode *G =
                       dyn_cast<GlobalAddressSDNode>(Callee)) {
          Name = G->getGlobal()->getName();
        }
        BoundsDetails =
            (Twine("setting varargs bounds for call to ") + Name).str();
      }
      PtrOff = DAG.getCSetBounds(
          PtrOff, DL, LastOffset, Align(), "MIPS variadic call lowering",
          cheri::SetBoundsPointerSource::Stack, BoundsDetails);
      PtrOff = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, CapType,
          DAG.getConstant(Intrinsic::cheri_cap_perms_and, DL, MVT::i64), PtrOff,
          DAG.getIntPtrConstant(0xFFD7, DL));
      RegsToPass.push_back(std::make_pair(Mips::C13, PtrOff));
    } else {
      bool ShouldClearC13 = false;
      // We only need to clear $c13 (for on-stack arguments if the calling
      // function had args on the stack (i.e. $c13 was a live in)
      // However, $c13 being a live in is only set once lowerVASTART is processed
      // so we just look if the current function is variadic in the IR Decl
      // TODO: should we just mark c13 as live-in in for all variadic functions
      // in MipsTargetLowering::LowerFormalArguments()?
      if (MF.getFunction().isVarArg())
        ShouldClearC13 = true;
      // Also clear $c13 if it was marked as live-in due to having
      if (MF.getRegInfo().isLiveIn(Mips::C13))
        ShouldClearC13 = true;
      // Also clear $c13 if the the callee is a variadic function but we're not
      // passing any variadic arguments.
      if (IsVarArg)
        ShouldClearC13 = true;
      if (ShouldClearC13) {
        LLVM_DEBUG(dbgs() << "Clearing $c13 in " << MF.getName()
                          << "(is varargs: " << MF.getFunction().isVarArg()
                          << ") callee = ");
        LLVM_DEBUG(Callee.dump(&DAG););
        RegsToPass.push_back(std::make_pair(Mips::C13, DAG.getNullCapability(DL)));
      }
    }
  }
  // If we're doing a CCall then any unused arg registers should be zero.
  if (!UseClearRegs && (CallConv == CallingConv::CHERI_CCall)) {
    assert(CapArgs >= 2);
    CapArgs -= 2;
    // Optional argument registers for CCall calling convention
    static const unsigned RegList[] = { Mips::C3, Mips::C4, Mips::C5, Mips::C6,
      Mips::C7, Mips::C8, Mips::C9, Mips::C10 };
    for (unsigned i=CapArgs ; i<8 ; i++) {
      SDValue Zero = DAG.getNullCapability(DL);
      RegsToPass.push_back(std::make_pair(RegList[i], Zero));
    }
    static const unsigned IntRegList[] = { Mips::A0_64, Mips::A1_64,
      Mips::A2_64, Mips::A3_64, Mips::T0_64, Mips::T1_64, Mips::T2_64,
      Mips::T3_64 };
    for (unsigned i=IntArgs ; i<8 ; i++) {
      SDValue Zero = DAG.getConstant(0, DL, MVT::i64);
      RegsToPass.push_back(std::make_pair(IntRegList[i], Zero));
    }
  }

  // Transform all store nodes into one single node because all store
  // nodes are independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  // If the callee is a GlobalAddress/ExternalSymbol node (quite common, every
  // direct call is) turn it into a TargetGlobalAddress/TargetExternalSymbol
  // node so that legalize doesn't hack it.

  EVT Ty = Callee.getValueType();
  bool GlobalOrExternal = false, IsCallReloc = false;

  const bool CheriCapTable = Subtarget.useCheriCapTable();
  // The long-calls feature is ignored in case of PIC.
  // While we do not support -mshared / -mno-shared properly,
  // ignore long-calls in case of -mabicalls too.
  if (!Subtarget.isABICalls() && !IsPIC) {
    // If the function should be called using "long call",
    // get its address into a register to prevent using
    // of the `jal` instruction for the direct call.
    if (auto *N = dyn_cast<ExternalSymbolSDNode>(Callee)) {
      if (Subtarget.useLongCalls())
        Callee = Subtarget.hasSym32()
                     ? getAddrNonPIC(N, SDLoc(N), Ty, DAG)
                     : getAddrNonPICSym64(N, SDLoc(N), Ty, DAG);
    } else if (auto *N = dyn_cast<GlobalAddressSDNode>(Callee)) {
      bool UseLongCalls = Subtarget.useLongCalls();
      // If the function has long-call/far/near attribute
      // it overrides command line switch pased to the backend.
      if (auto *F = dyn_cast<Function>(N->getGlobal())) {
        if (F->hasFnAttribute("long-call"))
          UseLongCalls = true;
        else if (F->hasFnAttribute("short-call"))
          UseLongCalls = false;
      }
      if (UseLongCalls)
        Callee = Subtarget.hasSym32()
                     ? getAddrNonPIC(N, SDLoc(N), Ty, DAG)
                     : getAddrNonPICSym64(N, SDLoc(N), Ty, DAG);
    }
  }

  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
      const GlobalValue *Val = G->getGlobal();
    if (CheriCapTable) {
        Callee = getCallTargetFromCapTable(G, DL, CapType, DAG, Chain,
                                           FuncInfo->callPtrInfo(MF, Val));
        IsCallReloc = true;
      } else if (IsPIC || ABI.IsCheriPureCap()) {
        // Legacy ABI also needs to load from GOT in non-PIC since otherwise
        // it will attempt to use a JAL relocation in a CGetPCCSetOffset (and
        // corrupt the instruction)
        InternalLinkage = Val->hasInternalLinkage();

        if (InternalLinkage)
          Callee = getAddrLocal(G, DL, Ty, DAG, ABI.IsN32() || ABI.IsN64(),
                                Val->isThreadLocal());
        else if (Subtarget.useXGOT()) {
          Callee = getAddrGlobalLargeGOT(
              G, DL, Ty, DAG, MipsII::MO_CALL_HI16, MipsII::MO_CALL_LO16, Chain,
              FuncInfo->callPtrInfo(MF, Val), Val->isThreadLocal());
          IsCallReloc = true;
        } else {
          Callee = getAddrGlobal(G, DL, Ty, DAG, MipsII::MO_GOT_CALL, Chain,
                                 FuncInfo->callPtrInfo(MF, Val),
                                 Val->isThreadLocal());
          IsCallReloc = true;
        }
      } else {
        assert(!ABI.IsCheriPureCap());
        Callee = DAG.getTargetGlobalAddress(
            Val, DL, getPointerTy(DAG.getDataLayout(), 0), 0,
            MipsII::MO_NO_FLAG);
      }
      GlobalOrExternal = true;
  }
  else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    const char *Sym = S->getSymbol();

    if (CheriCapTable) {
      Callee = getCallTargetFromCapTable(S, DL, CapType, DAG, Chain,
                                         FuncInfo->callPtrInfo(MF, Sym));
      IsCallReloc = true;
    } else if (!IsPIC) { // static
      assert(!ABI.IsCheriPureCap());
      // NOTE: address space zero is correct here since we are in the legacy ABI
      Callee = DAG.getTargetExternalSymbol(
          Sym, getPointerTy(DAG.getDataLayout(), 0), MipsII::MO_NO_FLAG);
    } else if (Subtarget.useXGOT()) {
      Callee = getAddrGlobalLargeGOT(S, DL, Ty, DAG, MipsII::MO_CALL_HI16,
                                     MipsII::MO_CALL_LO16, Chain,
                                     FuncInfo->callPtrInfo(MF, Sym), /*IsForTls=*/false);
      IsCallReloc = true;
    } else { // PIC
      Callee = getAddrGlobal(S, DL, Ty, DAG, MipsII::MO_GOT_CALL, Chain,
                             FuncInfo->callPtrInfo(MF, Sym), /*IsForTls=*/false);
      IsCallReloc = true;
    }

    GlobalOrExternal = true;
  }

  SmallVector<SDValue, 8> Ops(1, Chain);
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

  getOpndList(Ops, RegsToPass, IsPIC, GlobalOrExternal, InternalLinkage,
              IsCallReloc, CLI, Callee, Chain);

  // If we're doing a CCall then any unused arg registers should be zero.
  if (UseClearRegs && (CallConv == CallingConv::CHERI_CCall)) {
    assert(CapArgs >= 2);
    uint32_t ClearRegsMask = 0;
    // Cap registers C1-C10 are used for arguments (C1/C2 for the CHERI object)
    for (unsigned i=CapArgs+1 ; i<11 ; i++) {
      ClearRegsMask |= 1 << i;
    }
    assert(ClearRegsMask <= 0xffff);
    ClearRegsMask <<= 16;
    // Int registers $4-$11 are used for arguments and v0 ($1) for the method
    // number
    for (unsigned i=IntArgs+4 ; i<12 ; i++) {
      ClearRegsMask |= 1 << i;
      assert((1 << i) <= 0xffff);
    }
    Ops.insert(Ops.begin()+2, DAG.getConstant(ClearRegsMask, DL, MVT::i32));
    Chain = DAG.getNode(MipsISD::CheriJmpLink, DL, NodeTys, Ops);
  } else {
    if (IsTailCall) {
      MF.getFrameInfo().setHasTailCall();
      SDValue Ret = DAG.getNode(MipsISD::TailCall, DL, MVT::Other, Ops);
      DAG.addCallSiteInfo(Ret.getNode(), std::move(CSInfo));
      return Ret;
    }

    if (ABI.IsCheriPureCap())
      Chain = DAG.getNode(MipsISD::CapJmpLink, DL, NodeTys, Ops);
    else
      Chain = DAG.getNode(MipsISD::JmpLink, DL, NodeTys, Ops);
  }
  SDValue InFlag = Chain.getValue(1);

  DAG.addCallSiteInfo(Chain.getNode(), std::move(CSInfo));

  // Create the CALLSEQ_END node in the case of where it is not a call to
  // memcpy.
  if (!(MemcpyInByVal)) {
    Chain = DAG.getCALLSEQ_END(Chain, NextStackOffsetVal,
                               DAG.getIntPtrConstant(0, DL, true), InFlag, DL);
    InFlag = Chain.getValue(1);
  }

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, IsVarArg, Ins, DL, DAG,
                         InVals, CLI);
}

SDValue MipsTargetLowering::setPccOffset(SelectionDAG &DAG, const SDLoc DL,
                                         SDValue Offset) const {
  auto GetPCC = DAG.getConstant(Intrinsic::cheri_pcc_get, DL, MVT::i64);
  auto SetOffset =
      DAG.getConstant(Intrinsic::cheri_cap_offset_set, DL, MVT::i64);
  auto PCC = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, CapType, GetPCC);
  return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, CapType, SetOffset, PCC,
                     Offset);
}

SDValue MipsTargetLowering::cFromDDC(SelectionDAG &DAG, const SDLoc DL,
                                     SDValue Offset) const {
  auto GetDDC = DAG.getConstant(Intrinsic::cheri_ddc_get, DL, MVT::i64);
  auto DDC = DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, CapType, GetDDC);
  auto CFromPtr =
      DAG.getConstant(Intrinsic::cheri_cap_from_pointer, DL, MVT::i64);
  return DAG.getNode(ISD::INTRINSIC_WO_CHAIN, DL, CapType, CFromPtr, DDC,
                     Offset);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue MipsTargetLowering::LowerCallResult(
    SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals,
    TargetLowering::CallLoweringInfo &CLI) const {
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  MipsCCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                     *DAG.getContext());

  const ExternalSymbolSDNode *ES =
      dyn_cast_or_null<const ExternalSymbolSDNode>(CLI.Callee.getNode());
  CCInfo.AnalyzeCallResult(Ins, RetCC_Mips, CLI.RetTy,
                           ES ? ES->getSymbol() : nullptr);


  // Calling an external function could change the value of $cgp. We need to
  // restore it to the $cgp on entry before calling the next function.
  // In the PLT ABI we must restore the old value of $cgp prior to return if
  // we called an external function or a function pointer since these may have
  // changed the $cgp value. This ensures that the callee still has the correct
  // value for $cgp after calling any local function (even if that function
  // calls an external function).
  // This could cause errors in the following sequence of calls:
  //
  // func_in_dso1() calls func_in_dso2() -> on return $cgp is set to the $cgp of
  // DSO2. If func_in_dso1() then calls another function in DSO1 (i.e. any call
  // without a trampoline that changes $cgp) that function will still have the
  // $cgp of DSO2 and will crash (or load the wrong globals).
  //
  // Note: we do not need to do this for the pc-relative ABI since each function
  // derives the value of $cgp from the $pcc on entry ($c12)
  //
  // TODO: This could be optimized to only be done prior to the next call or
  // return, but I think this will only remove very few instructions
  // E.g. we can omit the restore in the case where we call a function pointer
  // next since that will not use $cgp
  if (ABI.IsCheriPureCap() && MCTargetOptions::cheriCapabilityTableABI() !=
                                       CheriCapabilityTableABI::Pcrel) {
    bool LocalCallOptimization = false;
    bool IsPerFileOrPerFunctionCapTable = false; // TODO: add option
    // FIXME: for captable per file/function this should always be false!
    // For now we just require -O0 for per-file/per-function captable
    // When optimization is enabled, we can omit the restoring of $cgp
    if (!IsPerFileOrPerFunctionCapTable && !isOptNone(DAG.getMachineFunction())) {
      if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(CLI.Callee)) {
        LocalCallOptimization = G->getGlobal()->isDSOLocal();
      }
    }

    if (!LocalCallOptimization) {
      // Note: we don't need to restore the entry $cgp when calling a DSO local
      // function since all local functions ensure that $cgp on entry == $cgp on exit
      Chain = DAG.getCopyToReg(Chain, DL, ABI.GetGlobalCapability(), getCapGlobalReg(CLI.DAG, CapType), InFlag);
      InFlag = Chain.getValue(1);
    } else {
      // TODO: at -O1 assert that $cgp before and after is the same?
#if 0
      SDNode *Equal = DAG.getMachineNode(Mips::CEXEQ32, DL, MVT::i32,
          DAG.getRegister(ABI.GetGlobalCapability(), CapType), getCapGlobalReg(CLI.DAG, CapType));
      SDNode *TrapNotEqual = DAG.getMachineNode(Mips::TEQ, DL, MVT::Other, DAG.getConstant(0, DL, MVT::i32, true), SDValue(Equal, 0), DAG.getConstant(0, DL, MVT::i32, true));
      // FIXME: how to add this to chain correctly?
      DAG.dump();
#endif
    }
  }

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue Val = DAG.getCopyFromReg(Chain, DL, RVLocs[i].getLocReg(),
                                     RVLocs[i].getLocVT(), InFlag);
    Chain = Val.getValue(1);
    InFlag = Val.getValue(2);

    if (VA.isUpperBitsInLoc()) {
      unsigned ValSizeInBits = Ins[i].ArgVT.getSizeInBits();
      unsigned LocSizeInBits = VA.getLocVT().getSizeInBits();
      unsigned Shift =
          VA.getLocInfo() == CCValAssign::ZExtUpper ? ISD::SRL : ISD::SRA;
      Val = DAG.getNode(
          Shift, DL, VA.getLocVT(), Val,
          DAG.getConstant(LocSizeInBits - ValSizeInBits, DL, VA.getLocVT()));
    }

    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::BCvt:
      Val = DAG.getNode(ISD::BITCAST, DL, VA.getValVT(), Val);
      break;
    case CCValAssign::AExt:
    case CCValAssign::AExtUpper:
      Val = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), Val);
      break;
    case CCValAssign::ZExt:
    case CCValAssign::ZExtUpper:
      Val = DAG.getNode(ISD::AssertZext, DL, VA.getLocVT(), Val,
                        DAG.getValueType(VA.getValVT()));
      Val = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), Val);
      break;
    case CCValAssign::SExt:
    case CCValAssign::SExtUpper:
      Val = DAG.getNode(ISD::AssertSext, DL, VA.getLocVT(), Val,
                        DAG.getValueType(VA.getValVT()));
      Val = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), Val);
      break;
    }

    InVals.push_back(Val);
  }

  return Chain;
}

static SDValue UnpackFromArgumentSlot(SDValue Val, const CCValAssign &VA,
                                      EVT ArgVT, const SDLoc &DL,
                                      SelectionDAG &DAG) {
  MVT LocVT = VA.getLocVT();
  EVT ValVT = VA.getValVT();

  // Shift into the upper bits if necessary.
  switch (VA.getLocInfo()) {
  default:
    break;
  case CCValAssign::AExtUpper:
  case CCValAssign::SExtUpper:
  case CCValAssign::ZExtUpper: {
    unsigned ValSizeInBits = ArgVT.getSizeInBits();
    unsigned LocSizeInBits = VA.getLocVT().getSizeInBits();
    unsigned Opcode =
        VA.getLocInfo() == CCValAssign::ZExtUpper ? ISD::SRL : ISD::SRA;
    Val = DAG.getNode(
        Opcode, DL, VA.getLocVT(), Val,
        DAG.getConstant(LocSizeInBits - ValSizeInBits, DL, VA.getLocVT()));
    break;
  }
  }

  // If this is an value smaller than the argument slot size (32-bit for O32,
  // 64-bit for N32/N64), it has been promoted in some way to the argument slot
  // size. Extract the value and insert any appropriate assertions regarding
  // sign/zero extension.
  switch (VA.getLocInfo()) {
  default:
    llvm_unreachable("Unknown loc info!");
  case CCValAssign::Full:
    break;
  case CCValAssign::AExtUpper:
  case CCValAssign::AExt:
    Val = DAG.getNode(ISD::TRUNCATE, DL, ValVT, Val);
    break;
  case CCValAssign::SExtUpper:
  case CCValAssign::SExt:
    Val = DAG.getNode(ISD::AssertSext, DL, LocVT, Val, DAG.getValueType(ValVT));
    Val = DAG.getNode(ISD::TRUNCATE, DL, ValVT, Val);
    break;
  case CCValAssign::ZExtUpper:
  case CCValAssign::ZExt:
    Val = DAG.getNode(ISD::AssertZext, DL, LocVT, Val, DAG.getValueType(ValVT));
    Val = DAG.getNode(ISD::TRUNCATE, DL, ValVT, Val);
    break;
  case CCValAssign::BCvt:
    Val = DAG.getNode(ISD::BITCAST, DL, ValVT, Val);
    break;
  }

  return Val;
}

//===----------------------------------------------------------------------===//
//             Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//
/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue MipsTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();

  MipsFI->setVarArgsFrameIndex(0);

  // Used with vargs to acumulate store chains.
  std::vector<SDValue> OutChains;

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  MipsCCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                     *DAG.getContext());
  CCInfo.AllocateStack(ABI.GetCalleeAllocdArgSizeInBytes(CallConv), Align(1));
  const Function &Func = DAG.getMachineFunction().getFunction();
  Function::const_arg_iterator FuncArg = Func.arg_begin();

  if (Func.hasFnAttribute("interrupt") && !Func.arg_empty())
    report_fatal_error(
        "Functions with the interrupt attribute cannot have arguments!");

  CCInfo.AnalyzeFormalArguments(Ins, CC_Mips_FixedArg);
  MipsFI->setFormalArgInfo(CCInfo.getNextStackOffset(),
                           CCInfo.getInRegsParamsCount() > 0);

  unsigned CurArgIdx = 0;
  CCInfo.rewindByValRegsInfo();

  for (unsigned i = 0, e = ArgLocs.size(), InsIdx = 0; i != e; ++i, ++InsIdx) {
    CCValAssign &VA = ArgLocs[i];
    if (Ins[InsIdx].isOrigArg()) {
      std::advance(FuncArg, Ins[InsIdx].getOrigArgIndex() - CurArgIdx);
      CurArgIdx = Ins[InsIdx].getOrigArgIndex();
    }
    EVT ValVT = VA.getValVT();
    ISD::ArgFlagsTy Flags = Ins[InsIdx].Flags;
    bool IsRegLoc = VA.isRegLoc();

    if (Flags.isByVal()) {
      assert(Ins[InsIdx].isOrigArg() && "Byval arguments cannot be implicit");
      unsigned FirstByValReg, LastByValReg;
      unsigned ByValIdx = CCInfo.getInRegsParamsProcessed();
      CCInfo.getInRegsParamInfo(ByValIdx, FirstByValReg, LastByValReg);

      assert(Flags.getByValSize() &&
             "ByVal args of size 0 should have been ignored by front-end.");
      assert(ByValIdx < CCInfo.getInRegsParamsCount());
      copyByValRegs(Chain, DL, OutChains, DAG, Flags, InVals, &*FuncArg,
                    FirstByValReg, LastByValReg, VA, CCInfo);
      CCInfo.nextInRegsParam();
      continue;
    }

    // Arguments stored on registers
    if (IsRegLoc) {
      MVT RegVT = VA.getLocVT();
      Register ArgReg = VA.getLocReg();
      const TargetRegisterClass *RC = getRegClassFor(RegVT);

      // Transform the arguments stored on
      // physical registers into virtual ones
      unsigned Reg = addLiveIn(DAG.getMachineFunction(), ArgReg, RC);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);

      ArgValue =
          UnpackFromArgumentSlot(ArgValue, VA, Ins[InsIdx].ArgVT, DL, DAG);

      // Handle floating point arguments passed in integer registers and
      // long double arguments passed in floating point registers.
      if ((RegVT == MVT::i32 && ValVT == MVT::f32) ||
          (RegVT == MVT::i64 && ValVT == MVT::f64) ||
          (RegVT == MVT::f64 && ValVT == MVT::i64))
        ArgValue = DAG.getNode(ISD::BITCAST, DL, ValVT, ArgValue);
      else if (ABI.IsO32() && RegVT == MVT::i32 &&
               ValVT == MVT::f64) {
        assert(VA.needsCustom() && "Expected custom argument for f64 split");
        CCValAssign &NextVA = ArgLocs[++i];
        unsigned Reg2 =
            addLiveIn(DAG.getMachineFunction(), NextVA.getLocReg(), RC);
        SDValue ArgValue2 = DAG.getCopyFromReg(Chain, DL, Reg2, RegVT);
        if (!Subtarget.isLittle())
          std::swap(ArgValue, ArgValue2);
        ArgValue = DAG.getNode(MipsISD::BuildPairF64, DL, MVT::f64,
                               ArgValue, ArgValue2);
      }

      InVals.push_back(ArgValue);
    } else { // VA.isRegLoc()
      MVT LocVT = VA.getLocVT();

      assert(!VA.needsCustom() && "unexpected custom memory argument");

      if (ABI.IsO32()) {
        // We ought to be able to use LocVT directly but O32 sets it to i32
        // when allocating floating point values to integer registers.
        // This shouldn't influence how we load the value into registers unless
        // we are targeting softfloat.
        if (VA.getValVT().isFloatingPoint() && !Subtarget.useSoftFloat())
          LocVT = VA.getValVT();
      }

      // sanity check
      assert(VA.isMemLoc());


      // Create load nodes to retrieve arguments from the stack
      SDValue ArgValue;
      if (ABI.IsCheriPureCap()) {
        Register CapArgReg = MF.addLiveIn(Mips::C13, getRegClassFor(CapType));
        SDValue Addr = DAG.getPointerAdd(DL, DAG.getCopyFromReg(Chain, DL,
              CapArgReg, CapType), VA.getLocMemOffset());
        ArgValue = DAG.getLoad(LocVT, DL, Chain, Addr, MachinePointerInfo());
      } else {
        // The stack pointer offset is relative to the caller stack frame.
        int FI = MFI.CreateFixedObject(LocVT.getSizeInBits() / 8,
                                       VA.getLocMemOffset(), true);
        SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout(),
                    ABI.StackAddrSpace()));
        ArgValue = DAG.getLoad(
            LocVT, DL, Chain, FIN,
            MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI));
      }
      OutChains.push_back(ArgValue.getValue(1));

      ArgValue =
          UnpackFromArgumentSlot(ArgValue, VA, Ins[InsIdx].ArgVT, DL, DAG);

      InVals.push_back(ArgValue);
    }
  }

  for (unsigned i = 0, e = ArgLocs.size(), InsIdx = 0; i != e; ++i, ++InsIdx) {

    if (ArgLocs[i].needsCustom()) {
      ++i;
      continue;
    }

    // The mips ABIs for returning structs by value requires that we copy
    // the sret argument into $v0 for the return. Save the argument into
    // a virtual register so that we can access it from the return points.
    if (Ins[InsIdx].Flags.isSRet()) {
      unsigned Reg = MipsFI->getSRetReturnReg();
      if (!Reg) {
        Reg = MF.getRegInfo().createVirtualRegister(
            getRegClassFor(ABI.IsCheriPureCap() ? CapType :
                             (ABI.IsN64() ? MVT::i64 : MVT::i32)));
        MipsFI->setSRetReturnReg(Reg);
      }
      SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), DL, Reg, InVals[i]);
      Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, Copy, Chain);
      break;
    }
  }

  if (IsVarArg)
    writeVarArgRegs(OutChains, Chain, DL, DAG, CCInfo);

  // All stores are grouped in one node to allow the matching between
  // the size of Ins and InVals. This only happens when on varg functions
  if (!OutChains.empty()) {
    OutChains.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, OutChains);
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

bool
MipsTargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                   MachineFunction &MF, bool IsVarArg,
                                   const SmallVectorImpl<ISD::OutputArg> &Outs,
                                   LLVMContext &Context) const {
  SmallVector<CCValAssign, 16> RVLocs;
  MipsCCState CCInfo(CallConv, IsVarArg, MF, RVLocs, Context);
  return CCInfo.CheckReturn(Outs, RetCC_Mips);
}

bool MipsTargetLowering::shouldSignExtendTypeInLibCall(EVT Type,
                                                       bool IsSigned) const {
  if ((ABI.IsN32() || ABI.IsN64()) && Type == MVT::i32)
      return true;

  return IsSigned;
}

SDValue
MipsTargetLowering::LowerInterruptReturn(SmallVectorImpl<SDValue> &RetOps,
                                         const SDLoc &DL,
                                         SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();

  MipsFI->setISR();

  return DAG.getNode(MipsISD::ERet, DL, MVT::Other, RetOps);
}

SDValue
MipsTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                bool IsVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                const SDLoc &DL, SelectionDAG &DAG) const {
  // CCValAssign - represent the assignment of
  // the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;
  MachineFunction &MF = DAG.getMachineFunction();

  // CCState - Info about the registers and stack slot.
  MipsCCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());

  // Analyze return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_Mips);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);
  bool zeroV0 = true;
  bool zeroV1 = true;
  bool zeroC3 = true;
  bool zeroC4 = true;

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    SDValue Val = OutVals[i];
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");
    bool UseUpperBits = false;

    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::BCvt:
      Val = DAG.getNode(ISD::BITCAST, DL, VA.getLocVT(), Val);
      break;
    case CCValAssign::AExtUpper:
      UseUpperBits = true;
      LLVM_FALLTHROUGH;
    case CCValAssign::AExt:
      Val = DAG.getNode(ISD::ANY_EXTEND, DL, VA.getLocVT(), Val);
      break;
    case CCValAssign::ZExtUpper:
      UseUpperBits = true;
      LLVM_FALLTHROUGH;
    case CCValAssign::ZExt:
      Val = DAG.getNode(ISD::ZERO_EXTEND, DL, VA.getLocVT(), Val);
      break;
    case CCValAssign::SExtUpper:
      UseUpperBits = true;
      LLVM_FALLTHROUGH;
    case CCValAssign::SExt:
      Val = DAG.getNode(ISD::SIGN_EXTEND, DL, VA.getLocVT(), Val);
      break;
    }

    if (UseUpperBits) {
      unsigned ValSizeInBits = Outs[i].ArgVT.getSizeInBits();
      unsigned LocSizeInBits = VA.getLocVT().getSizeInBits();
      Val = DAG.getNode(
          ISD::SHL, DL, VA.getLocVT(), Val,
          DAG.getConstant(LocSizeInBits - ValSizeInBits, DL, VA.getLocVT()));
    }

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Val, Flag);
    switch (VA.getLocReg()) {
      case Mips::V0_64:
      case Mips::V0:
        zeroV0 = false;
        break;
      case Mips::V1_64:
      case Mips::V1:
        zeroV1 = false;
        break;
      case Mips::C3:
        zeroC3 = false;
        break;
      case Mips::C4:
        zeroC4 = false;
        break;
    }

    // Guarantee that all emitted copies are stuck together with flags.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }
  if (CallConv == CallingConv::CHERI_CCall ||
      CallConv == CallingConv::CHERI_CCallee) {
    if (zeroV0) {
      Chain = DAG.getCopyToReg(Chain, DL, Mips::V0_64,
          DAG.getConstant(0, DL, MVT::i64), Flag);
      Flag = Chain.getValue(1);
      RetOps.push_back(DAG.getRegister(Mips::V0_64, MVT::i64));
    }
    if (zeroV1) {
      Chain = DAG.getCopyToReg(Chain, DL, Mips::V1_64,
          DAG.getConstant(0, DL, MVT::i64), Flag);
      Flag = Chain.getValue(1);
      RetOps.push_back(DAG.getRegister(Mips::V1_64, MVT::i64));
    }
    if (zeroC3) {
      Chain = DAG.getCopyToReg(Chain, DL, Mips::C3,
                               DAG.getNullCapability(DL), Flag);
      Flag = Chain.getValue(1);
      RetOps.push_back(DAG.getRegister(Mips::C3, CapType));
    }
    if (zeroC4) {
      Chain = DAG.getCopyToReg(Chain, DL, Mips::C4,
                               DAG.getNullCapability(DL), Flag);
      Flag = Chain.getValue(1);
      RetOps.push_back(DAG.getRegister(Mips::C4, CapType));
    }
  }

  // The mips ABIs for returning structs by value requires that we copy
  // the sret argument into $v0 for the return. We saved the argument into
  // a virtual register in the entry block, so now we copy the value out
  // and into $v0.
  if (MF.getFunction().hasStructRetAttr()) {
    EVT SRetTy = getPointerTy(DAG.getDataLayout(), ABI.StackAddrSpace());
    unsigned V0 = ABI.IsN64() ? Mips::V0_64 : Mips::V0;
    if (ABI.IsCheriPureCap()) {
      V0 = Mips::C3;
      SRetTy = CapType;
    }
    MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();
    unsigned Reg = MipsFI->getSRetReturnReg();

    if (!Reg)
      llvm_unreachable("sret virtual register not created in the entry block");
    SDValue Val = DAG.getCopyFromReg(Chain, DL, Reg, SRetTy);

    Chain = DAG.getCopyToReg(Chain, DL, V0, Val, Flag);
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(V0, SRetTy));
  }

  if (ABI.IsCheriPureCap()) {
    // If this function used $c13 we want to zero it before returning so that
    // all functions without on-stack arguments can assume that $c13 is null
    // on entry
    if (MF.getRegInfo().isLiveIn(Mips::C13) || IsVarArg) {
      LLVM_DEBUG(dbgs() << "Lowering return for function with $c13 live-in: "
                        << MF.getName() << "(is varargs: "
                        << MF.getFunction().isVarArg() << ")\n");
      Chain = DAG.getCopyToReg(Chain, DL, Mips::C13,
                               DAG.getNullCapability(DL), Flag);
      Flag = Chain.getValue(1);
      RetOps.push_back(DAG.getRegister(Mips::C13, CapType));
    }
  }

  RetOps[0] = Chain;  // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  if (ABI.IsCheriPureCap())
    return DAG.getNode(MipsISD::CapRet, DL, MVT::Other, RetOps);

  // ISRs must use "eret".
  if (DAG.getMachineFunction().getFunction().hasFnAttribute("interrupt"))
    return LowerInterruptReturn(RetOps, DL, DAG);

  // Standard return on Mips is a "jr $ra"
  return DAG.getNode(MipsISD::Ret, DL, MVT::Other, RetOps);
}

//===----------------------------------------------------------------------===//
//                           Mips Inline Assembly Support
//===----------------------------------------------------------------------===//

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
MipsTargetLowering::ConstraintType
MipsTargetLowering::getConstraintType(StringRef Constraint) const {
  // Mips specific constraints
  // GCC config/mips/constraints.md
  //
  // 'd' : An address register. Equivalent to r
  //       unless generating MIPS16 code.
  // 'y' : Equivalent to r; retained for
  //       backwards compatibility.
  // 'c' : A register suitable for use in an indirect
  //       jump. This will always be $25 for -mabicalls.
  // 'l' : The lo register. 1 word storage.
  // 'x' : The hilo register pair. Double word storage.
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
      default : break;
      case 'd':
      case 'y':
      case 'f':
      case 'c':
      case 'l':
      case 'x':
      case 'C':
        return C_RegisterClass;
      case 'R':
        return C_Memory;
    }
  }

  if (Constraint == "ZC")
    return C_Memory;

  return TargetLowering::getConstraintType(Constraint);
}

/// Examine constraint type and operand type and determine a weight value.
/// This object must already have been set up with the operand type
/// and the current alternative constraint selected.
TargetLowering::ConstraintWeight
MipsTargetLowering::getSingleConstraintMatchWeight(
    AsmOperandInfo &info, const char *constraint) const {
  ConstraintWeight weight = CW_Invalid;
  Value *CallOperandVal = info.CallOperandVal;
    // If we don't have a value, we can't do a match,
    // but allow it at the lowest weight.
  if (!CallOperandVal)
    return CW_Default;
  Type *type = CallOperandVal->getType();
  // Look at the constraint type.
  switch (*constraint) {
  default:
    weight = TargetLowering::getSingleConstraintMatchWeight(info, constraint);
    break;
  case 'd':
  case 'y':
    if (type->isIntegerTy())
      weight = CW_Register;
    break;
  case 'f': // FPU or MSA register
    if (Subtarget.hasMSA() && type->isVectorTy() &&
        type->getPrimitiveSizeInBits().getFixedSize() == 128)
      weight = CW_Register;
    else if (type->isFloatTy())
      weight = CW_Register;
    break;
  case 'c': // $25 for indirect jumps
  case 'l': // lo register
  case 'x': // hilo register pair
    if (type->isIntegerTy())
      weight = CW_SpecificReg;
    break;
  case 'I': // signed 16 bit immediate
  case 'J': // integer zero
  case 'K': // unsigned 16 bit immediate
  case 'L': // signed 32 bit immediate where lower 16 bits are 0
  case 'N': // immediate in the range of -65535 to -1 (inclusive)
  case 'O': // signed 15 bit immediate (+- 16383)
  case 'P': // immediate in the range of 65535 to 1 (inclusive)
    if (isa<ConstantInt>(CallOperandVal))
      weight = CW_Constant;
    break;
  case 'R':
    weight = CW_Memory;
    break;
  }
  return weight;
}

/// This is a helper function to parse a physical register string and split it
/// into non-numeric and numeric parts (Prefix and Reg). The first boolean flag
/// that is returned indicates whether parsing was successful. The second flag
/// is true if the numeric part exists.
static std::pair<bool, bool> parsePhysicalReg(StringRef C, StringRef &Prefix,
                                              unsigned long long &Reg) {
  if (C.empty() || C.front() != '{' || C.back() != '}')
    return std::make_pair(false, false);

  // Search for the first numeric character.
  StringRef::const_iterator I, B = C.begin() + 1, E = C.end() - 1;
  I = std::find_if(B, E, isdigit);

  Prefix = StringRef(B, I - B);

  // The second flag is set to false if no numeric characters were found.
  if (I == E)
    return std::make_pair(true, false);

  // Parse the numeric characters.
  return std::make_pair(!getAsUnsignedInteger(StringRef(I, E - I), 10, Reg),
                        true);
}

EVT MipsTargetLowering::getTypeForExtReturn(LLVMContext &Context, EVT VT,
                                            ISD::NodeType) const {
  bool Cond = !Subtarget.isABI_O32() && VT.getSizeInBits() == 32;
  EVT MinVT = getRegisterType(Context, Cond ? MVT::i64 : MVT::i32);
  return VT.bitsLT(MinVT) ? MinVT : VT;
}

std::pair<unsigned, const TargetRegisterClass *> MipsTargetLowering::
parseRegForInlineAsmConstraint(StringRef C, MVT VT) const {
  const TargetRegisterInfo *TRI =
      Subtarget.getRegisterInfo();
  const TargetRegisterClass *RC;
  StringRef Prefix;
  unsigned long long Reg;

  std::pair<bool, bool> R = parsePhysicalReg(C, Prefix, Reg);

  if (!R.first)
    return std::make_pair(0U, nullptr);

  if ((Prefix == "hi" || Prefix == "lo")) { // Parse hi/lo.
    // No numeric characters follow "hi" or "lo".
    if (R.second)
      return std::make_pair(0U, nullptr);

    RC = TRI->getRegClass(Prefix == "hi" ?
                          Mips::HI32RegClassID : Mips::LO32RegClassID);
    return std::make_pair(*(RC->begin()), RC);
  } else if (Prefix.startswith("$msa")) {
    // Parse $msa(ir|csr|access|save|modify|request|map|unmap)

    // No numeric characters follow the name.
    if (R.second)
      return std::make_pair(0U, nullptr);

    Reg = StringSwitch<unsigned long long>(Prefix)
              .Case("$msair", Mips::MSAIR)
              .Case("$msacsr", Mips::MSACSR)
              .Case("$msaaccess", Mips::MSAAccess)
              .Case("$msasave", Mips::MSASave)
              .Case("$msamodify", Mips::MSAModify)
              .Case("$msarequest", Mips::MSARequest)
              .Case("$msamap", Mips::MSAMap)
              .Case("$msaunmap", Mips::MSAUnmap)
              .Default(0);

    if (!Reg)
      return std::make_pair(0U, nullptr);

    RC = TRI->getRegClass(Mips::MSACtrlRegClassID);
    return std::make_pair(Reg, RC);
  }

  if (!R.second)
    return std::make_pair(0U, nullptr);

  if (Prefix == "$f") { // Parse $f0-$f31.
    // If the size of FP registers is 64-bit or Reg is an even number, select
    // the 64-bit register class. Otherwise, select the 32-bit register class.
    if (VT == MVT::Other)
      VT = (Subtarget.isFP64bit() || !(Reg % 2)) ? MVT::f64 : MVT::f32;

    RC = getRegClassFor(VT);

    if (RC == &Mips::AFGR64RegClass) {
      assert(Reg % 2 == 0);
      Reg >>= 1;
    }
  } else if (Prefix == "$fcc") // Parse $fcc0-$fcc7.
    RC = TRI->getRegClass(Mips::FCCRegClassID);
  else if (Prefix == "$w") { // Parse $w0-$w31.
    RC = getRegClassFor((VT == MVT::Other) ? MVT::v16i8 : VT);
  } else if (Prefix == "$c") { // Parse $c0-$c31.
    if (Reg == 0)
      report_fatal_error("$c0 should not be used as an inline asm constraint");
    RC = getRegClassFor(CapType);
    assert(RC == &Mips::CheriGPRRegClass);
    assert(!RC->contains(Mips::CNULL));
    assert(!RC->contains(Mips::DDC));
    // Note: The CheriGPR register class does not contain CNULL so we have to
    // subtrace one to get the actual register number.
    Reg--;
  } else { // Parse $0-$31.
    assert(Prefix == "$");
    RC = getRegClassFor((VT == MVT::Other) ? MVT::i32 : VT);
  }

  assert(Reg < RC->getNumRegs());
  return std::make_pair(*(RC->begin() + Reg), RC);
}

/// Given a register class constraint, like 'r', if this corresponds directly
/// to an LLVM register class, return a register of 0 and the register class
/// pointer.
std::pair<unsigned, const TargetRegisterClass *>
MipsTargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                 StringRef Constraint,
                                                 MVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'd': // Address register. Same as 'r' unless generating MIPS16 code.
    case 'y': // Same as 'r'. Exists for compatibility.
    case 'r':
      if (VT == MVT::i32 || VT == MVT::i16 || VT == MVT::i8) {
        if (Subtarget.inMips16Mode())
          return std::make_pair(0U, &Mips::CPU16RegsRegClass);
        return std::make_pair(0U, &Mips::GPR32RegClass);
      }
      if (VT == MVT::i64 && !Subtarget.isGP64bit())
        return std::make_pair(0U, &Mips::GPR32RegClass);
      if (VT == MVT::i64 && Subtarget.isGP64bit())
        return std::make_pair(0U, &Mips::GPR64RegClass);
      LLVM_FALLTHROUGH; // also allow capability with r (for compatibility)
    case 'C':
      if (VT == CapType)
        return std::make_pair(0U, &Mips::CheriGPRRegClass);
      // This will generate an error message
      return std::make_pair(0U, nullptr);
    case 'f': // FPU or MSA register
      if (VT == MVT::v16i8)
        return std::make_pair(0U, &Mips::MSA128BRegClass);
      else if (VT == MVT::v8i16 || VT == MVT::v8f16)
        return std::make_pair(0U, &Mips::MSA128HRegClass);
      else if (VT == MVT::v4i32 || VT == MVT::v4f32)
        return std::make_pair(0U, &Mips::MSA128WRegClass);
      else if (VT == MVT::v2i64 || VT == MVT::v2f64)
        return std::make_pair(0U, &Mips::MSA128DRegClass);
      else if (VT == MVT::f32)
        return std::make_pair(0U, &Mips::FGR32RegClass);
      else if ((VT == MVT::f64) && (!Subtarget.isSingleFloat())) {
        if (Subtarget.isFP64bit())
          return std::make_pair(0U, &Mips::FGR64RegClass);
        return std::make_pair(0U, &Mips::AFGR64RegClass);
      }
      break;
    case 'c': // register suitable for indirect jump
      if (VT == MVT::i32)
        return std::make_pair((unsigned)Mips::T9, &Mips::GPR32RegClass);
      if (VT == MVT::i64)
        return std::make_pair((unsigned)Mips::T9_64, &Mips::GPR64RegClass);
      // This will generate an error message
      return std::make_pair(0U, nullptr);
    case 'l': // use the `lo` register to store values
              // that are no bigger than a word
      if (VT == MVT::i32 || VT == MVT::i16 || VT == MVT::i8)
        return std::make_pair((unsigned)Mips::LO0, &Mips::LO32RegClass);
      return std::make_pair((unsigned)Mips::LO0_64, &Mips::LO64RegClass);
    case 'x': // use the concatenated `hi` and `lo` registers
              // to store doubleword values
      // Fixme: Not triggering the use of both hi and low
      // This will generate an error message
      return std::make_pair(0U, nullptr);
    }
  }

  if (!Constraint.empty()) {
    std::pair<unsigned, const TargetRegisterClass *> R;
    R = parseRegForInlineAsmConstraint(Constraint, VT);

    if (R.second)
      return R;
  }

  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}

/// LowerAsmOperandForConstraint - Lower the specified operand into the Ops
/// vector.  If it is invalid, don't add anything to Ops.
void MipsTargetLowering::LowerAsmOperandForConstraint(SDValue Op,
                                                     std::string &Constraint,
                                                     std::vector<SDValue>&Ops,
                                                     SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Result;

  // Only support length 1 constraints for now.
  if (Constraint.length() > 1) return;

  char ConstraintLetter = Constraint[0];
  switch (ConstraintLetter) {
  default: break; // This will fall through to the generic implementation
  case 'I': // Signed 16 bit constant
    // If this fails, the parent routine will give an error
    if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op)) {
      EVT Type = Op.getValueType();
      int64_t Val = C->getSExtValue();
      if (isInt<16>(Val)) {
        Result = DAG.getTargetConstant(Val, DL, Type);
        break;
      }
    }
    return;
  case 'J': // integer zero
    if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op)) {
      EVT Type = Op.getValueType();
      int64_t Val = C->getZExtValue();
      if (Val == 0) {
        Result = DAG.getTargetConstant(0, DL, Type);
        break;
      }
    }
    return;
  case 'K': // unsigned 16 bit immediate
    if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op)) {
      EVT Type = Op.getValueType();
      uint64_t Val = (uint64_t)C->getZExtValue();
      if (isUInt<16>(Val)) {
        Result = DAG.getTargetConstant(Val, DL, Type);
        break;
      }
    }
    return;
  case 'L': // signed 32 bit immediate where lower 16 bits are 0
    if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op)) {
      EVT Type = Op.getValueType();
      int64_t Val = C->getSExtValue();
      if ((isInt<32>(Val)) && ((Val & 0xffff) == 0)){
        Result = DAG.getTargetConstant(Val, DL, Type);
        break;
      }
    }
    return;
  case 'N': // immediate in the range of -65535 to -1 (inclusive)
    if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op)) {
      EVT Type = Op.getValueType();
      int64_t Val = C->getSExtValue();
      if ((Val >= -65535) && (Val <= -1)) {
        Result = DAG.getTargetConstant(Val, DL, Type);
        break;
      }
    }
    return;
  case 'O': // signed 15 bit immediate
    if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op)) {
      EVT Type = Op.getValueType();
      int64_t Val = C->getSExtValue();
      if ((isInt<15>(Val))) {
        Result = DAG.getTargetConstant(Val, DL, Type);
        break;
      }
    }
    return;
  case 'P': // immediate in the range of 1 to 65535 (inclusive)
    if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op)) {
      EVT Type = Op.getValueType();
      int64_t Val = C->getSExtValue();
      if ((Val <= 65535) && (Val >= 1)) {
        Result = DAG.getTargetConstant(Val, DL, Type);
        break;
      }
    }
    return;
  }

  if (Result.getNode()) {
    Ops.push_back(Result);
    return;
  }

  TargetLowering::LowerAsmOperandForConstraint(Op, Constraint, Ops, DAG);
}

bool MipsTargetLowering::isLegalAddressingMode(const DataLayout &DL,
                                               const AddrMode &AM, Type *Ty,
                                               unsigned AS,
                                               Instruction *I) const {
  // No global is ever allowed as a base.
  if (AM.BaseGV)
    return false;

  if (isCheriPointer(AS, &DL)) {
    switch (AM.Scale) {
    case 0: // "r+i" or just "i", depending on HasBaseReg.
    case 1:
      return true;
    default:
      return false;
    }
  }

  switch (AM.Scale) {
  case 0: // "r+i" or just "i", depending on HasBaseReg.
    break;
  case 1:
    if (!AM.HasBaseReg) // allow "r+i".
      break;
    return false; // disallow "r+r" or "r+r+i".
  default:
    return false;
  }

  return true;
}

bool
MipsTargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
  // The Mips target isn't yet aware of offsets.
  return false;
}

EVT MipsTargetLowering::getOptimalMemOpType(
    const MemOp &Op, const AttributeList &FuncAttributes) const {
  // If the source alignment is 32 then we are copying something that may
  // contain capabilities.  If the destination alignment is 32, then we are
  // copying to something that may contain capabilities.  If both of these is
  // true, then we must use capability copies to do preserve tag bits.
  // Otherwise, we do copying that will clear the capability tags.  For memset,
  // the source align will be 0.  We don't want to use capabilities in this
  // case, because the capability tag will always be 0.  For very long memsets,
  // we can use the capability registers in the library implementation.

  // We can't use capability stores as an optimisation for memset unless zeroing.
  // For bzero() we can (and want to) always use capability stores of $cnull.
  bool IsNonZeroMemset = Op.isMemset() && !Op.isZeroMemset();
  Align CapAlign = Subtarget.isCheri() ? Subtarget.getCapAlignment() : Align();
  if (CapAlign > 1 && Op.size() >= CapAlign.value() && !IsNonZeroMemset) {
    LLVM_DEBUG(dbgs() << __func__ << " Size=" << Op.size() << " DstAlign="
                      << (Op.isFixedDstAlign() ? Op.getDstAlign().value() : 0)
                      << " SrcAlign="
                      << (Op.isMemset() ? 0 : Op.getSrcAlign().value())
                      << " CapAlign=" << CapAlign.value() << "\n");
    // If sufficiently aligned, we must use capability loads/stores if
    // copying, and can use cnull for a zeroing memset.
    if (Op.isAligned(CapAlign)) {
      return CapType;
    } else if (!Op.isMemset()) {
      // Otherwise if this is a copy then tell SelectionDAG to do a real
      // memcpy/memmove call (by returning MVT::isVoid), since it could still
      // contain a capability if sufficiently aligned at runtime. Zeroing
      // memsets can fall back on non-capability loads/stores.
      return MVT::isVoid;
    }
    if (Subtarget.isGP64bit() && Op.isAligned(Align(8)))
      return MVT::i64;
    if (Op.isAligned(Align(4)))
      return MVT::i32;
    if (Op.isAligned(Align(2)))
      return MVT::i16;
    return MVT::i8;
  }

  if (Subtarget.isGP64bit())
    return MVT::i64;

  return MVT::i32;
}

bool MipsTargetLowering::isFPImmLegal(const APFloat &Imm, EVT VT,
                                      bool ForCodeSize) const {
  if (VT != MVT::f32 && VT != MVT::f64)
    return false;
  if (Imm.isNegZero())
    return false;
  return Imm.isZero();
}

unsigned MipsTargetLowering::getJumpTableEncoding() const {
  // XXXAR: should we emit capabilities instead?
  if (ABI.IsCheriPureCap())
    return MachineJumpTableInfo::EK_LabelDifference32;
  // TODO: use EK_Custom32 with function entry point - label address?

  // FIXME: For space reasons this should be: EK_GPRel32BlockAddress.
  if (ABI.IsN64() && isPositionIndependent())
    return MachineJumpTableInfo::EK_GPRel64BlockAddress;

  return TargetLowering::getJumpTableEncoding();
}

bool MipsTargetLowering::useSoftFloat() const {
  return Subtarget.useSoftFloat();
}

void MipsTargetLowering::copyByValRegs(
    SDValue Chain, const SDLoc &DL, std::vector<SDValue> &OutChains,
    SelectionDAG &DAG, const ISD::ArgFlagsTy &Flags,
    SmallVectorImpl<SDValue> &InVals, const Argument *FuncArg,
    unsigned FirstReg, unsigned LastReg, const CCValAssign &VA,
    MipsCCState &State) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  unsigned GPRSizeInBytes = Subtarget.getGPRSizeInBytes();
  unsigned NumRegs = LastReg - FirstReg;
  unsigned RegAreaSize = NumRegs * GPRSizeInBytes;
  unsigned FrameObjSize = std::max(Flags.getByValSize(), RegAreaSize);
  int FrameObjOffset;
  ArrayRef<MCPhysReg> ByValArgRegs = ABI.GetByValArgRegs();

  if (RegAreaSize)
    FrameObjOffset =
        (int)ABI.GetCalleeAllocdArgSizeInBytes(State.getCallingConv()) -
        (int)((ByValArgRegs.size() - FirstReg) * GPRSizeInBytes);
  else
    FrameObjOffset = VA.getLocMemOffset();

  // Create frame object.
  EVT PtrTy = getPointerTy(DAG.getDataLayout(), ABI.StackAddrSpace());
  // Make the fixed object stored to mutable so that the load instructions
  // referencing it have their memory dependencies added.
  // Set the frame object as isAliased which clears the underlying objects
  // vector in ScheduleDAGInstrs::buildSchedGraph() resulting in addition of all
  // stores as dependencies for loads referencing this fixed object.
  int FI = MFI.CreateFixedObject(FrameObjSize, FrameObjOffset, false, true);
  SDValue FIN = DAG.getFrameIndex(FI, PtrTy);
  InVals.push_back(FIN);

  if (!NumRegs)
    return;

  assert(!ABI.IsCheriPureCap() &&
         "Byval args in integer registers does not work for purecap");

  // Copy arg registers.
  MVT RegTy = MVT::getIntegerVT(GPRSizeInBytes * 8);
  const TargetRegisterClass *RC = getRegClassFor(RegTy);

  for (unsigned I = 0; I < NumRegs; ++I) {
    unsigned ArgReg = ByValArgRegs[FirstReg + I];
    unsigned VReg = addLiveIn(MF, ArgReg, RC);
    unsigned Offset = I * GPRSizeInBytes;
    SDValue StorePtr = DAG.getPointerAdd(DL, FIN, Offset);
    SDValue Store = DAG.getStore(Chain, DL, DAG.getRegister(VReg, RegTy),
                                 StorePtr, MachinePointerInfo(FuncArg, Offset));
    OutChains.push_back(Store);
  }
}

// Copy byVal arg to registers and stack.
void MipsTargetLowering::passByValArg(
    SDValue Chain, const SDLoc &DL,
    std::deque<std::pair<unsigned, SDValue>> &RegsToPass,
    SmallVectorImpl<SDValue> &MemOpChains, SDValue StackPtr,
    MachineFrameInfo &MFI, SelectionDAG &DAG, SDValue Arg, unsigned FirstReg,
    unsigned LastReg, const ISD::ArgFlagsTy &Flags, bool isLittle,
    const CCValAssign &VA) const {
  unsigned ByValSizeInBytes = Flags.getByValSize();
  unsigned OffsetInBytes = 0; // From beginning of struct
  unsigned RegSizeInBytes = ABI.IsCheriPureCap()
                                ? Subtarget.getCapSizeInBytes()
                                : Subtarget.getGPRSizeInBytes();
  Align Alignment =
      std::min(Flags.getNonZeroByValAlign(), Align(RegSizeInBytes));
  EVT PtrTy = getPointerTy(DAG.getDataLayout(), ABI.StackAddrSpace()),
      RegTy = MVT::getIntegerVT(RegSizeInBytes * 8);
  unsigned NumRegs = LastReg - FirstReg;

  if (NumRegs) {
    // Don't pass parts of the struct in integer registers (will break caps)
    // TODO: should also not happen in hybrid abi for structs with caps
    assert(!ABI.IsCheriPureCap() &&
           "Byval args in integer registers does not work for purecap");
    ArrayRef<MCPhysReg> ArgRegs = ABI.GetByValArgRegs();
    bool LeftoverBytes = (NumRegs * RegSizeInBytes > ByValSizeInBytes);
    unsigned I = 0;

    // Copy words to registers.
    for (; I < NumRegs - LeftoverBytes; ++I, OffsetInBytes += RegSizeInBytes) {
      SDValue LoadPtr = DAG.getPointerAdd(DL, Arg, OffsetInBytes);
      SDValue LoadVal = DAG.getLoad(RegTy, DL, Chain, LoadPtr,
                                    MachinePointerInfo(), Alignment);
      MemOpChains.push_back(LoadVal.getValue(1));
      unsigned ArgReg = ArgRegs[FirstReg + I];
      RegsToPass.push_back(std::make_pair(ArgReg, LoadVal));
    }

    // Return if the struct has been fully copied.
    if (ByValSizeInBytes == OffsetInBytes)
      return;

    // Copy the remainder of the byval argument with sub-word loads and shifts.
    if (LeftoverBytes) {
      SDValue Val;

      for (unsigned LoadSizeInBytes = RegSizeInBytes / 2, TotalBytesLoaded = 0;
           OffsetInBytes < ByValSizeInBytes; LoadSizeInBytes /= 2) {
        unsigned RemainingSizeInBytes = ByValSizeInBytes - OffsetInBytes;

        if (RemainingSizeInBytes < LoadSizeInBytes)
          continue;

        // Load subword.
        SDValue LoadPtr = DAG.getNode(ISD::ADD, DL, PtrTy, Arg,
                                      DAG.getConstant(OffsetInBytes, DL,
                                                      PtrTy));
        SDValue LoadVal = DAG.getExtLoad(
            ISD::ZEXTLOAD, DL, RegTy, Chain, LoadPtr, MachinePointerInfo(),
            MVT::getIntegerVT(LoadSizeInBytes * 8), Alignment);
        MemOpChains.push_back(LoadVal.getValue(1));

        // Shift the loaded value.
        unsigned Shamt;

        if (isLittle)
          Shamt = TotalBytesLoaded * 8;
        else
          Shamt = (RegSizeInBytes - (TotalBytesLoaded + LoadSizeInBytes)) * 8;

        SDValue Shift = DAG.getNode(ISD::SHL, DL, RegTy, LoadVal,
                                    DAG.getConstant(Shamt, DL, MVT::i32));

        if (Val.getNode())
          Val = DAG.getNode(ISD::OR, DL, RegTy, Val, Shift);
        else
          Val = Shift;

        OffsetInBytes += LoadSizeInBytes;
        TotalBytesLoaded += LoadSizeInBytes;
        Alignment = std::min(Alignment, Align(LoadSizeInBytes));
      }

      unsigned ArgReg = ArgRegs[FirstReg + I];
      RegsToPass.push_back(std::make_pair(ArgReg, Val));
      return;
    }
  }

  // Copy remainder of byval arg to it with memcpy.
  unsigned MemCpySize = ByValSizeInBytes - OffsetInBytes;
  SDValue Src = DAG.getPointerAdd(DL, Arg, OffsetInBytes);
  SDValue Dst = DAG.getPointerAdd(DL, StackPtr, VA.getLocMemOffset());
  Chain = DAG.getMemcpy(Chain, DL, Dst, Src,
                        DAG.getIntPtrConstant(MemCpySize, DL), Alignment,
                        /*isVolatile=*/false, /*AlwaysInline=*/false,
                        /*isTailCall=*/false, llvm::PreserveCheriTags::Unknown,
                        MachinePointerInfo(), MachinePointerInfo());
  MemOpChains.push_back(Chain);
}

void MipsTargetLowering::writeVarArgRegs(std::vector<SDValue> &OutChains,
                                         SDValue Chain, const SDLoc &DL,
                                         SelectionDAG &DAG,
                                         CCState &State) const {
  ArrayRef<MCPhysReg> ArgRegs = ABI.GetVarArgRegs();
  unsigned Idx = State.getFirstUnallocated(ArgRegs);
  unsigned RegSizeInBytes = Subtarget.getGPRSizeInBytes();
  MVT RegTy = MVT::getIntegerVT(RegSizeInBytes * 8);
  const TargetRegisterClass *RC = getRegClassFor(RegTy);
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MipsFunctionInfo *MipsFI = MF.getInfo<MipsFunctionInfo>();

  // Offset of the first variable argument from stack pointer.
  int VaArgOffset;

  if (ABI.IsCheriPureCap()) {
    int FI = MFI.CreateFixedObject(Subtarget.getCapSizeInBytes(), 0, true);
    MipsFI->setVarArgsFrameIndex(FI);
    return;
  }

  if (ArgRegs.size() == Idx)
    VaArgOffset = alignTo(State.getNextStackOffset(), RegSizeInBytes);
  else {
    VaArgOffset =
        (int)ABI.GetCalleeAllocdArgSizeInBytes(State.getCallingConv()) -
        (int)(RegSizeInBytes * (ArgRegs.size() - Idx));
  }

  // Record the frame index of the first variable argument
  // which is a value necessary to VASTART.
  int FI = MFI.CreateFixedObject(RegSizeInBytes, VaArgOffset, true);
  MipsFI->setVarArgsFrameIndex(FI);

  // Copy the integer registers that have not been used for argument passing
  // to the argument register save area. For O32, the save area is allocated
  // in the caller's stack frame, while for N32/64, it is allocated in the
  // callee's stack frame.
  for (unsigned I = Idx; I < ArgRegs.size();
       ++I, VaArgOffset += RegSizeInBytes) {
    unsigned Reg = addLiveIn(MF, ArgRegs[I], RC);
    SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegTy);
    FI = MFI.CreateFixedObject(RegSizeInBytes, VaArgOffset, true);
    SDValue PtrOff = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout(),
                ABI.StackAddrSpace()));
    SDValue Store =
        DAG.getStore(Chain, DL, ArgValue, PtrOff, MachinePointerInfo());
    cast<StoreSDNode>(Store.getNode())->getMemOperand()->setValue(
        (Value *)nullptr);
    OutChains.push_back(Store);
  }
}

void MipsTargetLowering::HandleByVal(CCState *State, unsigned &Size,
                                     Align Alignment) const {
  const TargetFrameLowering *TFL = Subtarget.getFrameLowering();

  assert(Size && "Byval argument's size shouldn't be 0.");

  Alignment = std::min(Alignment, TFL->getStackAlign());

  unsigned FirstReg = 0;
  unsigned NumRegs = 0;

  if (State->getCallingConv() != CallingConv::Fast) {
    unsigned RegSizeInBytes = Subtarget.getGPRSizeInBytes();
    ArrayRef<MCPhysReg> IntArgRegs = ABI.GetByValArgRegs();
    // FIXME: The O32 case actually describes no shadow registers.
    const MCPhysReg *ShadowRegs =
        ABI.IsO32() ? IntArgRegs.data() : Mips64DPRegs;

    // We used to check the size as well but we can't do that anymore since
    // CCState::HandleByVal() rounds up the size after calling this function.
    assert(
        Alignment >= Align(RegSizeInBytes) &&
        "Byval argument's alignment should be a multiple of RegSizeInBytes.");

    FirstReg = State->getFirstUnallocated(IntArgRegs);

    // If Alignment > RegSizeInBytes, the first arg register must be even.
    // FIXME: This condition happens to do the right thing but it's not the
    //        right way to test it. We want to check that the stack frame offset
    //        of the register is aligned.
    if ((Alignment > RegSizeInBytes) && (FirstReg % 2)) {
      State->AllocateReg(IntArgRegs[FirstReg], ShadowRegs[FirstReg]);
      ++FirstReg;
    }

    // Mark the registers allocated.
    Size = alignTo(Size, RegSizeInBytes);
    for (unsigned I = FirstReg; Size > 0 && (I < IntArgRegs.size());
         Size -= RegSizeInBytes, ++I, ++NumRegs)
      State->AllocateReg(IntArgRegs[I], ShadowRegs[I]);
  }

  State->addInRegsParamInfo(FirstReg, FirstReg + NumRegs);
}

MachineBasicBlock *MipsTargetLowering::emitPseudoSELECT(MachineInstr &MI,
                                                        MachineBasicBlock *BB,
                                                        bool isFPCmp,
                                                        unsigned Opc) const {
  assert(!(Subtarget.hasMips4() || Subtarget.hasMips32()) &&
         "Subtarget already supports SELECT nodes with the use of"
         "conditional-move instructions.");

  const TargetInstrInfo *TII =
      Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  // To "insert" a SELECT instruction, we actually have to insert the
  // diamond control-flow pattern.  The incoming instruction knows the
  // destination vreg to set, the condition code register to branch on, the
  // true/false values to select between, and a branch opcode to use.
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator It = ++BB->getIterator();

  //  thisMBB:
  //  ...
  //   TrueVal = ...
  //   setcc r1, r2, r3
  //   bNE   r1, r0, copy1MBB
  //   fallthrough --> copy0MBB
  MachineBasicBlock *thisMBB  = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *sinkMBB  = F->CreateMachineBasicBlock(LLVM_BB);
  F->insert(It, copy0MBB);
  F->insert(It, sinkMBB);

  // Transfer the remainder of BB and its successor edges to sinkMBB.
  sinkMBB->splice(sinkMBB->begin(), BB,
                  std::next(MachineBasicBlock::iterator(MI)), BB->end());
  sinkMBB->transferSuccessorsAndUpdatePHIs(BB);

  // Next, add the true and fallthrough blocks as its successors.
  BB->addSuccessor(copy0MBB);
  BB->addSuccessor(sinkMBB);

  if (isFPCmp) {
    // bc1[tf] cc, sinkMBB
    BuildMI(BB, DL, TII->get(Opc))
        .addReg(MI.getOperand(1).getReg())
        .addMBB(sinkMBB);
  } else {
    // bne rs, $0, sinkMBB
    BuildMI(BB, DL, TII->get(Opc))
        .addReg(MI.getOperand(1).getReg())
        .addReg(Mips::ZERO)
        .addMBB(sinkMBB);
  }

  //  copy0MBB:
  //   %FalseValue = ...
  //   # fallthrough to sinkMBB
  BB = copy0MBB;

  // Update machine-CFG edges
  BB->addSuccessor(sinkMBB);

  //  sinkMBB:
  //   %Result = phi [ %TrueValue, thisMBB ], [ %FalseValue, copy0MBB ]
  //  ...
  BB = sinkMBB;

  BuildMI(*BB, BB->begin(), DL, TII->get(Mips::PHI), MI.getOperand(0).getReg())
      .addReg(MI.getOperand(2).getReg())
      .addMBB(thisMBB)
      .addReg(MI.getOperand(3).getReg())
      .addMBB(copy0MBB);

  MI.eraseFromParent(); // The pseudo instruction is gone now.

  return BB;
}

MachineBasicBlock *
MipsTargetLowering::emitPseudoD_SELECT(MachineInstr &MI,
                                       MachineBasicBlock *BB) const {
  assert(!(Subtarget.hasMips4() || Subtarget.hasMips32()) &&
         "Subtarget already supports SELECT nodes with the use of"
         "conditional-move instructions.");

  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  // D_SELECT substitutes two SELECT nodes that goes one after another and
  // have the same condition operand. On machines which don't have
  // conditional-move instruction, it reduces unnecessary branch instructions
  // which are result of using two diamond patterns that are result of two
  // SELECT pseudo instructions.
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator It = ++BB->getIterator();

  //  thisMBB:
  //  ...
  //   TrueVal = ...
  //   setcc r1, r2, r3
  //   bNE   r1, r0, copy1MBB
  //   fallthrough --> copy0MBB
  MachineBasicBlock *thisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *sinkMBB = F->CreateMachineBasicBlock(LLVM_BB);
  F->insert(It, copy0MBB);
  F->insert(It, sinkMBB);

  // Transfer the remainder of BB and its successor edges to sinkMBB.
  sinkMBB->splice(sinkMBB->begin(), BB,
                  std::next(MachineBasicBlock::iterator(MI)), BB->end());
  sinkMBB->transferSuccessorsAndUpdatePHIs(BB);

  // Next, add the true and fallthrough blocks as its successors.
  BB->addSuccessor(copy0MBB);
  BB->addSuccessor(sinkMBB);

  // bne rs, $0, sinkMBB
  BuildMI(BB, DL, TII->get(Mips::BNE))
      .addReg(MI.getOperand(2).getReg())
      .addReg(Mips::ZERO)
      .addMBB(sinkMBB);

  //  copy0MBB:
  //   %FalseValue = ...
  //   # fallthrough to sinkMBB
  BB = copy0MBB;

  // Update machine-CFG edges
  BB->addSuccessor(sinkMBB);

  //  sinkMBB:
  //   %Result = phi [ %TrueValue, thisMBB ], [ %FalseValue, copy0MBB ]
  //  ...
  BB = sinkMBB;

  // Use two PHI nodes to select two reults
  BuildMI(*BB, BB->begin(), DL, TII->get(Mips::PHI), MI.getOperand(0).getReg())
      .addReg(MI.getOperand(3).getReg())
      .addMBB(thisMBB)
      .addReg(MI.getOperand(5).getReg())
      .addMBB(copy0MBB);
  BuildMI(*BB, BB->begin(), DL, TII->get(Mips::PHI), MI.getOperand(1).getReg())
      .addReg(MI.getOperand(4).getReg())
      .addMBB(thisMBB)
      .addReg(MI.getOperand(6).getReg())
      .addMBB(copy0MBB);

  MI.eraseFromParent(); // The pseudo instruction is gone now.

  return BB;
}

// FIXME? Maybe this could be a TableGen attribute on some registers and
// this table could be generated automatically from RegInfo.
Register
MipsTargetLowering::getRegisterByName(const char *RegName, LLT VT,
                                      const MachineFunction &MF) const {
  // Named registers is expected to be fairly rare. For now, just support $28
  // since the linux kernel uses it.
  if (Subtarget.isGP64bit()) {
    Register Reg = StringSwitch<Register>(RegName)
                         .Case("$28", Mips::GP_64)
                         .Default(Register());
    if (Reg)
      return Reg;
  } else {
    Register Reg = StringSwitch<Register>(RegName)
                         .Case("$28", Mips::GP)
                         .Default(Register());
    if (Reg)
      return Reg;
  }
  report_fatal_error("Invalid register name global variable");
}

MachineBasicBlock *MipsTargetLowering::emitLDR_W(MachineInstr &MI,
                                                 MachineBasicBlock *BB) const {
  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  const bool IsLittle = Subtarget.isLittle();
  DebugLoc DL = MI.getDebugLoc();

  Register Dest = MI.getOperand(0).getReg();
  Register Address = MI.getOperand(1).getReg();
  unsigned Imm = MI.getOperand(2).getImm();

  MachineBasicBlock::iterator I(MI);

  if (Subtarget.hasMips32r6() || Subtarget.hasMips64r6()) {
    // Mips release 6 can load from adress that is not naturally-aligned.
    Register Temp = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    BuildMI(*BB, I, DL, TII->get(Mips::LW))
        .addDef(Temp)
        .addUse(Address)
        .addImm(Imm);
    BuildMI(*BB, I, DL, TII->get(Mips::FILL_W)).addDef(Dest).addUse(Temp);
  } else {
    // Mips release 5 needs to use instructions that can load from an unaligned
    // memory address.
    Register LoadHalf = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    Register LoadFull = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    Register Undef = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    BuildMI(*BB, I, DL, TII->get(Mips::IMPLICIT_DEF)).addDef(Undef);
    BuildMI(*BB, I, DL, TII->get(Mips::LWR))
        .addDef(LoadHalf)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 0 : 3))
        .addUse(Undef);
    BuildMI(*BB, I, DL, TII->get(Mips::LWL))
        .addDef(LoadFull)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 3 : 0))
        .addUse(LoadHalf);
    BuildMI(*BB, I, DL, TII->get(Mips::FILL_W)).addDef(Dest).addUse(LoadFull);
  }

  MI.eraseFromParent();
  return BB;
}

MachineBasicBlock *MipsTargetLowering::emitLDR_D(MachineInstr &MI,
                                                 MachineBasicBlock *BB) const {
  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  const bool IsLittle = Subtarget.isLittle();
  DebugLoc DL = MI.getDebugLoc();

  Register Dest = MI.getOperand(0).getReg();
  Register Address = MI.getOperand(1).getReg();
  unsigned Imm = MI.getOperand(2).getImm();

  MachineBasicBlock::iterator I(MI);

  if (Subtarget.hasMips32r6() || Subtarget.hasMips64r6()) {
    // Mips release 6 can load from adress that is not naturally-aligned.
    if (Subtarget.isGP64bit()) {
      Register Temp = MRI.createVirtualRegister(&Mips::GPR64RegClass);
      BuildMI(*BB, I, DL, TII->get(Mips::LD))
          .addDef(Temp)
          .addUse(Address)
          .addImm(Imm);
      BuildMI(*BB, I, DL, TII->get(Mips::FILL_D)).addDef(Dest).addUse(Temp);
    } else {
      Register Wtemp = MRI.createVirtualRegister(&Mips::MSA128WRegClass);
      Register Lo = MRI.createVirtualRegister(&Mips::GPR32RegClass);
      Register Hi = MRI.createVirtualRegister(&Mips::GPR32RegClass);
      BuildMI(*BB, I, DL, TII->get(Mips::LW))
          .addDef(Lo)
          .addUse(Address)
          .addImm(Imm + (IsLittle ? 0 : 4));
      BuildMI(*BB, I, DL, TII->get(Mips::LW))
          .addDef(Hi)
          .addUse(Address)
          .addImm(Imm + (IsLittle ? 4 : 0));
      BuildMI(*BB, I, DL, TII->get(Mips::FILL_W)).addDef(Wtemp).addUse(Lo);
      BuildMI(*BB, I, DL, TII->get(Mips::INSERT_W), Dest)
          .addUse(Wtemp)
          .addUse(Hi)
          .addImm(1);
    }
  } else {
    // Mips release 5 needs to use instructions that can load from an unaligned
    // memory address.
    Register LoHalf = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    Register LoFull = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    Register LoUndef = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    Register HiHalf = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    Register HiFull = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    Register HiUndef = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    Register Wtemp = MRI.createVirtualRegister(&Mips::MSA128WRegClass);
    BuildMI(*BB, I, DL, TII->get(Mips::IMPLICIT_DEF)).addDef(LoUndef);
    BuildMI(*BB, I, DL, TII->get(Mips::LWR))
        .addDef(LoHalf)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 0 : 7))
        .addUse(LoUndef);
    BuildMI(*BB, I, DL, TII->get(Mips::LWL))
        .addDef(LoFull)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 3 : 4))
        .addUse(LoHalf);
    BuildMI(*BB, I, DL, TII->get(Mips::IMPLICIT_DEF)).addDef(HiUndef);
    BuildMI(*BB, I, DL, TII->get(Mips::LWR))
        .addDef(HiHalf)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 4 : 3))
        .addUse(HiUndef);
    BuildMI(*BB, I, DL, TII->get(Mips::LWL))
        .addDef(HiFull)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 7 : 0))
        .addUse(HiHalf);
    BuildMI(*BB, I, DL, TII->get(Mips::FILL_W)).addDef(Wtemp).addUse(LoFull);
    BuildMI(*BB, I, DL, TII->get(Mips::INSERT_W), Dest)
        .addUse(Wtemp)
        .addUse(HiFull)
        .addImm(1);
  }

  MI.eraseFromParent();
  return BB;
}

MachineBasicBlock *MipsTargetLowering::emitSTR_W(MachineInstr &MI,
                                                 MachineBasicBlock *BB) const {
  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  const bool IsLittle = Subtarget.isLittle();
  DebugLoc DL = MI.getDebugLoc();

  Register StoreVal = MI.getOperand(0).getReg();
  Register Address = MI.getOperand(1).getReg();
  unsigned Imm = MI.getOperand(2).getImm();

  MachineBasicBlock::iterator I(MI);

  if (Subtarget.hasMips32r6() || Subtarget.hasMips64r6()) {
    // Mips release 6 can store to adress that is not naturally-aligned.
    Register BitcastW = MRI.createVirtualRegister(&Mips::MSA128WRegClass);
    Register Tmp = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    BuildMI(*BB, I, DL, TII->get(Mips::COPY)).addDef(BitcastW).addUse(StoreVal);
    BuildMI(*BB, I, DL, TII->get(Mips::COPY_S_W))
        .addDef(Tmp)
        .addUse(BitcastW)
        .addImm(0);
    BuildMI(*BB, I, DL, TII->get(Mips::SW))
        .addUse(Tmp)
        .addUse(Address)
        .addImm(Imm);
  } else {
    // Mips release 5 needs to use instructions that can store to an unaligned
    // memory address.
    Register Tmp = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    BuildMI(*BB, I, DL, TII->get(Mips::COPY_S_W))
        .addDef(Tmp)
        .addUse(StoreVal)
        .addImm(0);
    BuildMI(*BB, I, DL, TII->get(Mips::SWR))
        .addUse(Tmp)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 0 : 3));
    BuildMI(*BB, I, DL, TII->get(Mips::SWL))
        .addUse(Tmp)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 3 : 0));
  }

  MI.eraseFromParent();

  return BB;
}

MachineBasicBlock *MipsTargetLowering::emitSTR_D(MachineInstr &MI,
                                                 MachineBasicBlock *BB) const {
  MachineFunction *MF = BB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  const TargetInstrInfo *TII = Subtarget.getInstrInfo();
  const bool IsLittle = Subtarget.isLittle();
  DebugLoc DL = MI.getDebugLoc();

  Register StoreVal = MI.getOperand(0).getReg();
  Register Address = MI.getOperand(1).getReg();
  unsigned Imm = MI.getOperand(2).getImm();

  MachineBasicBlock::iterator I(MI);

  if (Subtarget.hasMips32r6() || Subtarget.hasMips64r6()) {
    // Mips release 6 can store to adress that is not naturally-aligned.
    if (Subtarget.isGP64bit()) {
      Register BitcastD = MRI.createVirtualRegister(&Mips::MSA128DRegClass);
      Register Lo = MRI.createVirtualRegister(&Mips::GPR64RegClass);
      BuildMI(*BB, I, DL, TII->get(Mips::COPY))
          .addDef(BitcastD)
          .addUse(StoreVal);
      BuildMI(*BB, I, DL, TII->get(Mips::COPY_S_D))
          .addDef(Lo)
          .addUse(BitcastD)
          .addImm(0);
      BuildMI(*BB, I, DL, TII->get(Mips::SD))
          .addUse(Lo)
          .addUse(Address)
          .addImm(Imm);
    } else {
      Register BitcastW = MRI.createVirtualRegister(&Mips::MSA128WRegClass);
      Register Lo = MRI.createVirtualRegister(&Mips::GPR32RegClass);
      Register Hi = MRI.createVirtualRegister(&Mips::GPR32RegClass);
      BuildMI(*BB, I, DL, TII->get(Mips::COPY))
          .addDef(BitcastW)
          .addUse(StoreVal);
      BuildMI(*BB, I, DL, TII->get(Mips::COPY_S_W))
          .addDef(Lo)
          .addUse(BitcastW)
          .addImm(0);
      BuildMI(*BB, I, DL, TII->get(Mips::COPY_S_W))
          .addDef(Hi)
          .addUse(BitcastW)
          .addImm(1);
      BuildMI(*BB, I, DL, TII->get(Mips::SW))
          .addUse(Lo)
          .addUse(Address)
          .addImm(Imm + (IsLittle ? 0 : 4));
      BuildMI(*BB, I, DL, TII->get(Mips::SW))
          .addUse(Hi)
          .addUse(Address)
          .addImm(Imm + (IsLittle ? 4 : 0));
    }
  } else {
    // Mips release 5 needs to use instructions that can store to an unaligned
    // memory address.
    Register Bitcast = MRI.createVirtualRegister(&Mips::MSA128WRegClass);
    Register Lo = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    Register Hi = MRI.createVirtualRegister(&Mips::GPR32RegClass);
    BuildMI(*BB, I, DL, TII->get(Mips::COPY)).addDef(Bitcast).addUse(StoreVal);
    BuildMI(*BB, I, DL, TII->get(Mips::COPY_S_W))
        .addDef(Lo)
        .addUse(Bitcast)
        .addImm(0);
    BuildMI(*BB, I, DL, TII->get(Mips::COPY_S_W))
        .addDef(Hi)
        .addUse(Bitcast)
        .addImm(1);
    BuildMI(*BB, I, DL, TII->get(Mips::SWR))
        .addUse(Lo)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 0 : 3));
    BuildMI(*BB, I, DL, TII->get(Mips::SWL))
        .addUse(Lo)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 3 : 0));
    BuildMI(*BB, I, DL, TII->get(Mips::SWR))
        .addUse(Hi)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 4 : 7));
    BuildMI(*BB, I, DL, TII->get(Mips::SWL))
        .addUse(Hi)
        .addUse(Address)
        .addImm(Imm + (IsLittle ? 7 : 4));
  }

  MI.eraseFromParent();
  return BB;
}
