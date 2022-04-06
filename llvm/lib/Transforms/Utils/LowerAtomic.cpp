//===- LowerAtomic.cpp - Lower atomic intrinsics --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass lowers atomic intrinsics to non-atomic form for use in a known
// non-preemptible environment.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LowerAtomic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
using namespace llvm;

#define DEBUG_TYPE "loweratomic"

bool llvm::lowerAtomicCmpXchgInst(AtomicCmpXchgInst *CXI) {
  IRBuilder<> Builder(CXI);
  Value *Ptr = CXI->getPointerOperand();
  Value *Cmp = CXI->getCompareOperand();
  Value *Val = CXI->getNewValOperand();

  LoadInst *Orig = Builder.CreateLoad(Val->getType(), Ptr);
  Value *Equal = Builder.CreateICmpEQ(Orig, Cmp);
  Value *Res = Builder.CreateSelect(Equal, Val, Orig);
  Builder.CreateStore(Res, Ptr);

  Res = Builder.CreateInsertValue(UndefValue::get(CXI->getType()), Orig, 0);
  Res = Builder.CreateInsertValue(Res, Equal, 1);

  CXI->replaceAllUsesWith(Res);
  CXI->eraseFromParent();
  return true;
}

Value *llvm::buildAtomicRMWValue(AtomicRMWInst::BinOp Op,
                                 IRBuilderBase &Builder, Value *Loaded,
                                 Value *Inc) {
  Module *M = Builder.GetInsertBlock()->getModule();
  const DataLayout &DL = M->getDataLayout();
  if (DL.isFatPointer(Loaded->getType())) {
    switch (Op) {
    case AtomicRMWInst::Xchg:
      break;
    case AtomicRMWInst::Add:
    case AtomicRMWInst::Sub:
    case AtomicRMWInst::And:
    case AtomicRMWInst::Nand:
    case AtomicRMWInst::Or:
    case AtomicRMWInst::Xor: {
      LLVMContext &C = M->getContext();
      unsigned AS = Loaded->getType()->getPointerAddressSpace();
      Type *I8CapTy = Type::getInt8PtrTy(C, AS);
      Type *SizeTy = Type::getIntNTy(C, DL.getIndexSizeInBits(AS));
      Function *GetAddress = Intrinsic::getDeclaration(
          M, Intrinsic::cheri_cap_address_get, SizeTy);
      Function *SetAddress = Intrinsic::getDeclaration(
          M, Intrinsic::cheri_cap_address_set, SizeTy);
      Value *LoadedI8Cap = Builder.CreateBitCast(Loaded, I8CapTy);
      Value *LoadedInt = Builder.CreateCall(GetAddress, {LoadedI8Cap});
      Value *IncI8Cap = Builder.CreateBitCast(Inc, I8CapTy);
      Value *IncInt = Builder.CreateCall(GetAddress, {IncI8Cap});
      Value *NewInt = buildAtomicRMWValue(Op, Builder, LoadedInt, IncInt);
      Value *NewI8Cap = Builder.CreateCall(SetAddress, {LoadedI8Cap, NewInt});
      return Builder.CreateBitCast(NewI8Cap, Loaded->getType());
    }
    case AtomicRMWInst::Max:
    case AtomicRMWInst::Min:
    case AtomicRMWInst::UMax:
    case AtomicRMWInst::UMin:
      break;
    case AtomicRMWInst::FAdd:
    case AtomicRMWInst::FSub:
      llvm_unreachable("Cannot use FP atomic op on capability value");
    default:
      llvm_unreachable("Unknown atomic op");
    }
  }

  Value *NewVal;
  switch (Op) {
  case AtomicRMWInst::Xchg:
    return Inc;
  case AtomicRMWInst::Add:
    return Builder.CreateAdd(Loaded, Inc, "new");
  case AtomicRMWInst::Sub:
    return Builder.CreateSub(Loaded, Inc, "new");
  case AtomicRMWInst::And:
    return Builder.CreateAnd(Loaded, Inc, "new");
  case AtomicRMWInst::Nand:
    return Builder.CreateNot(Builder.CreateAnd(Loaded, Inc), "new");
  case AtomicRMWInst::Or:
    return Builder.CreateOr(Loaded, Inc, "new");
  case AtomicRMWInst::Xor:
    return Builder.CreateXor(Loaded, Inc, "new");
  case AtomicRMWInst::Max:
    NewVal = Builder.CreateICmpSGT(Loaded, Inc);
    return Builder.CreateSelect(NewVal, Loaded, Inc, "new");
  case AtomicRMWInst::Min:
    NewVal = Builder.CreateICmpSLE(Loaded, Inc);
    return Builder.CreateSelect(NewVal, Loaded, Inc, "new");
  case AtomicRMWInst::UMax:
    NewVal = Builder.CreateICmpUGT(Loaded, Inc);
    return Builder.CreateSelect(NewVal, Loaded, Inc, "new");
  case AtomicRMWInst::UMin:
    NewVal = Builder.CreateICmpULE(Loaded, Inc);
    return Builder.CreateSelect(NewVal, Loaded, Inc, "new");
  case AtomicRMWInst::FAdd:
    return Builder.CreateFAdd(Loaded, Inc, "new");
  case AtomicRMWInst::FSub:
    return Builder.CreateFSub(Loaded, Inc, "new");
  default:
    llvm_unreachable("Unknown atomic op");
  }
}

bool llvm::lowerAtomicRMWInst(AtomicRMWInst *RMWI) {
  IRBuilder<> Builder(RMWI);
  Value *Ptr = RMWI->getPointerOperand();
  Value *Val = RMWI->getValOperand();

  LoadInst *Orig = Builder.CreateLoad(Val->getType(), Ptr);
  Value *Res = buildAtomicRMWValue(RMWI->getOperation(), Builder, Orig, Val);
  Builder.CreateStore(Res, Ptr);
  RMWI->replaceAllUsesWith(Orig);
  RMWI->eraseFromParent();
  return true;
}
