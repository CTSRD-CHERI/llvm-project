//===-- MipsSelectionDAGInfo.h - Mips SelectionDAG Info ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Mips subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPSSELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_MIPS_MIPSSELECTIONDAGINFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

namespace llvm {

class MipsTargetMachine;

class MipsSelectionDAGInfo : public SelectionDAGTargetInfo {
public:
  virtual ~MipsSelectionDAGInfo();
  SDValue EmitTargetCodeForMemcpy(SelectionDAG &DAG, const SDLoc &dl,
                                SDValue Chain, SDValue Op1,
                                SDValue Op2, SDValue Op3,
                                unsigned Align, bool isVolatile,
                                bool AlwaysInline,
                                MachinePointerInfo DstPtrInfo,
                                MachinePointerInfo SrcPtrInfo) const override;

  SDValue EmitTargetCodeForMemmove( SelectionDAG &DAG, const SDLoc &dl, SDValue
      Chain, SDValue Op1, SDValue Op2, SDValue Op3, unsigned Align, bool
      isVolatile, MachinePointerInfo DstPtrInfo, MachinePointerInfo SrcPtrInfo)
    const override;

  SDValue EmitTargetCodeForMemset(SelectionDAG &DAG, const SDLoc &dl,
                                  SDValue Chain, SDValue Op1,
                                  SDValue Op2, SDValue Op3,
                                  unsigned Align, bool isVolatile,
                                  MachinePointerInfo DstPtrInfo) const override;


};

}

#endif
