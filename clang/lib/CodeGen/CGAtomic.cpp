//===--- CGAtomic.cpp - Emit LLVM IR for atomic operations ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the code for emitting atomic operations.
//
//===----------------------------------------------------------------------===//

#include "CGCall.h"
#include "CGRecordLayout.h"
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Operator.h"

using namespace clang;
using namespace CodeGen;

namespace {

bool isAtomicLoadOp(AtomicExpr::AtomicOp Op) {
  return Op == AtomicExpr::AO__c11_atomic_load ||
         Op == AtomicExpr::AO__opencl_atomic_load ||
         Op == AtomicExpr::AO__hip_atomic_load ||
         Op == AtomicExpr::AO__atomic_load ||
         Op == AtomicExpr::AO__atomic_load_n;
}

bool isAtomicStoreOp(AtomicExpr::AtomicOp Op) {
  return Op == AtomicExpr::AO__c11_atomic_store ||
         Op == AtomicExpr::AO__opencl_atomic_store ||
         Op == AtomicExpr::AO__hip_atomic_store ||
         Op == AtomicExpr::AO__atomic_store ||
         Op == AtomicExpr::AO__atomic_store_n;
}

  class AtomicInfo {
    CodeGenFunction &CGF;
    QualType AtomicTy;
    QualType ValueTy;
    uint64_t AtomicSizeInBits;
    uint64_t ValueSizeInBits;
    CharUnits AtomicAlign;
    CharUnits ValueAlign;
    TypeEvaluationKind EvaluationKind;
    bool UseLibcall;
    LValue LVal;
    CGBitFieldInfo BFI;
  public:
    AtomicInfo(CodeGenFunction &CGF, LValue &lvalue, AtomicExpr::AtomicOp Op)
        : CGF(CGF), AtomicSizeInBits(0), ValueSizeInBits(0),
          EvaluationKind(TEK_Scalar), UseLibcall(true) {
      assert(!lvalue.isGlobalReg());
      ASTContext &C = CGF.getContext();
      if (lvalue.isSimple()) {
        AtomicTy = lvalue.getType();
        if (auto *ATy = AtomicTy->getAs<AtomicType>())
          ValueTy = ATy->getValueType();
        else
          ValueTy = AtomicTy;
        EvaluationKind = CGF.getEvaluationKind(ValueTy);

        uint64_t ValueAlignInBits;
        uint64_t AtomicAlignInBits;
        TypeInfo ValueTI = C.getTypeInfo(ValueTy);
        ValueSizeInBits = ValueTI.Width;
        ValueAlignInBits = ValueTI.Align;

        TypeInfo AtomicTI = C.getTypeInfo(AtomicTy);
        AtomicSizeInBits = AtomicTI.Width;
        AtomicAlignInBits = AtomicTI.Align;

        assert(ValueSizeInBits <= AtomicSizeInBits);
        assert(ValueAlignInBits <= AtomicAlignInBits);

        AtomicAlign = C.toCharUnitsFromBits(AtomicAlignInBits);
        ValueAlign = C.toCharUnitsFromBits(ValueAlignInBits);
        if (lvalue.getAlignment().isZero())
          lvalue.setAlignment(AtomicAlign);

        LVal = lvalue;
      } else if (lvalue.isBitField()) {
        ValueTy = lvalue.getType();
        ValueSizeInBits = C.getTypeSize(ValueTy);
        auto &OrigBFI = lvalue.getBitFieldInfo();
        auto Offset = OrigBFI.Offset % C.toBits(lvalue.getAlignment());
        AtomicSizeInBits = C.toBits(
            C.toCharUnitsFromBits(Offset + OrigBFI.Size + C.getCharWidth() - 1)
                .alignTo(lvalue.getAlignment()));
        llvm::Value *BitFieldPtr = lvalue.getBitFieldPointer();
        auto OffsetInChars =
            (C.toCharUnitsFromBits(OrigBFI.Offset) / lvalue.getAlignment()) *
            lvalue.getAlignment();
        llvm::Value *StoragePtr = CGF.Builder.CreateConstGEP1_64(
            CGF.Int8Ty, BitFieldPtr, OffsetInChars.getQuantity());
        StoragePtr = CGF.Builder.CreateAddrSpaceCast(
            StoragePtr,
            llvm::PointerType::get(
                CGF.getLLVMContext(),
                CGF.CGM.getTargetCodeGenInfo().getDefaultAS()),
            "atomic_bitfield_base");
        BFI = OrigBFI;
        BFI.Offset = Offset;
        BFI.StorageSize = AtomicSizeInBits;
        BFI.StorageOffset += OffsetInChars;
        llvm::Type *StorageTy = CGF.Builder.getIntNTy(AtomicSizeInBits);
        LVal = LValue::MakeBitfield(
            Address(StoragePtr, StorageTy, lvalue.getAlignment()), BFI,
            lvalue.getType(), lvalue.getBaseInfo(), lvalue.getTBAAInfo());
        AtomicTy = C.getIntTypeForBitwidth(AtomicSizeInBits, OrigBFI.IsSigned);
        if (AtomicTy.isNull()) {
          llvm::APInt Size(
              /*numBits=*/32,
              C.toCharUnitsFromBits(AtomicSizeInBits).getQuantity());
          AtomicTy =
              C.getConstantArrayType(C.CharTy, Size, nullptr, ArrayType::Normal,
                                     /*IndexTypeQuals=*/0);
        }
        AtomicAlign = ValueAlign = lvalue.getAlignment();
      } else if (lvalue.isVectorElt()) {
        ValueTy = lvalue.getType()->castAs<VectorType>()->getElementType();
        ValueSizeInBits = C.getTypeSize(ValueTy);
        AtomicTy = lvalue.getType();
        AtomicSizeInBits = C.getTypeSize(AtomicTy);
        AtomicAlign = ValueAlign = lvalue.getAlignment();
        LVal = lvalue;
      } else {
        assert(lvalue.isExtVectorElt());
        ValueTy = lvalue.getType();
        ValueSizeInBits = C.getTypeSize(ValueTy);
        AtomicTy = ValueTy = CGF.getContext().getExtVectorType(
            lvalue.getType(), cast<llvm::FixedVectorType>(
                                  lvalue.getExtVectorAddress().getElementType())
                                  ->getNumElements());
        AtomicSizeInBits = C.getTypeSize(AtomicTy);
        AtomicAlign = ValueAlign = lvalue.getAlignment();
        LVal = lvalue;
      }
      UseLibcall = !C.getTargetInfo().hasBuiltinAtomic(
          AtomicSizeInBits, C.toBits(lvalue.getAlignment()),
          AtomicTy->isCHERICapabilityType(CGF.CGM.getContext()));
    }

    QualType getAtomicType() const { return AtomicTy; }
    QualType getValueType() const { return ValueTy; }
    CharUnits getAtomicAlignment() const { return AtomicAlign; }
    uint64_t getAtomicSizeInBits() const { return AtomicSizeInBits; }
    uint64_t getValueSizeInBits() const { return ValueSizeInBits; }
    TypeEvaluationKind getEvaluationKind() const { return EvaluationKind; }
    bool shouldUseLibcall() const { return UseLibcall; }
    const LValue &getAtomicLValue() const { return LVal; }
    llvm::Value *getAtomicPointer() const {
      if (LVal.isSimple())
        return LVal.getPointer(CGF);
      else if (LVal.isBitField())
        return LVal.getBitFieldPointer();
      else if (LVal.isVectorElt())
        return LVal.getVectorPointer();
      assert(LVal.isExtVectorElt());
      return LVal.getExtVectorPointer();
    }
    Address getAtomicAddress() const {
      llvm::Type *ElTy;
      if (LVal.isSimple())
        ElTy = LVal.getAddress(CGF).getElementType();
      else if (LVal.isBitField())
        ElTy = LVal.getBitFieldAddress().getElementType();
      else if (LVal.isVectorElt())
        ElTy = LVal.getVectorAddress().getElementType();
      else
        ElTy = LVal.getExtVectorAddress().getElementType();
      return Address(getAtomicPointer(), ElTy, getAtomicAlignment());
    }

    Address getAtomicAddressAsAtomicIntPointer() const {
      return castToAtomicIntPointer(getAtomicAddress());
    }

    /// Is the atomic size larger than the underlying value type?
    ///
    /// Note that the absence of padding does not mean that atomic
    /// objects are completely interchangeable with non-atomic
    /// objects: we might have promoted the alignment of a type
    /// without making it bigger.
    bool hasPadding() const {
      return (ValueSizeInBits != AtomicSizeInBits);
    }

    bool emitMemSetZeroIfNecessary() const;

    llvm::Value *getAtomicSizeValue() const {
      CharUnits size = CGF.getContext().toCharUnitsFromBits(AtomicSizeInBits);
      return CGF.CGM.getSize(size);
    }

    /// Cast the given pointer to an integer pointer suitable for atomic
    /// operations if the source.
    Address castToAtomicIntPointer(Address Addr) const;

    /// If Addr is compatible with the iN that will be used for an atomic
    /// operation, bitcast it. Otherwise, create a temporary that is suitable
    /// and copy the value across.
    Address convertToAtomicIntPointer(Address Addr) const;

    /// Turn an atomic-layout object into an r-value.
    RValue convertAtomicTempToRValue(Address addr, AggValueSlot resultSlot,
                                     SourceLocation loc, bool AsValue) const;

    /// Converts a rvalue to integer value.
    llvm::Value *convertRValueToInt(RValue RVal) const;

    RValue ConvertIntToValueOrAtomic(llvm::Value *IntVal,
                                     AggValueSlot ResultSlot,
                                     SourceLocation Loc, bool AsValue) const;

    /// Copy an atomic r-value into atomic-layout memory.
    void emitCopyIntoMemory(RValue rvalue) const;

    /// Project an l-value down to the value field.
    LValue projectValue() const {
      assert(LVal.isSimple());
      Address addr = getAtomicAddress();
      if (hasPadding())
        addr = CGF.Builder.CreateStructGEP(addr, 0);

      return LValue::MakeAddr(addr, getValueType(), CGF.getContext(),
                              LVal.getBaseInfo(), LVal.getTBAAInfo());
    }

    /// Emits atomic load.
    /// \returns Loaded value.
    RValue EmitAtomicLoad(AggValueSlot ResultSlot, SourceLocation Loc,
                          bool AsValue, llvm::AtomicOrdering AO,
                          bool IsVolatile);

    /// Emits atomic compare-and-exchange sequence.
    /// \param Expected Expected value.
    /// \param Desired Desired value.
    /// \param Success Atomic ordering for success operation.
    /// \param Failure Atomic ordering for failed operation.
    /// \param IsWeak true if atomic operation is weak, false otherwise.
    /// \returns Pair of values: previous value from storage (value type) and
    /// boolean flag (i1 type) with true if success and false otherwise.
    std::pair<RValue, llvm::Value *>
    EmitAtomicCompareExchange(RValue Expected, RValue Desired,
                              llvm::AtomicOrdering Success =
                                  llvm::AtomicOrdering::SequentiallyConsistent,
                              llvm::AtomicOrdering Failure =
                                  llvm::AtomicOrdering::SequentiallyConsistent,
                              bool IsWeak = false);

    /// Emits atomic update.
    /// \param AO Atomic ordering.
    /// \param UpdateOp Update operation for the current lvalue.
    void EmitAtomicUpdate(llvm::AtomicOrdering AO,
                          const llvm::function_ref<RValue(RValue)> &UpdateOp,
                          bool IsVolatile);
    /// Emits atomic update.
    /// \param AO Atomic ordering.
    void EmitAtomicUpdate(llvm::AtomicOrdering AO, RValue UpdateRVal,
                          bool IsVolatile);

    /// Materialize an atomic r-value in atomic-layout memory.
    Address materializeRValue(RValue rvalue) const;

    /// Creates temp alloca for intermediate operations on atomic value.
    Address CreateTempAlloca() const;
  private:
    bool requiresMemSetZero(llvm::Type *type) const;


    /// Emits atomic load as a libcall.
    void EmitAtomicLoadLibcall(llvm::Value *AddForLoaded,
                               llvm::AtomicOrdering AO, bool IsVolatile);
    /// Emits atomic load as LLVM instruction.
    llvm::Value *EmitAtomicLoadOp(llvm::AtomicOrdering AO, bool IsVolatile);
    /// Emits atomic compare-and-exchange op as a libcall.
    llvm::Value *EmitAtomicCompareExchangeLibcall(
        llvm::Value *ExpectedAddr, llvm::Value *DesiredAddr,
        llvm::AtomicOrdering Success =
            llvm::AtomicOrdering::SequentiallyConsistent,
        llvm::AtomicOrdering Failure =
            llvm::AtomicOrdering::SequentiallyConsistent);
    /// Emits atomic compare-and-exchange op as LLVM instruction.
    std::pair<llvm::Value *, llvm::Value *> EmitAtomicCompareExchangeOp(
        llvm::Value *ExpectedVal, llvm::Value *DesiredVal,
        llvm::AtomicOrdering Success =
            llvm::AtomicOrdering::SequentiallyConsistent,
        llvm::AtomicOrdering Failure =
            llvm::AtomicOrdering::SequentiallyConsistent,
        bool IsWeak = false);
    /// Emit atomic update as libcalls.
    void
    EmitAtomicUpdateLibcall(llvm::AtomicOrdering AO,
                            const llvm::function_ref<RValue(RValue)> &UpdateOp,
                            bool IsVolatile);
    /// Emit atomic update as LLVM instructions.
    void EmitAtomicUpdateOp(llvm::AtomicOrdering AO,
                            const llvm::function_ref<RValue(RValue)> &UpdateOp,
                            bool IsVolatile);
    /// Emit atomic update as libcalls.
    void EmitAtomicUpdateLibcall(llvm::AtomicOrdering AO, RValue UpdateRVal,
                                 bool IsVolatile);
    /// Emit atomic update as LLVM instructions.
    void EmitAtomicUpdateOp(llvm::AtomicOrdering AO, RValue UpdateRal,
                            bool IsVolatile);
  };
}

Address AtomicInfo::CreateTempAlloca() const {
  Address TempAlloca = CGF.CreateMemTemp(
      (LVal.isBitField() && ValueSizeInBits > AtomicSizeInBits) ? ValueTy
                                                                : AtomicTy,
      getAtomicAlignment(),
      "atomic-temp");
  // Cast to pointer to value type for bitfields.
  if (LVal.isBitField())
    return CGF.Builder.CreatePointerBitCastOrAddrSpaceCast(
        TempAlloca, getAtomicAddress().getType(),
        getAtomicAddress().getElementType());
  return TempAlloca;
}

static RValue emitAtomicLibcall(CodeGenFunction &CGF,
                                StringRef fnName,
                                QualType resultType,
                                CallArgList &args) {
  const CGFunctionInfo &fnInfo =
    CGF.CGM.getTypes().arrangeBuiltinFunctionCall(resultType, args);
  llvm::FunctionType *fnTy = CGF.CGM.getTypes().GetFunctionType(fnInfo);
  llvm::AttrBuilder fnAttrB(CGF.getLLVMContext());
  fnAttrB.addAttribute(llvm::Attribute::NoUnwind);
  fnAttrB.addAttribute(llvm::Attribute::WillReturn);
  llvm::AttributeList fnAttrs = llvm::AttributeList::get(
      CGF.getLLVMContext(), llvm::AttributeList::FunctionIndex, fnAttrB);

  llvm::FunctionCallee fn =
      CGF.CGM.CreateRuntimeFunction(fnTy, fnName, fnAttrs);
  auto callee = CGCallee::forDirect(fn);
  return CGF.EmitCall(fnInfo, callee, ReturnValueSlot(), args);
}

/// Does a store of the given IR type modify the full expected width?
static bool isFullSizeType(CodeGenModule &CGM, llvm::Type *type,
                           uint64_t expectedSize) {
  return (CGM.getDataLayout().getTypeStoreSize(type) * 8 == expectedSize);
}

/// Does the atomic type require memsetting to zero before initialization?
///
/// The IR type is provided as a way of making certain queries faster.
bool AtomicInfo::requiresMemSetZero(llvm::Type *type) const {
  // If the atomic type has size padding, we definitely need a memset.
  if (hasPadding()) return true;

  // Otherwise, do some simple heuristics to try to avoid it:
  switch (getEvaluationKind()) {
  // For scalars and complexes, check whether the store size of the
  // type uses the full size.
  case TEK_Scalar:
    return !isFullSizeType(CGF.CGM, type, AtomicSizeInBits);
  case TEK_Complex:
    return !isFullSizeType(CGF.CGM, type->getStructElementType(0),
                           AtomicSizeInBits / 2);

  // Padding in structs has an undefined bit pattern.  User beware.
  case TEK_Aggregate:
    return false;
  }
  llvm_unreachable("bad evaluation kind");
}

bool AtomicInfo::emitMemSetZeroIfNecessary() const {
  assert(LVal.isSimple());
  Address addr = LVal.getAddress(CGF);
  if (!requiresMemSetZero(addr.getElementType()))
    return false;

  CGF.Builder.CreateMemSet(
      addr.getPointer(), llvm::ConstantInt::get(CGF.Int8Ty, 0),
      CGF.getContext().toCharUnitsFromBits(AtomicSizeInBits).getQuantity(),
      LVal.getAlignment().getAsAlign());
  return true;
}

static void emitAtomicCmpXchg(CodeGenFunction &CGF, AtomicExpr *E, bool IsWeak,
                              Address Dest, Address Ptr,
                              Address Val1, Address Val2,
                              uint64_t Size,
                              llvm::AtomicOrdering SuccessOrder,
                              llvm::AtomicOrdering FailureOrder,
                              llvm::SyncScope::ID Scope) {
  // Note that cmpxchg doesn't support weak cmpxchg, at least at the moment.
  llvm::Value *Expected = CGF.Builder.CreateLoad(Val1);
  llvm::Value *Desired = CGF.Builder.CreateLoad(Val2);

  llvm::AtomicCmpXchgInst *Pair = CGF.Builder.CreateAtomicCmpXchg(
      Ptr.getPointer(), Expected, Desired, SuccessOrder, FailureOrder,
      Scope);
  Pair->setVolatile(E->isVolatile());
  Pair->setWeak(IsWeak);

  // Cmp holds the result of the compare-exchange operation: true on success,
  // false on failure.
  llvm::Value *Old = CGF.Builder.CreateExtractValue(Pair, 0);
  llvm::Value *Cmp = CGF.Builder.CreateExtractValue(Pair, 1);

  // This basic block is used to hold the store instruction if the operation
  // failed.
  llvm::BasicBlock *StoreExpectedBB =
      CGF.createBasicBlock("cmpxchg.store_expected", CGF.CurFn);

  // This basic block is the exit point of the operation, we should end up
  // here regardless of whether or not the operation succeeded.
  llvm::BasicBlock *ContinueBB =
      CGF.createBasicBlock("cmpxchg.continue", CGF.CurFn);

  // Update Expected if Expected isn't equal to Old, otherwise branch to the
  // exit point.
  CGF.Builder.CreateCondBr(Cmp, ContinueBB, StoreExpectedBB);

  CGF.Builder.SetInsertPoint(StoreExpectedBB);
  // Update the memory at Expected with Old's value.
  CGF.Builder.CreateStore(Old, Val1);
  // Finally, branch to the exit point.
  CGF.Builder.CreateBr(ContinueBB);

  CGF.Builder.SetInsertPoint(ContinueBB);
  // Update the memory at Dest with Cmp's value.
  CGF.EmitStoreOfScalar(Cmp, CGF.MakeAddrLValue(Dest, E->getType()));
}

/// Given an ordering required on success, emit all possible cmpxchg
/// instructions to cope with the provided (but possibly only dynamically known)
/// FailureOrder.
static void emitAtomicCmpXchgFailureSet(CodeGenFunction &CGF, AtomicExpr *E,
                                        bool IsWeak, Address Dest, Address Ptr,
                                        Address Val1, Address Val2,
                                        llvm::Value *FailureOrderVal,
                                        uint64_t Size,
                                        llvm::AtomicOrdering SuccessOrder,
                                        llvm::SyncScope::ID Scope) {
  llvm::AtomicOrdering FailureOrder;
  if (llvm::ConstantInt *FO = dyn_cast<llvm::ConstantInt>(FailureOrderVal)) {
    auto FOS = FO->getSExtValue();
    if (!llvm::isValidAtomicOrderingCABI(FOS))
      FailureOrder = llvm::AtomicOrdering::Monotonic;
    else
      switch ((llvm::AtomicOrderingCABI)FOS) {
      case llvm::AtomicOrderingCABI::relaxed:
      // 31.7.2.18: "The failure argument shall not be memory_order_release
      // nor memory_order_acq_rel". Fallback to monotonic.
      case llvm::AtomicOrderingCABI::release:
      case llvm::AtomicOrderingCABI::acq_rel:
        FailureOrder = llvm::AtomicOrdering::Monotonic;
        break;
      case llvm::AtomicOrderingCABI::consume:
      case llvm::AtomicOrderingCABI::acquire:
        FailureOrder = llvm::AtomicOrdering::Acquire;
        break;
      case llvm::AtomicOrderingCABI::seq_cst:
        FailureOrder = llvm::AtomicOrdering::SequentiallyConsistent;
        break;
      }
    // Prior to c++17, "the failure argument shall be no stronger than the
    // success argument". This condition has been lifted and the only
    // precondition is 31.7.2.18. Effectively treat this as a DR and skip
    // language version checks.
    emitAtomicCmpXchg(CGF, E, IsWeak, Dest, Ptr, Val1, Val2, Size, SuccessOrder,
                      FailureOrder, Scope);
    return;
  }

  // Create all the relevant BB's
  auto *MonotonicBB = CGF.createBasicBlock("monotonic_fail", CGF.CurFn);
  auto *AcquireBB = CGF.createBasicBlock("acquire_fail", CGF.CurFn);
  auto *SeqCstBB = CGF.createBasicBlock("seqcst_fail", CGF.CurFn);
  auto *ContBB = CGF.createBasicBlock("atomic.continue", CGF.CurFn);

  // MonotonicBB is arbitrarily chosen as the default case; in practice, this
  // doesn't matter unless someone is crazy enough to use something that
  // doesn't fold to a constant for the ordering.
  llvm::SwitchInst *SI = CGF.Builder.CreateSwitch(FailureOrderVal, MonotonicBB);
  // Implemented as acquire, since it's the closest in LLVM.
  SI->addCase(CGF.Builder.getInt32((int)llvm::AtomicOrderingCABI::consume),
              AcquireBB);
  SI->addCase(CGF.Builder.getInt32((int)llvm::AtomicOrderingCABI::acquire),
              AcquireBB);
  SI->addCase(CGF.Builder.getInt32((int)llvm::AtomicOrderingCABI::seq_cst),
              SeqCstBB);

  // Emit all the different atomics
  CGF.Builder.SetInsertPoint(MonotonicBB);
  emitAtomicCmpXchg(CGF, E, IsWeak, Dest, Ptr, Val1, Val2,
                    Size, SuccessOrder, llvm::AtomicOrdering::Monotonic, Scope);
  CGF.Builder.CreateBr(ContBB);

  CGF.Builder.SetInsertPoint(AcquireBB);
  emitAtomicCmpXchg(CGF, E, IsWeak, Dest, Ptr, Val1, Val2, Size, SuccessOrder,
                    llvm::AtomicOrdering::Acquire, Scope);
  CGF.Builder.CreateBr(ContBB);

  CGF.Builder.SetInsertPoint(SeqCstBB);
  emitAtomicCmpXchg(CGF, E, IsWeak, Dest, Ptr, Val1, Val2, Size, SuccessOrder,
                    llvm::AtomicOrdering::SequentiallyConsistent, Scope);
  CGF.Builder.CreateBr(ContBB);

  CGF.Builder.SetInsertPoint(ContBB);
}

/// Duplicate the atomic min/max operation in conventional IR for the builtin
/// variants that return the new rather than the original value.
static llvm::Value *EmitPostAtomicMinMax(CGBuilderTy &Builder,
                                         AtomicExpr::AtomicOp Op,
                                         bool IsSigned,
                                         llvm::Value *OldVal,
                                         llvm::Value *RHS) {
  llvm::CmpInst::Predicate Pred;
  switch (Op) {
  default:
    llvm_unreachable("Unexpected min/max operation");
  case AtomicExpr::AO__atomic_max_fetch:
    Pred = IsSigned ? llvm::CmpInst::ICMP_SGT : llvm::CmpInst::ICMP_UGT;
    break;
  case AtomicExpr::AO__atomic_min_fetch:
    Pred = IsSigned ? llvm::CmpInst::ICMP_SLT : llvm::CmpInst::ICMP_ULT;
    break;
  }
  llvm::Value *Cmp = Builder.CreateICmp(Pred, OldVal, RHS, "tst");
  return Builder.CreateSelect(Cmp, OldVal, RHS, "newval");
}

static void EmitAtomicOp(CodeGenFunction &CGF, AtomicExpr *E, Address Dest,
                         Address Ptr, Address Val1, Address Val2,
                         llvm::Value *IsWeak, llvm::Value *FailureOrder,
                         uint64_t Size, llvm::AtomicOrdering Order,
                         llvm::SyncScope::ID Scope) {
  llvm::AtomicRMWInst::BinOp Op = llvm::AtomicRMWInst::Add;
  bool PostOpMinMax = false;
  unsigned PostOp = 0;
  QualType AtomicTy = E->getPtr()->getType()->getPointeeType();
  bool IsCheriCap = AtomicTy->isCHERICapabilityType(CGF.CGM.getContext());

  switch (E->getOp()) {
  case AtomicExpr::AO__c11_atomic_init:
  case AtomicExpr::AO__opencl_atomic_init:
    llvm_unreachable("Already handled!");

  case AtomicExpr::AO__c11_atomic_compare_exchange_strong:
  case AtomicExpr::AO__hip_atomic_compare_exchange_strong:
  case AtomicExpr::AO__opencl_atomic_compare_exchange_strong:
    emitAtomicCmpXchgFailureSet(CGF, E, false, Dest, Ptr, Val1, Val2,
                                FailureOrder, Size, Order, Scope);
    return;
  case AtomicExpr::AO__c11_atomic_compare_exchange_weak:
  case AtomicExpr::AO__opencl_atomic_compare_exchange_weak:
  case AtomicExpr::AO__hip_atomic_compare_exchange_weak:
    emitAtomicCmpXchgFailureSet(CGF, E, true, Dest, Ptr, Val1, Val2,
                                FailureOrder, Size, Order, Scope);
    return;
  case AtomicExpr::AO__atomic_compare_exchange:
  case AtomicExpr::AO__atomic_compare_exchange_n: {
    if (llvm::ConstantInt *IsWeakC = dyn_cast<llvm::ConstantInt>(IsWeak)) {
      emitAtomicCmpXchgFailureSet(CGF, E, IsWeakC->getZExtValue(), Dest, Ptr,
                                  Val1, Val2, FailureOrder, Size, Order, Scope);
    } else {
      // Create all the relevant BB's
      llvm::BasicBlock *StrongBB =
          CGF.createBasicBlock("cmpxchg.strong", CGF.CurFn);
      llvm::BasicBlock *WeakBB = CGF.createBasicBlock("cmxchg.weak", CGF.CurFn);
      llvm::BasicBlock *ContBB =
          CGF.createBasicBlock("cmpxchg.continue", CGF.CurFn);

      llvm::SwitchInst *SI = CGF.Builder.CreateSwitch(IsWeak, WeakBB);
      SI->addCase(CGF.Builder.getInt1(false), StrongBB);

      CGF.Builder.SetInsertPoint(StrongBB);
      emitAtomicCmpXchgFailureSet(CGF, E, false, Dest, Ptr, Val1, Val2,
                                  FailureOrder, Size, Order, Scope);
      CGF.Builder.CreateBr(ContBB);

      CGF.Builder.SetInsertPoint(WeakBB);
      emitAtomicCmpXchgFailureSet(CGF, E, true, Dest, Ptr, Val1, Val2,
                                  FailureOrder, Size, Order, Scope);
      CGF.Builder.CreateBr(ContBB);

      CGF.Builder.SetInsertPoint(ContBB);
    }
    return;
  }
  case AtomicExpr::AO__c11_atomic_load:
  case AtomicExpr::AO__opencl_atomic_load:
  case AtomicExpr::AO__hip_atomic_load:
  case AtomicExpr::AO__atomic_load_n:
  case AtomicExpr::AO__atomic_load: {
    llvm::LoadInst *Load = CGF.Builder.CreateLoad(Ptr);
    Load->setAtomic(Order, Scope);
    Load->setVolatile(E->isVolatile());
    CGF.Builder.CreateStore(Load, Dest);
    return;
  }

  case AtomicExpr::AO__c11_atomic_store:
  case AtomicExpr::AO__opencl_atomic_store:
  case AtomicExpr::AO__hip_atomic_store:
  case AtomicExpr::AO__atomic_store:
  case AtomicExpr::AO__atomic_store_n: {
    llvm::Value *LoadVal1 = CGF.Builder.CreateLoad(Val1);
    llvm::StoreInst *Store = CGF.Builder.CreateStore(LoadVal1, Ptr);
    Store->setAtomic(Order, Scope);
    Store->setVolatile(E->isVolatile());
    return;
  }

  case AtomicExpr::AO__c11_atomic_exchange:
  case AtomicExpr::AO__hip_atomic_exchange:
  case AtomicExpr::AO__opencl_atomic_exchange:
  case AtomicExpr::AO__atomic_exchange_n:
  case AtomicExpr::AO__atomic_exchange:
    Op = llvm::AtomicRMWInst::Xchg;
    break;

  case AtomicExpr::AO__atomic_add_fetch:
    PostOp = E->getValueType()->isFloatingType() ? llvm::Instruction::FAdd
                                                 : llvm::Instruction::Add;
    [[fallthrough]];
  case AtomicExpr::AO__c11_atomic_fetch_add:
  case AtomicExpr::AO__hip_atomic_fetch_add:
  case AtomicExpr::AO__opencl_atomic_fetch_add:
  case AtomicExpr::AO__atomic_fetch_add:
    Op = E->getValueType()->isFloatingType() ? llvm::AtomicRMWInst::FAdd
                                             : llvm::AtomicRMWInst::Add;
    break;

  case AtomicExpr::AO__atomic_sub_fetch:
    PostOp = E->getValueType()->isFloatingType() ? llvm::Instruction::FSub
                                                 : llvm::Instruction::Sub;
    [[fallthrough]];
  case AtomicExpr::AO__c11_atomic_fetch_sub:
  case AtomicExpr::AO__hip_atomic_fetch_sub:
  case AtomicExpr::AO__opencl_atomic_fetch_sub:
  case AtomicExpr::AO__atomic_fetch_sub:
    Op = E->getValueType()->isFloatingType() ? llvm::AtomicRMWInst::FSub
                                             : llvm::AtomicRMWInst::Sub;
    break;

  case AtomicExpr::AO__atomic_min_fetch:
    PostOpMinMax = true;
    [[fallthrough]];
  case AtomicExpr::AO__c11_atomic_fetch_min:
  case AtomicExpr::AO__hip_atomic_fetch_min:
  case AtomicExpr::AO__opencl_atomic_fetch_min:
  case AtomicExpr::AO__atomic_fetch_min:
    Op = E->getValueType()->isFloatingType()
             ? llvm::AtomicRMWInst::FMin
             : (E->getValueType()->isSignedIntegerType()
                    ? llvm::AtomicRMWInst::Min
                    : llvm::AtomicRMWInst::UMin);
    break;

  case AtomicExpr::AO__atomic_max_fetch:
    PostOpMinMax = true;
    [[fallthrough]];
  case AtomicExpr::AO__c11_atomic_fetch_max:
  case AtomicExpr::AO__hip_atomic_fetch_max:
  case AtomicExpr::AO__opencl_atomic_fetch_max:
  case AtomicExpr::AO__atomic_fetch_max:
    Op = E->getValueType()->isFloatingType()
             ? llvm::AtomicRMWInst::FMax
             : (E->getValueType()->isSignedIntegerType()
                    ? llvm::AtomicRMWInst::Max
                    : llvm::AtomicRMWInst::UMax);
    break;

  case AtomicExpr::AO__atomic_and_fetch:
    PostOp = llvm::Instruction::And;
    [[fallthrough]];
  case AtomicExpr::AO__c11_atomic_fetch_and:
  case AtomicExpr::AO__hip_atomic_fetch_and:
  case AtomicExpr::AO__opencl_atomic_fetch_and:
  case AtomicExpr::AO__atomic_fetch_and:
    Op = llvm::AtomicRMWInst::And;
    break;

  case AtomicExpr::AO__atomic_or_fetch:
    PostOp = llvm::Instruction::Or;
    [[fallthrough]];
  case AtomicExpr::AO__c11_atomic_fetch_or:
  case AtomicExpr::AO__hip_atomic_fetch_or:
  case AtomicExpr::AO__opencl_atomic_fetch_or:
  case AtomicExpr::AO__atomic_fetch_or:
    Op = llvm::AtomicRMWInst::Or;
    break;

  case AtomicExpr::AO__atomic_xor_fetch:
    PostOp = llvm::Instruction::Xor;
    [[fallthrough]];
  case AtomicExpr::AO__c11_atomic_fetch_xor:
  case AtomicExpr::AO__hip_atomic_fetch_xor:
  case AtomicExpr::AO__opencl_atomic_fetch_xor:
  case AtomicExpr::AO__atomic_fetch_xor:
    Op = llvm::AtomicRMWInst::Xor;
    break;

  case AtomicExpr::AO__atomic_nand_fetch:
    PostOp = llvm::Instruction::And; // the NOT is special cased below
    [[fallthrough]];
  case AtomicExpr::AO__c11_atomic_fetch_nand:
  case AtomicExpr::AO__atomic_fetch_nand:
    Op = llvm::AtomicRMWInst::Nand;
    break;
  }

  llvm::Value *LoadVal1 = CGF.Builder.CreateLoad(Val1);
  llvm::AtomicRMWInst *RMWI =
      CGF.Builder.CreateAtomicRMW(Op, Ptr.getPointer(), LoadVal1, Order, Scope);
  RMWI->setVolatile(E->isVolatile());

  // For __atomic_*_fetch operations, perform the operation again to
  // determine the value which was written.
  llvm::Value *Result = RMWI;
  if (PostOpMinMax)
    Result = EmitPostAtomicMinMax(CGF.Builder, E->getOp(),
                                  E->getValueType()->isSignedIntegerType(),
                                  RMWI, LoadVal1);
  else if (PostOp) {
    if (IsCheriCap) {
      Result = CGF.getCapabilityIntegerValue(RMWI);
      LoadVal1 = CGF.getCapabilityIntegerValue(LoadVal1);
    }
    Result = CGF.Builder.CreateBinOp((llvm::Instruction::BinaryOps)PostOp,
                                     Result, LoadVal1);
  }
  if (E->getOp() == AtomicExpr::AO__atomic_nand_fetch)
    Result = CGF.Builder.CreateNot(Result);
  if (IsCheriCap && !PostOpMinMax && PostOp)
    Result = CGF.setCapabilityIntegerValue(RMWI, Result, E->getExprLoc());
  CGF.Builder.CreateStore(Result, Dest);
}

// This function emits any expression (scalar, complex, or aggregate)
// into a temporary alloca.
static Address
EmitValToTemp(CodeGenFunction &CGF, Expr *E) {
  Address DeclPtr = CGF.CreateMemTemp(E->getType(), ".atomictmp");
  CGF.EmitAnyExprToMem(E, DeclPtr, E->getType().getQualifiers(),
                       /*Init*/ true);
  return DeclPtr;
}

static void EmitAtomicOp(CodeGenFunction &CGF, AtomicExpr *Expr, Address Dest,
                         Address Ptr, Address Val1, Address Val2,
                         llvm::Value *IsWeak, llvm::Value *FailureOrder,
                         uint64_t Size, llvm::AtomicOrdering Order,
                         llvm::Value *Scope) {
  auto ScopeModel = Expr->getScopeModel();

  // LLVM atomic instructions always have synch scope. If clang atomic
  // expression has no scope operand, use default LLVM synch scope.
  if (!ScopeModel) {
    EmitAtomicOp(CGF, Expr, Dest, Ptr, Val1, Val2, IsWeak, FailureOrder, Size,
                 Order, CGF.CGM.getLLVMContext().getOrInsertSyncScopeID(""));
    return;
  }

  // Handle constant scope.
  if (auto SC = dyn_cast<llvm::ConstantInt>(Scope)) {
    auto SCID = CGF.getTargetHooks().getLLVMSyncScopeID(
        CGF.CGM.getLangOpts(), ScopeModel->map(SC->getZExtValue()),
        Order, CGF.CGM.getLLVMContext());
    EmitAtomicOp(CGF, Expr, Dest, Ptr, Val1, Val2, IsWeak, FailureOrder, Size,
                 Order, SCID);
    return;
  }

  // Handle non-constant scope.
  auto &Builder = CGF.Builder;
  auto Scopes = ScopeModel->getRuntimeValues();
  llvm::DenseMap<unsigned, llvm::BasicBlock *> BB;
  for (auto S : Scopes)
    BB[S] = CGF.createBasicBlock(getAsString(ScopeModel->map(S)), CGF.CurFn);

  llvm::BasicBlock *ContBB =
      CGF.createBasicBlock("atomic.scope.continue", CGF.CurFn);

  auto *SC = Builder.CreateIntCast(Scope, Builder.getInt32Ty(), false);
  // If unsupported synch scope is encountered at run time, assume a fallback
  // synch scope value.
  auto FallBack = ScopeModel->getFallBackValue();
  llvm::SwitchInst *SI = Builder.CreateSwitch(SC, BB[FallBack]);
  for (auto S : Scopes) {
    auto *B = BB[S];
    if (S != FallBack)
      SI->addCase(Builder.getInt32(S), B);

    Builder.SetInsertPoint(B);
    EmitAtomicOp(CGF, Expr, Dest, Ptr, Val1, Val2, IsWeak, FailureOrder, Size,
                 Order,
                 CGF.getTargetHooks().getLLVMSyncScopeID(CGF.CGM.getLangOpts(),
                                                         ScopeModel->map(S),
                                                         Order,
                                                         CGF.getLLVMContext()));
    Builder.CreateBr(ContBB);
  }

  Builder.SetInsertPoint(ContBB);
}

static void
AddDirectArgument(CodeGenFunction &CGF, CallArgList &Args,
                  bool UseOptimizedLibcall, llvm::Value *Val, QualType ValTy,
                  SourceLocation Loc, CharUnits SizeInChars) {
  if (UseOptimizedLibcall) {
    // Load value and pass it to the function directly.
    CharUnits Align = CGF.getContext().getTypeAlignInChars(ValTy);
    int64_t SizeInBits = CGF.getContext().toBits(SizeInChars);
    llvm::Type *ITy;
    if (ValTy->isCHERICapabilityType(CGF.getContext())) {
      ValTy = CGF.getContext().getPointerType(CGF.getContext().VoidTy,
                                              PIK_Capability);
      ITy = CGF.Int8CheriCapTy;
    } else {
      ValTy =
          CGF.getContext().getIntTypeForBitwidth(SizeInBits, /*Signed=*/false);
      ITy = llvm::IntegerType::get(CGF.getLLVMContext(), SizeInBits);
    }
    llvm::Type *IPtrTy = llvm::PointerType::get(
        CGF.getLLVMContext(), CGF.CGM.getTargetCodeGenInfo().getDefaultAS());
    Address Ptr = Address(CGF.Builder.CreateBitCast(Val, IPtrTy), ITy, Align);
    Val = CGF.EmitLoadOfScalar(Ptr, false,
                               CGF.getContext().getPointerType(ValTy), Loc);
    // Coerce the value into an appropriately sized integer or into a
    // void/i8 capability type.
    Args.add(RValue::get(Val), ValTy);
  } else {
    // Non-optimized functions always take a reference.
    // NB: Capabilities must be passed directly to the optimized libcall
    assert(!ValTy->isCHERICapabilityType(CGF.getContext()) &&
           "Capabilities should not be passed to the generic libcall");
    Args.add(RValue::get(Val), CGF.getContext().VoidPtrTy);
  }
}

RValue CodeGenFunction::EmitAtomicExpr(AtomicExpr *E) {
  QualType AtomicTy = E->getPtr()->getType()->getPointeeType();
  QualType MemTy = AtomicTy;
  if (const AtomicType *AT = AtomicTy->getAs<AtomicType>())
    MemTy = AT->getValueType();
  llvm::Value *IsWeak = nullptr, *OrderFail = nullptr;

  Address Val1 = Address::invalid();
  Address Val2 = Address::invalid();
  Address Dest = Address::invalid();
  Address Ptr = EmitPointerWithAlignment(E->getPtr());

  if (E->getOp() == AtomicExpr::AO__c11_atomic_init ||
      E->getOp() == AtomicExpr::AO__opencl_atomic_init) {
    LValue lvalue = MakeAddrLValue(Ptr, AtomicTy);
    EmitAtomicInit(E->getVal1(), lvalue);
    return RValue::get(nullptr);
  }

  auto TInfo = getContext().getTypeInfoInChars(AtomicTy);
  uint64_t Size = TInfo.Width.getQuantity();
  unsigned MaxInlineWidthInBits = getTarget().getMaxAtomicInlineWidth();

  bool IsCheriCap = AtomicTy->isCHERICapabilityType(CGM.getContext());
  bool Oversized = (!IsCheriCap &&
                    getContext().toBits(TInfo.Width) > MaxInlineWidthInBits) ||
                   (IsCheriCap && MaxInlineWidthInBits == 0);
  bool Misaligned = (Ptr.getAlignment() % TInfo.Width) != 0;
  bool UseLibcall = Misaligned | Oversized;
  bool ShouldCastToIntPtrTy = true;

  CharUnits MaxInlineWidth =
      getContext().toCharUnitsFromBits(MaxInlineWidthInBits);

  DiagnosticsEngine &Diags = CGM.getDiags();

  if (Misaligned) {
    Diags.Report(E->getBeginLoc(), diag::warn_atomic_op_misaligned)
        << (int)TInfo.Width.getQuantity()
        << (int)Ptr.getAlignment().getQuantity();
  }

  if (Oversized) {
    Diags.Report(E->getBeginLoc(), diag::warn_atomic_op_oversized)
        << (int)TInfo.Width.getQuantity() << (int)MaxInlineWidth.getQuantity();
  }

  llvm::Value *Order = EmitScalarExpr(E->getOrder());
  llvm::Value *Scope =
      E->getScopeModel() ? EmitScalarExpr(E->getScope()) : nullptr;

  switch (E->getOp()) {
  case AtomicExpr::AO__c11_atomic_init:
  case AtomicExpr::AO__opencl_atomic_init:
    llvm_unreachable("Already handled above with EmitAtomicInit!");

  case AtomicExpr::AO__c11_atomic_load:
  case AtomicExpr::AO__opencl_atomic_load:
  case AtomicExpr::AO__hip_atomic_load:
  case AtomicExpr::AO__atomic_load_n:
    break;

  case AtomicExpr::AO__atomic_load:
    Dest = EmitPointerWithAlignment(E->getVal1());
    break;

  case AtomicExpr::AO__atomic_store:
    Val1 = EmitPointerWithAlignment(E->getVal1());
    break;

  case AtomicExpr::AO__atomic_exchange:
    Val1 = EmitPointerWithAlignment(E->getVal1());
    Dest = EmitPointerWithAlignment(E->getVal2());
    break;

  case AtomicExpr::AO__c11_atomic_compare_exchange_strong:
  case AtomicExpr::AO__c11_atomic_compare_exchange_weak:
  case AtomicExpr::AO__opencl_atomic_compare_exchange_strong:
  case AtomicExpr::AO__hip_atomic_compare_exchange_strong:
  case AtomicExpr::AO__opencl_atomic_compare_exchange_weak:
  case AtomicExpr::AO__hip_atomic_compare_exchange_weak:
  case AtomicExpr::AO__atomic_compare_exchange_n:
  case AtomicExpr::AO__atomic_compare_exchange:
    Val1 = EmitPointerWithAlignment(E->getVal1());
    if (E->getOp() == AtomicExpr::AO__atomic_compare_exchange)
      Val2 = EmitPointerWithAlignment(E->getVal2());
    else
      Val2 = EmitValToTemp(*this, E->getVal2());
    OrderFail = EmitScalarExpr(E->getOrderFail());
    if (E->getOp() == AtomicExpr::AO__atomic_compare_exchange_n ||
        E->getOp() == AtomicExpr::AO__atomic_compare_exchange)
      IsWeak = EmitScalarExpr(E->getWeak());
    break;

  case AtomicExpr::AO__c11_atomic_fetch_add:
  case AtomicExpr::AO__c11_atomic_fetch_sub:
  case AtomicExpr::AO__hip_atomic_fetch_add:
  case AtomicExpr::AO__hip_atomic_fetch_sub:
  case AtomicExpr::AO__opencl_atomic_fetch_add:
  case AtomicExpr::AO__opencl_atomic_fetch_sub:
    if (MemTy->isPointerType()) {
      // For pointer arithmetic, we're required to do a bit of math:
      // adding 1 to an int* is not the same as adding 1 to a uintptr_t.
      // ... but only for the C11 builtins. The GNU builtins expect the
      // user to multiply by sizeof(T).
      QualType Val1Ty = E->getVal1()->getType();
      llvm::Value *Val1Scalar = EmitScalarExpr(E->getVal1());
      CharUnits PointeeIncAmt =
          getContext().getTypeSizeInChars(MemTy->getPointeeType());
      Val1Scalar = Builder.CreateMul(Val1Scalar, CGM.getSize(PointeeIncAmt));
      auto Temp = CreateMemTemp(Val1Ty, ".atomictmp");
      Val1 = Temp;
      EmitStoreOfScalar(Val1Scalar, MakeAddrLValue(Temp, Val1Ty));
      break;
    }
    [[fallthrough]];
  case AtomicExpr::AO__atomic_fetch_add:
  case AtomicExpr::AO__atomic_fetch_max:
  case AtomicExpr::AO__atomic_fetch_min:
  case AtomicExpr::AO__atomic_fetch_sub:
  case AtomicExpr::AO__atomic_add_fetch:
  case AtomicExpr::AO__atomic_max_fetch:
  case AtomicExpr::AO__atomic_min_fetch:
  case AtomicExpr::AO__atomic_sub_fetch:
  case AtomicExpr::AO__c11_atomic_fetch_max:
  case AtomicExpr::AO__c11_atomic_fetch_min:
  case AtomicExpr::AO__opencl_atomic_fetch_max:
  case AtomicExpr::AO__opencl_atomic_fetch_min:
  case AtomicExpr::AO__hip_atomic_fetch_max:
  case AtomicExpr::AO__hip_atomic_fetch_min:
    ShouldCastToIntPtrTy = !MemTy->isFloatingType();
    [[fallthrough]];

  case AtomicExpr::AO__c11_atomic_store:
  case AtomicExpr::AO__c11_atomic_exchange:
  case AtomicExpr::AO__opencl_atomic_store:
  case AtomicExpr::AO__hip_atomic_store:
  case AtomicExpr::AO__opencl_atomic_exchange:
  case AtomicExpr::AO__hip_atomic_exchange:
  case AtomicExpr::AO__atomic_store_n:
  case AtomicExpr::AO__atomic_exchange_n:
  case AtomicExpr::AO__c11_atomic_fetch_and:
  case AtomicExpr::AO__c11_atomic_fetch_or:
  case AtomicExpr::AO__c11_atomic_fetch_xor:
  case AtomicExpr::AO__c11_atomic_fetch_nand:
  case AtomicExpr::AO__opencl_atomic_fetch_and:
  case AtomicExpr::AO__opencl_atomic_fetch_or:
  case AtomicExpr::AO__opencl_atomic_fetch_xor:
  case AtomicExpr::AO__atomic_fetch_and:
  case AtomicExpr::AO__hip_atomic_fetch_and:
  case AtomicExpr::AO__atomic_fetch_or:
  case AtomicExpr::AO__hip_atomic_fetch_or:
  case AtomicExpr::AO__atomic_fetch_xor:
  case AtomicExpr::AO__hip_atomic_fetch_xor:
  case AtomicExpr::AO__atomic_fetch_nand:
  case AtomicExpr::AO__atomic_and_fetch:
  case AtomicExpr::AO__atomic_or_fetch:
  case AtomicExpr::AO__atomic_xor_fetch:
  case AtomicExpr::AO__atomic_nand_fetch:
    Val1 = EmitValToTemp(*this, E->getVal1());
    break;
  }

  QualType RValTy = E->getType().getUnqualifiedType();

  // The inlined atomics only function on iN types, where N is a power of 2. We
  // need to make sure (via temporaries if necessary) that all incoming values
  // are compatible.
  LValue AtomicVal = MakeAddrLValue(Ptr, AtomicTy);
  AtomicInfo Atomics(*this, AtomicVal, E->getOp());
  assert(Atomics.shouldUseLibcall() == UseLibcall);

  if (ShouldCastToIntPtrTy) {
    Ptr = Atomics.castToAtomicIntPointer(Ptr);
    if (Val1.isValid())
      Val1 = Atomics.convertToAtomicIntPointer(Val1);
    if (Val2.isValid())
      Val2 = Atomics.convertToAtomicIntPointer(Val2);
  }
  if (Dest.isValid()) {
    if (ShouldCastToIntPtrTy)
      Dest = Atomics.castToAtomicIntPointer(Dest);
  } else if (E->isCmpXChg())
    Dest = CreateMemTemp(RValTy, "cmpxchg.bool");
  else if (!RValTy->isVoidType()) {
    Dest = Atomics.CreateTempAlloca();
    if (ShouldCastToIntPtrTy)
      Dest = Atomics.castToAtomicIntPointer(Dest);
  }

  bool IsStore = isAtomicStoreOp(E->getOp());
  bool IsLoad = isAtomicLoadOp(E->getOp());

  // Use a library call.  See: http://gcc.gnu.org/wiki/Atomic/GCCMM/LIbrary .
  if (UseLibcall) {
    bool UseOptimizedLibcall = false;
    switch (E->getOp()) {
    case AtomicExpr::AO__c11_atomic_init:
    case AtomicExpr::AO__opencl_atomic_init:
      llvm_unreachable("Already handled above with EmitAtomicInit!");

    case AtomicExpr::AO__c11_atomic_fetch_add:
    case AtomicExpr::AO__opencl_atomic_fetch_add:
    case AtomicExpr::AO__atomic_fetch_add:
    case AtomicExpr::AO__hip_atomic_fetch_add:
    case AtomicExpr::AO__c11_atomic_fetch_and:
    case AtomicExpr::AO__opencl_atomic_fetch_and:
    case AtomicExpr::AO__hip_atomic_fetch_and:
    case AtomicExpr::AO__atomic_fetch_and:
    case AtomicExpr::AO__c11_atomic_fetch_or:
    case AtomicExpr::AO__opencl_atomic_fetch_or:
    case AtomicExpr::AO__hip_atomic_fetch_or:
    case AtomicExpr::AO__atomic_fetch_or:
    case AtomicExpr::AO__c11_atomic_fetch_nand:
    case AtomicExpr::AO__atomic_fetch_nand:
    case AtomicExpr::AO__c11_atomic_fetch_sub:
    case AtomicExpr::AO__opencl_atomic_fetch_sub:
    case AtomicExpr::AO__atomic_fetch_sub:
    case AtomicExpr::AO__hip_atomic_fetch_sub:
    case AtomicExpr::AO__c11_atomic_fetch_xor:
    case AtomicExpr::AO__opencl_atomic_fetch_xor:
    case AtomicExpr::AO__opencl_atomic_fetch_min:
    case AtomicExpr::AO__opencl_atomic_fetch_max:
    case AtomicExpr::AO__atomic_fetch_xor:
    case AtomicExpr::AO__hip_atomic_fetch_xor:
    case AtomicExpr::AO__c11_atomic_fetch_max:
    case AtomicExpr::AO__c11_atomic_fetch_min:
    case AtomicExpr::AO__atomic_add_fetch:
    case AtomicExpr::AO__atomic_and_fetch:
    case AtomicExpr::AO__atomic_nand_fetch:
    case AtomicExpr::AO__atomic_or_fetch:
    case AtomicExpr::AO__atomic_sub_fetch:
    case AtomicExpr::AO__atomic_xor_fetch:
    case AtomicExpr::AO__atomic_fetch_max:
    case AtomicExpr::AO__hip_atomic_fetch_max:
    case AtomicExpr::AO__atomic_fetch_min:
    case AtomicExpr::AO__hip_atomic_fetch_min:
    case AtomicExpr::AO__atomic_max_fetch:
    case AtomicExpr::AO__atomic_min_fetch:
      // For these, only library calls for certain sizes exist.
      UseOptimizedLibcall = true;
      break;

    case AtomicExpr::AO__atomic_load:
    case AtomicExpr::AO__atomic_store:
    case AtomicExpr::AO__atomic_exchange:
    case AtomicExpr::AO__atomic_compare_exchange:
      // Use the generic version if we don't know that the operand will be
      // suitably aligned for the optimized version.
      if (Misaligned)
        break;
      [[fallthrough]];
    case AtomicExpr::AO__c11_atomic_load:
    case AtomicExpr::AO__c11_atomic_store:
    case AtomicExpr::AO__c11_atomic_exchange:
    case AtomicExpr::AO__c11_atomic_compare_exchange_weak:
    case AtomicExpr::AO__c11_atomic_compare_exchange_strong:
    case AtomicExpr::AO__hip_atomic_compare_exchange_strong:
    case AtomicExpr::AO__opencl_atomic_load:
    case AtomicExpr::AO__hip_atomic_load:
    case AtomicExpr::AO__opencl_atomic_store:
    case AtomicExpr::AO__hip_atomic_store:
    case AtomicExpr::AO__opencl_atomic_exchange:
    case AtomicExpr::AO__hip_atomic_exchange:
    case AtomicExpr::AO__opencl_atomic_compare_exchange_weak:
    case AtomicExpr::AO__hip_atomic_compare_exchange_weak:
    case AtomicExpr::AO__opencl_atomic_compare_exchange_strong:
    case AtomicExpr::AO__atomic_load_n:
    case AtomicExpr::AO__atomic_store_n:
    case AtomicExpr::AO__atomic_exchange_n:
    case AtomicExpr::AO__atomic_compare_exchange_n:
      // Only use optimized library calls for sizes for which they exist.
      // FIXME: Size == 16 optimized library functions exist too.
      if (IsCheriCap || (Size == 1 || Size == 2 || Size == 4 || Size == 8))
        UseOptimizedLibcall = true;
      break;
    }

    CallArgList Args;
    if (!UseOptimizedLibcall) {
      // For non-optimized library calls, the size is the first parameter
      Args.add(RValue::get(llvm::ConstantInt::get(SizeTy, Size)),
               getContext().getSizeType());
    }
    // Atomic address is the first or second parameter
    // The OpenCL atomic library functions only accept pointer arguments to
    // generic address space.
    auto CastToGenericAddrSpace = [&](llvm::Value *V, QualType PT) {
      if (!E->isOpenCL())
        return V;
      auto AS = PT->castAs<PointerType>()->getPointeeType().getAddressSpace();
      if (AS == LangAS::opencl_generic)
        return V;
      auto DestAS = getContext().getTargetAddressSpace(LangAS::opencl_generic);
      auto *DestType = llvm::PointerType::get(getLLVMContext(), DestAS);

      return getTargetHooks().performAddrSpaceCast(
          *this, V, AS, LangAS::opencl_generic, DestType, false);
    };

    Args.add(RValue::get(CastToGenericAddrSpace(Ptr.getPointer(),
                                                E->getPtr()->getType())),
             getContext().VoidPtrTy);

    std::string LibCallName;
    QualType LoweredMemTy =
        IsCheriCap
            ? getContext().IntCapTy
            : (MemTy->isPointerType() ? getContext().getIntPtrType() : MemTy);
    QualType RetTy;
    bool HaveRetTy = false;
    llvm::Instruction::BinaryOps PostOp = (llvm::Instruction::BinaryOps)0;
    bool PostOpMinMax = false;
    switch (E->getOp()) {
    case AtomicExpr::AO__c11_atomic_init:
    case AtomicExpr::AO__opencl_atomic_init:
      llvm_unreachable("Already handled!");

    // There is only one libcall for compare an exchange, because there is no
    // optimisation benefit possible from a libcall version of a weak compare
    // and exchange.
    // bool __atomic_compare_exchange(size_t size, void *mem, void *expected,
    //                                void *desired, int success, int failure)
    // bool __atomic_compare_exchange_N(T *mem, T *expected, T desired,
    //                                  int success, int failure)
    case AtomicExpr::AO__c11_atomic_compare_exchange_weak:
    case AtomicExpr::AO__c11_atomic_compare_exchange_strong:
    case AtomicExpr::AO__opencl_atomic_compare_exchange_weak:
    case AtomicExpr::AO__hip_atomic_compare_exchange_weak:
    case AtomicExpr::AO__opencl_atomic_compare_exchange_strong:
    case AtomicExpr::AO__hip_atomic_compare_exchange_strong:
    case AtomicExpr::AO__atomic_compare_exchange:
    case AtomicExpr::AO__atomic_compare_exchange_n:
      LibCallName = "__atomic_compare_exchange";
      RetTy = getContext().BoolTy;
      HaveRetTy = true;
      Args.add(RValue::get(CastToGenericAddrSpace(Val1.getPointer(),
                                                  E->getVal1()->getType())),
               getContext().VoidPtrTy);
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val2.getPointer(),
                        MemTy, E->getExprLoc(), TInfo.Width);
      Args.add(RValue::get(Order), getContext().IntTy);
      Order = OrderFail;
      break;
    // void __atomic_exchange(size_t size, void *mem, void *val, void *return,
    //                        int order)
    // T __atomic_exchange_N(T *mem, T val, int order)
    case AtomicExpr::AO__c11_atomic_exchange:
    case AtomicExpr::AO__opencl_atomic_exchange:
    case AtomicExpr::AO__atomic_exchange_n:
    case AtomicExpr::AO__atomic_exchange:
    case AtomicExpr::AO__hip_atomic_exchange:
      LibCallName = "__atomic_exchange";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        MemTy, E->getExprLoc(), TInfo.Width);
      break;
    // void __atomic_store(size_t size, void *mem, void *val, int order)
    // void __atomic_store_N(T *mem, T val, int order)
    case AtomicExpr::AO__c11_atomic_store:
    case AtomicExpr::AO__opencl_atomic_store:
    case AtomicExpr::AO__hip_atomic_store:
    case AtomicExpr::AO__atomic_store:
    case AtomicExpr::AO__atomic_store_n:
      LibCallName = "__atomic_store";
      RetTy = getContext().VoidTy;
      HaveRetTy = true;
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        MemTy, E->getExprLoc(), TInfo.Width);
      break;
    // void __atomic_load(size_t size, void *mem, void *return, int order)
    // T __atomic_load_N(T *mem, int order)
    case AtomicExpr::AO__c11_atomic_load:
    case AtomicExpr::AO__opencl_atomic_load:
    case AtomicExpr::AO__hip_atomic_load:
    case AtomicExpr::AO__atomic_load:
    case AtomicExpr::AO__atomic_load_n:
      LibCallName = "__atomic_load";
      break;
    // T __atomic_add_fetch_N(T *mem, T val, int order)
    // T __atomic_fetch_add_N(T *mem, T val, int order)
    case AtomicExpr::AO__atomic_add_fetch:
      PostOp = llvm::Instruction::Add;
      [[fallthrough]];
    case AtomicExpr::AO__c11_atomic_fetch_add:
    case AtomicExpr::AO__opencl_atomic_fetch_add:
    case AtomicExpr::AO__atomic_fetch_add:
    case AtomicExpr::AO__hip_atomic_fetch_add:
      LibCallName = "__atomic_fetch_add";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        LoweredMemTy, E->getExprLoc(), TInfo.Width);
      break;
    // T __atomic_and_fetch_N(T *mem, T val, int order)
    // T __atomic_fetch_and_N(T *mem, T val, int order)
    case AtomicExpr::AO__atomic_and_fetch:
      PostOp = llvm::Instruction::And;
      [[fallthrough]];
    case AtomicExpr::AO__c11_atomic_fetch_and:
    case AtomicExpr::AO__opencl_atomic_fetch_and:
    case AtomicExpr::AO__hip_atomic_fetch_and:
    case AtomicExpr::AO__atomic_fetch_and:
      LibCallName = "__atomic_fetch_and";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        MemTy, E->getExprLoc(), TInfo.Width);
      break;
    // T __atomic_or_fetch_N(T *mem, T val, int order)
    // T __atomic_fetch_or_N(T *mem, T val, int order)
    case AtomicExpr::AO__atomic_or_fetch:
      PostOp = llvm::Instruction::Or;
      [[fallthrough]];
    case AtomicExpr::AO__c11_atomic_fetch_or:
    case AtomicExpr::AO__opencl_atomic_fetch_or:
    case AtomicExpr::AO__hip_atomic_fetch_or:
    case AtomicExpr::AO__atomic_fetch_or:
      LibCallName = "__atomic_fetch_or";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        MemTy, E->getExprLoc(), TInfo.Width);
      break;
    // T __atomic_sub_fetch_N(T *mem, T val, int order)
    // T __atomic_fetch_sub_N(T *mem, T val, int order)
    case AtomicExpr::AO__atomic_sub_fetch:
      PostOp = llvm::Instruction::Sub;
      [[fallthrough]];
    case AtomicExpr::AO__c11_atomic_fetch_sub:
    case AtomicExpr::AO__opencl_atomic_fetch_sub:
    case AtomicExpr::AO__hip_atomic_fetch_sub:
    case AtomicExpr::AO__atomic_fetch_sub:
      LibCallName = "__atomic_fetch_sub";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        LoweredMemTy, E->getExprLoc(), TInfo.Width);
      break;
    // T __atomic_xor_fetch_N(T *mem, T val, int order)
    // T __atomic_fetch_xor_N(T *mem, T val, int order)
    case AtomicExpr::AO__atomic_xor_fetch:
      PostOp = llvm::Instruction::Xor;
      [[fallthrough]];
    case AtomicExpr::AO__c11_atomic_fetch_xor:
    case AtomicExpr::AO__opencl_atomic_fetch_xor:
    case AtomicExpr::AO__hip_atomic_fetch_xor:
    case AtomicExpr::AO__atomic_fetch_xor:
      LibCallName = "__atomic_fetch_xor";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        MemTy, E->getExprLoc(), TInfo.Width);
      break;
    case AtomicExpr::AO__atomic_min_fetch:
      PostOpMinMax = true;
      [[fallthrough]];
    case AtomicExpr::AO__c11_atomic_fetch_min:
    case AtomicExpr::AO__atomic_fetch_min:
    case AtomicExpr::AO__hip_atomic_fetch_min:
    case AtomicExpr::AO__opencl_atomic_fetch_min:
      LibCallName = E->getValueType()->isSignedIntegerType()
                        ? "__atomic_fetch_min"
                        : "__atomic_fetch_umin";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        LoweredMemTy, E->getExprLoc(), TInfo.Width);
      break;
    case AtomicExpr::AO__atomic_max_fetch:
      PostOpMinMax = true;
      [[fallthrough]];
    case AtomicExpr::AO__c11_atomic_fetch_max:
    case AtomicExpr::AO__atomic_fetch_max:
    case AtomicExpr::AO__hip_atomic_fetch_max:
    case AtomicExpr::AO__opencl_atomic_fetch_max:
      LibCallName = E->getValueType()->isSignedIntegerType()
                        ? "__atomic_fetch_max"
                        : "__atomic_fetch_umax";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        LoweredMemTy, E->getExprLoc(), TInfo.Width);
      break;
    // T __atomic_nand_fetch_N(T *mem, T val, int order)
    // T __atomic_fetch_nand_N(T *mem, T val, int order)
    case AtomicExpr::AO__atomic_nand_fetch:
      PostOp = llvm::Instruction::And; // the NOT is special cased below
      [[fallthrough]];
    case AtomicExpr::AO__c11_atomic_fetch_nand:
    case AtomicExpr::AO__atomic_fetch_nand:
      LibCallName = "__atomic_fetch_nand";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1.getPointer(),
                        MemTy, E->getExprLoc(), TInfo.Width);
      break;
    }

    if (E->isOpenCL()) {
      LibCallName = std::string("__opencl") +
          StringRef(LibCallName).drop_front(1).str();

    }
    // Optimized functions have the size in their name.
    // Libcalls operating on capabilities cannot use the the _8/_16 versions
    // since those do not know about capability tags and would result in
    // incorrect values being stored. We therefore call a different libcall that
    // uses _cap as the suffix.
    if (UseOptimizedLibcall)
      LibCallName += IsCheriCap ? "_cap" : ("_" + llvm::utostr(Size));
    // By default, assume we return a value of the atomic type.
    if (!HaveRetTy) {
      if (UseOptimizedLibcall) {
        // Value is returned directly.
        // The function returns an appropriately sized integer type.
        RetTy = IsCheriCap
                    ? getContext().UnsignedIntCapTy
                    : getContext().getIntTypeForBitwidth(
                          getContext().toBits(TInfo.Width), /*Signed=*/false);
      } else {
        // Value is returned through parameter before the order.
        RetTy = getContext().VoidTy;
        Args.add(RValue::get(Dest.getPointer()), getContext().VoidPtrTy);
      }
    }
    // order is always the last parameter
    Args.add(RValue::get(Order),
             getContext().IntTy);
    if (E->isOpenCL())
      Args.add(RValue::get(Scope), getContext().IntTy);

    // PostOp is only needed for the atomic_*_fetch operations, and
    // thus is only needed for and implemented in the
    // UseOptimizedLibcall codepath.
    assert(UseOptimizedLibcall || (!PostOp && !PostOpMinMax));

    RValue Res = emitAtomicLibcall(*this, LibCallName, RetTy, Args);
    // The value is returned directly from the libcall.
    if (E->isCmpXChg())
      return Res;

    // The value is returned directly for optimized libcalls but the expr
    // provided an out-param.
    if (UseOptimizedLibcall && Res.getScalarVal()) {
      llvm::Value *ResVal = Res.getScalarVal();
      if (PostOpMinMax) {
        llvm::Value *LoadVal1 = Args[1].getRValue(*this).getScalarVal();
        ResVal = EmitPostAtomicMinMax(Builder, E->getOp(),
                                      E->getValueType()->isSignedIntegerType(),
                                      ResVal, LoadVal1);
      } else if (PostOp) {
        llvm::Value *LoadVal1 = Args[1].getRValue(*this).getScalarVal();
        if (IsCheriCap) {
          ResVal = getCapabilityIntegerValue(ResVal);
          LoadVal1 = getCapabilityIntegerValue(LoadVal1);
        }
        ResVal = Builder.CreateBinOp(PostOp, ResVal, LoadVal1);
      }
      if (E->getOp() == AtomicExpr::AO__atomic_nand_fetch)
        ResVal = Builder.CreateNot(ResVal);
      if (IsCheriCap && !PostOpMinMax && PostOp)
        ResVal = setCapabilityIntegerValue(Res.getScalarVal(), ResVal,
                                           E->getExprLoc());

      Builder.CreateStore(ResVal, Dest.withElementType(ResVal->getType()));
    }

    if (RValTy->isVoidType())
      return RValue::get(nullptr);

    return convertTempToRValue(Dest.withElementType(ConvertTypeForMem(RValTy)),
                               RValTy, E->getExprLoc());
  }

  if (isa<llvm::ConstantInt>(Order)) {
    auto ord = cast<llvm::ConstantInt>(Order)->getZExtValue();
    // We should not ever get to a case where the ordering isn't a valid C ABI
    // value, but it's hard to enforce that in general.
    if (llvm::isValidAtomicOrderingCABI(ord))
      switch ((llvm::AtomicOrderingCABI)ord) {
      case llvm::AtomicOrderingCABI::relaxed:
        EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
                     llvm::AtomicOrdering::Monotonic, Scope);
        break;
      case llvm::AtomicOrderingCABI::consume:
      case llvm::AtomicOrderingCABI::acquire:
        if (IsStore)
          break; // Avoid crashing on code with undefined behavior
        EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
                     llvm::AtomicOrdering::Acquire, Scope);
        break;
      case llvm::AtomicOrderingCABI::release:
        if (IsLoad)
          break; // Avoid crashing on code with undefined behavior
        EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
                     llvm::AtomicOrdering::Release, Scope);
        break;
      case llvm::AtomicOrderingCABI::acq_rel:
        if (IsLoad || IsStore)
          break; // Avoid crashing on code with undefined behavior
        EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
                     llvm::AtomicOrdering::AcquireRelease, Scope);
        break;
      case llvm::AtomicOrderingCABI::seq_cst:
        EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
                     llvm::AtomicOrdering::SequentiallyConsistent, Scope);
        break;
      }
    if (RValTy->isVoidType())
      return RValue::get(nullptr);

    return convertTempToRValue(Dest.withElementType(ConvertTypeForMem(RValTy)),
                               RValTy, E->getExprLoc());
  }

  // Long case, when Order isn't obviously constant.

  // Create all the relevant BB's
  llvm::BasicBlock *MonotonicBB = nullptr, *AcquireBB = nullptr,
                   *ReleaseBB = nullptr, *AcqRelBB = nullptr,
                   *SeqCstBB = nullptr;
  MonotonicBB = createBasicBlock("monotonic", CurFn);
  if (!IsStore)
    AcquireBB = createBasicBlock("acquire", CurFn);
  if (!IsLoad)
    ReleaseBB = createBasicBlock("release", CurFn);
  if (!IsLoad && !IsStore)
    AcqRelBB = createBasicBlock("acqrel", CurFn);
  SeqCstBB = createBasicBlock("seqcst", CurFn);
  llvm::BasicBlock *ContBB = createBasicBlock("atomic.continue", CurFn);

  // Create the switch for the split
  // MonotonicBB is arbitrarily chosen as the default case; in practice, this
  // doesn't matter unless someone is crazy enough to use something that
  // doesn't fold to a constant for the ordering.
  Order = Builder.CreateIntCast(Order, Builder.getInt32Ty(), false);
  llvm::SwitchInst *SI = Builder.CreateSwitch(Order, MonotonicBB);

  // Emit all the different atomics
  Builder.SetInsertPoint(MonotonicBB);
  EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
               llvm::AtomicOrdering::Monotonic, Scope);
  Builder.CreateBr(ContBB);
  if (!IsStore) {
    Builder.SetInsertPoint(AcquireBB);
    EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
                 llvm::AtomicOrdering::Acquire, Scope);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32((int)llvm::AtomicOrderingCABI::consume),
                AcquireBB);
    SI->addCase(Builder.getInt32((int)llvm::AtomicOrderingCABI::acquire),
                AcquireBB);
  }
  if (!IsLoad) {
    Builder.SetInsertPoint(ReleaseBB);
    EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
                 llvm::AtomicOrdering::Release, Scope);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32((int)llvm::AtomicOrderingCABI::release),
                ReleaseBB);
  }
  if (!IsLoad && !IsStore) {
    Builder.SetInsertPoint(AcqRelBB);
    EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
                 llvm::AtomicOrdering::AcquireRelease, Scope);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32((int)llvm::AtomicOrderingCABI::acq_rel),
                AcqRelBB);
  }
  Builder.SetInsertPoint(SeqCstBB);
  EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail, Size,
               llvm::AtomicOrdering::SequentiallyConsistent, Scope);
  Builder.CreateBr(ContBB);
  SI->addCase(Builder.getInt32((int)llvm::AtomicOrderingCABI::seq_cst),
              SeqCstBB);

  // Cleanup and return
  Builder.SetInsertPoint(ContBB);
  if (RValTy->isVoidType())
    return RValue::get(nullptr);

  assert(Atomics.getValueSizeInBits() <= Atomics.getAtomicSizeInBits());
  return convertTempToRValue(Dest.withElementType(ConvertTypeForMem(RValTy)),
                             RValTy, E->getExprLoc());
}

Address AtomicInfo::castToAtomicIntPointer(Address addr) const {
  llvm::Type *ty;
  if (AtomicTy->isCHERICapabilityType(CGF.getContext())) {
    // If capability atomics are natively supported the instruction expects
    // a capability type. We also pass capabilities directly to the atomic
    // libcalls (i.e. always use optimized ones) since this is required to
    // support the RMW operations and special-casing the load/store/xchg to
    // use the generic libcalls (with mutex+memcpy) adds unncessary complexity.
    if (!UseLibcall) {
      // If we aren't using a libcall there is no need to cast to i8*
      return addr.withElementType(getAtomicAddress().getElementType());
    }
    ty = CGF.CGM.Int8CheriCapTy;
  } else {
    ty = llvm::IntegerType::get(CGF.getLLVMContext(), AtomicSizeInBits);
  }
  return addr.withElementType(ty);
}

Address AtomicInfo::convertToAtomicIntPointer(Address Addr) const {
  llvm::Type *Ty = Addr.getElementType();
  // correctly convert fetch_add, etc arguments:
  if (AtomicTy->isCHERICapabilityType(CGF.getContext()) && Ty->isIntegerTy()) {
    llvm::Type *AtomicLLVMTy = CGF.ConvertTypeForMem(AtomicTy);
    auto AlignChars = CGF.getContext().getTypeAlignInChars(AtomicTy);
    auto *Tmp = CGF.CreateTempAlloca(AtomicLLVMTy, "atomic-intcap-arg");
    CGF.Builder.CreateAlignedStore(
        CGF.getNullDerivedCapability(AtomicLLVMTy,
                                     CGF.Builder.CreateLoad(Addr)),
        Tmp, AlignChars.getAsAlign());
    return Address(Tmp, AtomicLLVMTy, AlignChars);
  }
  uint64_t SourceSizeInBits = CGF.CGM.getDataLayout().getTypeSizeInBits(Ty);
  if (SourceSizeInBits != AtomicSizeInBits) {
    Address Tmp = CreateTempAlloca();
    CGF.Builder.CreateMemCpy(Tmp, Addr,
                             std::min(AtomicSizeInBits, SourceSizeInBits) / 8);
    Addr = Tmp;
  }

  return castToAtomicIntPointer(Addr);
}

RValue AtomicInfo::convertAtomicTempToRValue(Address addr,
                                             AggValueSlot resultSlot,
                                             SourceLocation loc,
                                             bool asValue) const {
  if (LVal.isSimple()) {
    if (EvaluationKind == TEK_Aggregate)
      return resultSlot.asRValue();

    // Drill into the padding structure if we have one.
    if (hasPadding())
      addr = CGF.Builder.CreateStructGEP(addr, 0);

    // Otherwise, just convert the temporary to an r-value using the
    // normal conversion routine.
    return CGF.convertTempToRValue(addr, getValueType(), loc);
  }
  if (!asValue)
    // Get RValue from temp memory as atomic for non-simple lvalues
    return RValue::get(CGF.Builder.CreateLoad(addr));
  if (LVal.isBitField())
    return CGF.EmitLoadOfBitfieldLValue(
        LValue::MakeBitfield(addr, LVal.getBitFieldInfo(), LVal.getType(),
                             LVal.getBaseInfo(), TBAAAccessInfo()), loc);
  if (LVal.isVectorElt())
    return CGF.EmitLoadOfLValue(
        LValue::MakeVectorElt(addr, LVal.getVectorIdx(), LVal.getType(),
                              LVal.getBaseInfo(), TBAAAccessInfo()), loc);
  assert(LVal.isExtVectorElt());
  return CGF.EmitLoadOfExtVectorElementLValue(LValue::MakeExtVectorElt(
      addr, LVal.getExtVectorElts(), LVal.getType(),
      LVal.getBaseInfo(), TBAAAccessInfo()));
}

RValue AtomicInfo::ConvertIntToValueOrAtomic(llvm::Value *IntVal,
                                             AggValueSlot ResultSlot,
                                             SourceLocation Loc,
                                             bool AsValue) const {
  if (AtomicTy->isCHERICapabilityType(CGF.getContext())) {
    auto *ValTy = AsValue ? CGF.ConvertTypeForMem(ValueTy)
                      : getAtomicAddress().getElementType();
    assert(ValTy->isPointerTy());
    return RValue::get(CGF.Builder.CreateBitCast(IntVal, ValTy));
  }

  // Try not to in some easy cases.
  assert(IntVal->getType()->isIntegerTy() && "Expected integer value");
  if (getEvaluationKind() == TEK_Scalar &&
      (((!LVal.isBitField() ||
         LVal.getBitFieldInfo().Size == ValueSizeInBits) &&
        !hasPadding()) ||
       !AsValue)) {
    auto *ValTy = AsValue
                      ? CGF.ConvertTypeForMem(ValueTy)
                      : getAtomicAddress().getElementType();
    if (ValTy->isIntegerTy()) {
      assert(IntVal->getType() == ValTy && "Different integer types.");
      return RValue::get(CGF.EmitFromMemory(IntVal, ValueTy));
    } else if (ValTy->isPointerTy())
      return RValue::get(CGF.Builder.CreateIntToPtr(IntVal, ValTy));
    else if (llvm::CastInst::isBitCastable(IntVal->getType(), ValTy))
      return RValue::get(CGF.Builder.CreateBitCast(IntVal, ValTy));
  }

  // Create a temporary.  This needs to be big enough to hold the
  // atomic integer.
  Address Temp = Address::invalid();
  bool TempIsVolatile = false;
  if (AsValue && getEvaluationKind() == TEK_Aggregate) {
    assert(!ResultSlot.isIgnored());
    Temp = ResultSlot.getAddress();
    TempIsVolatile = ResultSlot.isVolatile();
  } else {
    Temp = CreateTempAlloca();
  }

  // Slam the integer into the temporary.
  Address CastTemp = castToAtomicIntPointer(Temp);
  CGF.Builder.CreateStore(IntVal, CastTemp)
      ->setVolatile(TempIsVolatile);

  return convertAtomicTempToRValue(Temp, ResultSlot, Loc, AsValue);
}

void AtomicInfo::EmitAtomicLoadLibcall(llvm::Value *AddForLoaded,
                                       llvm::AtomicOrdering AO, bool) {
  // void __atomic_load(size_t size, void *mem, void *return, int order);
  CallArgList Args;
  Args.add(RValue::get(getAtomicSizeValue()), CGF.getContext().getSizeType());
  Args.add(RValue::get(getAtomicPointer()), CGF.getContext().VoidPtrTy);
  Args.add(RValue::get(AddForLoaded), CGF.getContext().VoidPtrTy);
  Args.add(
      RValue::get(llvm::ConstantInt::get(CGF.IntTy, (int)llvm::toCABI(AO))),
      CGF.getContext().IntTy);
  emitAtomicLibcall(CGF, "__atomic_load", CGF.getContext().VoidTy, Args);
}

llvm::Value *AtomicInfo::EmitAtomicLoadOp(llvm::AtomicOrdering AO,
                                          bool IsVolatile) {
  // Okay, we're doing this natively.
  Address Addr = getAtomicAddressAsAtomicIntPointer();
  llvm::LoadInst *Load = CGF.Builder.CreateLoad(Addr, "atomic-load");
  Load->setAtomic(AO);

  // Other decoration.
  if (IsVolatile)
    Load->setVolatile(true);
  CGF.CGM.DecorateInstructionWithTBAA(Load, LVal.getTBAAInfo());
  return Load;
}

/// An LValue is a candidate for having its loads and stores be made atomic if
/// we are operating under /volatile:ms *and* the LValue itself is volatile and
/// performing such an operation can be performed without a libcall.
bool CodeGenFunction::LValueIsSuitableForInlineAtomic(LValue LV) {
  if (!CGM.getLangOpts().MSVolatile) return false;
  AtomicInfo AI(*this, LV, AtomicExpr::AO__c11_atomic_load);
  bool IsVolatile = LV.isVolatile() || hasVolatileMember(LV.getType());
  // An atomic is inline if we don't need to use a libcall.
  bool AtomicIsInline = !AI.shouldUseLibcall();
  // MSVC doesn't seem to do this for types wider than a pointer.
  if (getContext().getTypeSize(LV.getType()) >
      getContext().getTypeSize(getContext().getIntPtrType()))
    return false;
  return IsVolatile && AtomicIsInline;
}

RValue CodeGenFunction::EmitAtomicLoad(LValue LV, SourceLocation SL,
                                       AggValueSlot Slot) {
  llvm::AtomicOrdering AO;
  bool IsVolatile = LV.isVolatileQualified();
  if (LV.getType()->isAtomicType()) {
    AO = llvm::AtomicOrdering::SequentiallyConsistent;
  } else {
    AO = llvm::AtomicOrdering::Acquire;
    IsVolatile = true;
  }
  return EmitAtomicLoad(LV, SL, AO, IsVolatile, Slot);
}

RValue AtomicInfo::EmitAtomicLoad(AggValueSlot ResultSlot, SourceLocation Loc,
                                  bool AsValue, llvm::AtomicOrdering AO,
                                  bool IsVolatile) {
  // Check whether we should use a library call.
  if (shouldUseLibcall()) {
    Address TempAddr = Address::invalid();
    if (LVal.isSimple() && !ResultSlot.isIgnored()) {
      assert(getEvaluationKind() == TEK_Aggregate);
      TempAddr = ResultSlot.getAddress();
    } else
      TempAddr = CreateTempAlloca();

    EmitAtomicLoadLibcall(TempAddr.getPointer(), AO, IsVolatile);

    // Okay, turn that back into the original value or whole atomic (for
    // non-simple lvalues) type.
    return convertAtomicTempToRValue(TempAddr, ResultSlot, Loc, AsValue);
  }

  // Okay, we're doing this natively.
  auto *Load = EmitAtomicLoadOp(AO, IsVolatile);

  // If we're ignoring an aggregate return, don't do anything.
  if (getEvaluationKind() == TEK_Aggregate && ResultSlot.isIgnored())
    return RValue::getAggregate(Address::invalid(), false);

  // Okay, turn that back into the original value or atomic (for non-simple
  // lvalues) type.
  return ConvertIntToValueOrAtomic(Load, ResultSlot, Loc, AsValue);
}

/// Emit a load from an l-value of atomic type.  Note that the r-value
/// we produce is an r-value of the atomic *value* type.
RValue CodeGenFunction::EmitAtomicLoad(LValue src, SourceLocation loc,
                                       llvm::AtomicOrdering AO, bool IsVolatile,
                                       AggValueSlot resultSlot) {
  AtomicInfo Atomics(*this, src, AtomicExpr::AO__c11_atomic_load);
  return Atomics.EmitAtomicLoad(resultSlot, loc, /*AsValue=*/true, AO,
                                IsVolatile);
}

/// Copy an r-value into memory as part of storing to an atomic type.
/// This needs to create a bit-pattern suitable for atomic operations.
void AtomicInfo::emitCopyIntoMemory(RValue rvalue) const {
  assert(LVal.isSimple());
  // If we have an r-value, the rvalue should be of the atomic type,
  // which means that the caller is responsible for having zeroed
  // any padding.  Just do an aggregate copy of that type.
  if (rvalue.isAggregate()) {
    LValue Dest = CGF.MakeAddrLValue(getAtomicAddress(), getAtomicType());
    LValue Src = CGF.MakeAddrLValue(rvalue.getAggregateAddress(),
                                    getAtomicType());
    bool IsVolatile = rvalue.isVolatileQualified() ||
                      LVal.isVolatileQualified();
    CGF.EmitAggregateCopy(Dest, Src, getAtomicType(),
                          AggValueSlot::DoesNotOverlap, IsVolatile);
    return;
  }

  // Okay, otherwise we're copying stuff.

  // Zero out the buffer if necessary.
  emitMemSetZeroIfNecessary();

  // Drill past the padding if present.
  LValue TempLVal = projectValue();

  // Okay, store the rvalue in.
  if (rvalue.isScalar()) {
    CGF.EmitStoreOfScalar(rvalue.getScalarVal(), TempLVal, /*init*/ true);
  } else {
    CGF.EmitStoreOfComplex(rvalue.getComplexVal(), TempLVal, /*init*/ true);
  }
}


/// Materialize an r-value into memory for the purposes of storing it
/// to an atomic type.
Address AtomicInfo::materializeRValue(RValue rvalue) const {
  // Aggregate r-values are already in memory, and EmitAtomicStore
  // requires them to be values of the atomic type.
  if (rvalue.isAggregate())
    return rvalue.getAggregateAddress();

  // Otherwise, make a temporary and materialize into it.
  LValue TempLV = CGF.MakeAddrLValue(CreateTempAlloca(), getAtomicType());
  AtomicInfo Atomics(CGF, TempLV, AtomicExpr::AO__c11_atomic_store);
  Atomics.emitCopyIntoMemory(rvalue);
  return TempLV.getAddress(CGF);
}

llvm::Value *AtomicInfo::convertRValueToInt(RValue RVal) const {
  // If we've got a scalar value of the right size, try to avoid going
  // through memory.
  if (RVal.isScalar() && (!hasPadding() || !LVal.isSimple())) {
    llvm::Value *Value = RVal.getScalarVal();
    if (isa<llvm::IntegerType>(Value->getType()))
      return CGF.EmitToMemory(Value, ValueTy);
    else {
      if (AtomicTy->isCHERICapabilityType(CGF.getContext())) {
        // Cast to the atomic ptr element type for CHERI capabilities
        return CGF.Builder.CreateBitCast(Value,
                                         LVal.getAddress(CGF).getElementType());
      }
      llvm::IntegerType *InputIntTy = llvm::IntegerType::get(
          CGF.getLLVMContext(),
          LVal.isSimple() ? getValueSizeInBits() : getAtomicSizeInBits());
      if (isa<llvm::PointerType>(Value->getType()))
        return CGF.Builder.CreatePtrToInt(Value, InputIntTy);
      else if (llvm::BitCastInst::isBitCastable(Value->getType(), InputIntTy))
        return CGF.Builder.CreateBitCast(Value, InputIntTy);
    }
  }
  // Otherwise, we need to go through memory.
  // Put the r-value in memory.
  Address Addr = materializeRValue(RVal);

  // Cast the temporary to the atomic int type and pull a value out.
  Addr = castToAtomicIntPointer(Addr);
  return CGF.Builder.CreateLoad(Addr);
}

std::pair<llvm::Value *, llvm::Value *> AtomicInfo::EmitAtomicCompareExchangeOp(
    llvm::Value *ExpectedVal, llvm::Value *DesiredVal,
    llvm::AtomicOrdering Success, llvm::AtomicOrdering Failure, bool IsWeak) {
  // Do the atomic store.
  Address Addr = getAtomicAddressAsAtomicIntPointer();
  auto *Inst = CGF.Builder.CreateAtomicCmpXchg(Addr.getPointer(),
                                               ExpectedVal, DesiredVal,
                                               Success, Failure);
  // Other decoration.
  Inst->setVolatile(LVal.isVolatileQualified());
  Inst->setWeak(IsWeak);

  // Okay, turn that back into the original value type.
  auto *PreviousVal = CGF.Builder.CreateExtractValue(Inst, /*Idxs=*/0);
  auto *SuccessFailureVal = CGF.Builder.CreateExtractValue(Inst, /*Idxs=*/1);
  return std::make_pair(PreviousVal, SuccessFailureVal);
}

llvm::Value *
AtomicInfo::EmitAtomicCompareExchangeLibcall(llvm::Value *ExpectedAddr,
                                             llvm::Value *DesiredAddr,
                                             llvm::AtomicOrdering Success,
                                             llvm::AtomicOrdering Failure) {
  // bool __atomic_compare_exchange(size_t size, void *obj, void *expected,
  // void *desired, int success, int failure);
  CallArgList Args;
  Args.add(RValue::get(getAtomicSizeValue()), CGF.getContext().getSizeType());
  Args.add(RValue::get(getAtomicPointer()), CGF.getContext().VoidPtrTy);
  Args.add(RValue::get(ExpectedAddr), CGF.getContext().VoidPtrTy);
  Args.add(RValue::get(DesiredAddr), CGF.getContext().VoidPtrTy);
  Args.add(RValue::get(
               llvm::ConstantInt::get(CGF.IntTy, (int)llvm::toCABI(Success))),
           CGF.getContext().IntTy);
  Args.add(RValue::get(
               llvm::ConstantInt::get(CGF.IntTy, (int)llvm::toCABI(Failure))),
           CGF.getContext().IntTy);
  auto SuccessFailureRVal = emitAtomicLibcall(CGF, "__atomic_compare_exchange",
                                              CGF.getContext().BoolTy, Args);

  return SuccessFailureRVal.getScalarVal();
}

std::pair<RValue, llvm::Value *> AtomicInfo::EmitAtomicCompareExchange(
    RValue Expected, RValue Desired, llvm::AtomicOrdering Success,
    llvm::AtomicOrdering Failure, bool IsWeak) {
  // Check whether we should use a library call.
  if (shouldUseLibcall()) {
    // Produce a source address.
    Address ExpectedAddr = materializeRValue(Expected);
    Address DesiredAddr = materializeRValue(Desired);
    auto *Res = EmitAtomicCompareExchangeLibcall(ExpectedAddr.getPointer(),
                                                 DesiredAddr.getPointer(),
                                                 Success, Failure);
    return std::make_pair(
        convertAtomicTempToRValue(ExpectedAddr, AggValueSlot::ignored(),
                                  SourceLocation(), /*AsValue=*/false),
        Res);
  }

  // If we've got a scalar value of the right size, try to avoid going
  // through memory.
  auto *ExpectedVal = convertRValueToInt(Expected);
  auto *DesiredVal = convertRValueToInt(Desired);
  auto Res = EmitAtomicCompareExchangeOp(ExpectedVal, DesiredVal, Success,
                                         Failure, IsWeak);
  return std::make_pair(
      ConvertIntToValueOrAtomic(Res.first, AggValueSlot::ignored(),
                                SourceLocation(), /*AsValue=*/false),
      Res.second);
}

static void
EmitAtomicUpdateValue(CodeGenFunction &CGF, AtomicInfo &Atomics, RValue OldRVal,
                      const llvm::function_ref<RValue(RValue)> &UpdateOp,
                      Address DesiredAddr) {
  RValue UpRVal;
  LValue AtomicLVal = Atomics.getAtomicLValue();
  LValue DesiredLVal;
  if (AtomicLVal.isSimple()) {
    UpRVal = OldRVal;
    DesiredLVal = CGF.MakeAddrLValue(DesiredAddr, AtomicLVal.getType());
  } else {
    // Build new lvalue for temp address.
    Address Ptr = Atomics.materializeRValue(OldRVal);
    LValue UpdateLVal;
    if (AtomicLVal.isBitField()) {
      UpdateLVal =
          LValue::MakeBitfield(Ptr, AtomicLVal.getBitFieldInfo(),
                               AtomicLVal.getType(),
                               AtomicLVal.getBaseInfo(),
                               AtomicLVal.getTBAAInfo());
      DesiredLVal =
          LValue::MakeBitfield(DesiredAddr, AtomicLVal.getBitFieldInfo(),
                               AtomicLVal.getType(), AtomicLVal.getBaseInfo(),
                               AtomicLVal.getTBAAInfo());
    } else if (AtomicLVal.isVectorElt()) {
      UpdateLVal = LValue::MakeVectorElt(Ptr, AtomicLVal.getVectorIdx(),
                                         AtomicLVal.getType(),
                                         AtomicLVal.getBaseInfo(),
                                         AtomicLVal.getTBAAInfo());
      DesiredLVal = LValue::MakeVectorElt(
          DesiredAddr, AtomicLVal.getVectorIdx(), AtomicLVal.getType(),
          AtomicLVal.getBaseInfo(), AtomicLVal.getTBAAInfo());
    } else {
      assert(AtomicLVal.isExtVectorElt());
      UpdateLVal = LValue::MakeExtVectorElt(Ptr, AtomicLVal.getExtVectorElts(),
                                            AtomicLVal.getType(),
                                            AtomicLVal.getBaseInfo(),
                                            AtomicLVal.getTBAAInfo());
      DesiredLVal = LValue::MakeExtVectorElt(
          DesiredAddr, AtomicLVal.getExtVectorElts(), AtomicLVal.getType(),
          AtomicLVal.getBaseInfo(), AtomicLVal.getTBAAInfo());
    }
    UpRVal = CGF.EmitLoadOfLValue(UpdateLVal, SourceLocation());
  }
  // Store new value in the corresponding memory area.
  RValue NewRVal = UpdateOp(UpRVal);
  if (NewRVal.isScalar()) {
    CGF.EmitStoreThroughLValue(NewRVal, DesiredLVal);
  } else {
    assert(NewRVal.isComplex());
    CGF.EmitStoreOfComplex(NewRVal.getComplexVal(), DesiredLVal,
                           /*isInit=*/false);
  }
}

void AtomicInfo::EmitAtomicUpdateLibcall(
    llvm::AtomicOrdering AO, const llvm::function_ref<RValue(RValue)> &UpdateOp,
    bool IsVolatile) {
  auto Failure = llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(AO);

  Address ExpectedAddr = CreateTempAlloca();

  EmitAtomicLoadLibcall(ExpectedAddr.getPointer(), AO, IsVolatile);
  auto *ContBB = CGF.createBasicBlock("atomic_cont");
  auto *ExitBB = CGF.createBasicBlock("atomic_exit");
  CGF.EmitBlock(ContBB);
  Address DesiredAddr = CreateTempAlloca();
  if ((LVal.isBitField() && BFI.Size != ValueSizeInBits) ||
      requiresMemSetZero(getAtomicAddress().getElementType())) {
    auto *OldVal = CGF.Builder.CreateLoad(ExpectedAddr);
    CGF.Builder.CreateStore(OldVal, DesiredAddr);
  }
  auto OldRVal = convertAtomicTempToRValue(ExpectedAddr,
                                           AggValueSlot::ignored(),
                                           SourceLocation(), /*AsValue=*/false);
  EmitAtomicUpdateValue(CGF, *this, OldRVal, UpdateOp, DesiredAddr);
  auto *Res =
      EmitAtomicCompareExchangeLibcall(ExpectedAddr.getPointer(),
                                       DesiredAddr.getPointer(),
                                       AO, Failure);
  CGF.Builder.CreateCondBr(Res, ExitBB, ContBB);
  CGF.EmitBlock(ExitBB, /*IsFinished=*/true);
}

void AtomicInfo::EmitAtomicUpdateOp(
    llvm::AtomicOrdering AO, const llvm::function_ref<RValue(RValue)> &UpdateOp,
    bool IsVolatile) {
  auto Failure = llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(AO);

  // Do the atomic load.
  auto *OldVal = EmitAtomicLoadOp(Failure, IsVolatile);
  // For non-simple lvalues perform compare-and-swap procedure.
  auto *ContBB = CGF.createBasicBlock("atomic_cont");
  auto *ExitBB = CGF.createBasicBlock("atomic_exit");
  auto *CurBB = CGF.Builder.GetInsertBlock();
  CGF.EmitBlock(ContBB);
  llvm::PHINode *PHI = CGF.Builder.CreatePHI(OldVal->getType(),
                                             /*NumReservedValues=*/2);
  PHI->addIncoming(OldVal, CurBB);
  Address NewAtomicAddr = CreateTempAlloca();
  Address NewAtomicIntAddr = castToAtomicIntPointer(NewAtomicAddr);
  if ((LVal.isBitField() && BFI.Size != ValueSizeInBits) ||
      requiresMemSetZero(getAtomicAddress().getElementType())) {
    CGF.Builder.CreateStore(PHI, NewAtomicIntAddr);
  }
  auto OldRVal = ConvertIntToValueOrAtomic(PHI, AggValueSlot::ignored(),
                                           SourceLocation(), /*AsValue=*/false);
  EmitAtomicUpdateValue(CGF, *this, OldRVal, UpdateOp, NewAtomicAddr);
  auto *DesiredVal = CGF.Builder.CreateLoad(NewAtomicIntAddr);
  // Try to write new value using cmpxchg operation.
  auto Res = EmitAtomicCompareExchangeOp(PHI, DesiredVal, AO, Failure);
  PHI->addIncoming(Res.first, CGF.Builder.GetInsertBlock());
  CGF.Builder.CreateCondBr(Res.second, ExitBB, ContBB);
  CGF.EmitBlock(ExitBB, /*IsFinished=*/true);
}

static void EmitAtomicUpdateValue(CodeGenFunction &CGF, AtomicInfo &Atomics,
                                  RValue UpdateRVal, Address DesiredAddr) {
  LValue AtomicLVal = Atomics.getAtomicLValue();
  LValue DesiredLVal;
  // Build new lvalue for temp address.
  if (AtomicLVal.isBitField()) {
    DesiredLVal =
        LValue::MakeBitfield(DesiredAddr, AtomicLVal.getBitFieldInfo(),
                             AtomicLVal.getType(), AtomicLVal.getBaseInfo(),
                             AtomicLVal.getTBAAInfo());
  } else if (AtomicLVal.isVectorElt()) {
    DesiredLVal =
        LValue::MakeVectorElt(DesiredAddr, AtomicLVal.getVectorIdx(),
                              AtomicLVal.getType(), AtomicLVal.getBaseInfo(),
                              AtomicLVal.getTBAAInfo());
  } else {
    assert(AtomicLVal.isExtVectorElt());
    DesiredLVal = LValue::MakeExtVectorElt(
        DesiredAddr, AtomicLVal.getExtVectorElts(), AtomicLVal.getType(),
        AtomicLVal.getBaseInfo(), AtomicLVal.getTBAAInfo());
  }
  // Store new value in the corresponding memory area.
  assert(UpdateRVal.isScalar());
  CGF.EmitStoreThroughLValue(UpdateRVal, DesiredLVal);
}

void AtomicInfo::EmitAtomicUpdateLibcall(llvm::AtomicOrdering AO,
                                         RValue UpdateRVal, bool IsVolatile) {
  auto Failure = llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(AO);

  Address ExpectedAddr = CreateTempAlloca();

  EmitAtomicLoadLibcall(ExpectedAddr.getPointer(), AO, IsVolatile);
  auto *ContBB = CGF.createBasicBlock("atomic_cont");
  auto *ExitBB = CGF.createBasicBlock("atomic_exit");
  CGF.EmitBlock(ContBB);
  Address DesiredAddr = CreateTempAlloca();
  if ((LVal.isBitField() && BFI.Size != ValueSizeInBits) ||
      requiresMemSetZero(getAtomicAddress().getElementType())) {
    auto *OldVal = CGF.Builder.CreateLoad(ExpectedAddr);
    CGF.Builder.CreateStore(OldVal, DesiredAddr);
  }
  EmitAtomicUpdateValue(CGF, *this, UpdateRVal, DesiredAddr);
  auto *Res =
      EmitAtomicCompareExchangeLibcall(ExpectedAddr.getPointer(),
                                       DesiredAddr.getPointer(),
                                       AO, Failure);
  CGF.Builder.CreateCondBr(Res, ExitBB, ContBB);
  CGF.EmitBlock(ExitBB, /*IsFinished=*/true);
}

void AtomicInfo::EmitAtomicUpdateOp(llvm::AtomicOrdering AO, RValue UpdateRVal,
                                    bool IsVolatile) {
  auto Failure = llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(AO);

  // Do the atomic load.
  auto *OldVal = EmitAtomicLoadOp(Failure, IsVolatile);
  // For non-simple lvalues perform compare-and-swap procedure.
  auto *ContBB = CGF.createBasicBlock("atomic_cont");
  auto *ExitBB = CGF.createBasicBlock("atomic_exit");
  auto *CurBB = CGF.Builder.GetInsertBlock();
  CGF.EmitBlock(ContBB);
  llvm::PHINode *PHI = CGF.Builder.CreatePHI(OldVal->getType(),
                                             /*NumReservedValues=*/2);
  PHI->addIncoming(OldVal, CurBB);
  Address NewAtomicAddr = CreateTempAlloca();
  Address NewAtomicIntAddr = castToAtomicIntPointer(NewAtomicAddr);
  if ((LVal.isBitField() && BFI.Size != ValueSizeInBits) ||
      requiresMemSetZero(getAtomicAddress().getElementType())) {
    CGF.Builder.CreateStore(PHI, NewAtomicIntAddr);
  }
  EmitAtomicUpdateValue(CGF, *this, UpdateRVal, NewAtomicAddr);
  auto *DesiredVal = CGF.Builder.CreateLoad(NewAtomicIntAddr);
  // Try to write new value using cmpxchg operation.
  auto Res = EmitAtomicCompareExchangeOp(PHI, DesiredVal, AO, Failure);
  PHI->addIncoming(Res.first, CGF.Builder.GetInsertBlock());
  CGF.Builder.CreateCondBr(Res.second, ExitBB, ContBB);
  CGF.EmitBlock(ExitBB, /*IsFinished=*/true);
}

void AtomicInfo::EmitAtomicUpdate(
    llvm::AtomicOrdering AO, const llvm::function_ref<RValue(RValue)> &UpdateOp,
    bool IsVolatile) {
  if (shouldUseLibcall()) {
    EmitAtomicUpdateLibcall(AO, UpdateOp, IsVolatile);
  } else {
    EmitAtomicUpdateOp(AO, UpdateOp, IsVolatile);
  }
}

void AtomicInfo::EmitAtomicUpdate(llvm::AtomicOrdering AO, RValue UpdateRVal,
                                  bool IsVolatile) {
  if (shouldUseLibcall()) {
    EmitAtomicUpdateLibcall(AO, UpdateRVal, IsVolatile);
  } else {
    EmitAtomicUpdateOp(AO, UpdateRVal, IsVolatile);
  }
}

void CodeGenFunction::EmitAtomicStore(RValue rvalue, LValue lvalue,
                                      bool isInit) {
  bool IsVolatile = lvalue.isVolatileQualified();
  llvm::AtomicOrdering AO;
  if (lvalue.getType()->isAtomicType()) {
    AO = llvm::AtomicOrdering::SequentiallyConsistent;
  } else {
    AO = llvm::AtomicOrdering::Release;
    IsVolatile = true;
  }
  return EmitAtomicStore(rvalue, lvalue, AO, IsVolatile, isInit);
}

/// Emit a store to an l-value of atomic type.
///
/// Note that the r-value is expected to be an r-value *of the atomic
/// type*; this means that for aggregate r-values, it should include
/// storage for any padding that was necessary.
void CodeGenFunction::EmitAtomicStore(RValue rvalue, LValue dest,
                                      llvm::AtomicOrdering AO, bool IsVolatile,
                                      bool isInit) {
  // If this is an aggregate r-value, it should agree in type except
  // maybe for address-space qualification.
  assert(!rvalue.isAggregate() ||
         rvalue.getAggregateAddress().getElementType() ==
             dest.getAddress(*this).getElementType());

  AtomicInfo atomics(*this, dest, AtomicExpr::AO__c11_atomic_store);
  LValue LVal = atomics.getAtomicLValue();

  // If this is an initialization, just put the value there normally.
  if (LVal.isSimple()) {
    if (isInit) {
      atomics.emitCopyIntoMemory(rvalue);
      return;
    }

    // Check whether we should use a library call.
    if (atomics.shouldUseLibcall()) {
      // Produce a source address.
      Address srcAddr = atomics.materializeRValue(rvalue);

      // void __atomic_store(size_t size, void *mem, void *val, int order)
      CallArgList args;
      args.add(RValue::get(atomics.getAtomicSizeValue()),
               getContext().getSizeType());
      args.add(RValue::get(atomics.getAtomicPointer()), getContext().VoidPtrTy);
      args.add(RValue::get(srcAddr.getPointer()), getContext().VoidPtrTy);
      args.add(
          RValue::get(llvm::ConstantInt::get(IntTy, (int)llvm::toCABI(AO))),
          getContext().IntTy);
      emitAtomicLibcall(*this, "__atomic_store", getContext().VoidTy, args);
      return;
    }

    // Okay, we're doing this natively.
    llvm::Value *intValue = atomics.convertRValueToInt(rvalue);

    // Do the atomic store.
    Address addr = atomics.castToAtomicIntPointer(atomics.getAtomicAddress());
    intValue = Builder.CreateIntCast(
        intValue, addr.getElementType(), /*isSigned=*/false);
    llvm::StoreInst *store = Builder.CreateStore(intValue, addr);

    if (AO == llvm::AtomicOrdering::Acquire)
      AO = llvm::AtomicOrdering::Monotonic;
    else if (AO == llvm::AtomicOrdering::AcquireRelease)
      AO = llvm::AtomicOrdering::Release;
    // Initializations don't need to be atomic.
    if (!isInit)
      store->setAtomic(AO);

    // Other decoration.
    if (IsVolatile)
      store->setVolatile(true);
    CGM.DecorateInstructionWithTBAA(store, dest.getTBAAInfo());
    return;
  }

  // Emit simple atomic update operation.
  atomics.EmitAtomicUpdate(AO, rvalue, IsVolatile);
}

/// Emit a compare-and-exchange op for atomic type.
///
std::pair<RValue, llvm::Value *> CodeGenFunction::EmitAtomicCompareExchange(
    LValue Obj, RValue Expected, RValue Desired, SourceLocation Loc,
    llvm::AtomicOrdering Success, llvm::AtomicOrdering Failure, bool IsWeak,
    AggValueSlot Slot) {
  // If this is an aggregate r-value, it should agree in type except
  // maybe for address-space qualification.
  assert(!Expected.isAggregate() ||
         Expected.getAggregateAddress().getElementType() ==
             Obj.getAddress(*this).getElementType());
  assert(!Desired.isAggregate() ||
         Desired.getAggregateAddress().getElementType() ==
             Obj.getAddress(*this).getElementType());
  AtomicInfo Atomics(*this, Obj, AtomicExpr::AO__atomic_compare_exchange);

  return Atomics.EmitAtomicCompareExchange(Expected, Desired, Success, Failure,
                                           IsWeak);
}

void CodeGenFunction::EmitAtomicUpdate(
    LValue LVal, llvm::AtomicOrdering AO,
    const llvm::function_ref<RValue(RValue)> &UpdateOp, bool IsVolatile) {
  AtomicInfo Atomics(*this, LVal, AtomicExpr::AO__atomic_compare_exchange);
  Atomics.EmitAtomicUpdate(AO, UpdateOp, IsVolatile);
}

void CodeGenFunction::EmitAtomicInit(Expr *init, LValue dest) {
  AtomicInfo atomics(*this, dest, AtomicExpr::AO__c11_atomic_init);

  switch (atomics.getEvaluationKind()) {
  case TEK_Scalar: {
    llvm::Value *value = EmitScalarExpr(init);
    atomics.emitCopyIntoMemory(RValue::get(value));
    return;
  }

  case TEK_Complex: {
    ComplexPairTy value = EmitComplexExpr(init);
    atomics.emitCopyIntoMemory(RValue::getComplex(value));
    return;
  }

  case TEK_Aggregate: {
    // Fix up the destination if the initializer isn't an expression
    // of atomic type.
    bool Zeroed = false;
    if (!init->getType()->isAtomicType()) {
      Zeroed = atomics.emitMemSetZeroIfNecessary();
      dest = atomics.projectValue();
    }

    // Evaluate the expression directly into the destination.
    AggValueSlot slot = AggValueSlot::forLValue(
        dest, *this, AggValueSlot::IsNotDestructed,
        AggValueSlot::DoesNotNeedGCBarriers, AggValueSlot::IsNotAliased,
        AggValueSlot::DoesNotOverlap,
        Zeroed ? AggValueSlot::IsZeroed : AggValueSlot::IsNotZeroed);

    EmitAggExpr(init, slot);
    return;
  }
  }
  llvm_unreachable("bad evaluation kind");
}
