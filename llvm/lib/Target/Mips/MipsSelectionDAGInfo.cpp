//===-- MipsSelectionDAGInfo.cpp - Mips SelectionDAG Info -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the MipsSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#include "MipsTargetMachine.h"
#include "MipsSubtarget.h"
using namespace llvm;

#define DEBUG_TYPE "mips-selectiondag-info"

namespace {

/// Helper function to get the MIPS subclass of the subtarget
const MipsSubtarget &getMipsSubtarget(SelectionDAG &DAG) {
  return reinterpret_cast<const MipsSubtarget&>(DAG.getSubtarget());
}

/// Helper function that generates a DAG node for calling a memory function.
SDValue callFunction(SelectionDAG &DAG, SDLoc dl, SDValue Chain, const char
    *fnName, SDValue Dst, SDValue Src, SDValue Size) {
  auto &Ctx = *DAG.getContext();
  auto &STI = getMipsSubtarget(DAG);
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

  SDValue memOpFn = DAG.getExternalSymbol(fnName, STI.isGP64bit() ?
      MVT::i64 : MVT::i32);

  TargetLowering::CallLoweringInfo CLI(DAG);
  CLI.setDebugLoc(dl)
      .setChain(Chain)
      .setLibCallee(CallingConv::C,
                    Dst.getValueType().getTypeForEVT(Ctx),
                    memOpFn,
                    std::move(Args))
      .setDiscardResult();

  const MipsTargetLowering *TLI = STI.getTargetLowering();
  std::pair<SDValue,SDValue> CallResult = TLI->LowerCallTo(CLI);
  return CallResult.second;
}

/// Helper that emits the memcpy / memmove call, as required.
SDValue EmitTargetCodeForMemOp(SelectionDAG &DAG, const
    SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src, SDValue Size, unsigned
    Align, bool isVolatile, bool AlwaysInline, MachinePointerInfo DstPtrInfo,
    MachinePointerInfo SrcPtrInfo, bool isMemCpy) {
  // If AlwaysInline is set, let SelectionDAG expand this.
  if (AlwaysInline)
    return SDValue();
  // If we're copying AS0 to AS0, do the normal thing.
  unsigned DstAS = DstPtrInfo.getAddrSpace();
  unsigned SrcAS = SrcPtrInfo.getAddrSpace();
  if ((DstAS == 0) && (SrcAS == 0))
    return SDValue();
  auto &STI = getMipsSubtarget(DAG);
  // If either argument is AS0, make it a capability.
  MVT CapType = STI.typeForCapabilities();
  if (DstAS == 0)
    Dst = DAG.getAddrSpaceCast(dl, CapType, Dst, 0, 200);
  if (SrcAS == 0)
    Src = DAG.getAddrSpaceCast(dl, CapType, Src, 0, 200);

  
  const char *memFnName = isMemCpy ?
    (STI.isABI_CheriPureCap() ?  "memcpy" : "memcpy_c") :
    (STI.isABI_CheriPureCap() ?  "memmove" : "memmove_c");
  return callFunction(DAG, dl, Chain, memFnName, Dst, Src, Size);
}

}

MipsSelectionDAGInfo::~MipsSelectionDAGInfo() {
}

SDValue MipsSelectionDAGInfo::EmitTargetCodeForMemcpy(SelectionDAG &DAG, const
    SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src, SDValue Size, unsigned
    Align, bool isVolatile, bool AlwaysInline, MachinePointerInfo DstPtrInfo,
    MachinePointerInfo SrcPtrInfo) const {
  return EmitTargetCodeForMemOp(DAG, dl, Chain, Dst, Src, Size, Align,
      isVolatile, AlwaysInline, DstPtrInfo, SrcPtrInfo, true);
}

SDValue MipsSelectionDAGInfo::EmitTargetCodeForMemmove( SelectionDAG &DAG,
    const SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src, SDValue Size,
    unsigned Align, bool isVolatile, MachinePointerInfo DstPtrInfo,
    MachinePointerInfo SrcPtrInfo) const {
  return EmitTargetCodeForMemOp(DAG, dl, Chain, Dst, Src, Size, Align,
      isVolatile, false, DstPtrInfo, SrcPtrInfo, false);
}

SDValue
MipsSelectionDAGInfo::EmitTargetCodeForMemset(SelectionDAG &DAG, const SDLoc &dl,
                                          SDValue Chain, SDValue Dst,
                                          SDValue Src, SDValue Size,
                                          unsigned Align, bool isVolatile,
                                          MachinePointerInfo DstPtrInfo) const {
  // If we're setting via an AS0 pointer, do the normal thing.
  unsigned DstAS = DstPtrInfo.getAddrSpace();
  if (DstAS == 0)
    return SDValue();

  auto &STI = getMipsSubtarget(DAG);
  unsigned CapSize = STI.getCapSizeInBytes();
  // If this is capability aligned, but not a multiple of capability size, we
  // might have given up too early trying to emit capability instructions.
  if (Align >= CapSize) {
    if (auto ConstantSize = dyn_cast<ConstantSDNode>(Size)) {
      uint64_t SizeVal = ConstantSize->getZExtValue();
      // If this size is a small constant, and the value we're writing is zero,
      // then let's emit some stores instead.
      if ((SizeVal < (CapSize * 8)))
        if (isa<ConstantSDNode>(Src) && cast<ConstantSDNode>(Src)->isNullValue()) {
          MVT CapType = STI.typeForCapabilities();
          SmallVector<SDValue, 8> OutChains;
          SDValue ZeroCap = DAG.getConstant(0, dl, CapType);
          for (uint64_t i=0 ; i<(SizeVal / CapSize) ; i++) {
            uint64_t DstOff = i*CapSize;
            SDValue Store = DAG.getStore(Chain, dl, ZeroCap,
                DAG.getMemBasePlusOffset(Dst, DstOff, dl),
                DstPtrInfo.getWithOffset(DstOff), Align, isVolatile ?
                MachineMemOperand::MOVolatile : MachineMemOperand::MONone);
            OutChains.push_back(Store);
          }
          uint64_t Remainder = SizeVal % CapSize;
          SDValue Zero64 = DAG.getConstant(0, dl, MVT::i64);
          uint64_t Done = (SizeVal / CapSize) * CapSize;
          // Write zero, one or two doubles.
          while (Remainder >= 8) {
            SDValue Store = DAG.getStore(Chain, dl, Zero64,
                DAG.getMemBasePlusOffset(Dst, Done, dl),
                DstPtrInfo.getWithOffset(Done), Align, isVolatile ?
                MachineMemOperand::MOVolatile : MachineMemOperand::MONone);
            OutChains.push_back(Store);
            Done += 8;
            Remainder -= 8;
          }
          // We can always do the remaining 1-7 bytes in at most two
          // instructions, either two adjacent stores or an unaligned
          // overlapping store (which will need a separate CIncOffset to
          // calculate the address, because CHERI offsets are scaled).
          // We prefer the two-store version, because it reduces dependencies
          // between instructions.
          while (Remainder > 0) {
            SDValue Zero = Zero64;
            uint64_t DstOff;
            switch (Remainder) {
              default: llvm_unreachable("Remainder must be < 8");
              case 7:
                Zero = Zero64;
                DstOff = SizeVal - 8;
                Remainder -= 7;
                break;
              case 6: case 5: case 4:
                Zero = DAG.getConstant(0, dl, MVT::i32);;
                DstOff = SizeVal - Remainder;
                Remainder -= 4;
                break;
              case 3: case 2:
                Zero = DAG.getConstant(0, dl, MVT::i16);;
                DstOff = SizeVal - Remainder;
                Remainder -= 2;
                break;
              case 1:
                Zero = DAG.getConstant(0, dl, MVT::i8);;
                DstOff = SizeVal - Remainder;
                Remainder -= 1;
                break;
            }
            SDValue Store = DAG.getStore(Chain, dl, Zero,
                DAG.getMemBasePlusOffset(Dst, DstOff, dl),
                DstPtrInfo.getWithOffset(DstOff), Align, isVolatile ?
                MachineMemOperand::MOVolatile : MachineMemOperand::MONone);
            OutChains.push_back(Store);
          }
          return DAG.getNode(ISD::TokenFactor, dl, MVT::Other, OutChains);
        }
    }
  }


  const char *memFnName = 
    STI.isABI_CheriPureCap() ?  "memset" : "memset_c";
  return callFunction(DAG, dl, Chain, memFnName, Dst, Src, Size);
}

