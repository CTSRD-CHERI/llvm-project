//===------- CommonCheriTargetCodeGenInfo.h - -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "ABIInfo.h"
#include "CodeGenFunction.h"
#include "TargetInfo.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/Type.h"

#ifndef LLVM_CLANG_CODEGEN_TARGS_COMMON_CHERI_TARGET_INFO
#define LLVM_CLANG_CODEGEN_TARGS_COMMON_CHERI_TARGET_INFO

namespace clang {
namespace CodeGen {

class CommonCheriTargetCodeGenInfo : public TargetCodeGenInfo {
  mutable llvm::Function *GetOffset = nullptr;
  mutable llvm::Function *SetOffset = nullptr;
  mutable llvm::Function *SetAddr = nullptr;
  mutable llvm::Function *GetBase = nullptr;
  mutable llvm::Function *GetAddress = nullptr;
  mutable llvm::Function *SetBounds = nullptr;
  mutable llvm::Function *GetTag = nullptr;
  llvm::PointerType *getI8CapTy(CodeGen::CodeGenFunction &CGF) const {
    return CGF.Int8CheriCapTy;
  }

public:
  CommonCheriTargetCodeGenInfo(std::unique_ptr<ABIInfo> info)
      : TargetCodeGenInfo(std::move(info)) {}

  llvm::Value *setPointerOffset(CodeGen::CodeGenFunction &CGF, llvm::Value *Ptr,
                                llvm::Value *Offset, const llvm::Twine &Name,
                                SourceLocation Loc) const override {
    if (isa<llvm::ConstantPointerNull>(Ptr)) {
      // Avoid unnecessary work for instcombine by returning a null-derived cap:
      return CGF.getNullDerivedCapability(Ptr->getType(), Offset);
    }
    if (!SetOffset)
      SetOffset = CGF.CGM.getIntrinsic(llvm::Intrinsic::cheri_cap_offset_set,
                                       CGF.SizeTy);
    llvm::Type *DstTy = Ptr->getType();
    auto &B = CGF.Builder;
    Ptr = B.CreateBitCast(Ptr, getI8CapTy(CGF));
    assert(Offset->getType()->getIntegerBitWidth() ==
           CGF.CGM.getDataLayout().getIndexTypeSizeInBits(DstTy));
    llvm::Value *Result = B.CreateCall(SetOffset, {Ptr, Offset}, Name);
    if (CGF.SanOpts.has(SanitizerKind::CheriUnrepresentable))
      Result = CGF.EmitCapabilityArithmeticCheck(Ptr, Result, Loc);
    return B.CreateBitCast(Result, DstTy);
  }

  llvm::Value *setPointerAddress(CodeGen::CodeGenFunction &CGF,
                                 llvm::Value *Ptr, llvm::Value *Addr,
                                 const llvm::Twine &Name,
                                 SourceLocation Loc) const override {
    if (isa<llvm::ConstantPointerNull>(Ptr)) {
      // Avoid unnecessary work for instcombine by returning a null-derived cap:
      return CGF.getNullDerivedCapability(Ptr->getType(), Addr);
    }
    if (!SetAddr)
      SetAddr = CGF.CGM.getIntrinsic(llvm::Intrinsic::cheri_cap_address_set,
                                     CGF.IntPtrTy);
    llvm::Type *DstTy = Ptr->getType();
    auto &B = CGF.Builder;
    Ptr = B.CreateBitCast(Ptr, getI8CapTy(CGF));
    assert(Addr->getType()->getIntegerBitWidth() ==
           CGF.CGM.getDataLayout().getIndexTypeSizeInBits(DstTy));
    llvm::Value *Result = B.CreateCall(SetAddr, {Ptr, Addr}, Name);
    if (CGF.SanOpts.has(SanitizerKind::CheriUnrepresentable))
      Result = CGF.EmitCapabilityArithmeticCheck(Ptr, Result, Loc);
    return B.CreateBitCast(Result, DstTy);
  }

  llvm::Value *setPointerBounds(CodeGen::CodeGenFunction &CGF, llvm::Value *Ptr,
                                llvm::Value *Size,
                                const llvm::Twine &Name) const override {
    if (!SetBounds)
      SetBounds = CGF.CGM.getIntrinsic(llvm::Intrinsic::cheri_cap_bounds_set,
                                       CGF.SizeTy);
    llvm::Type *DstTy = Ptr->getType();
    auto &B = CGF.Builder;
    Ptr = B.CreateBitCast(Ptr, getI8CapTy(CGF));
    assert(Size->getType()->getIntegerBitWidth() ==
           CGF.CGM.getDataLayout().getIndexTypeSizeInBits(DstTy));
    return B.CreateBitCast(B.CreateCall(SetBounds, {Ptr, Size}), DstTy, Name);
  }

  llvm::Value *getPointerValidity(CodeGen::CodeGenFunction &CGF, llvm::Value *V,
                                  const llvm::Twine &Name) const override {
    if (!GetTag)
      GetTag = CGF.CGM.getIntrinsic(llvm::Intrinsic::cheri_cap_tag_get);
    V = CGF.Builder.CreateBitCast(V, getI8CapTy(CGF));
    return CGF.Builder.CreateCall(GetTag, V, Name);
  }

  llvm::Value *getPointerBase(CodeGen::CodeGenFunction &CGF,
                              llvm::Value *V) const override {
    if (!GetBase)
      GetBase = CGF.CGM.getIntrinsic(llvm::Intrinsic::cheri_cap_base_get,
                                     CGF.IntPtrTy);
    V = CGF.Builder.CreateBitCast(V, getI8CapTy(CGF));
    return CGF.Builder.CreateCall(GetBase, V);
  }

  llvm::Value *getPointerOffset(CodeGen::CodeGenFunction &CGF,
                                llvm::Value *V) const override {
    assert(CGF.CGM.getDataLayout().isFatPointer(V->getType()));
    if (!GetOffset)
      GetOffset = CGF.CGM.getIntrinsic(llvm::Intrinsic::cheri_cap_offset_get,
                                       CGF.SizeTy);
    V = CGF.Builder.CreateBitCast(V, getI8CapTy(CGF));
    return CGF.Builder.CreateCall(GetOffset, V);
  }

  llvm::Value *getPointerAddress(CodeGen::CodeGenFunction &CGF, llvm::Value *V,
                                 const llvm::Twine &Name) const override {
    assert(isa<llvm::PointerType>(V->getType()));
    if (!CGF.CGM.getDataLayout().isFatPointer(V->getType())) {
      return CGF.Builder.CreatePtrToInt(V, CGF.PtrDiffTy);
    }
    if (!GetAddress)
      GetAddress = CGF.CGM.getIntrinsic(llvm::Intrinsic::cheri_cap_address_get,
                                        CGF.IntPtrTy);
    V = CGF.Builder.CreateBitCast(V, getI8CapTy(CGF));
    return CGF.Builder.CreateCall(GetAddress, V, Name);
  }
};
} // namespace CodeGen
} // namespace clang
#endif // LLVM_CLANG_CODEGEN_TARGS_COMMON_CHERI_TARGET_INFO
