//===- Mips.cpp -----------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"
#include "CommonCheriTargetCodeGenInfo.h"

using namespace clang;
using namespace clang::CodeGen;

//===----------------------------------------------------------------------===//
// MIPS ABI Implementation.  This works for both little-endian and
// big-endian variants.
//===----------------------------------------------------------------------===//

namespace {
class MipsABIInfo : public ABIInfo {
  bool IsO32;
  CodeGenModule &CGM;
  const unsigned MinABIStackAlignInBytes;
  unsigned StackAlignInBytes;
  void CoerceToIntArgs(uint64_t TySize,
                       SmallVectorImpl<llvm::Type *> &ArgList) const;
  llvm::Type* HandleAggregates(QualType Ty, uint64_t TySize) const;
  llvm::Type* returnAggregateInRegs(QualType RetTy, uint64_t Size) const;
  llvm::Type* getPaddingType(uint64_t Align, uint64_t Offset) const;
public:
  MipsABIInfo(CodeGenTypes &CGT, bool _IsO32, CodeGenModule &_CGM) :
    ABIInfo(CGT),
    IsO32(_IsO32), CGM(_CGM), MinABIStackAlignInBytes(IsO32 ? 4 : 8) {
      const TargetInfo& TI = _CGM.getTarget();
      if (TI.areAllPointersCapabilities())
        StackAlignInBytes = TI.getCHERICapabilityWidth();
      else
        StackAlignInBytes = (IsO32 ? 8 : 16);
  }

  ABIArgInfo classifyReturnType(QualType RetTy) const;
  ABIArgInfo classifyArgumentType(QualType RetTy, bool IsFixed,
                                  uint64_t &Offset, bool &HasV0) const;
  void computeInfo(CGFunctionInfo &FI) const override;
  Address EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                    QualType Ty) const override;
  ABIArgInfo extendType(QualType Ty) const;
};

class MIPSTargetCodeGenInfo : public CommonCheriTargetCodeGenInfo {
  unsigned SizeOfUnwindException;
public:
  MIPSTargetCodeGenInfo(CodeGenTypes &CGT, bool IsO32, CodeGenModule &CGM)
      : CommonCheriTargetCodeGenInfo(
            std::make_unique<MipsABIInfo>(CGT, IsO32, CGM)),
        SizeOfUnwindException(IsO32 ? 24 : 32) {}

  int getDwarfEHStackPointer(CodeGen::CodeGenModule &CGM) const override {
    return 29;
  }

  void setTargetAttributes(const Decl *D, llvm::GlobalValue *GV,
                           CodeGen::CodeGenModule &CGM) const override {
    const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D);
    if (!FD) return;
    llvm::Function *Fn = cast<llvm::Function>(GV);

    if (FD->hasAttr<MipsLongCallAttr>())
      Fn->addFnAttr("long-call");
    else if (FD->hasAttr<MipsShortCallAttr>())
      Fn->addFnAttr("short-call");

    // Other attributes do not have a meaning for declarations.
    if (GV->isDeclaration())
      return;

    if (FD->hasAttr<Mips16Attr>()) {
      Fn->addFnAttr("mips16");
    }
    else if (FD->hasAttr<NoMips16Attr>()) {
      Fn->addFnAttr("nomips16");
    }

    if (FD->hasAttr<MicroMipsAttr>())
      Fn->addFnAttr("micromips");
    else if (FD->hasAttr<NoMicroMipsAttr>())
      Fn->addFnAttr("nomicromips");

    const MipsInterruptAttr *Attr = FD->getAttr<MipsInterruptAttr>();
    if (!Attr)
      return;

    const char *Kind;
    switch (Attr->getInterrupt()) {
    case MipsInterruptAttr::eic:     Kind = "eic"; break;
    case MipsInterruptAttr::sw0:     Kind = "sw0"; break;
    case MipsInterruptAttr::sw1:     Kind = "sw1"; break;
    case MipsInterruptAttr::hw0:     Kind = "hw0"; break;
    case MipsInterruptAttr::hw1:     Kind = "hw1"; break;
    case MipsInterruptAttr::hw2:     Kind = "hw2"; break;
    case MipsInterruptAttr::hw3:     Kind = "hw3"; break;
    case MipsInterruptAttr::hw4:     Kind = "hw4"; break;
    case MipsInterruptAttr::hw5:     Kind = "hw5"; break;
    }

    Fn->addFnAttr("interrupt", Kind);

  }

  bool initDwarfEHRegSizeTable(CodeGen::CodeGenFunction &CGF,
                               llvm::Value *Address) const override;

  unsigned getSizeOfUnwindException() const override {
    return SizeOfUnwindException;
  }

  unsigned getDefaultAS() const override {
    const TargetInfo &Target = getABIInfo().getContext().getTargetInfo();
    return Target.areAllPointersCapabilities() ? getCHERICapabilityAS() : 0;
  }
  unsigned getTlsAddressSpace() const override {
    const TargetInfo &Target = getABIInfo().getContext().getTargetInfo();
    if (Target.areAllPointersCapabilities()) {
      return getCHERICapabilityAS();
    }
    return getDefaultAS();
  }
  unsigned getCHERICapabilityAS() const override {
    return 200;
  }
};
}

void MipsABIInfo::CoerceToIntArgs(
    uint64_t TySize, SmallVectorImpl<llvm::Type *> &ArgList) const {
  llvm::IntegerType *IntTy =
    llvm::IntegerType::get(getVMContext(), MinABIStackAlignInBytes * 8);

  // Add (TySize / MinABIStackAlignInBytes) args of IntTy.
  for (unsigned N = TySize / (MinABIStackAlignInBytes * 8); N; --N)
    ArgList.push_back(IntTy);

  // If necessary, add one more integer type to ArgList.
  unsigned R = TySize % (MinABIStackAlignInBytes * 8);

  if (R)
    ArgList.push_back(llvm::IntegerType::get(getVMContext(), R));
}

// In N32/64, an aligned double precision floating point field is passed in
// a register.
llvm::Type* MipsABIInfo::HandleAggregates(QualType Ty, uint64_t TySize) const {
  SmallVector<llvm::Type*, 8> ArgList, IntArgList;

  if (IsO32) {
    CoerceToIntArgs(TySize, ArgList);
    return llvm::StructType::get(getVMContext(), ArgList);
  }

  if (Ty->isComplexType())
    return CGT.ConvertType(Ty);

  const RecordType *RT = Ty->getAs<RecordType>();
  
  // On CHERI, we must pass unions containing capabilities in capability
  // registers.  Otherwise, pass them as integers.
  if (RT &&
      (RT->isUnionType() && getContext().containsCapabilities(RT->getDecl()))
      && getTarget().SupportsCapabilities())
    return llvm::Type::getInt8Ty(getVMContext())->getPointerTo(
                            CGM.getTargetCodeGenInfo().getCHERICapabilityAS());

  // Unions/vectors are passed in integer registers.
  if (!RT || !RT->isStructureOrClassType()) {
    const TargetInfo &Target = getContext().getTargetInfo();
    if (Target.areAllPointersCapabilities()) {
      if (Ty->isMemberFunctionPointerType()) {
        return llvm::StructType::get(CGM.VoidPtrTy, CGM.PtrDiffTy);
      }
    }
    CoerceToIntArgs(TySize, ArgList);
    return llvm::StructType::get(getVMContext(), ArgList);
  }

  const RecordDecl *RD = RT->getDecl();
  const ASTRecordLayout &Layout = getContext().getASTRecordLayout(RD);
  assert(!(TySize % 8) && "Size of structure must be multiple of 8.");

  uint64_t LastOffset = 0;
  llvm::IntegerType *I64 = llvm::IntegerType::get(getVMContext(), 64);
  unsigned CapSize = getTarget().getCHERICapabilityWidth();

  // If this is a C++ record, look for any capabilities in base classes first
  if (const CXXRecordDecl *CXXRD = dyn_cast<CXXRecordDecl>(RD)) {
    for (const auto &I : CXXRD->bases()) {
      unsigned idx = 0;
      const CXXRecordDecl *BRD =
          cast<CXXRecordDecl>(I.getType()->castAs<RecordType>()->getDecl());
      uint64_t BaseOffset = getContext().toBits(Layout.getBaseClassOffset(BRD));
      const ASTRecordLayout &BaseLayout = getContext().getASTRecordLayout(BRD);
      for (RecordDecl::field_iterator i = BRD->field_begin(), e = BRD->field_end();
           i != e; ++i, ++idx) {
        const QualType Ty = i->getType();
        uint64_t Offset = BaseOffset + BaseLayout.getFieldOffset(idx);
        if (const RecordType *FRT = Ty->getAs<RecordType>()) {
          if (getContext().containsCapabilities(FRT->getDecl())) {
            uint64_t FieldSize = getContext().getTypeSize(Ty);
            LastOffset = Offset + FieldSize;
            ArgList.push_back(HandleAggregates(Ty, FieldSize));
            continue;
          }
        }
        if (!Ty->isConstantArrayType() && Ty->isCHERICapabilityType(getContext())) {
          // Add ((Offset - LastOffset) / 64) args of type i64.
          for (unsigned j = (Offset - LastOffset) / 64; j > 0; --j)
            ArgList.push_back(I64);
          const uint64_t TySize = getContext().getTypeSize(Ty);
          LastOffset = Offset + TySize;
          assert(CapSize == TySize || (Ty->isMemberFunctionPointerType() && TySize == 2 * CapSize));
          ArgList.push_back(CGT.ConvertType(Ty));
          continue;
        }
      }
    }
  }
  unsigned idx = 0;

  // Iterate over fields in the struct/class and check if there are any aligned
  // double fields.
  for (RecordDecl::field_iterator i = RD->field_begin(), e = RD->field_end();
       i != e; ++i, ++idx) {
    const QualType Ty = i->getType();
    const BuiltinType *BT = Ty->getAs<BuiltinType>();
    uint64_t Offset = Layout.getFieldOffset(idx);

    if (const RecordType *FRT = Ty->getAs<RecordType>()) {
      if (getContext().containsCapabilities(FRT->getDecl())) {
        uint64_t FieldSize = getContext().getTypeSize(Ty);
        LastOffset = Layout.getFieldOffset(idx) + FieldSize;
        ArgList.push_back(HandleAggregates(Ty, FieldSize));
        continue;
      }
    }
    if (!Ty->isConstantArrayType() && Ty->isCHERICapabilityType(getContext())) {
      // Add ((Offset - LastOffset) / 64) args of type i64.
      for (unsigned j = (Offset - LastOffset) / 64; j > 0; --j)
        ArgList.push_back(I64);
      const uint64_t TySize = getContext().getTypeSize(Ty);
      LastOffset = Layout.getFieldOffset(idx) + TySize;
      assert(CapSize == TySize || (Ty->isMemberFunctionPointerType() && TySize == 2 * CapSize));
      ArgList.push_back(CGT.ConvertType(Ty));
      continue;
    }
    if (const ConstantArrayType *CAT = dyn_cast<ConstantArrayType>(Ty)) {
      auto ElementType = CAT->getElementType();
      unsigned Elements = CAT->getSize().getLimitedValue();
      if (ElementType->isCHERICapabilityType(getContext())) {
        llvm::Type *ElTy = CGT.ConvertType(ElementType);
        for (unsigned i=0 ; i<Elements ; ++i)
          ArgList.push_back(ElTy);
        const uint64_t ElemSize = getContext().getTypeSize(ElementType);
        LastOffset += Elements * ElemSize;
        assert(CapSize == ElemSize || (Ty->isMemberFunctionPointerType() && ElemSize == 2 * CapSize));
        continue;
      } else if (getContext().containsCapabilities(ElementType)) {
        uint64_t FieldSize = getContext().getTypeSize(ElementType);
        LastOffset += FieldSize * Elements;
        auto ElTy = HandleAggregates(ElementType, FieldSize);
        for (unsigned i=0 ; i<Elements ; ++i)
          ArgList.push_back(ElTy);
        continue;
      }
    }

    if (!BT || BT->getKind() != BuiltinType::Double)
      continue;

    if (Offset % 64) // Ignore doubles that are not aligned.
      continue;

    // Add ((Offset - LastOffset) / 64) args of type i64.
    for (unsigned j = (Offset - LastOffset) / 64; j > 0; --j)
      ArgList.push_back(I64);

    // Add double type.
    ArgList.push_back(llvm::Type::getDoubleTy(getVMContext()));
    LastOffset = Offset + 64;
  }

  CoerceToIntArgs(TySize - LastOffset, IntArgList);
  ArgList.append(IntArgList.begin(), IntArgList.end());

  return llvm::StructType::get(getVMContext(), ArgList);
}

llvm::Type *MipsABIInfo::getPaddingType(uint64_t OrigOffset,
                                        uint64_t Offset) const {
  if (OrigOffset + MinABIStackAlignInBytes > Offset)
    return nullptr;

  return llvm::IntegerType::get(getVMContext(), (Offset - OrigOffset) * 8);
}

ABIArgInfo
MipsABIInfo::classifyArgumentType(QualType Ty, bool IsFixed,
                                  uint64_t &Offset, bool &HasV0) const {
  Ty = useFirstFieldIfTransparentUnion(Ty);

  uint64_t OrigOffset = Offset;
  uint64_t TySize = getContext().getTypeSize(Ty);
  uint64_t Align = getContext().getTypeAlign(Ty) / 8;

  Align = std::clamp(Align, (uint64_t)MinABIStackAlignInBytes,
                     (uint64_t)StackAlignInBytes);
  unsigned CurrOffset = llvm::alignTo(Offset, Align);
  Offset = CurrOffset + llvm::alignTo(TySize, Align * 8) / 8;

  if (isAggregateTypeForABI(Ty) || Ty->isVectorType()) {
    // Ignore empty aggregates.
    if (TySize == 0)
      return ABIArgInfo::getIgnore();

    if (CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI())) {
      Offset = OrigOffset + MinABIStackAlignInBytes;
      return getNaturalAlignIndirect(Ty, RAA == CGCXXABI::RAA_DirectInMemory);
    }

    // Use indirect if the aggregate cannot fit into registers for
    // passing arguments according to the ABI
    unsigned Threshold = IsO32 ? 16 : 64;
    const TargetInfo &Target = getContext().getTargetInfo();
    bool PassIndirect = false;
    bool ByVal = true;
    if (Target.areAllPointersCapabilities()) {
      // assume we can pass structs up to 8 capabilities in size directly
      Threshold = 8 * (Target.getCHERICapabilityWidth() / 8);
    } else if (const auto *RT = Ty->getAs<RecordType>()) {
      // Aggregates containing capabilities are passed indirectly for hybrid
      // varargs, not just on the stack.
      if (!IsFixed && getContext().containsCapabilities(RT->getDecl())) {
        PassIndirect = true;
        ByVal = false;
      }
    }
    if (getContext().getTypeSizeInChars(Ty) > CharUnits::fromQuantity(Threshold))
      PassIndirect = true;
    if (PassIndirect)
      return ABIArgInfo::getIndirect(CharUnits::fromQuantity(Align), ByVal,
                                     getContext().getTypeAlign(Ty) / 8 > Align);

    // If we have reached here, aggregates are passed directly by coercing to
    // another structure type. Padding is inserted if the offset of the
    // aggregate is unaligned.
    ABIArgInfo ArgInfo =
        ABIArgInfo::getDirect(HandleAggregates(Ty, TySize), 0,
                              getPaddingType(OrigOffset, CurrOffset));
    ArgInfo.setInReg(true);
    return ArgInfo;
  }

  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = Ty->getAs<EnumType>())
    Ty = EnumTy->getDecl()->getIntegerType();

  // Make sure we pass indirectly things that are too large.
  if (const auto *EIT = Ty->getAs<BitIntType>())
    if (EIT->getNumBits() > 128 ||
        (EIT->getNumBits() > 64 &&
         !getContext().getTargetInfo().hasInt128Type()))
      return getNaturalAlignIndirect(Ty);

  // All integral types are promoted to the GPR width.
  if (Ty->isIntegralOrEnumerationType() &&
      !Ty->isCHERICapabilityType(getContext())) {
    if (HasV0) {
      // This is the special method number argument; don't count it as a
      // parameter slot. otherwise we risk inserting padding directly after
      // this argument if we next see an aggregate, which disagrees with the
      // non-CCall calling convention when dropping the object and method, but
      // the two must match to allow cheri_invoke and the libcheri CCall
      // vectors to act as trampolines.
      //
      // TODO: This is insufficient if the first integral type comes as part of
      //       an aggregate above. Ideally the front-end would enforce that
      //       CCall functions have sane arguments (two capabilities and an
      //       integer (up to one machine word) before anything else, with the
      //       capabilities possibly in a struct).
      HasV0 = false;
      Offset = OrigOffset;
    }
    return extendType(Ty);
  }

  if (Ty->isCHERICapabilityType(getContext())) {
    // Capabilities are passed indirectly for hybrid varargs, not just on the
    // stack.
    if (!IsFixed && !getContext().getTargetInfo().areAllPointersCapabilities())
      return getNaturalAlignIndirect(Ty, /*ByVal=*/false);
    else
      return ABIArgInfo::getDirect(CGT.ConvertType(Ty));
  }

  return ABIArgInfo::getDirect(
      nullptr, 0, IsO32 ? nullptr : getPaddingType(OrigOffset, CurrOffset));
}

llvm::Type*
MipsABIInfo::returnAggregateInRegs(QualType RetTy, uint64_t Size) const {
  const RecordType *RT = RetTy->getAs<RecordType>();
  SmallVector<llvm::Type*, 8> RTList;

  if (RT && RT->isStructureOrClassType()) {
    const RecordDecl *RD = RT->getDecl();
    const ASTRecordLayout &Layout = getContext().getASTRecordLayout(RD);
    unsigned FieldCnt = Layout.getFieldCount();

    // N32/64 returns struct/classes in floating point registers if the
    // following conditions are met:
    // 1. The size of the struct/class is no larger than 128-bit.
    // 2. The struct/class has one or two fields all of which are floating
    //    point types.
    // 3. The offset of the first field is zero (this follows what gcc does).
    //
    // Any other composite results are returned in integer registers.
    //
    if (FieldCnt && (FieldCnt <= 2) && Layout.getFieldOffset(0) == 0) {
      RecordDecl::field_iterator b = RD->field_begin(), e = RD->field_end();
      for (; b != e; ++b) {
        const BuiltinType *BT = b->getType()->getAs<BuiltinType>();

        if (!BT || !BT->isFloatingPoint())
          break;

        RTList.push_back(CGT.ConvertType(b->getType()));
      }

      if (b == e)
        return llvm::StructType::get(getVMContext(), RTList,
                                     RD->hasAttr<PackedAttr>());

      RTList.clear();
    }
  }

  // TODO: if containscapabilities
  // XXXAR: keeping old upstream code here in case I broke something
  // CoerceToIntArgs(Size, RTList);
  // return llvm::StructType::get(getVMContext(), RTList);
  return HandleAggregates(RetTy, Size);
}

static bool mipsCanReturnDirect(const ASTContext& Ctx, const RecordDecl *RD, unsigned& NumCaps, unsigned& NumInts);
static bool mipsCanReturnDirect(const ASTContext& Ctx, QualType Ty, unsigned& NumCaps, unsigned& NumInts) {
  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = Ty->getAs<EnumType>())
    Ty = EnumTy->getDecl()->getIntegerType();
  if (Ty->isMemberPointerType()) {
    if (Ty->isMemberFunctionPointerType()) {
      // Returned as { i8 addrspace(200)*, i64 }
      NumCaps++;
      NumInts++;
    } else {
      NumInts++;       // Single offset  value
    }
  }
  else if (Ty->isCHERICapabilityType(Ctx)) {
    NumCaps++;
  } else if (Ty->isIntegerType()) {
    // Can't return integer types that would need more than the address range
    // directly. Note: a single __int128 struct is still returned directly since
    // that case is handled after the CHERI-specific check.
    uint64_t CapRange = Ctx.getTargetInfo().getPointerRangeForCHERICapability();
    if (Ctx.getTypeSize(Ty) > CapRange)
      return false;
    NumInts++;
  } else if (const RecordType *RT = Ty->getAs<RecordType>()) {
    if (!mipsCanReturnDirect(Ctx, RT->getDecl(), NumCaps, NumInts))
      return false;
  } else if (Ty->isConstantArrayType()) {
    auto CAT = cast<ConstantArrayType>(Ty->getAsArrayTypeUnsafe());
    if (CAT->getSize().ugt(2))
      return false;
    unsigned ArrayCaps = 0;
    unsigned ArrayInts = 0;
    if (!mipsCanReturnDirect(Ctx, CAT->getElementType(), ArrayCaps, ArrayInts))
      return false;
    NumCaps += ArrayCaps * CAT->getSize().getZExtValue();
    NumInts += ArrayInts * CAT->getSize().getZExtValue();
  } else {
    // FIXME: floating-point types, for now just return cap+float indirectly
    // Unknown type -> Can't return direct
    return false;
  }
  return NumCaps + NumInts <= 2;
}

static bool mipsCanReturnDirect(const ASTContext& Ctx, const RecordDecl *RD, unsigned& NumCaps, unsigned& NumInts) {
  // We can return up to two capabilities or or one capability and one int
  // in registers.
  for (auto i = RD->field_begin(), e = RD->field_end(); i != e; ++i) {
    if (!mipsCanReturnDirect(Ctx, i->getType(), NumCaps, NumInts))
      return false;
  }
  // In the case of C++ classes, also check base classes
  if (const CXXRecordDecl *CRD = dyn_cast<CXXRecordDecl>(RD)) {
    for (auto i = CRD->bases_begin(), e = CRD->bases_end(); i != e; ++i) {
      const QualType Ty = i->getType();
      if (const RecordType *RT = Ty->getAs<RecordType>())
        if (!mipsCanReturnDirect(Ctx, RT->getDecl(), NumCaps, NumInts))
          return false;
    }
  }
  return NumCaps + NumInts <= 2;
}

ABIArgInfo MipsABIInfo::classifyReturnType(QualType RetTy) const {
  uint64_t Size = getContext().getTypeSize(RetTy);

  if (RetTy->isVoidType())
    return ABIArgInfo::getIgnore();

  // O32 doesn't treat zero-sized structs differently from other structs.
  // However, N32/N64 ignores zero sized return values.
  if (!IsO32 && Size == 0)
    return ABIArgInfo::getIgnore();

  if (isAggregateTypeForABI(RetTy) || RetTy->isVectorType()) {
    if (getTarget().SupportsCapabilities() &&
        Size <= 2 * getTarget().getCHERICapabilityWidth()) {
      // On CHERI, we can return unions/structs containing at most two
      // capabilities directly. We also allow return one integer and one
      // capability directly, but no more than that to minimize differences
      // with the N64 ABI (which only returns pairs of integers).
      unsigned NumCaps = 0;
      unsigned NumInts = 0;
      if (mipsCanReturnDirect(getContext(), RetTy, NumCaps, NumInts)) {
        ABIArgInfo ArgInfo =
            ABIArgInfo::getDirect(returnAggregateInRegs(RetTy, Size));
        ArgInfo.setInReg(true);
        return ArgInfo;
      }
    }
    if (Size <= 128) {
      if (RetTy->isAnyComplexType())
        return ABIArgInfo::getDirect();

      // O32 returns integer vectors in registers and N32/N64 returns all small
      // aggregates in registers.
      if (!IsO32 ||
          (RetTy->isVectorType() && !RetTy->hasFloatingRepresentation())) {
        ABIArgInfo ArgInfo =
            ABIArgInfo::getDirect(returnAggregateInRegs(RetTy, Size));
        ArgInfo.setInReg(true);
        return ArgInfo;
      }
    }
    return getNaturalAlignIndirect(RetTy);
  }

  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = RetTy->getAs<EnumType>())
    RetTy = EnumTy->getDecl()->getIntegerType();

  // Make sure we pass indirectly things that are too large.
  if (const auto *EIT = RetTy->getAs<BitIntType>())
    if (EIT->getNumBits() > 128 ||
        (EIT->getNumBits() > 64 &&
         !getContext().getTargetInfo().hasInt128Type()))
      return getNaturalAlignIndirect(RetTy);

  if (isPromotableIntegerTypeForABI(RetTy))
    return ABIArgInfo::getExtend(RetTy);

  if ((RetTy->isUnsignedIntegerOrEnumerationType() ||
      RetTy->isSignedIntegerOrEnumerationType()) && Size == 32 && !IsO32)
    return ABIArgInfo::getSignExtend(RetTy);

  return ABIArgInfo::getDirect();
}

void MipsABIInfo::computeInfo(CGFunctionInfo &FI) const {
  ABIArgInfo &RetInfo = FI.getReturnInfo();
  if (!getCXXABI().classifyReturnType(FI))
    RetInfo = classifyReturnType(FI.getReturnType());

  // Check if a pointer to an aggregate is passed as a hidden argument.
  uint64_t Offset = RetInfo.isIndirect() ? MinABIStackAlignInBytes : 0;
  bool HasV0 = FI.getCallingConvention() == llvm::CallingConv::CHERI_CCall;
  int NumFixedArgs = FI.getNumRequiredArgs();

  int ArgNum = 0;
  for (auto &I : FI.arguments()) {
    bool IsFixed = ArgNum < NumFixedArgs;
    I.info = classifyArgumentType(I.type, IsFixed, Offset, HasV0);
    ArgNum++;
  }
}

Address MipsABIInfo::EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                               QualType OrigTy) const {
  QualType Ty = OrigTy;

  // Integer arguments are promoted to 32-bit on O32 and 64-bit on N32/N64.
  // Pointers are also promoted in the same way but this only matters for N32.
  unsigned SlotSizeInBits = IsO32 ? 32 : 64;
  unsigned PtrWidth = getTarget().getPointerWidth(LangAS::Default);
  bool DidPromote = false;
  if ((Ty->isIntegerType() &&
          getContext().getIntWidth(Ty) < SlotSizeInBits) ||
      (Ty->isPointerType() && PtrWidth < SlotSizeInBits)) {
    DidPromote = true;
    Ty = getContext().getIntTypeForBitwidth(SlotSizeInBits,
                                            Ty->isSignedIntegerType());
  }

  auto TyInfo = getContext().getTypeInfoInChars(Ty);

  // The alignment of things in the argument area is never larger than
  // StackAlignInBytes for O32.
  if (IsO32)
    TyInfo.Align =
      std::min(TyInfo.Align, CharUnits::fromQuantity(StackAlignInBytes));

  // MinABIStackAlignInBytes is the size of argument slots on the stack.
  CharUnits ArgSlotSize = CharUnits::fromQuantity(MinABIStackAlignInBytes);

  // Arguments containing capabilities are passed indirectly in the hybrid ABI.
  bool IsIndirect = false;
  if (!getContext().getTargetInfo().areAllPointersCapabilities()) {
    if (Ty->isCHERICapabilityType(getContext()))
      IsIndirect = true;
    else if (const auto *RT = Ty->getAs<RecordType>()) {
      if (getContext().containsCapabilities(RT->getDecl()))
        IsIndirect = true;
    }
  }

  Address Addr = emitVoidPtrVAArg(CGF, VAListAddr, Ty, IsIndirect,
                          TyInfo, ArgSlotSize, /*AllowHigherAlign*/ true);


  // If there was a promotion, "unpromote" into a temporary.
  // TODO: can we just use a pointer into a subset of the original slot?
  if (DidPromote) {
    Address Temp = CGF.CreateMemTemp(OrigTy, "vaarg.promotion-temp");
    llvm::Value *Promoted = CGF.Builder.CreateLoad(Addr);

    // Truncate down to the right width.
    llvm::Type *IntTy = (OrigTy->isIntegerType() ? Temp.getElementType()
                                                 : CGF.IntPtrTy);
    llvm::Value *V = CGF.Builder.CreateTrunc(Promoted, IntTy);
    if (OrigTy->isPointerType())
      V = CGF.Builder.CreateIntToPtr(V, Temp.getElementType());

    CGF.Builder.CreateStore(V, Temp);
    Addr = Temp;
  }

  return Addr;
}

ABIArgInfo MipsABIInfo::extendType(QualType Ty) const {
  int TySize = getContext().getTypeSize(Ty);

  // MIPS64 ABI requires unsigned 32 bit integers to be sign extended.
  if (Ty->isUnsignedIntegerOrEnumerationType() && TySize == 32)
    return ABIArgInfo::getSignExtend(Ty);

  return ABIArgInfo::getExtend(Ty);
}

bool
MIPSTargetCodeGenInfo::initDwarfEHRegSizeTable(CodeGen::CodeGenFunction &CGF,
                                               llvm::Value *Address) const {
  // This information comes from gcc's implementation, which seems to
  // as canonical as it gets.

  // Everything on MIPS is 4 bytes.  Double-precision FP registers
  // are aliased to pairs of single-precision FP registers.
  llvm::Value *Four8 = llvm::ConstantInt::get(CGF.Int8Ty, 4);

  // 0-31 are the general purpose registers, $0 - $31.
  // 32-63 are the floating-point registers, $f0 - $f31.
  // 64 and 65 are the multiply/divide registers, $hi and $lo.
  // 66 is the (notional, I think) register for signal-handler return.
  AssignToArrayRange(CGF.Builder, Address, Four8, 0, 65);

  // 67-74 are the floating-point status registers, $fcc0 - $fcc7.
  // They are one bit wide and ignored here.

  // 80-111 are the coprocessor 0 registers, $c0r0 - $c0r31.
  // (coprocessor 1 is the FP unit)
  // 112-143 are the coprocessor 2 registers, $c2r0 - $c2r31.
  // 144-175 are the coprocessor 3 registers, $c3r0 - $c3r31.
  // 176-181 are the DSP accumulator registers.
  AssignToArrayRange(CGF.Builder, Address, Four8, 80, 181);
  return false;
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createMIPSTargetCodeGenInfo(CodeGenModule &CGM, bool IsOS32) {
  return std::make_unique<MIPSTargetCodeGenInfo>(CGM.getTypes(), IsOS32, CGM);
}
