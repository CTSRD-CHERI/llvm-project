//===-- RISCVSelectionDAGInfo.cpp - RISCV SelectionDAG Info ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RISCVSelectionDAGInfo.h"
#include "RISCVSubtarget.h"
#include "MCTargetDesc/RISCVBaseInfo.h"

using namespace llvm;

namespace {

/// Helper function to get the RISCV subclass of the subtarget
const RISCVSubtarget &getRISCVSubtarget(SelectionDAG &DAG) {
  return reinterpret_cast<const RISCVSubtarget&>(DAG.getSubtarget());
}

/// Helper function that generates a DAG node for calling a memory function.
SDValue callFunction(SelectionDAG &DAG, SDLoc dl, SDValue Chain, const char
    *fnName, SDValue Dst, SDValue Src, SDValue Size) {
  auto &Ctx = *DAG.getContext();
  auto &STI = getRISCVSubtarget(DAG);
  TargetLowering::ArgListTy Args;
  auto pushArg = [&](SDValue &Op) {
    TargetLowering::ArgListEntry Entry;
    Entry.Node = Op;
    Entry.Ty = Op.getValueType().getTypeForEVT(Ctx);
    Args.push_back(Entry);
  };
  pushArg(Dst);
  pushArg(Src);
  pushArg(Size);

  SDValue memOpFn = DAG.getExternalFunctionSymbol(fnName);

  TargetLowering::CallLoweringInfo CLI(DAG);
  CLI.setDebugLoc(dl)
      .setChain(Chain)
      .setLibCallee(CallingConv::C,
                    Dst.getValueType().getTypeForEVT(Ctx),
                    memOpFn,
                    std::move(Args))
      .setDiscardResult();

  const RISCVTargetLowering *TLI = STI.getTargetLowering();
  std::pair<SDValue,SDValue> CallResult = TLI->LowerCallTo(CLI);
  return CallResult.second;
}

/// Helper that emits the memcpy / memmove call, as required.
SDValue EmitTargetCodeForMemOp(SelectionDAG &DAG, const SDLoc &dl,
                               SDValue Chain, SDValue Dst, SDValue Src,
                               SDValue Size, Align Alignment, bool isVolatile,
                               bool AlwaysInline,
                               PreserveCheriTags PreserveTags,
                               MachinePointerInfo DstPtrInfo,
                               MachinePointerInfo SrcPtrInfo, bool isMemCpy) {
  // If AlwaysInline is set, let SelectionDAG expand this.
  if (AlwaysInline) {
    return SDValue();
  }
  // If we're copying AS0 to AS0, do the normal thing.
  unsigned DstAS = DstPtrInfo.getAddrSpace();
  unsigned SrcAS = SrcPtrInfo.getAddrSpace();
  if ((DstAS == 0) && (SrcAS == 0))
    return SDValue();
  auto &STI = getRISCVSubtarget(DAG);
  // If either argument is AS0, make it a capability.
  MVT CapType = STI.typeForCapabilities();
  if (DstAS == 0)
    Dst = DAG.getAddrSpaceCast(dl, CapType, Dst, 0, 200);
  if (SrcAS == 0)
    Src = DAG.getAddrSpaceCast(dl, CapType, Src, 0, 200);

  const char *memFnName = isMemCpy ?
    (RISCVABI::isCheriPureCapABI(STI.getTargetABI()) ?  "memcpy" : "memcpy_c") :
    (RISCVABI::isCheriPureCapABI(STI.getTargetABI()) ?  "memmove" : "memmove_c");
  return callFunction(DAG, dl, Chain, memFnName, Dst, Src, Size);
}
}

RISCVSelectionDAGInfo::~RISCVSelectionDAGInfo() {
}

SDValue RISCVSelectionDAGInfo::EmitTargetCodeForMemcpy(
    SelectionDAG &DAG, const SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src,
    SDValue Size, Align Alignment, bool isVolatile, bool AlwaysInline,
    PreserveCheriTags PreserveTags, MachinePointerInfo DstPtrInfo,
    MachinePointerInfo SrcPtrInfo) const {
  return EmitTargetCodeForMemOp(DAG, dl, Chain, Dst, Src, Size, Alignment,
                                isVolatile, AlwaysInline, PreserveTags,
                                DstPtrInfo, SrcPtrInfo, true);
}

SDValue RISCVSelectionDAGInfo::EmitTargetCodeForMemmove(
    SelectionDAG &DAG, const SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src,
    SDValue Size, Align Alignment, bool isVolatile,
    PreserveCheriTags PreserveTags, MachinePointerInfo DstPtrInfo,
    MachinePointerInfo SrcPtrInfo) const {
  return EmitTargetCodeForMemOp(DAG, dl, Chain, Dst, Src, Size, Alignment,
                                isVolatile, false, PreserveTags, DstPtrInfo,
                                SrcPtrInfo, false);
}

SDValue RISCVSelectionDAGInfo::EmitTargetCodeForMemset(
    SelectionDAG &DAG, const SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src,
    SDValue Size, Align Alignment, bool isVolatile,
    MachinePointerInfo DstPtrInfo) const {
  // If we're setting via an AS0 pointer, do the normal thing.
  unsigned DstAS = DstPtrInfo.getAddrSpace();
  if (DstAS == 0)
    return SDValue();

  auto &STI = getRISCVSubtarget(DAG);
  MVT CapType = STI.typeForCapabilities();
  unsigned CLen = CapType.getSizeInBits();
  unsigned CLenInBytes = CLen / 8;
  // If this is capability aligned, but not a multiple of capability size, we
  // might have given up too early trying to emit capability instructions.
  if (Alignment >= CLenInBytes) {
    if (auto ConstantSize = dyn_cast<ConstantSDNode>(Size)) {
      uint64_t SizeVal = ConstantSize->getZExtValue();
      // If this size is a small constant, and the value we're writing is zero,
      // then let's emit some stores instead.
      if (SizeVal < (CLenInBytes * 8))
        if (isa<ConstantSDNode>(Src) &&
            cast<ConstantSDNode>(Src)->isNullValue()) {
          SmallVector<SDValue, 8> OutChains;
          SDValue ZeroCap = DAG.getNullCapability(dl);
          for (uint64_t i = 0; i < (SizeVal / CLenInBytes); i++) {
            uint64_t DstOff = i * CLenInBytes;
            SDValue Store = DAG.getStore(
                Chain, dl, ZeroCap,
                DAG.getMemBasePlusOffset(Dst, TypeSize::Fixed(DstOff), dl),
                DstPtrInfo.getWithOffset(DstOff), Alignment,
                isVolatile ? MachineMemOperand::MOVolatile
                           : MachineMemOperand::MONone);
            OutChains.push_back(Store);
          }
          MVT XLenVT = STI.getXLenVT();
          unsigned XLenInBytes = STI.getXLen() / 8;
          unsigned Remainder = SizeVal % CLenInBytes;
          SDValue ZeroXLen = DAG.getConstant(0, dl, XLenVT);
          uint64_t Done = (SizeVal / CLenInBytes) * CLenInBytes;
          // Write zero or one XLen words.
          while (Remainder >= XLenInBytes) {
            SDValue Store = DAG.getStore(
                Chain, dl, ZeroXLen,
                DAG.getMemBasePlusOffset(Dst, TypeSize::Fixed(Done), dl),
                DstPtrInfo.getWithOffset(Done), Alignment,
                isVolatile ? MachineMemOperand::MOVolatile
                           : MachineMemOperand::MONone);
            OutChains.push_back(Store);
            Done += XLenInBytes;
            Remainder -= XLenInBytes;
          }
          // We can always do the remaining 1 to (XLen-1) bytes in at most two
          // instructions, either two adjacent stores or an unaligned
          // overlapping store.
          // We prefer the two-store version, because it reduces dependencies
          // between instructions.
          while (Remainder > 0) {
            SDValue Zero;
            uint64_t DstOff;
            switch (Remainder) {
            default:
              llvm_unreachable("Remainder must be < 8");
            case 7:
            case 6:
            case 5:
            case 4:
              assert(XLenInBytes >= 8);
              Zero = DAG.getConstant(0, dl, MVT::i32);
              ;
              DstOff = SizeVal - Remainder;
              Remainder -= 4;
              break;
            case 3:
            case 2:
              Zero = DAG.getConstant(0, dl, MVT::i16);
              ;
              DstOff = SizeVal - Remainder;
              Remainder -= 2;
              break;
            case 1:
              Zero = DAG.getConstant(0, dl, MVT::i8);
              ;
              DstOff = SizeVal - Remainder;
              Remainder -= 1;
              break;
            }
            SDValue Store = DAG.getStore(
                Chain, dl, Zero,
                DAG.getMemBasePlusOffset(Dst, TypeSize::Fixed(DstOff), dl),
                DstPtrInfo.getWithOffset(DstOff), Alignment,
                isVolatile ? MachineMemOperand::MOVolatile
                           : MachineMemOperand::MONone);
            OutChains.push_back(Store);
          }
          return DAG.getNode(ISD::TokenFactor, dl, MVT::Other, OutChains);
        }
    }
  }

  const char *memFnName =
      RISCVABI::isCheriPureCapABI(STI.getTargetABI()) ? "memset" : "memset_c";
  return callFunction(DAG, dl, Chain, memFnName, Dst, Src, Size);
}
