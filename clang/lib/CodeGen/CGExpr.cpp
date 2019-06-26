//===--- CGExpr.cpp - Emit LLVM Code from Expressions ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit Expr nodes as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CGCXXABI.h"
#include "CGCall.h"
#include "CGCleanup.h"
#include "CGDebugInfo.h"
#include "CGObjCRuntime.h"
#include "CGOpenMPRuntime.h"
#include "CGRecordLayout.h"
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "ConstantEmitter.h"
#include "TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/NSAPI.h"
#include "clang/Basic/CodeGenOptions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Cheri.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/Path.h"
#include "llvm/Transforms/Utils/SanitizerStats.h"

#include <string>

using namespace clang;
using namespace CodeGen;

//===--------------------------------------------------------------------===//
//                        Miscellaneous Helper Methods
//===--------------------------------------------------------------------===//

llvm::Value *CodeGenFunction::EmitCastToVoidPtr(llvm::Value *value) {
  unsigned addressSpace =
      cast<llvm::PointerType>(value->getType())->getAddressSpace();

  llvm::PointerType *destType = Int8PtrTy;
  if (addressSpace)
    destType = llvm::Type::getInt8PtrTy(getLLVMContext(), addressSpace);

  return EmitPointerCast(value, destType);
}

/// CreateTempAlloca - This creates a alloca and inserts it into the entry
/// block.
Address CodeGenFunction::CreateTempAllocaWithoutCast(llvm::Type *Ty,
                                                     CharUnits Align,
                                                     const Twine &Name,
                                                     llvm::Value *ArraySize) {
  auto Alloca = CreateTempAlloca(Ty, Name, ArraySize);
  Alloca->setAlignment(Align.getQuantity());
  return Address(Alloca, Align);
}

/// CreateTempAlloca - This creates a alloca and inserts it into the entry
/// block. The alloca is casted to default address space if necessary.
Address CodeGenFunction::CreateTempAlloca(llvm::Type *Ty, CharUnits Align,
                                          const Twine &Name,
                                          llvm::Value *ArraySize,
                                          Address *AllocaAddr) {
  auto Alloca = CreateTempAllocaWithoutCast(Ty, Align, Name, ArraySize);
  if (AllocaAddr)
    *AllocaAddr = Alloca;
  llvm::Value *V = Alloca.getPointer();
  // Alloca always returns a pointer in alloca address space, which may
  // be different from the type defined by the language. For example,
  // in C++ the auto variables are in the default address space. Therefore
  // cast alloca to the default address space when necessary.
  if (getASTAllocaAddressSpace() != LangAS::Default) {
    auto DestAddrSpace = CGM.getTargetAddressSpace(LangAS::Default);
    llvm::IRBuilderBase::InsertPointGuard IPG(Builder);
    // When ArraySize is nullptr, alloca is inserted at AllocaInsertPt,
    // otherwise alloca is inserted at the current insertion point of the
    // builder.
    if (!ArraySize)
      Builder.SetInsertPoint(AllocaInsertPt);
    V = getTargetHooks().performAddrSpaceCast(
        *this, V, getASTAllocaAddressSpace(), LangAS::Default,
        Ty->getPointerTo(DestAddrSpace), /*non-null*/ true);
  }

  return Address(V, Align);
}

/// CreateTempAlloca - This creates an alloca and inserts it into the entry
/// block if \p ArraySize is nullptr, otherwise inserts it at the current
/// insertion point of the builder.
llvm::AllocaInst *CodeGenFunction::CreateTempAlloca(llvm::Type *Ty,
                                                    const Twine &Name,
                                                    llvm::Value *ArraySize) {
  if (ArraySize)
    return Builder.CreateAlloca(Ty, ArraySize, Name);
  return new llvm::AllocaInst(Ty, CGM.getDataLayout().getAllocaAddrSpace(),
                              ArraySize, Name, AllocaInsertPt);
}

/// CreateDefaultAlignTempAlloca - This creates an alloca with the
/// default alignment of the corresponding LLVM type, which is *not*
/// guaranteed to be related in any way to the expected alignment of
/// an AST type that might have been lowered to Ty.
Address CodeGenFunction::CreateDefaultAlignTempAlloca(llvm::Type *Ty,
                                                      const Twine &Name) {
  CharUnits Align =
    CharUnits::fromQuantity(CGM.getDataLayout().getABITypeAlignment(Ty));
  return CreateTempAlloca(Ty, Align, Name);
}

void CodeGenFunction::InitTempAlloca(Address Var, llvm::Value *Init) {
  assert(isa<llvm::AllocaInst>(Var.getPointer()));
  auto *Store = new llvm::StoreInst(Init, Var.getPointer());
  Store->setAlignment(Var.getAlignment().getQuantity());
  llvm::BasicBlock *Block = AllocaInsertPt->getParent();
  Block->getInstList().insertAfter(AllocaInsertPt->getIterator(), Store);
}

Address CodeGenFunction::CreateIRTemp(QualType Ty, const Twine &Name) {
  CharUnits Align = getContext().getTypeAlignInChars(Ty);
  return CreateTempAlloca(ConvertType(Ty), Align, Name);
}

Address CodeGenFunction::CreateMemTemp(QualType Ty, const Twine &Name,
                                       Address *Alloca) {
  // FIXME: Should we prefer the preferred type alignment here?
  return CreateMemTemp(Ty, getContext().getTypeAlignInChars(Ty), Name, Alloca);
}

Address CodeGenFunction::CreateMemTemp(QualType Ty, CharUnits Align,
                                       const Twine &Name, Address *Alloca) {
  return CreateTempAlloca(ConvertTypeForMem(Ty), Align, Name,
                          /*ArraySize=*/nullptr, Alloca);
}

Address CodeGenFunction::CreateMemTempWithoutCast(QualType Ty, CharUnits Align,
                                                  const Twine &Name) {
  return CreateTempAllocaWithoutCast(ConvertTypeForMem(Ty), Align, Name);
}

Address CodeGenFunction::CreateMemTempWithoutCast(QualType Ty,
                                                  const Twine &Name) {
  return CreateMemTempWithoutCast(Ty, getContext().getTypeAlignInChars(Ty),
                                  Name);
}

/// EvaluateExprAsBool - Perform the usual unary conversions on the specified
/// expression and compare the result against zero, returning an Int1Ty value.
llvm::Value *CodeGenFunction::EvaluateExprAsBool(const Expr *E) {
  PGO.setCurrentStmt(E);
  if (const MemberPointerType *MPT = E->getType()->getAs<MemberPointerType>()) {
    llvm::Value *MemPtr = EmitScalarExpr(E);
    return CGM.getCXXABI().EmitMemberPointerIsNotNull(*this, MemPtr, MPT);
  }

  QualType BoolTy = getContext().BoolTy;
  SourceLocation Loc = E->getExprLoc();
  if (!E->getType()->isAnyComplexType())
    return EmitScalarConversion(EmitScalarExpr(E), E->getType(), BoolTy, Loc);

  return EmitComplexToScalarConversion(EmitComplexExpr(E), E->getType(), BoolTy,
                                       Loc);
}

/// EmitIgnoredExpr - Emit code to compute the specified expression,
/// ignoring the result.
void CodeGenFunction::EmitIgnoredExpr(const Expr *E) {
  if (E->isRValue())
    return (void) EmitAnyExpr(E, AggValueSlot::ignored(), true);

  // Just emit it as an l-value and drop the result.
  EmitLValue(E);
}

/// EmitAnyExpr - Emit code to compute the specified expression which
/// can have any type.  The result is returned as an RValue struct.
/// If this is an aggregate expression, AggSlot indicates where the
/// result should be returned.
RValue CodeGenFunction::EmitAnyExpr(const Expr *E,
                                    AggValueSlot aggSlot,
                                    bool ignoreResult) {
  switch (getEvaluationKind(E->getType())) {
  case TEK_Scalar:
    return RValue::get(EmitScalarExpr(E, ignoreResult));
  case TEK_Complex:
    return RValue::getComplex(EmitComplexExpr(E, ignoreResult, ignoreResult));
  case TEK_Aggregate:
    if (!ignoreResult && aggSlot.isIgnored())
      aggSlot = CreateAggTemp(E->getType(), "agg-temp");
    EmitAggExpr(E, aggSlot);
    return aggSlot.asRValue();
  }
  llvm_unreachable("bad evaluation kind");
}

/// EmitAnyExprToTemp - Similar to EmitAnyExpr(), however, the result will
/// always be accessible even if no aggregate location is provided.
RValue CodeGenFunction::EmitAnyExprToTemp(const Expr *E) {
  AggValueSlot AggSlot = AggValueSlot::ignored();

  if (hasAggregateEvaluationKind(E->getType()))
    AggSlot = CreateAggTemp(E->getType(), "agg.tmp");
  return EmitAnyExpr(E, AggSlot);
}

/// EmitAnyExprToMem - Evaluate an expression into a given memory
/// location.
void CodeGenFunction::EmitAnyExprToMem(const Expr *E,
                                       Address Location,
                                       Qualifiers Quals,
                                       bool IsInit) {
  // FIXME: This function should take an LValue as an argument.
  switch (getEvaluationKind(E->getType())) {
  case TEK_Complex:
    EmitComplexExprIntoLValue(E, MakeAddrLValue(Location, E->getType()),
                              /*isInit*/ false);
    return;

  case TEK_Aggregate: {
    EmitAggExpr(E, AggValueSlot::forAddr(Location, Quals,
                                         AggValueSlot::IsDestructed_t(IsInit),
                                         AggValueSlot::DoesNotNeedGCBarriers,
                                         AggValueSlot::IsAliased_t(!IsInit),
                                         AggValueSlot::MayOverlap));
    return;
  }

  case TEK_Scalar: {
    RValue RV = RValue::get(EmitScalarExpr(E, /*Ignore*/ false));
    LValue LV = MakeAddrLValue(Location, E->getType());
    EmitStoreThroughLValue(RV, LV);
    return;
  }
  }
  llvm_unreachable("bad evaluation kind");
}

static void
pushTemporaryCleanup(CodeGenFunction &CGF, const MaterializeTemporaryExpr *M,
                     const Expr *E, Address ReferenceTemporary) {
  // Objective-C++ ARC:
  //   If we are binding a reference to a temporary that has ownership, we
  //   need to perform retain/release operations on the temporary.
  //
  // FIXME: This should be looking at E, not M.
  if (auto Lifetime = M->getType().getObjCLifetime()) {
    switch (Lifetime) {
    case Qualifiers::OCL_None:
    case Qualifiers::OCL_ExplicitNone:
      // Carry on to normal cleanup handling.
      break;

    case Qualifiers::OCL_Autoreleasing:
      // Nothing to do; cleaned up by an autorelease pool.
      return;

    case Qualifiers::OCL_Strong:
    case Qualifiers::OCL_Weak:
      switch (StorageDuration Duration = M->getStorageDuration()) {
      case SD_Static:
        // Note: we intentionally do not register a cleanup to release
        // the object on program termination.
        return;

      case SD_Thread:
        // FIXME: We should probably register a cleanup in this case.
        return;

      case SD_Automatic:
      case SD_FullExpression:
        CodeGenFunction::Destroyer *Destroy;
        CleanupKind CleanupKind;
        if (Lifetime == Qualifiers::OCL_Strong) {
          const ValueDecl *VD = M->getExtendingDecl();
          bool Precise =
              VD && isa<VarDecl>(VD) && VD->hasAttr<ObjCPreciseLifetimeAttr>();
          CleanupKind = CGF.getARCCleanupKind();
          Destroy = Precise ? &CodeGenFunction::destroyARCStrongPrecise
                            : &CodeGenFunction::destroyARCStrongImprecise;
        } else {
          // __weak objects always get EH cleanups; otherwise, exceptions
          // could cause really nasty crashes instead of mere leaks.
          CleanupKind = NormalAndEHCleanup;
          Destroy = &CodeGenFunction::destroyARCWeak;
        }
        if (Duration == SD_FullExpression)
          CGF.pushDestroy(CleanupKind, ReferenceTemporary,
                          M->getType(), *Destroy,
                          CleanupKind & EHCleanup);
        else
          CGF.pushLifetimeExtendedDestroy(CleanupKind, ReferenceTemporary,
                                          M->getType(),
                                          *Destroy, CleanupKind & EHCleanup);
        return;

      case SD_Dynamic:
        llvm_unreachable("temporary cannot have dynamic storage duration");
      }
      llvm_unreachable("unknown storage duration");
    }
  }

  CXXDestructorDecl *ReferenceTemporaryDtor = nullptr;
  if (const RecordType *RT =
          E->getType()->getBaseElementTypeUnsafe()->getAs<RecordType>()) {
    // Get the destructor for the reference temporary.
    auto *ClassDecl = cast<CXXRecordDecl>(RT->getDecl());
    if (!ClassDecl->hasTrivialDestructor())
      ReferenceTemporaryDtor = ClassDecl->getDestructor();
  }

  if (!ReferenceTemporaryDtor)
    return;

  // Call the destructor for the temporary.
  switch (M->getStorageDuration()) {
  case SD_Static:
  case SD_Thread: {
    llvm::FunctionCallee CleanupFn;
    llvm::Constant *CleanupArg;
    if (E->getType()->isArrayType()) {
      CleanupFn = CodeGenFunction(CGF.CGM).generateDestroyHelper(
          ReferenceTemporary, E->getType(),
          CodeGenFunction::destroyCXXObject, CGF.getLangOpts().Exceptions,
          dyn_cast_or_null<VarDecl>(M->getExtendingDecl()));
      CleanupArg = llvm::Constant::getNullValue(CGF.Int8PtrTy);
    } else {
      CleanupFn = CGF.CGM.getAddrAndTypeOfCXXStructor(
          GlobalDecl(ReferenceTemporaryDtor, Dtor_Complete));
      CleanupArg = cast<llvm::Constant>(ReferenceTemporary.getPointer());
    }
    CGF.CGM.getCXXABI().registerGlobalDtor(
        CGF, *cast<VarDecl>(M->getExtendingDecl()), CleanupFn, CleanupArg);
    break;
  }

  case SD_FullExpression:
    CGF.pushDestroy(NormalAndEHCleanup, ReferenceTemporary, E->getType(),
                    CodeGenFunction::destroyCXXObject,
                    CGF.getLangOpts().Exceptions);
    break;

  case SD_Automatic:
    CGF.pushLifetimeExtendedDestroy(NormalAndEHCleanup,
                                    ReferenceTemporary, E->getType(),
                                    CodeGenFunction::destroyCXXObject,
                                    CGF.getLangOpts().Exceptions);
    break;

  case SD_Dynamic:
    llvm_unreachable("temporary cannot have dynamic storage duration");
  }
}

static Address createReferenceTemporary(CodeGenFunction &CGF,
                                        const MaterializeTemporaryExpr *M,
                                        const Expr *Inner,
                                        Address *Alloca = nullptr) {
  auto &TCG = CGF.getTargetHooks();
  switch (M->getStorageDuration()) {
  case SD_FullExpression:
  case SD_Automatic: {
    // If we have a constant temporary array or record try to promote it into a
    // constant global under the same rules a normal constant would've been
    // promoted. This is easier on the optimizer and generally emits fewer
    // instructions.
    QualType Ty = Inner->getType();
    if (CGF.CGM.getCodeGenOpts().MergeAllConstants &&
        (Ty->isArrayType() || Ty->isRecordType()) &&
        CGF.CGM.isTypeConstant(Ty, true))
      if (auto Init = ConstantEmitter(CGF).tryEmitAbstract(Inner, Ty)) {
        if (auto AddrSpace = CGF.getTarget().getConstantAddressSpace()) {
          auto AS = AddrSpace.getValue();
          auto *GV = new llvm::GlobalVariable(
              CGF.CGM.getModule(), Init->getType(), /*isConstant=*/true,
              llvm::GlobalValue::PrivateLinkage, Init, ".ref.tmp", nullptr,
              llvm::GlobalValue::NotThreadLocal,
              CGF.CGM.getTargetAddressSpace(AS));
          CharUnits alignment = CGF.getContext().getTypeAlignInChars(Ty);
          GV->setAlignment(alignment.getQuantity());
          llvm::Constant *C = GV;
          if (AS != LangAS::Default)
            C = TCG.performAddrSpaceCast(
                CGF.CGM, GV, AS, LangAS::Default,
                GV->getValueType()->getPointerTo(
                    CGF.CGM.getTargetAddressSpace(LangAS::Default)));
          // FIXME: Should we put the new global into a COMDAT?
          return Address(C, alignment);
        }
      }
    return CGF.CreateMemTemp(Ty, "ref.tmp", Alloca);
  }
  case SD_Thread:
  case SD_Static:
    return CGF.CGM.GetAddrOfGlobalTemporary(M, Inner);

  case SD_Dynamic:
    llvm_unreachable("temporary can't have dynamic storage duration");
  }
  llvm_unreachable("unknown storage duration");
}

LValue CodeGenFunction::
EmitMaterializeTemporaryExpr(const MaterializeTemporaryExpr *M) {
  const Expr *E = M->GetTemporaryExpr();

  assert((!M->getExtendingDecl() || !isa<VarDecl>(M->getExtendingDecl()) ||
          !cast<VarDecl>(M->getExtendingDecl())->isARCPseudoStrong()) &&
         "Reference should never be pseudo-strong!");

  // FIXME: ideally this would use EmitAnyExprToMem, however, we cannot do so
  // as that will cause the lifetime adjustment to be lost for ARC
  auto ownership = M->getType().getObjCLifetime();
  if (ownership != Qualifiers::OCL_None &&
      ownership != Qualifiers::OCL_ExplicitNone) {
    Address Object = createReferenceTemporary(*this, M, E);
    if (auto *Var = dyn_cast<llvm::GlobalVariable>(Object.getPointer())) {
      Object = Address(llvm::ConstantExpr::getBitCast(Var,
                           ConvertTypeForMem(E->getType())
                             ->getPointerTo(Object.getAddressSpace())),
                       Object.getAlignment());

      // createReferenceTemporary will promote the temporary to a global with a
      // constant initializer if it can.  It can only do this to a value of
      // ARC-manageable type if the value is global and therefore "immune" to
      // ref-counting operations.  Therefore we have no need to emit either a
      // dynamic initialization or a cleanup and we can just return the address
      // of the temporary.
      if (Var->hasInitializer())
        return MakeAddrLValue(Object, M->getType(), AlignmentSource::Decl);

      Var->setInitializer(CGM.EmitNullConstant(E->getType()));
    }
    LValue RefTempDst = MakeAddrLValue(Object, M->getType(),
                                       AlignmentSource::Decl);

    switch (getEvaluationKind(E->getType())) {
    default: llvm_unreachable("expected scalar or aggregate expression");
    case TEK_Scalar:
      EmitScalarInit(E, M->getExtendingDecl(), RefTempDst, false);
      break;
    case TEK_Aggregate: {
      EmitAggExpr(E, AggValueSlot::forAddr(Object,
                                           E->getType().getQualifiers(),
                                           AggValueSlot::IsDestructed,
                                           AggValueSlot::DoesNotNeedGCBarriers,
                                           AggValueSlot::IsNotAliased,
                                           AggValueSlot::DoesNotOverlap));
      break;
    }
    }

    pushTemporaryCleanup(*this, M, E, Object);
    return RefTempDst;
  }

  SmallVector<const Expr *, 2> CommaLHSs;
  SmallVector<SubobjectAdjustment, 2> Adjustments;
  E = E->skipRValueSubobjectAdjustments(CommaLHSs, Adjustments);

  for (const auto &Ignored : CommaLHSs)
    EmitIgnoredExpr(Ignored);

  if (const auto *opaque = dyn_cast<OpaqueValueExpr>(E)) {
    if (opaque->getType()->isRecordType()) {
      assert(Adjustments.empty());
      return EmitOpaqueValueLValue(opaque);
    }
  }

  // Create and initialize the reference temporary.
  Address Alloca = Address::invalid();
  Address Object = createReferenceTemporary(*this, M, E, &Alloca);
  if (auto *Var = dyn_cast<llvm::GlobalVariable>(
          Object.getPointer()->stripPointerCasts())) {
    unsigned AS = CGM.getTargetCodeGenInfo().getDefaultAS();
    Object = Address(llvm::ConstantExpr::getPointerBitCastOrAddrSpaceCast(
                         cast<llvm::Constant>(Object.getPointer()),
                         ConvertTypeForMem(E->getType())->getPointerTo(AS)),
                     Object.getAlignment());
    // If the temporary is a global and has a constant initializer or is a
    // constant temporary that we promoted to a global, we may have already
    // initialized it.
    if (!Var->hasInitializer()) {
      Var->setInitializer(CGM.EmitNullConstant(E->getType()));
      EmitAnyExprToMem(E, Object, Qualifiers(), /*IsInit*/true);
    }
  } else {
    switch (M->getStorageDuration()) {
    case SD_Automatic:
      if (auto *Size = EmitLifetimeStart(
              CGM.getDataLayout().getTypeAllocSize(Alloca.getElementType()),
              Alloca.getPointer())) {
        pushCleanupAfterFullExpr<CallLifetimeEnd>(NormalEHLifetimeMarker,
                                                  Alloca, Size);
      }
      break;

    case SD_FullExpression: {
      if (!ShouldEmitLifetimeMarkers)
        break;

      // Avoid creating a conditional cleanup just to hold an llvm.lifetime.end
      // marker. Instead, start the lifetime of a conditional temporary earlier
      // so that it's unconditional. Don't do this in ASan's use-after-scope
      // mode so that it gets the more precise lifetime marks. If the type has
      // a non-trivial destructor, we'll have a cleanup block for it anyway,
      // so this typically doesn't help; skip it in that case.
      ConditionalEvaluation *OldConditional = nullptr;
      CGBuilderTy::InsertPoint OldIP;
      if (isInConditionalBranch() && !E->getType().isDestructedType() &&
          !CGM.getCodeGenOpts().SanitizeAddressUseAfterScope) {
        OldConditional = OutermostConditional;
        OutermostConditional = nullptr;

        OldIP = Builder.saveIP();
        llvm::BasicBlock *Block = OldConditional->getStartingBlock();
        Builder.restoreIP(CGBuilderTy::InsertPoint(
            Block, llvm::BasicBlock::iterator(Block->back())));
      }

      if (auto *Size = EmitLifetimeStart(
              CGM.getDataLayout().getTypeAllocSize(Alloca.getElementType()),
              Alloca.getPointer())) {
        pushFullExprCleanup<CallLifetimeEnd>(NormalEHLifetimeMarker, Alloca,
                                             Size);
      }

      if (OldConditional) {
        OutermostConditional = OldConditional;
        Builder.restoreIP(OldIP);
      }
      break;
    }

    default:
      break;
    }
    EmitAnyExprToMem(E, Object, Qualifiers(), /*IsInit*/true);
  }
  pushTemporaryCleanup(*this, M, E, Object);

  // Perform derived-to-base casts and/or field accesses, to get from the
  // temporary object we created (and, potentially, for which we extended
  // the lifetime) to the subobject we're binding the reference to.
  for (unsigned I = Adjustments.size(); I != 0; --I) {
    SubobjectAdjustment &Adjustment = Adjustments[I-1];
    switch (Adjustment.Kind) {
    case SubobjectAdjustment::DerivedToBaseAdjustment:
      Object =
          GetAddressOfBaseClass(Object, Adjustment.DerivedToBase.DerivedClass,
                                Adjustment.DerivedToBase.BasePath->path_begin(),
                                Adjustment.DerivedToBase.BasePath->path_end(),
                                /*NullCheckValue=*/ false, E->getExprLoc());
      break;

    case SubobjectAdjustment::FieldAdjustment: {
      LValue LV = MakeAddrLValue(Object, E->getType(), AlignmentSource::Decl);
      LV = EmitLValueForField(LV, Adjustment.Field);
      assert(LV.isSimple() &&
             "materialized temporary field is not a simple lvalue");
      Object = LV.getAddress();
      break;
    }

    case SubobjectAdjustment::MemberPointerAdjustment: {
      llvm::Value *Ptr = EmitScalarExpr(Adjustment.Ptr.RHS);
      Object = EmitCXXMemberDataPointerAddress(E, Object, Ptr,
                                               Adjustment.Ptr.MPT);
      break;
    }
    }
  }

  return MakeAddrLValue(Object, M->getType(), AlignmentSource::Decl);
}

#define CHERI_BOUNDS_DBG(x) DEBUG_WITH_TYPE("cheri-bounds", llvm::dbgs() x)
#define DEBUG_TYPE "cheri-bounds"
STATISTIC(NumReferencesCheckedForBoundsTightening,
          "Number of references checked for tightening bounds");
STATISTIC(NumTightBoundsSetOnReferences,
          "Number of references where bounds were tightened");
STATISTIC(NumContainerBoundsSetOnReferences,
          "Number of references operators where container bounds were used");
STATISTIC(NumRemainingSizeBoundsSetOnReferences,
          "Number of references where remaining allocation size was used");

STATISTIC(NumAddrOfCheckedForBoundsTightening,
          "Number of & operators checked for tightening bounds");
STATISTIC(NumTightBoundsSetOnAddrOf,
          "Number of & operators where bounds were tightened");
STATISTIC(NumContainerBoundsSetOnAddrOf,
          "Number of & operators where container bounds were used");
STATISTIC(NumRemainingSizeBoundsSetOnAddrOf,
          "Number of & operators where remaining allocation size was used");

STATISTIC(NumArraySubscriptCheckedForBoundsTightening,
          "Number of [] operators checked for tightening bounds");
STATISTIC(NumTightBoundsSetOnArraySubscript,
          "Number of [] operators where bounds were tightened");
STATISTIC(NumRemainingSizeBoundsSetOnArraySubscript,
          "Number of [] operators where remaining allocation size was used");

STATISTIC(NumArrayDecayCheckedForBoundsTightening,
          "Number of array-to-pointer-decays checked for tightening bounds");
STATISTIC(NumTightBoundsSetOnArrayDecay,
          "Number of array-to-pointer-decays where bounds were tightened");
STATISTIC(NumRemainingSizeBoundsSetOnArrayDecay,
          "Number of array-to-pointer-decays where remaining allocation size "
          "was used");
#undef DEBUG_TYPE

static StringRef boundsKindStr(SubObjectBoundsKind Kind) {
  switch (Kind) {
  case SubObjectBoundsKind::Reference:
    return "reference";
  case SubObjectBoundsKind::AddrOf:
    return "address";
  case SubObjectBoundsKind::ArraySubscript:
    return "subscript";
  case SubObjectBoundsKind::ArrayDecay:
    return "decay";
  }
  llvm_unreachable("Invalid kind");
}

static llvm::cl::opt<unsigned> SubobjectBoundsSWPerm(
    "cheri-subobject-bounds-clear-swperm", llvm::cl::ZeroOrMore,
    llvm::cl::Hidden,
    llvm::cl::desc("Clear the given software permission whenever "
                   "subobject-bounds narrowed the bounds"));

// If we are setting bounds on the full container size, the csetbounds must
// be done before the cincoffset!
static llvm::Value *
tightenCHERIBounds(CodeGenFunction &CGF, SubObjectBoundsKind Kind,
                   llvm::Value *Value, const Expr *LocExpr, QualType Ty,
                   const CodeGenFunction::TightenBoundsResult &TBR) {
  llvm::Value *ValueToBound = Value;
  llvm::GetElementPtrInst *GEP = nullptr;
  StringRef KindStr = boundsKindStr(Kind);
  if (TBR.IsContainerSize) {
    if ((GEP = dyn_cast<llvm::GetElementPtrInst>(Value))) {
      ValueToBound = GEP->getPointerOperand();
    } else if (auto CE = dyn_cast<llvm::ConstantExpr>(Value)) {
      // If the source Value is not a GEP instruction but a GEP constantexp
      // (e.g. bounds on a global &array[index]) we need to convert it to an
      // instruction so that it can work with the result of a CSetBounds Inst
      if (CE->getOpcode() == llvm::Instruction::GetElementPtr) {
        GEP = cast<llvm::GetElementPtrInst>(CE->getAsInstruction());
        CGF.Builder.Insert(GEP);
        ValueToBound = GEP->getPointerOperand();
      } else {
        assert(CE->isCast() && "expected GEP or cast");
      }
    } else if (auto I = dyn_cast<llvm::Instruction>(Value)) {
      // Other case where we use container size is unions -> must be a cast
      assert(I->isCast() && "expected GEP or cast");
    } else {
      llvm_unreachable("Container type is not GEP or cast");
    }
  }
  llvm::Type *BoundedTy = ValueToBound->getType();

  SourceLocation Loc = LocExpr->getExprLoc();
  SourceRange Range = LocExpr->getSourceRange();
  StringRef StatsPrefix;
  if (Kind == SubObjectBoundsKind::AddrOf) {
    StatsPrefix = "addrof operator on ";
  } else if (Kind == SubObjectBoundsKind::Reference) {
    StatsPrefix = "C++ reference to ";
  } else if (Kind == SubObjectBoundsKind::ArraySubscript) {
    StatsPrefix = "array subscript for ";
  } else if (Kind == SubObjectBoundsKind::ArrayDecay) {
    StatsPrefix = "array decay for ";
  } else {
    llvm_unreachable("Invalid kind");
  }
  llvm::Value *SetBoundsSize = nullptr;
  llvm::Value *SrcLength = nullptr;
  std::string SizeStr;
  if (TBR.UseRemainingSize) {
    SizeStr = TBR.Size ? ("min(" + Twine(*TBR.Size) + ", remaining)").str()
                       : "remaining";
    auto SrcAsI8 = CGF.Builder.CreateBitCast(ValueToBound, CGF.Int8CheriCapTy);
    auto Offset = CGF.Builder.CreateIntrinsic(
        llvm::Intrinsic::cheri_cap_offset_get, CGF.PtrDiffTy, SrcAsI8, nullptr,
        "cur_offset");
    SrcLength =
        CGF.Builder.CreateIntrinsic(llvm::Intrinsic::cheri_cap_length_get,
                                    CGF.PtrDiffTy, SrcAsI8, nullptr, "cur_len");
    SetBoundsSize = CGF.Builder.CreateSub(SrcLength, Offset, "remaining_bytes");
    if (TBR.Size) {
      auto MaxConst = llvm::ConstantInt::get(CGF.PtrDiffTy, *TBR.Size);
      auto LessThanMax = CGF.Builder.CreateICmpULT(SetBoundsSize, MaxConst,
                                                   "less_than_max_size");
      SetBoundsSize = CGF.Builder.CreateSelect(
          LessThanMax, SetBoundsSize, MaxConst, "bounded_remaining_size");
    }
  } else {
    SizeStr = llvm::utostr(*TBR.Size);
    SetBoundsSize = llvm::ConstantInt::get(CGF.PtrDiffTy, *TBR.Size);
  }
  CHERI_BOUNDS_DBG(<< "setting bounds for '" << Ty.getAsString() << "' "
                   << KindStr << " to " << SizeStr << "\n");
  if (TBR.TargetField) {
    assert(TBR.IsSubObject);
    CGF.CGM.getDiags().Report(Loc,
                              diag::remark_setting_cheri_subobject_bounds_field)
        << TBR.TargetField << (int)Kind << Ty << SizeStr << TBR.DiagMessage
        << Range;
  } else {
    CGF.CGM.getDiags().Report(Loc, diag::remark_setting_cheri_subobject_bounds)
        << TBR.IsSubObject << (int)Kind << Ty << SizeStr << TBR.DiagMessage
        << Range;
  }
  llvm::Value *Result = CGF.setPointerBounds(
      ValueToBound, SetBoundsSize, Loc, KindStr + ".with.bounds",
      "Add subobject bounds", TBR.IsSubObject, StatsPrefix + Ty.getAsString());

  if (SubobjectBoundsSWPerm.getNumOccurrences() > 0) {
    // Clear the given software permission bit to differentiate subobject-bounds
    // violations from normal CHERI traps
    if (!SrcLength) {
      auto SrcAsI8 =
          CGF.Builder.CreateBitCast(ValueToBound, CGF.Int8CheriCapTy);
      SrcLength = CGF.Builder.CreateIntrinsic(
          llvm::Intrinsic::cheri_cap_length_get, CGF.PtrDiffTy, SrcAsI8,
          nullptr, "cur_len");
    }
    auto ResultAsI8 =
        CGF.Builder.CreateBitCast(Result, CGF.Int8CheriCapTy);
    auto ResultLength = CGF.Builder.CreateIntrinsic(
        llvm::Intrinsic::cheri_cap_length_get, CGF.PtrDiffTy, ResultAsI8,
        nullptr, "new_len");
    auto NewPermsMask = llvm::ConstantInt::get(
        CGF.PtrDiffTy, ~((UINT64_C(1) << SubobjectBoundsSWPerm)
                         << llvm::cheri::MIPS_UPERMS_SHIFT));
    auto WithClearedPerms = CGF.Builder.CreateIntrinsic(
        llvm::Intrinsic::cheri_cap_perms_and, CGF.PtrDiffTy,
        {ResultAsI8, NewPermsMask}, nullptr, "new_len");
    auto BoundsSmaller =
        CGF.Builder.CreateICmpULT(ResultLength, SrcLength, "new_bounds_less");
    Result = CGF.Builder.CreateSelect(BoundsSmaller, WithClearedPerms,
                                      ResultAsI8, "result");
  }

  Result = CGF.Builder.CreatePointerBitCastOrAddrSpaceCast(Result, BoundedTy);
  if (GEP) {
    // Replace the GEP source with the new bounded source
    GEP->setOperand(0, Result);
    GEP->moveAfter(cast<llvm::Instruction>(Result));
    Result = GEP;
  }
  return Result;
}

llvm::Value *CodeGenFunction::setCHERIBoundsOnReference(llvm::Value *Value,
                                                        QualType Ty,
                                                        const Expr *E) {
  if (getLangOpts().getCheriBounds() < LangOptions::CBM_References)
    return Value; // Not enabled

  // CHERI_BOUNDS_DBG(<< "Trying to set CHERI bounds on reference ";
  //                  E->dump(llvm::dbgs()));

  NumReferencesCheckedForBoundsTightening++;
  constexpr auto Kind = SubObjectBoundsKind::Reference;
  if (auto TBR = canTightenCheriBounds(Value, Ty, E, Kind)) {
    if (TBR->IsContainerSize) {
      NumContainerBoundsSetOnReferences++;
    } else if (TBR->UseRemainingSize) {
      NumRemainingSizeBoundsSetOnReferences++;
    } else {
      NumTightBoundsSetOnReferences++;
    }
    return tightenCHERIBounds(*this, Kind, Value, E, Ty, *TBR);
  }
  return Value;
}

llvm::Value *CodeGenFunction::setCHERIBoundsOnAddrOf(llvm::Value *Value,
                                                     clang::QualType Ty,
                                                     const Expr *E,
                                                     const Expr *LocExpr) {
  assert(getLangOpts().getCheriBounds() >= LangOptions::CBM_SubObjectsSafe);
  // CHERI_BOUNDS_DBG(<< "Trying to set CHERI bounds on addrof operator ";
  //                  E->dump(llvm::dbgs()));

  NumAddrOfCheckedForBoundsTightening++;
  constexpr auto Kind = SubObjectBoundsKind::AddrOf;
  if (auto TBR = canTightenCheriBounds(Value, Ty, E, Kind)) {
    if (TBR->IsContainerSize) {
      NumContainerBoundsSetOnAddrOf++;
    } else if (TBR->UseRemainingSize) {
      NumRemainingSizeBoundsSetOnAddrOf++;
    } else {
      NumTightBoundsSetOnAddrOf++;
    }
    return tightenCHERIBounds(*this, Kind, Value, LocExpr, Ty, *TBR);
  }
  return Value;
}

llvm::Value *
CodeGenFunction::setCHERIBoundsOnArraySubscript(llvm::Value *Value,
                                                const ArraySubscriptExpr *ASE) {
  assert(getLangOpts().getCheriBounds() >= LangOptions::CBM_SubObjectsSafe);
  // CHERI_BOUNDS_DBG(<< "Trying to set CHERI bounds on [] operator ";
  //                  E->dump(llvm::dbgs()));
  // We want to set the bounds on the subscript base type (NOT the result of
  // the ASE which will be char for a char[10] subscript!)
  const Expr *Base = ASE->getBase()->IgnoreParenImpCasts();
  QualType Ty = Base->getType();
  NumArraySubscriptCheckedForBoundsTightening++;
  constexpr auto Kind = SubObjectBoundsKind::ArraySubscript;
  if (auto TBR = canTightenCheriBounds(Value, Ty, Base, Kind)) {
    assert(!TBR->IsContainerSize && "Container size should not be use for array subscript bounds");
    if (TBR->UseRemainingSize) {
      NumRemainingSizeBoundsSetOnArraySubscript++;
    } else {
      NumTightBoundsSetOnArraySubscript++;
    }
    return tightenCHERIBounds(*this, Kind, Value, Base, Ty, *TBR);
  }
  return Value;
}

llvm::Value *
CodeGenFunction::setCHERIBoundsOnArrayDecay(llvm::Value *Value, const Expr *E) {
  // TODO: only do this for MemberExprs? Stack variables and globals should
  // already be bounded by the backend.

  assert(getLangOpts().getCheriBounds() >= LangOptions::CBM_SubObjectsSafe);
  // CHERI_BOUNDS_DBG(<< "Trying to set CHERI bounds on array decay ";
  //                  E->dump(llvm::dbgs()));

  const Expr *Base = E->IgnoreParenImpCasts();
  QualType Ty = Base->getType();
  assert(Ty->isArrayType());
  NumArrayDecayCheckedForBoundsTightening++;
  constexpr auto Kind = SubObjectBoundsKind::ArrayDecay;
  if (auto TBR = canTightenCheriBounds(Value, Ty, Base, Kind)) {
    assert(!TBR->IsContainerSize && "Container size should not be use for array decay bounds");
    if (TBR->UseRemainingSize) {
      NumRemainingSizeBoundsSetOnArrayDecay++;
    } else {
      NumTightBoundsSetOnArrayDecay++;
    }
    return tightenCHERIBounds(*this, Kind, Value, Base, Ty, *TBR);
  }
  return Value;
}

static Optional<CodeGenFunction::TightenBoundsResult>
cannotSetBounds(const CodeGenFunction &CGF, const Expr *E, QualType Type,
                SubObjectBoundsKind Kind, const Twine &Reason) {
  CGF.CGM.getDiags().Report(E->getExprLoc(),
                            diag::remark_no_cheri_subobject_bounds)
      << (int)Kind << Type << Reason.str() << E->getSourceRange();
  CHERI_BOUNDS_DBG(<< Reason << " -> not setting bounds\n");
  return None;
}

static bool hasBoundsOptOutAnnotation(const CodeGenFunction &CGF, const Expr *E,
                                      QualType Ty, SubObjectBoundsKind Kind,
                                      const Twine &Msg) {
  if (Ty.isNull())
    return false; // unknown type
  if (Ty->hasAttr(attr::CHERINoSubobjectBounds)) {
    CHERI_BOUNDS_DBG(<< "opt-out: ");
    cannotSetBounds(CGF, E, Ty, Kind, Msg + " has opt-out attribute");
    return true;
  }
  if (RecordDecl *RD = Ty->getAsRecordDecl()) {
    if (RD->hasAttr<CHERINoSubobjectBoundsAttr>()) {
      CHERI_BOUNDS_DBG(<< "opt-out: ");
      cannotSetBounds(CGF, E, QualType(RD->getTypeForDecl(), 0), Kind,
                      Msg + " declaration has opt-out attribute");
      return true;
    }
  }
  return false;
}

static bool hasBoundsOptOutAnnotation(const CodeGenFunction &CGF, const Expr *E,
                                      const Decl *D, SubObjectBoundsKind Kind,
                                      const StringRef Msg) {
  if (D->hasAttr<CHERINoSubobjectBoundsAttr>()) {
    StringRef Name = "<anonymous>";
    if (auto ND = dyn_cast<NamedDecl>(D))
      Name = ND->getName();
    CHERI_BOUNDS_DBG(<< "opt-out: ");
    std::string DiagStr = (Msg + " has opt-out attribute").str();
    if (auto TD = dyn_cast<TypeDecl>(D)) {
      cannotSetBounds(CGF, E, QualType(TD->getTypeForDecl(), 0), Kind, DiagStr);
    } else {
      CGF.CGM.getDiags().Report(E->getExprLoc(),
                                diag::remark_no_cheri_subobject_bounds)
          << (int)Kind << ("field '" + Name + "'").str() << DiagStr
          << E->getSourceRange();
      CHERI_BOUNDS_DBG(<< DiagStr << " -> not setting bounds\n");
    }
    return true;
  }
  if (auto VD = dyn_cast<ValueDecl>(D)) {
    return hasBoundsOptOutAnnotation(CGF, E, VD->getType(), Kind,
                                     Msg + " type");
  }
  return false;
}

// FIXME: should not tighten bounds on addresses of globals!

static void remarkUsingContainerSize(const CodeGenFunction &CGF, const Expr *E,
                                     QualType BaseTy, QualType Ty,
                                     const char *Msg) {
  CHERI_BOUNDS_DBG(<< Msg << " -> using container size -> ");
  CGF.CGM.getDiags().Report(E->getExprLoc(),
                            diag::remark_subobject_using_container_size)
      << BaseTy << Ty << Msg;
}

enum class ArrayBoundsResult {
  Never = 0,
  Always,
  UseFullArray,
  DependsOnType,
};

static ArrayBoundsResult canSetBoundsOnArraySubscript(
    const Expr *E, const ArraySubscriptExpr *ASE, SubObjectBoundsKind Kind,
    LangOptions::CheriBoundsMode BoundsMode, const CodeGenFunction &CGF) {
  // TODO: should we opt-out even for references?
  // TODO: I guess cheri_no_bounds should have a argument that identifies
  // which kinds of narrowing are fine
  if (hasBoundsOptOutAnnotation(CGF, E, ASE->getType(), Kind, "array type"))
    return ArrayBoundsResult::Never;
  const Expr *Base = ASE->getBase()->IgnoreParenImpCasts();
  const QualType BaseTy = Base->getType();
  if (hasBoundsOptOutAnnotation(CGF, E, BaseTy, Kind, "array base type"))
    return ArrayBoundsResult::Never;

  if (Kind == SubObjectBoundsKind::Reference &&
      BoundsMode >= LangOptions::CBM_References) {
    CHERI_BOUNDS_DBG(<< "using C++ reference -> ");
    return ArrayBoundsResult::Always;
  }
  const Expr *Index = ASE->getIdx();
  Expr::EvalResult ConstLengthResult;
  if (!Index->EvaluateAsInt(ConstLengthResult, CGF.getContext())) {
    // If the index is not a constant we should be able to set bounds:
    // This indicates the code is something like
    // for (int i = 0; i < max; i++) { do_something(array[i]); }
    // and therefore we should be able to tightly bound.
    CHERI_BOUNDS_DBG(
        << "Index is not a constant (probably in a per-element loop) -> ");
    // Use the full array bounds in safe mode since there is lots of code that
    // uses &foo[n] to get an unbounded member access instead of foo + n
    if (BoundsMode <= LangOptions::CBM_SubObjectsSafe) {
      if (BaseTy->isConstantArrayType())
        remarkUsingContainerSize(CGF, E, BaseTy, ASE->getType(), "&array[n]");
      return ArrayBoundsResult::UseFullArray;
    }
    return ArrayBoundsResult::DependsOnType;
  }
  llvm::APSInt ConstLength = ConstLengthResult.Val.getInt();
  CHERI_BOUNDS_DBG(<< "Index is a constant -> ");
  if (BoundsMode >= LangOptions::CBM_VeryAggressive) {
    CHERI_BOUNDS_DBG(<< "bounds-mode is very-aggressive -> bounds on "
                        "array[CONST] are fine -> ");
    return ArrayBoundsResult::DependsOnType;
  }
  // In aggressive mode we set bounds on everything except &array[0],
  // &array[last_index] and &array[last_index+1]
  // since those may be used to pass start and end indices to functions like
  // `for_each_elem(&array[0], &array[last_index]);
  if (ConstLength == 0) {
    if (BaseTy->isConstantArrayType())
      remarkUsingContainerSize(CGF, E, BaseTy, ASE->getType(), "&array[0]");
    return ArrayBoundsResult::UseFullArray;
  }

  if (BoundsMode < LangOptions::CBM_Aggressive) {
    // don't set bounds on array[constant] for subobject-safe
    if (BaseTy->isConstantArrayType())
      remarkUsingContainerSize(CGF, E, BaseTy, ASE->getType(),
                               "&array[<CONSTANT>]");
    return ArrayBoundsResult::UseFullArray;
  }

  assert(BoundsMode == LangOptions::CBM_Aggressive);
  // can't use operator<= here since it asserts
  if (const ConstantArrayType *CAT = dyn_cast<ConstantArrayType>(BaseTy)) {
    auto ArraySizeMinusOne = llvm::APSInt(CAT->getSize() - 1, false);
    if (llvm::APSInt::compareValues(ConstLength, ArraySizeMinusOne) >= 0) {
      remarkUsingContainerSize(CGF, E, BaseTy, ASE->getType(),
                               "bounds on &array[<last index>]");
      return ArrayBoundsResult::UseFullArray;
    }
  }
  CHERI_BOUNDS_DBG(
      << "const array index is not end and bounds==aggressive -> ");
  return ArrayBoundsResult::DependsOnType;
}

static FieldDecl* findPossibleVLA(const RecordDecl *RD) {
  const bool CheckingUnion = RD->isUnion();
  for (auto i = RD->field_begin(), end = RD->field_end(); i != end; ++i) {
    // We only check the last field (except for unions!)
    if (!CheckingUnion) {
      auto i2 = i;
      const bool IsLastField = (++i2 == end);
      if (!IsLastField)
        continue;
    }

    auto FieldTy = i->getType();
    // If a nested struct has a flexible array member, this union/struct also
    // has one.
    if (FieldTy->isRecordType()) {
      FieldDecl* NestedVLA = findPossibleVLA(FieldTy->getAsRecordDecl());
      if (NestedVLA)
        return NestedVLA;
    }
    if (FieldTy->isVariableArrayType() || FieldTy->isIncompleteArrayType())
      return *i;

    if (FieldTy->isConstantArrayType()) {
      if (const ConstantArrayType *CAT =
              dyn_cast<ConstantArrayType>(FieldTy.getTypePtr())) {
        // Assume that size 0 and size 1 arrays are meant to be
        // variable length arrays since that was the only way of
        // doing it before C99
        if (CAT->getSize() == 0 || CAT->getSize() == 1) {
          return *i;
        }
      }
    }
  }
  return nullptr;
}

static bool containsVariableLengthArray(LangOptions::CheriBoundsMode BoundsMode,
                                        QualType Ty) {
  auto RD = Ty->getAsRecordDecl();
  if (!RD)
    return false;

  if (RD->hasFlexibleArrayMember()) {
    CHERI_BOUNDS_DBG(<< "found real VLA in " << Ty.getAsString() << " -> ");
    return true;
  }
  if (BoundsMode >= LangOptions::CBM_VeryAggressive) {
    // In very-aggressive mode only accept real VLAs and not size 0 / size 1
    return false;
  }

  return findPossibleVLA(RD) != nullptr;
}

Optional<CodeGenFunction::TightenBoundsResult>
CodeGenFunction::canTightenCheriBounds(llvm::Value *Value, QualType Ty,
                                       const Expr *E,
                                       SubObjectBoundsKind Kind) {
  const auto BoundsMode = getLangOpts().getCheriBounds();
  assert(BoundsMode > LangOptions::CBM_Conservative);
  CHERI_BOUNDS_DBG(<< boundsKindStr(Kind) << " '" << Ty.getAsString()
                   << "' subobj bounds check: ");
  if (!CGM.getDataLayout().isFatPointer(Value->getType())) {
    CHERI_BOUNDS_DBG(<< "Cannot set bounds on non-capability IR type ";
                     Value->getType()->print(llvm::dbgs(), true);
                     llvm::dbgs() << "\n");
    return None;
  }

  if (Ty->isIncompleteType() && !Ty->isIncompleteArrayType()) {
    return cannotSetBounds(*this, E, Ty, Kind, "incomplete type");
  }

  if (Ty->isDependentType()) {
    return cannotSetBounds(*this, E, Ty, Kind, "dependent type");
  }

  if (Ty->isFunctionType()) {
    return cannotSetBounds(*this, E, Ty, Kind,
                           Kind == SubObjectBoundsKind::Reference
                               ? "reference to function"
                               : "address of function");
  }
  assert(CGM.getDataLayout().isFatPointer(Value->getType()));

  E = E->IgnoreParenImpCasts(); // ignore array-to-pointer decay, etc
  // And we also neede to ignore parenexprs since oterwise we get a ParenExpr
  // instead of ArraySubscriptExpr/MemberExpr!

  // Any expression other than DeclRefExpr (e.g. in the case &x) will be a
  // sub-object expression (array index, member expression (&x.a)
  const bool IsSubObject = !isa<DeclRefExpr>(E);
  // Check if the type of the expression or the container of the member expr
  // is annotated with no subobject bounds. In this case we must never set
  // bounds (even in everywhere-unsafe mode!)
  int64_t TypeSize = getContext().getTypeSizeInChars(Ty).getQuantity();

  const auto BoundsOnContainer = [this, IsSubObject, Kind](QualType Container) {
    CodeGenFunction::TightenBoundsResult Result;
    Result.IsSubObject = IsSubObject;
    Result.IsContainerSize = true;
    Result.Size = getContext().getTypeSizeInChars(Container).getQuantity();
    assert(Kind != SubObjectBoundsKind::ArraySubscript);
    return Result;
  };
  ValueDecl* TargetField = nullptr;
  const auto ExactBounds = [IsSubObject, &TargetField](int64_t Size) {
    CodeGenFunction::TightenBoundsResult Result;
    Result.IsSubObject = IsSubObject;
    Result.IsContainerSize = false;
    Result.TargetField = TargetField;
    Result.Size = Size;
    return Result;
  };
  const auto UseRemainingSize = [IsSubObject,
                                 &TargetField](Twine Msg, uint64_t MaxSize = 0,
                                               bool IsContainerSize = false) {
    CodeGenFunction::TightenBoundsResult Result;
    Result.IsSubObject = IsSubObject;
    Result.IsContainerSize = IsContainerSize;
    Result.UseRemainingSize = true;
    if (MaxSize != 0) {
      Result.Size = MaxSize;
    }
    Result.TargetField = TargetField;
    if (!Msg.isTriviallyEmpty())
      Result.DiagMessage = (" (" + Msg + ")").str();
    return Result;
  };

  // Do not set bounds on &foo if foo is already a reference (we can just
  // trust the original reference size. If that size is wrong, it means that
  // the calling code was compiled without sub-object bounds and therefore
  // tightening the bounds might not be safe!
  //
  // Note: the same also applies for C++ references (there is no need to add
  // another csetbounds if we are just forwarding a reference to another call)
  if (getLangOpts().CPlusPlus && (Kind == SubObjectBoundsKind::AddrOf ||
                                  Kind == SubObjectBoundsKind::Reference)) {
    // TODO: fix getRealReferenceType() to take ASTContext&
    bool IsReference = false;
    if (const auto *CE = dyn_cast<const CallExpr>(E)) {
      QualType CallTy = CE->getCallReturnType(getContext());
      IsReference = CallTy->isReferenceType();
      IsReference = CE->getCallReturnType(getContext())->isReferenceType();
      if (IsReference && Kind == SubObjectBoundsKind::AddrOf) {
        // diagnose address-of std::string/vector operator[]/.at();
        // This would set bounds to one element but most uses seem to be &buf[0]
        // which clearly wants the full buffer as a pointer.
        if (auto FD = CE->getDirectCallee()) {
          if (auto CXXMD = dyn_cast<CXXMethodDecl>(FD)) {
            bool KnownBad = CXXMD->getOverloadedOperator() == OO_Subscript;
            auto Name = CXXMD->getDeclName();
            if (!KnownBad && Name.isIdentifier()) {
              // Handle taking the address of .at(N)/.front()/.back()
              // NumParams == 1 for .at() due to implicit this not being counted
              auto ExpectedParams =
                  llvm::StringSwitch<Optional<unsigned>>(CXXMD->getName())
                      .Case("at", 1)
                      .Cases("front", "back", 0)
                      .Default(None);
              if (ExpectedParams && CXXMD->getNumParams() == *ExpectedParams)
                KnownBad = true;
            }
            if (KnownBad) {
              CGM.getDiags().Report(
                  E->getExprLoc(),
                  diag::warn_subobject_bounds_addressof_subscript)
                  << FD << E->getSourceRange();
              CGM.getDiags().Report(
                  E->getExprLoc(),
                  diag::note_subobject_bounds_addressof_subscript_fixit)
                  << CallTy << E->getSourceRange();
            }
          }
        }
      }
    } else {
      // We really only want a reference type for DeclRefExprs here.
      // getRealReferenceType() returns a reference type for all non-pointer
      // LValue expressions that have an underlying capability by default so
      // pass false for LValuesAsReferences.
      IsReference =
          E->getRealReferenceType(getContext(), /*LValuesAsReferences=*/false)
              ->isReferenceType();
    }
    if (IsReference)
      return cannotSetBounds(*this, E, Ty, Kind,
                             "source is a C++ reference and therefore should "
                             "already have sub-object bounds");
  }

  if (auto DRE = dyn_cast<DeclRefExpr>(E)) {
    // Don't set bounds on weak symbols since they might be NULL
    // See https://github.com/CTSRD-CHERI/llvm-project/issues/317
    // TODO: we could add a branch and only do the csetbounds if non-null.
    // However, these symbols will be global symbols so there is really no need
    // to set bounds (since the linker should initialize them correctly).
    // The only advantage of setting bounds here would be to detect mismatch
    // between the extern type declaration and the real declaration.
    // FIXME: should this be getDecl or getFoundDecl?
    auto Found = DRE->getFoundDecl();
    auto ValDecl = dyn_cast<ValueDecl>(Found);
    if ((ValDecl && ValDecl->isWeak()) || Found->hasAttr<WeakAttr>()) {
      return cannotSetBounds(
          *this, E, Ty, Kind,
          "referenced value is a weak symbol and could therefore be NULL");
    }
  }
  if (auto ME = dyn_cast<MemberExpr>(E)) {
    CHERI_BOUNDS_DBG(<< "got MemberExpr -> ");
    // TODO: should we do this recusively? E.g. for &foo.a.b.c.d if type a is
    // annotated with no bounds should that apply to d?
    auto BaseTy = ME->getBase()->getType();
    if (ME->isArrow() && !BaseTy->getPointeeType().isNull()) {
      if (hasBoundsOptOutAnnotation(*this, E, BaseTy, Kind,
                                    "base pointer type"))
        return None;
      BaseTy = BaseTy->getPointeeType();
    }
    if (hasBoundsOptOutAnnotation(*this, E, BaseTy, Kind, "base type"))
      return None;
    if (hasBoundsOptOutAnnotation(*this, E, ME->getMemberDecl(), Kind, "field"))
      return None;

    // Check whether the field or the field type has a "use-remaining-size" attr
    // TODO: also allow this attribute on typedefs?
    TargetField = ME->getMemberDecl();
    auto RSA = TargetField->getAttr<CHERISubobjectBoundsUseRemainingSizeAttr>();
    StringRef AttrSource;
    if (RSA) {
      AttrSource = "member";
    } else {
      AttrSource = "member type";
      if (auto TypeDecl = TargetField->getType()->getAsRecordDecl())
        RSA = TypeDecl->getAttr<CHERISubobjectBoundsUseRemainingSizeAttr>();
    }
    if (RSA) {
      CHERI_BOUNDS_DBG(<< "got RemainingSize(" << RSA->getMaxSize()
                       << ") attribute on " << AttrSource << " -> ");
      return UseRemainingSize(AttrSource + " has use-remaining-size attribute",
                              RSA->getMaxSize());
    }

    if (BoundsMode < LangOptions::CBM_VeryAggressive &&
        ME->getMemberDecl() == findPossibleVLA(BaseTy->getAsRecordDecl()))
      return UseRemainingSize("member is potential variable length array");

    // For unions we have to be careful since some code uses pointers to members
    // interchangably with a pointer to the main union. Use the containing type
    // unless we are doing an array subcript operation or are in aggressive
    // mode.
    if (Kind != SubObjectBoundsKind::ArraySubscript && Kind != SubObjectBoundsKind::ArrayDecay && BaseTy->isUnionType() &&
        BoundsMode < LangOptions::CBM_VeryAggressive) {
      // FIXME: we should set bounds to the whole union rather than not setting bounds at all
      // FIXME: should we set bounds for references?
      if (BoundsMode < LangOptions::CBM_References)
        return cannotSetBounds(*this, E, Ty, Kind, "container is union");

      if (containsVariableLengthArray(BoundsMode, BaseTy))
        return UseRemainingSize(
            "containing union includes a variable length array");

      remarkUsingContainerSize(*this, E, BaseTy, Ty, "union member");
      return BoundsOnContainer(BaseTy);
    }
  }

  // Array subscripts are the easiest case to handle:
  // We always set bounds if it is a constant size array, otherwise never set
  // them since we can't known the actual size. This is very similar to
  // -fsanitize=array-size and is equivalent to having a trap if index greater
  // constant size (but with a CHERI bounds violation instead)
  if (Kind == SubObjectBoundsKind::ArraySubscript || Kind == SubObjectBoundsKind::ArrayDecay) {
    // E->dumpColor();
    // E->dumpPretty(getContext());
    // For array subscripts we can only set bounds for array types, and not
    // pointers since those have an unknown size
    // TODO: we could ignore all casts and check the underlying type
    // i.e. ((char*)buf)[10] if buf is an array of different type
    StringRef KindStr = boundsKindStr(Kind);
    if (!Ty->isArrayType()) {
      return cannotSetBounds(*this, E, Ty, Kind,
                             "array " + KindStr + " on non-array type");
    } else if (Ty->isIncompleteArrayType() || !Ty->isConstantSizeType()) {
      return UseRemainingSize("array " + KindStr + " on variable size type");
    }
    CHERI_BOUNDS_DBG(<< KindStr << " on constant size array -> ");
    return ExactBounds(TypeSize);
  }

  if (auto ASE = dyn_cast<ArraySubscriptExpr>(E)) {
    CHERI_BOUNDS_DBG(<< "Found array subscript -> ");
    switch (canSetBoundsOnArraySubscript(E, ASE, Kind, BoundsMode, *this)) {
    case ArrayBoundsResult::Never:
      return None;
    case ArrayBoundsResult::Always:
      return ExactBounds(TypeSize);
    case ArrayBoundsResult::UseFullArray: {
      const Expr *Base = ASE->getBase()->IgnoreParenImpCasts();
      if (Base->getType()->isConstantArrayType()) {
        return BoundsOnContainer(Base->getType());
      } else if (Base->getType()->isArrayType()) {
        UseRemainingSize("bounds on full array but size not known", 0, true);
      }
      // Otherwise we have a non-constant array type -> don't set bounds to
      // avoid crashes at runtime
      return cannotSetBounds(
          *this, E, Ty, Kind,
          "should set bounds on full array but size is not known");
    }
    case ArrayBoundsResult::DependsOnType:
      break;
    }
  }

  // General opt-out based on the type of the expression
  if (hasBoundsOptOutAnnotation(*this, E, Ty, Kind, "expression"))
    return None;

  // if (Ty->isArrayType()) {
  if (const ArrayType *AT = getContext().getAsArrayType(Ty)) {
    if (isa<ConstantArrayType>(AT)) {
      CHERI_BOUNDS_DBG(<< "Found constant size array type -> ");
      // FIXME: what about size 0/size 1 VLA emulation for pre-C99 code
      return ExactBounds(TypeSize);
    } else if (isa<VariableArrayType>(AT)) {
      return UseRemainingSize("variable length array type");
    } else if (isa<IncompleteArrayType>(AT)) {
      // int a[]
      return UseRemainingSize("incomplete array type");
    } else if (isa<DependentSizedArrayType>(AT)) {
      llvm_unreachable("Should not get dependent types in subobject codegen");
      return cannotSetBounds(*this, E, Ty, Kind, "dependent size array type");
    } else {
      llvm_unreachable("Unhandled array type");
    }
  }

  assert(Ty->isConstantSizeType() && "unexpected variable size type.");
  // Zero-size structs are okay, but any other zero-size type should already
  // have benn handled:
  assert((Ty->isRecordType() || TypeSize != 0) && "Unknown size should already have been handled.");

  if (BoundsMode >= LangOptions::CBM_EverywhereUnsafe) {
    CHERI_BOUNDS_DBG(<< "Bounds mode is everywhere-unsafe -> ");
    return ExactBounds(TypeSize);
  }

  if (auto AT = Ty->getAs<AtomicType>()) {
    CHERI_BOUNDS_DBG(<< "unwrapping _Atomic type -> ");
    Ty = AT->getValueType();
  }

  // It should be possible to set the size for all scalar types
  if (Ty->isScalarType()) {
    CHERI_BOUNDS_DBG(<< "Found scalar type -> ");
    return ExactBounds(TypeSize);
  }
  // Same for vector types (they are fixed size)
  if (Ty->isVectorType()) {
    CHERI_BOUNDS_DBG(<< "Found vector type -> ");
    return ExactBounds(TypeSize);
  }

  assert(!Ty->isArrayType());
  // It because a bit more tricky for class types since they might be
  // downcasted to something with a larger size.
  // TODO: can we try to set bounds on all classes without a vtable
  // I guess final classes would work
  if (Ty->isRecordType()) {
    CHERI_BOUNDS_DBG(<< "Found record type '" << Ty.getAsString() << "' -> ");
    if (containsVariableLengthArray(BoundsMode, Ty)) {
      return UseRemainingSize("record has flexible array member");
    }
    // FIXME: some of this is too conservative and we could actually set bounds
    if (Ty->isStructureOrClassType() && !getLangOpts().CPlusPlus) {
      // No inheritance or vtables in C -> we should be able to set bounds on
      // all structurs that don't have flexible array members and aren't
      // annotated as opt-out
      CHERI_BOUNDS_DBG(<< "compiling C and no flexible array -> ");
      return ExactBounds(TypeSize);
    } else if (Ty->isCXXStructureOrClassType()) {
      CXXRecordDecl *CRD = Ty->getAsCXXRecordDecl();
      const bool IsFinalClass = CRD->hasAttr<FinalAttr>();
      // TODO: isCLike() -> safe to set bounds? hopefully not inherited from?
      if (!IsFinalClass && BoundsMode <= LangOptions::CBM_SubObjectsSafe) {
        return cannotSetBounds(
            *this, E, Ty, Kind,
            "non-final class and using sub-object-safe mode");
      }
      // No bounds on classes with vtables
      if (CRD->isCLike()) {
        CHERI_BOUNDS_DBG(<< "is C-like struct type and is marked as final -> ");
        return ExactBounds(TypeSize);
      }
      // Final class: check it doesn't have any virtual bases
      // TODO: check there are no flexible array members
      if (!CRD->isLiteral()) {
        return cannotSetBounds(
            *this, E, Ty, Kind,
            "final but not a literal type -> size might by dynamic");
      } else {
        assert(CRD->getNumVBases() == 0);
        CHERI_BOUNDS_DBG(<< "is literal type and is marked as final -> ");
        return ExactBounds(TypeSize);
      }
    }
    return cannotSetBounds(*this, E, Ty, Kind, "not a struct/class");
  }
  // Otherwise this type is unhandled, let's print a message:
  CGM.getDiags().Report(E->getExprLoc(),
                        diag::warn_subobject_bounds_unknown_type)
      << Ty << E->getSourceRange();
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  llvm::errs() << "Don't know whether to set bounds:\n";
  E->dumpColor();
  llvm::errs() << "\tExpression type:\n";
  Ty.dump();
#endif
  // TODO: assert here to find all the cases?
  // llvm_unreachable("Don't know whether to set bounds on type");
  return None;
}

RValue
CodeGenFunction::EmitReferenceBindingToExpr(const Expr *E) {
  // Emit the expression as an lvalue.
  LValue LV = EmitLValue(E);
  assert(LV.isSimple());
  llvm::Value *Value = LV.getPointer();
  QualType Ty = E->getType();
  if (sanitizePerformTypeCheck() && !Ty->isFunctionType()) {
    // C++11 [dcl.ref]p5 (as amended by core issue 453):
    //   If a glvalue to which a reference is directly bound designates neither
    //   an existing object or function of an appropriate type nor a region of
    //   storage of suitable size and alignment to contain an object of the
    //   reference's type, the behavior is undefined.
    QualType Ty = E->getType();
    EmitTypeCheck(TCK_ReferenceBinding, E->getExprLoc(), Value, Ty);
  }
  // For CHERI we want to insert bounds on references (at least for simple
  // types) unless they are references to functions
  // TODO: we should probably check if references are capabilities instead since
  // there could be a mode where references are capabilities but pointers aren't
  if (CGM.getTarget().areAllPointersCapabilities() && !Ty->isFunctionType())
    Value = setCHERIBoundsOnReference(Value, Ty, E);
  return RValue::get(Value, LV.getAlignment().getQuantity());
}


/// getAccessedFieldNo - Given an encoded value and a result number, return the
/// input field number being accessed.
unsigned CodeGenFunction::getAccessedFieldNo(unsigned Idx,
                                             const llvm::Constant *Elts) {
  return cast<llvm::ConstantInt>(Elts->getAggregateElement(Idx))
      ->getZExtValue();
}

/// Emit the hash_16_bytes function from include/llvm/ADT/Hashing.h.
static llvm::Value *emitHash16Bytes(CGBuilderTy &Builder, llvm::Value *Low,
                                    llvm::Value *High) {
  llvm::Value *KMul = Builder.getInt64(0x9ddfea08eb382d69ULL);
  llvm::Value *K47 = Builder.getInt64(47);
  llvm::Value *A0 = Builder.CreateMul(Builder.CreateXor(Low, High), KMul);
  llvm::Value *A1 = Builder.CreateXor(Builder.CreateLShr(A0, K47), A0);
  llvm::Value *B0 = Builder.CreateMul(Builder.CreateXor(High, A1), KMul);
  llvm::Value *B1 = Builder.CreateXor(Builder.CreateLShr(B0, K47), B0);
  return Builder.CreateMul(B1, KMul);
}

bool CodeGenFunction::isNullPointerAllowed(TypeCheckKind TCK) {
  return TCK == TCK_DowncastPointer || TCK == TCK_Upcast ||
         TCK == TCK_UpcastToVirtualBase || TCK == TCK_DynamicOperation;
}

bool CodeGenFunction::isVptrCheckRequired(TypeCheckKind TCK, QualType Ty) {
  CXXRecordDecl *RD = Ty->getAsCXXRecordDecl();
  return (RD && RD->hasDefinition() && RD->isDynamicClass()) &&
         (TCK == TCK_MemberAccess || TCK == TCK_MemberCall ||
          TCK == TCK_DowncastPointer || TCK == TCK_DowncastReference ||
          TCK == TCK_UpcastToVirtualBase || TCK == TCK_DynamicOperation);
}

bool CodeGenFunction::sanitizePerformTypeCheck() const {
  return SanOpts.has(SanitizerKind::Null) |
         SanOpts.has(SanitizerKind::Alignment) |
         SanOpts.has(SanitizerKind::ObjectSize) |
         SanOpts.has(SanitizerKind::Vptr);
}

void CodeGenFunction::EmitTypeCheck(TypeCheckKind TCK, SourceLocation Loc,
                                    llvm::Value *Ptr, QualType Ty,
                                    CharUnits Alignment,
                                    SanitizerSet SkippedChecks,
                                    llvm::Value *ArraySize) {
  // TODO: if this was called everywhere we might be able to add CHERI reduced
  // bounds in a central place instead of having toChecks
  if (!sanitizePerformTypeCheck())
    return;

  // Don't check pointers outside the default address space. The null check
  // isn't correct, the object-size check isn't supported by LLVM, and we can't
  // communicate the addresses to the runtime handler for the vptr check.
  if (Ptr->getType()->getPointerAddressSpace())
    return;

  // Don't check pointers to volatile data. The behavior here is implementation-
  // defined.
  if (Ty.isVolatileQualified())
    return;

  SanitizerScope SanScope(this);

  SmallVector<std::pair<llvm::Value *, SanitizerMask>, 3> Checks;
  llvm::BasicBlock *Done = nullptr;

  // Quickly determine whether we have a pointer to an alloca. It's possible
  // to skip null checks, and some alignment checks, for these pointers. This
  // can reduce compile-time significantly.
  auto PtrToAlloca =
      dyn_cast<llvm::AllocaInst>(Ptr->stripPointerCastsNoFollowAliases());

  llvm::Value *True = llvm::ConstantInt::getTrue(getLLVMContext());
  llvm::Value *IsNonNull = nullptr;
  bool IsGuaranteedNonNull =
      SkippedChecks.has(SanitizerKind::Null) || PtrToAlloca;
  bool AllowNullPointers = isNullPointerAllowed(TCK);
  if ((SanOpts.has(SanitizerKind::Null) || AllowNullPointers) &&
      !IsGuaranteedNonNull) {
    // The glvalue must not be an empty glvalue.
    IsNonNull = Builder.CreateIsNotNull(Ptr);

    // The IR builder can constant-fold the null check if the pointer points to
    // a constant.
    IsGuaranteedNonNull = IsNonNull == True;

    // Skip the null check if the pointer is known to be non-null.
    if (!IsGuaranteedNonNull) {
      if (AllowNullPointers) {
        // When performing pointer casts, it's OK if the value is null.
        // Skip the remaining checks in that case.
        Done = createBasicBlock("null");
        llvm::BasicBlock *Rest = createBasicBlock("not.null");
        Builder.CreateCondBr(IsNonNull, Rest, Done);
        EmitBlock(Rest);
      } else {
        Checks.push_back(std::make_pair(IsNonNull, SanitizerKind::Null));
      }
    }
  }

  if (SanOpts.has(SanitizerKind::ObjectSize) &&
      !SkippedChecks.has(SanitizerKind::ObjectSize) &&
      !Ty->isIncompleteType()) {
    uint64_t TySize = getContext().getTypeSizeInChars(Ty).getQuantity();
    llvm::Value *Size = llvm::ConstantInt::get(IntPtrTy, TySize);
    if (ArraySize)
      Size = Builder.CreateMul(Size, ArraySize);

    // Degenerate case: new X[0] does not need an objectsize check.
    llvm::Constant *ConstantSize = dyn_cast<llvm::Constant>(Size);
    if (!ConstantSize || !ConstantSize->isNullValue()) {
      // The glvalue must refer to a large enough storage region.
      // FIXME: If Address Sanitizer is enabled, insert dynamic instrumentation
      //        to check this.
      // FIXME: Get object address space
      llvm::Type *Tys[2] = { IntPtrTy, Int8PtrTy };
      llvm::Function *F = CGM.getIntrinsic(llvm::Intrinsic::objectsize, Tys);
      llvm::Value *Min = Builder.getFalse();
      llvm::Value *NullIsUnknown = Builder.getFalse();
      llvm::Value *Dynamic = Builder.getFalse();
      llvm::Value *CastAddr = Builder.CreateBitCast(Ptr, Int8PtrTy);
      llvm::Value *LargeEnough = Builder.CreateICmpUGE(
          Builder.CreateCall(F, {CastAddr, Min, NullIsUnknown, Dynamic}), Size);
      Checks.push_back(std::make_pair(LargeEnough, SanitizerKind::ObjectSize));
    }
  }

  uint64_t AlignVal = 0;
  llvm::Value *PtrAsInt = nullptr;

  if (SanOpts.has(SanitizerKind::Alignment) &&
      !SkippedChecks.has(SanitizerKind::Alignment)) {
    AlignVal = Alignment.getQuantity();
    if (!Ty->isIncompleteType() && !AlignVal)
      AlignVal = getContext().getTypeAlignInChars(Ty).getQuantity();

    // The glvalue must be suitably aligned.
    if (AlignVal > 1 &&
        (!PtrToAlloca || PtrToAlloca->getAlignment() < AlignVal)) {
      PtrAsInt = Builder.CreatePtrToInt(Ptr, IntPtrTy);
      llvm::Value *Align = Builder.CreateAnd(
          PtrAsInt, llvm::ConstantInt::get(IntPtrTy, AlignVal - 1));
      llvm::Value *Aligned =
          Builder.CreateICmpEQ(Align, llvm::ConstantInt::get(IntPtrTy, 0));
      if (Aligned != True)
        Checks.push_back(std::make_pair(Aligned, SanitizerKind::Alignment));
    }
  }

  if (Checks.size() > 0) {
    // Make sure we're not losing information. Alignment needs to be a power of
    // 2
    assert(!AlignVal || (uint64_t)1 << llvm::Log2_64(AlignVal) == AlignVal);
    llvm::Constant *StaticData[] = {
        EmitCheckSourceLocation(Loc), EmitCheckTypeDescriptor(Ty),
        llvm::ConstantInt::get(Int8Ty, AlignVal ? llvm::Log2_64(AlignVal) : 1),
        llvm::ConstantInt::get(Int8Ty, TCK)};
    EmitCheck(Checks, SanitizerHandler::TypeMismatch, StaticData,
              PtrAsInt ? PtrAsInt : Ptr);
  }

  // If possible, check that the vptr indicates that there is a subobject of
  // type Ty at offset zero within this object.
  //
  // C++11 [basic.life]p5,6:
  //   [For storage which does not refer to an object within its lifetime]
  //   The program has undefined behavior if:
  //    -- the [pointer or glvalue] is used to access a non-static data member
  //       or call a non-static member function
  if (SanOpts.has(SanitizerKind::Vptr) &&
      !SkippedChecks.has(SanitizerKind::Vptr) && isVptrCheckRequired(TCK, Ty)) {
    // Ensure that the pointer is non-null before loading it. If there is no
    // compile-time guarantee, reuse the run-time null check or emit a new one.
    if (!IsGuaranteedNonNull) {
      if (!IsNonNull)
        IsNonNull = Builder.CreateIsNotNull(Ptr);
      if (!Done)
        Done = createBasicBlock("vptr.null");
      llvm::BasicBlock *VptrNotNull = createBasicBlock("vptr.not.null");
      Builder.CreateCondBr(IsNonNull, VptrNotNull, Done);
      EmitBlock(VptrNotNull);
    }

    // Compute a hash of the mangled name of the type.
    //
    // FIXME: This is not guaranteed to be deterministic! Move to a
    //        fingerprinting mechanism once LLVM provides one. For the time
    //        being the implementation happens to be deterministic.
    SmallString<64> MangledName;
    llvm::raw_svector_ostream Out(MangledName);
    CGM.getCXXABI().getMangleContext().mangleCXXRTTI(Ty.getUnqualifiedType(),
                                                     Out);

    // Blacklist based on the mangled type.
    if (!CGM.getContext().getSanitizerBlacklist().isBlacklistedType(
            SanitizerKind::Vptr, Out.str())) {
      llvm::hash_code TypeHash = hash_value(Out.str());

      // Load the vptr, and compute hash_16_bytes(TypeHash, vptr).
      llvm::Value *Low = llvm::ConstantInt::get(Int64Ty, TypeHash);
      llvm::Type *VPtrTy = llvm::PointerType::get(IntPtrTy, 0);
      Address VPtrAddr(Builder.CreateBitCast(Ptr, VPtrTy), getPointerAlign());
      llvm::Value *VPtrVal = Builder.CreateLoad(VPtrAddr);
      llvm::Value *High = Builder.CreateZExt(VPtrVal, Int64Ty);

      llvm::Value *Hash = emitHash16Bytes(Builder, Low, High);
      Hash = Builder.CreateTrunc(Hash, IntPtrTy);

      // Look the hash up in our cache.
      const int CacheSize = 128;
      llvm::Type *HashTable = llvm::ArrayType::get(IntPtrTy, CacheSize);
      llvm::Value *Cache = CGM.CreateRuntimeVariable(HashTable,
                                                     "__ubsan_vptr_type_cache");
      llvm::Value *Slot = Builder.CreateAnd(Hash,
                                            llvm::ConstantInt::get(IntPtrTy,
                                                                   CacheSize-1));
      llvm::Value *Indices[] = { Builder.getInt32(0), Slot };
      llvm::Value *CacheVal =
        Builder.CreateAlignedLoad(Builder.CreateInBoundsGEP(Cache, Indices),
                                  getPointerAlign());

      // If the hash isn't in the cache, call a runtime handler to perform the
      // hard work of checking whether the vptr is for an object of the right
      // type. This will either fill in the cache and return, or produce a
      // diagnostic.
      llvm::Value *EqualHash = Builder.CreateICmpEQ(CacheVal, Hash);
      llvm::Constant *StaticData[] = {
        EmitCheckSourceLocation(Loc),
        EmitCheckTypeDescriptor(Ty),
        CGM.GetAddrOfRTTIDescriptor(Ty.getUnqualifiedType()),
        llvm::ConstantInt::get(Int8Ty, TCK)
      };
      llvm::Value *DynamicData[] = { Ptr, Hash };
      EmitCheck(std::make_pair(EqualHash, SanitizerKind::Vptr),
                SanitizerHandler::DynamicTypeCacheMiss, StaticData,
                DynamicData);
    }
  }

  if (Done) {
    Builder.CreateBr(Done);
    EmitBlock(Done);
  }
}

/// Determine whether this expression refers to a flexible array member in a
/// struct. We disable array bounds checks for such members.
static bool isFlexibleArrayMemberExpr(const Expr *E) {
  // For compatibility with existing code, we treat arrays of length 0 or
  // 1 as flexible array members.
  const ArrayType *AT = E->getType()->castAsArrayTypeUnsafe();
  if (const auto *CAT = dyn_cast<ConstantArrayType>(AT)) {
    if (CAT->getSize().ugt(1))
      return false;
  } else if (!isa<IncompleteArrayType>(AT))
    return false;

  E = E->IgnoreParens();

  // A flexible array member must be the last member in the class.
  if (const auto *ME = dyn_cast<MemberExpr>(E)) {
    // FIXME: If the base type of the member expr is not FD->getParent(),
    // this should not be treated as a flexible array member access.
    if (const auto *FD = dyn_cast<FieldDecl>(ME->getMemberDecl())) {
      RecordDecl::field_iterator FI(
          DeclContext::decl_iterator(const_cast<FieldDecl *>(FD)));
      return ++FI == FD->getParent()->field_end();
    }
  } else if (const auto *IRE = dyn_cast<ObjCIvarRefExpr>(E)) {
    return IRE->getDecl()->getNextIvar() == nullptr;
  }

  return false;
}

llvm::Value *CodeGenFunction::LoadPassedObjectSize(const Expr *E,
                                                   QualType EltTy) {
  ASTContext &C = getContext();
  uint64_t EltSize = C.getTypeSizeInChars(EltTy).getQuantity();
  if (!EltSize)
    return nullptr;

  auto *ArrayDeclRef = dyn_cast<DeclRefExpr>(E->IgnoreParenImpCasts());
  if (!ArrayDeclRef)
    return nullptr;

  auto *ParamDecl = dyn_cast<ParmVarDecl>(ArrayDeclRef->getDecl());
  if (!ParamDecl)
    return nullptr;

  auto *POSAttr = ParamDecl->getAttr<PassObjectSizeAttr>();
  if (!POSAttr)
    return nullptr;

  // Don't load the size if it's a lower bound.
  int POSType = POSAttr->getType();
  if (POSType != 0 && POSType != 1)
    return nullptr;

  // Find the implicit size parameter.
  auto PassedSizeIt = SizeArguments.find(ParamDecl);
  if (PassedSizeIt == SizeArguments.end())
    return nullptr;

  const ImplicitParamDecl *PassedSizeDecl = PassedSizeIt->second;
  assert(LocalDeclMap.count(PassedSizeDecl) && "Passed size not loadable");
  Address AddrOfSize = LocalDeclMap.find(PassedSizeDecl)->second;
  llvm::Value *SizeInBytes = EmitLoadOfScalar(AddrOfSize, /*Volatile=*/false,
                                              C.getSizeType(), E->getExprLoc());
  llvm::Value *SizeOfElement =
      llvm::ConstantInt::get(SizeInBytes->getType(), EltSize);
  return Builder.CreateUDiv(SizeInBytes, SizeOfElement);
}

/// If Base is known to point to the start of an array, return the length of
/// that array. Return 0 if the length cannot be determined.
static llvm::Value *getArrayIndexingBound(
    CodeGenFunction &CGF, const Expr *Base, QualType &IndexedType) {
  // For the vector indexing extension, the bound is the number of elements.
  if (const VectorType *VT = Base->getType()->getAs<VectorType>()) {
    IndexedType = Base->getType();
    return CGF.Builder.getInt32(VT->getNumElements());
  }

  Base = Base->IgnoreParens();

  if (const auto *CE = dyn_cast<CastExpr>(Base)) {
    if (CE->getCastKind() == CK_ArrayToPointerDecay &&
        !isFlexibleArrayMemberExpr(CE->getSubExpr())) {
      IndexedType = CE->getSubExpr()->getType();
      const ArrayType *AT = IndexedType->castAsArrayTypeUnsafe();
      if (const auto *CAT = dyn_cast<ConstantArrayType>(AT))
        return CGF.Builder.getInt(CAT->getSize());
      else if (const auto *VAT = dyn_cast<VariableArrayType>(AT))
        return CGF.getVLASize(VAT).NumElts;
      // Ignore pass_object_size here. It's not applicable on decayed pointers.
    }
  }

  QualType EltTy{Base->getType()->getPointeeOrArrayElementType(), 0};
  if (llvm::Value *POS = CGF.LoadPassedObjectSize(Base, EltTy)) {
    IndexedType = Base->getType();
    return POS;
  }

  return nullptr;
}

void CodeGenFunction::EmitBoundsCheck(const Expr *E, const Expr *Base,
                                      llvm::Value *Index, QualType IndexType,
                                      bool Accessed) {
  assert(SanOpts.has(SanitizerKind::ArrayBounds) &&
         "should not be called unless adding bounds checks");
  SanitizerScope SanScope(this);

  QualType IndexedType;
  llvm::Value *Bound = getArrayIndexingBound(*this, Base, IndexedType);
  if (!Bound)
    return;

  bool IndexSigned = IndexType->isSignedIntegerOrEnumerationType();
  llvm::Value *IndexVal = Builder.CreateIntCast(Index, SizeTy, IndexSigned);
  llvm::Value *BoundVal = Builder.CreateIntCast(Bound, SizeTy, false);

  llvm::Constant *StaticData[] = {
    EmitCheckSourceLocation(E->getExprLoc()),
    EmitCheckTypeDescriptor(IndexedType),
    EmitCheckTypeDescriptor(IndexType)
  };
  llvm::Value *Check = Accessed ? Builder.CreateICmpULT(IndexVal, BoundVal)
                                : Builder.CreateICmpULE(IndexVal, BoundVal);
  EmitCheck(std::make_pair(Check, SanitizerKind::ArrayBounds),
            SanitizerHandler::OutOfBounds, StaticData, Index);
}


CodeGenFunction::ComplexPairTy CodeGenFunction::
EmitComplexPrePostIncDec(const UnaryOperator *E, LValue LV,
                         bool isInc, bool isPre) {
  ComplexPairTy InVal = EmitLoadOfComplex(LV, E->getExprLoc());

  llvm::Value *NextVal;
  if (isa<llvm::IntegerType>(InVal.first->getType())) {
    uint64_t AmountVal = isInc ? 1 : -1;
    NextVal = llvm::ConstantInt::get(InVal.first->getType(), AmountVal, true);

    // Add the inc/dec to the real part.
    NextVal = Builder.CreateAdd(InVal.first, NextVal, isInc ? "inc" : "dec");
  } else {
    QualType ElemTy = E->getType()->getAs<ComplexType>()->getElementType();
    llvm::APFloat FVal(getContext().getFloatTypeSemantics(ElemTy), 1);
    if (!isInc)
      FVal.changeSign();
    NextVal = llvm::ConstantFP::get(getLLVMContext(), FVal);

    // Add the inc/dec to the real part.
    NextVal = Builder.CreateFAdd(InVal.first, NextVal, isInc ? "inc" : "dec");
  }

  ComplexPairTy IncVal(NextVal, InVal.second);

  // Store the updated result through the lvalue.
  EmitStoreOfComplex(IncVal, LV, /*init*/ false);

  // If this is a postinc, return the value read from memory, otherwise use the
  // updated value.
  return isPre ? IncVal : InVal;
}

void CodeGenModule::EmitExplicitCastExprType(const ExplicitCastExpr *E,
                                             CodeGenFunction *CGF) {
  // Bind VLAs in the cast type.
  if (CGF && E->getType()->isVariablyModifiedType())
    CGF->EmitVariablyModifiedType(E->getType());

  if (CGDebugInfo *DI = getModuleDebugInfo())
    DI->EmitExplicitCastType(E->getType());
}

//===----------------------------------------------------------------------===//
//                         LValue Expression Emission
//===----------------------------------------------------------------------===//

/// EmitPointerWithAlignment - Given an expression of pointer type, try to
/// derive a more accurate bound on the alignment of the pointer.
Address CodeGenFunction::EmitPointerWithAlignment(const Expr *E,
                                                  LValueBaseInfo *BaseInfo,
                                                  TBAAAccessInfo *TBAAInfo) {
  // We allow this with ObjC object pointers because of fragile ABIs.
  assert(E->getType()->isPointerType() ||
         E->getType()->isObjCObjectPointerType());
  E = E->IgnoreParens();

  // Casts:
  if (const CastExpr *CE = dyn_cast<CastExpr>(E)) {
    if (const auto *ECE = dyn_cast<ExplicitCastExpr>(CE))
      CGM.EmitExplicitCastExprType(ECE, this);

    switch (CE->getCastKind()) {
    // Non-converting casts (but not C's implicit conversion from void*).
    case CK_BitCast:
    case CK_NoOp:
    case CK_AddressSpaceConversion:
      if (auto PtrTy = CE->getSubExpr()->getType()->getAs<PointerType>()) {
        if (PtrTy->getPointeeType()->isVoidType())
          break;

        LValueBaseInfo InnerBaseInfo;
        TBAAAccessInfo InnerTBAAInfo;
        Address Addr = EmitPointerWithAlignment(CE->getSubExpr(),
                                                &InnerBaseInfo,
                                                &InnerTBAAInfo);
        if (BaseInfo) *BaseInfo = InnerBaseInfo;
        if (TBAAInfo) *TBAAInfo = InnerTBAAInfo;

        if (isa<ExplicitCastExpr>(CE)) {
          LValueBaseInfo TargetTypeBaseInfo;
          TBAAAccessInfo TargetTypeTBAAInfo;
          CharUnits Align = getNaturalPointeeTypeAlignment(E->getType(),
                                                           &TargetTypeBaseInfo,
                                                           &TargetTypeTBAAInfo);
          if (TBAAInfo)
            *TBAAInfo = CGM.mergeTBAAInfoForCast(*TBAAInfo,
                                                 TargetTypeTBAAInfo);
          // If the source l-value is opaque, honor the alignment of the
          // casted-to type.
          if (InnerBaseInfo.getAlignmentSource() != AlignmentSource::Decl) {
            if (BaseInfo)
              BaseInfo->mergeForCast(TargetTypeBaseInfo);
            Addr = Address(Addr.getPointer(), Align);
          }
        }

        if (SanOpts.has(SanitizerKind::CFIUnrelatedCast) &&
            CE->getCastKind() == CK_BitCast) {
          if (auto PT = E->getType()->getAs<PointerType>())
            EmitVTablePtrCheckForCast(PT->getPointeeType(), Addr.getPointer(),
                                      /*MayBeNull=*/true,
                                      CodeGenFunction::CFITCK_UnrelatedCast,
                                      CE->getBeginLoc());
        }
        return CE->getCastKind() != CK_AddressSpaceConversion
                   ? Builder.CreateBitCast(Addr, ConvertType(E->getType()))
                   : Builder.CreateAddrSpaceCast(Addr,
                                                 ConvertType(E->getType()));
      }
      break;

    // Array-to-pointer decay.
    case CK_ArrayToPointerDecay:
      return EmitArrayToPointerDecay(CE->getSubExpr(), BaseInfo, TBAAInfo);

    // Derived-to-base conversions.
    case CK_UncheckedDerivedToBase:
    case CK_DerivedToBase: {
      // TODO: Support accesses to members of base classes in TBAA. For now, we
      // conservatively pretend that the complete object is of the base class
      // type.
      if (TBAAInfo)
        *TBAAInfo = CGM.getTBAAAccessInfo(E->getType());
      Address Addr = EmitPointerWithAlignment(CE->getSubExpr(), BaseInfo);
      auto Derived = CE->getSubExpr()->getType()->getPointeeCXXRecordDecl();
      return GetAddressOfBaseClass(Addr, Derived,
                                   CE->path_begin(), CE->path_end(),
                                   ShouldNullCheckClassCastValue(CE),
                                   CE->getExprLoc());
    }

    // TODO: Is there any reason to treat base-to-derived conversions
    // specially?
    default:
      break;
    }
  }

  // Unary &.
  if (const UnaryOperator *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() == UO_AddrOf) {
      // FIXME: set bounds here!
      LValue LV = EmitLValue(UO->getSubExpr());
      if (BaseInfo) *BaseInfo = LV.getBaseInfo();
      if (TBAAInfo) *TBAAInfo = LV.getTBAAInfo();
      return LV.getAddress();
    }
  }

  // TODO: conditional operators, comma.

  // Otherwise, use the alignment of the type.
  CharUnits Align = getNaturalPointeeTypeAlignment(E->getType(), BaseInfo,
                                                   TBAAInfo);
  return Address(EmitScalarExpr(E), Align);
}

RValue CodeGenFunction::GetUndefRValue(QualType Ty) {
  if (Ty->isVoidType())
    return RValue::get(nullptr);

  switch (getEvaluationKind(Ty)) {
  case TEK_Complex: {
    llvm::Type *EltTy =
      ConvertType(Ty->castAs<ComplexType>()->getElementType());
    llvm::Value *U = llvm::UndefValue::get(EltTy);
    return RValue::getComplex(std::make_pair(U, U));
  }

  // If this is a use of an undefined aggregate type, the aggregate must have an
  // identifiable address.  Just because the contents of the value are undefined
  // doesn't mean that the address can't be taken and compared.
  case TEK_Aggregate: {
    Address DestPtr = CreateMemTemp(Ty, "undef.agg.tmp");
    return RValue::getAggregate(DestPtr);
  }

  case TEK_Scalar:
    return RValue::get(llvm::UndefValue::get(ConvertType(Ty)));
  }
  llvm_unreachable("bad evaluation kind");
}

RValue CodeGenFunction::EmitUnsupportedRValue(const Expr *E,
                                              const char *Name) {
  ErrorUnsupported(E, Name);
  return GetUndefRValue(E->getType());
}

LValue CodeGenFunction::EmitUnsupportedLValue(const Expr *E,
                                              const char *Name) {
  ErrorUnsupported(E, Name);
  llvm::Type *Ty = CGM.getPointerInDefaultAS(ConvertType(E->getType()));
  return MakeAddrLValue(Address(llvm::UndefValue::get(Ty), CharUnits::One()),
                        E->getType());
}

bool CodeGenFunction::IsWrappedCXXThis(const Expr *Obj) {
  const Expr *Base = Obj;
  while (!isa<CXXThisExpr>(Base)) {
    // The result of a dynamic_cast can be null.
    if (isa<CXXDynamicCastExpr>(Base))
      return false;

    if (const auto *CE = dyn_cast<CastExpr>(Base)) {
      Base = CE->getSubExpr();
    } else if (const auto *PE = dyn_cast<ParenExpr>(Base)) {
      Base = PE->getSubExpr();
    } else if (const auto *UO = dyn_cast<UnaryOperator>(Base)) {
      if (UO->getOpcode() == UO_Extension)
        Base = UO->getSubExpr();
      else
        return false;
    } else {
      return false;
    }
  }
  return true;
}

LValue CodeGenFunction::EmitCheckedLValue(const Expr *E, TypeCheckKind TCK) {
  LValue LV;
  if (SanOpts.has(SanitizerKind::ArrayBounds) && isa<ArraySubscriptExpr>(E))
    LV = EmitArraySubscriptExpr(cast<ArraySubscriptExpr>(E), /*Accessed*/true);
  else
    LV = EmitLValue(E);
  if (!isa<DeclRefExpr>(E) && !LV.isBitField() && LV.isSimple()) {
    SanitizerSet SkippedChecks;
    if (const auto *ME = dyn_cast<MemberExpr>(E)) {
      bool IsBaseCXXThis = IsWrappedCXXThis(ME->getBase());
      if (IsBaseCXXThis)
        SkippedChecks.set(SanitizerKind::Alignment, true);
      if (IsBaseCXXThis || isa<DeclRefExpr>(ME->getBase()))
        SkippedChecks.set(SanitizerKind::Null, true);
    }
    EmitTypeCheck(TCK, E->getExprLoc(), LV.getPointer(),
                  E->getType(), LV.getAlignment(), SkippedChecks);
  }
  return LV;
}

/// EmitLValue - Emit code to compute a designator that specifies the location
/// of the expression.
///
/// This can return one of two things: a simple address or a bitfield reference.
/// In either case, the LLVM Value* in the LValue structure is guaranteed to be
/// an LLVM pointer type.
///
/// If this returns a bitfield reference, nothing about the pointee type of the
/// LLVM value is known: For example, it may not be a pointer to an integer.
///
/// If this returns a normal address, and if the lvalue's C type is fixed size,
/// this method guarantees that the returned pointer type will point to an LLVM
/// type of the same size of the lvalue's type.  If the lvalue has a variable
/// length type, this is not possible.
///
LValue CodeGenFunction::EmitLValue(const Expr *E) {
  ApplyDebugLocation DL(*this, E);
  switch (E->getStmtClass()) {
  default: return EmitUnsupportedLValue(E, "l-value expression");

  case Expr::ObjCPropertyRefExprClass:
    llvm_unreachable("cannot emit a property reference directly");

  case Expr::ObjCSelectorExprClass:
    return EmitObjCSelectorLValue(cast<ObjCSelectorExpr>(E));
  case Expr::ObjCIsaExprClass:
    return EmitObjCIsaExpr(cast<ObjCIsaExpr>(E));
  case Expr::BinaryOperatorClass:
    return EmitBinaryOperatorLValue(cast<BinaryOperator>(E));
  case Expr::CompoundAssignOperatorClass: {
    QualType Ty = E->getType();
    if (const AtomicType *AT = Ty->getAs<AtomicType>())
      Ty = AT->getValueType();
    if (!Ty->isAnyComplexType())
      return EmitCompoundAssignmentLValue(cast<CompoundAssignOperator>(E));
    return EmitComplexCompoundAssignmentLValue(cast<CompoundAssignOperator>(E));
  }
  case Expr::CallExprClass:
  case Expr::CXXMemberCallExprClass:
  case Expr::CXXOperatorCallExprClass:
  case Expr::UserDefinedLiteralClass:
    return EmitCallExprLValue(cast<CallExpr>(E));
  case Expr::VAArgExprClass:
    return EmitVAArgExprLValue(cast<VAArgExpr>(E));
  case Expr::DeclRefExprClass:
    return EmitDeclRefLValue(cast<DeclRefExpr>(E));
  case Expr::ConstantExprClass:
    return EmitLValue(cast<ConstantExpr>(E)->getSubExpr());
  case Expr::ParenExprClass:
    return EmitLValue(cast<ParenExpr>(E)->getSubExpr());
  case Expr::GenericSelectionExprClass:
    return EmitLValue(cast<GenericSelectionExpr>(E)->getResultExpr());
  case Expr::PredefinedExprClass:
    return EmitPredefinedLValue(cast<PredefinedExpr>(E));
  case Expr::StringLiteralClass:
    return EmitStringLiteralLValue(cast<StringLiteral>(E));
  case Expr::ObjCEncodeExprClass:
    return EmitObjCEncodeExprLValue(cast<ObjCEncodeExpr>(E));
  case Expr::PseudoObjectExprClass:
    return EmitPseudoObjectLValue(cast<PseudoObjectExpr>(E));
  case Expr::InitListExprClass:
    return EmitInitListLValue(cast<InitListExpr>(E));
  case Expr::CXXTemporaryObjectExprClass:
  case Expr::CXXConstructExprClass:
    return EmitCXXConstructLValue(cast<CXXConstructExpr>(E));
  case Expr::CXXBindTemporaryExprClass:
    return EmitCXXBindTemporaryLValue(cast<CXXBindTemporaryExpr>(E));
  case Expr::CXXUuidofExprClass:
    return EmitCXXUuidofLValue(cast<CXXUuidofExpr>(E));
  case Expr::LambdaExprClass:
    return EmitAggExprToLValue(E);

  case Expr::ExprWithCleanupsClass: {
    const auto *cleanups = cast<ExprWithCleanups>(E);
    enterFullExpression(cleanups);
    RunCleanupsScope Scope(*this);
    LValue LV = EmitLValue(cleanups->getSubExpr());
    if (LV.isSimple()) {
      // Defend against branches out of gnu statement expressions surrounded by
      // cleanups.
      llvm::Value *V = LV.getPointer();
      Scope.ForceCleanup({&V});
      return LValue::MakeAddr(Address(V, LV.getAlignment()), LV.getType(),
                              getContext(), LV.getBaseInfo(), LV.getTBAAInfo());
    }
    // FIXME: Is it possible to create an ExprWithCleanups that produces a
    // bitfield lvalue or some other non-simple lvalue?
    return LV;
  }

  case Expr::CXXDefaultArgExprClass: {
    auto *DAE = cast<CXXDefaultArgExpr>(E);
    CXXDefaultArgExprScope Scope(*this, DAE);
    return EmitLValue(DAE->getExpr());
  }
  case Expr::CXXDefaultInitExprClass: {
    auto *DIE = cast<CXXDefaultInitExpr>(E);
    CXXDefaultInitExprScope Scope(*this, DIE);
    return EmitLValue(DIE->getExpr());
  }
  case Expr::CXXTypeidExprClass:
    return EmitCXXTypeidLValue(cast<CXXTypeidExpr>(E));

  case Expr::ObjCMessageExprClass:
    return EmitObjCMessageExprLValue(cast<ObjCMessageExpr>(E));
  case Expr::ObjCIvarRefExprClass:
    return EmitObjCIvarRefLValue(cast<ObjCIvarRefExpr>(E));
  case Expr::StmtExprClass:
    return EmitStmtExprLValue(cast<StmtExpr>(E));
  case Expr::UnaryOperatorClass:
    return EmitUnaryOpLValue(cast<UnaryOperator>(E));
  case Expr::ArraySubscriptExprClass:
    return EmitArraySubscriptExpr(cast<ArraySubscriptExpr>(E));
  case Expr::OMPArraySectionExprClass:
    return EmitOMPArraySectionExpr(cast<OMPArraySectionExpr>(E));
  case Expr::ExtVectorElementExprClass:
    return EmitExtVectorElementExpr(cast<ExtVectorElementExpr>(E));
  case Expr::MemberExprClass:
    return EmitMemberExpr(cast<MemberExpr>(E));
  case Expr::CompoundLiteralExprClass:
    return EmitCompoundLiteralLValue(cast<CompoundLiteralExpr>(E));
  case Expr::ConditionalOperatorClass:
    return EmitConditionalOperatorLValue(cast<ConditionalOperator>(E));
  case Expr::BinaryConditionalOperatorClass:
    return EmitConditionalOperatorLValue(cast<BinaryConditionalOperator>(E));
  case Expr::ChooseExprClass:
    return EmitLValue(cast<ChooseExpr>(E)->getChosenSubExpr());
  case Expr::OpaqueValueExprClass:
    return EmitOpaqueValueLValue(cast<OpaqueValueExpr>(E));
  case Expr::SubstNonTypeTemplateParmExprClass:
    return EmitLValue(cast<SubstNonTypeTemplateParmExpr>(E)->getReplacement());
  case Expr::ImplicitCastExprClass:
  case Expr::CStyleCastExprClass:
  case Expr::CXXFunctionalCastExprClass:
  case Expr::CXXStaticCastExprClass:
  case Expr::CXXDynamicCastExprClass:
  case Expr::CXXReinterpretCastExprClass:
  case Expr::CXXConstCastExprClass:
  case Expr::ObjCBridgedCastExprClass:
    return EmitCastLValue(cast<CastExpr>(E));

  case Expr::MaterializeTemporaryExprClass:
    return EmitMaterializeTemporaryExpr(cast<MaterializeTemporaryExpr>(E));

  case Expr::CoawaitExprClass:
    return EmitCoawaitLValue(cast<CoawaitExpr>(E));
  case Expr::CoyieldExprClass:
    return EmitCoyieldLValue(cast<CoyieldExpr>(E));
  }
}

/// Given an object of the given canonical type, can we safely copy a
/// value out of it based on its initializer?
static bool isConstantEmittableObjectType(QualType type) {
  assert(type.isCanonical());
  assert(!type->isReferenceType());

  // Must be const-qualified but non-volatile.
  Qualifiers qs = type.getLocalQualifiers();
  if (!qs.hasConst() || qs.hasVolatile()) return false;

  // Otherwise, all object types satisfy this except C++ classes with
  // mutable subobjects or non-trivial copy/destroy behavior.
  if (const auto *RT = dyn_cast<RecordType>(type))
    if (const auto *RD = dyn_cast<CXXRecordDecl>(RT->getDecl()))
      if (RD->hasMutableFields() || !RD->isTrivial())
        return false;

  return true;
}

/// Can we constant-emit a load of a reference to a variable of the
/// given type?  This is different from predicates like
/// Decl::mightBeUsableInConstantExpressions because we do want it to apply
/// in situations that don't necessarily satisfy the language's rules
/// for this (e.g. C++'s ODR-use rules).  For example, we want to able
/// to do this with const float variables even if those variables
/// aren't marked 'constexpr'.
enum ConstantEmissionKind {
  CEK_None,
  CEK_AsReferenceOnly,
  CEK_AsValueOrReference,
  CEK_AsValueOnly
};
static ConstantEmissionKind checkVarTypeForConstantEmission(QualType type) {
  type = type.getCanonicalType();
  if (const auto *ref = dyn_cast<ReferenceType>(type)) {
    if (isConstantEmittableObjectType(ref->getPointeeType()))
      return CEK_AsValueOrReference;
    return CEK_AsReferenceOnly;
  }
  if (isConstantEmittableObjectType(type))
    return CEK_AsValueOnly;
  return CEK_None;
}

/// Try to emit a reference to the given value without producing it as
/// an l-value.  This is actually more than an optimization: we can't
/// produce an l-value for variables that we never actually captured
/// in a block or lambda, which means const int variables or constexpr
/// literals or similar.
CodeGenFunction::ConstantEmission
CodeGenFunction::tryEmitAsConstant(DeclRefExpr *refExpr) {
  ValueDecl *value = refExpr->getDecl();

  // The value needs to be an enum constant or a constant variable.
  ConstantEmissionKind CEK;
  if (isa<ParmVarDecl>(value)) {
    CEK = CEK_None;
  } else if (auto *var = dyn_cast<VarDecl>(value)) {
    CEK = checkVarTypeForConstantEmission(var->getType());
  } else if (isa<EnumConstantDecl>(value)) {
    CEK = CEK_AsValueOnly;
  } else {
    CEK = CEK_None;
  }
  if (CEK == CEK_None) return ConstantEmission();

  Expr::EvalResult result;
  bool resultIsReference;
  QualType resultType;

  // It's best to evaluate all the way as an r-value if that's permitted.
  if (CEK != CEK_AsReferenceOnly &&
      refExpr->EvaluateAsRValue(result, getContext())) {
    resultIsReference = false;
    resultType = refExpr->getType();

  // Otherwise, try to evaluate as an l-value.
  } else if (CEK != CEK_AsValueOnly &&
             refExpr->EvaluateAsLValue(result, getContext())) {
    resultIsReference = true;
    resultType = value->getType();

  // Failure.
  } else {
    return ConstantEmission();
  }

  // In any case, if the initializer has side-effects, abandon ship.
  if (result.HasSideEffects)
    return ConstantEmission();

  // Emit as a constant.
  auto C = ConstantEmitter(*this).emitAbstract(refExpr->getLocation(),
                                               result.Val, resultType);

  // Make sure we emit a debug reference to the global variable.
  // This should probably fire even for
  if (isa<VarDecl>(value)) {
    if (!getContext().DeclMustBeEmitted(cast<VarDecl>(value)))
      EmitDeclRefExprDbgValue(refExpr, result.Val);
  } else {
    assert(isa<EnumConstantDecl>(value));
    EmitDeclRefExprDbgValue(refExpr, result.Val);
  }

  // If we emitted a reference constant, we need to dereference that.
  if (resultIsReference)
    return ConstantEmission::forReference(C);

  return ConstantEmission::forValue(C);
}

static DeclRefExpr *tryToConvertMemberExprToDeclRefExpr(CodeGenFunction &CGF,
                                                        const MemberExpr *ME) {
  if (auto *VD = dyn_cast<VarDecl>(ME->getMemberDecl())) {
    // Try to emit static variable member expressions as DREs.
    return DeclRefExpr::Create(
        CGF.getContext(), NestedNameSpecifierLoc(), SourceLocation(), VD,
        /*RefersToEnclosingVariableOrCapture=*/false, ME->getExprLoc(),
        ME->getType(), ME->getValueKind(), nullptr, nullptr, ME->isNonOdrUse());
  }
  return nullptr;
}

CodeGenFunction::ConstantEmission
CodeGenFunction::tryEmitAsConstant(const MemberExpr *ME) {
  if (DeclRefExpr *DRE = tryToConvertMemberExprToDeclRefExpr(*this, ME))
    return tryEmitAsConstant(DRE);
  return ConstantEmission();
}

llvm::Value *CodeGenFunction::emitScalarConstant(
    const CodeGenFunction::ConstantEmission &Constant, Expr *E) {
  assert(Constant && "not a constant");
  if (Constant.isReference())
    return EmitLoadOfLValue(Constant.getReferenceLValue(*this, E),
                            E->getExprLoc())
        .getScalarVal();
  // XXXAR: For CHERI convert inttoptr instructions to setPointerOffset on
  // null See https://github.com/CTSRD-CHERI/llvm/issues/268
  return Constant.getValue(*this);
}

llvm::Value *CodeGenFunction::EmitLoadOfScalar(LValue lvalue,
                                               SourceLocation Loc) {
  return EmitLoadOfScalar(lvalue.getAddress(), lvalue.isVolatile(),
                          lvalue.getType(), Loc, lvalue.getBaseInfo(),
                          lvalue.getTBAAInfo(), lvalue.isNontemporal());
}

static bool hasBooleanRepresentation(QualType Ty) {
  if (Ty->isBooleanType())
    return true;

  if (const EnumType *ET = Ty->getAs<EnumType>())
    return ET->getDecl()->getIntegerType()->isBooleanType();

  if (const AtomicType *AT = Ty->getAs<AtomicType>())
    return hasBooleanRepresentation(AT->getValueType());

  return false;
}

static bool getRangeForType(CodeGenFunction &CGF, QualType Ty,
                            llvm::APInt &Min, llvm::APInt &End,
                            bool StrictEnums, bool IsBool) {
  const EnumType *ET = Ty->getAs<EnumType>();
  bool IsRegularCPlusPlusEnum = CGF.getLangOpts().CPlusPlus && StrictEnums &&
                                ET && !ET->getDecl()->isFixed();
  if (!IsBool && !IsRegularCPlusPlusEnum)
    return false;

  if (IsBool) {
    Min = llvm::APInt(CGF.getContext().getTypeSize(Ty), 0);
    End = llvm::APInt(CGF.getContext().getTypeSize(Ty), 2);
  } else {
    const EnumDecl *ED = ET->getDecl();
    llvm::Type *LTy = CGF.ConvertTypeForMem(ED->getIntegerType());
    unsigned Bitwidth = LTy->getScalarSizeInBits();
    unsigned NumNegativeBits = ED->getNumNegativeBits();
    unsigned NumPositiveBits = ED->getNumPositiveBits();

    if (NumNegativeBits) {
      unsigned NumBits = std::max(NumNegativeBits, NumPositiveBits + 1);
      assert(NumBits <= Bitwidth);
      End = llvm::APInt(Bitwidth, 1) << (NumBits - 1);
      Min = -End;
    } else {
      assert(NumPositiveBits <= Bitwidth);
      End = llvm::APInt(Bitwidth, 1) << NumPositiveBits;
      Min = llvm::APInt(Bitwidth, 0);
    }
  }
  return true;
}

llvm::MDNode *CodeGenFunction::getRangeForLoadFromType(QualType Ty) {
  llvm::APInt Min, End;
  if (!getRangeForType(*this, Ty, Min, End, CGM.getCodeGenOpts().StrictEnums,
                       hasBooleanRepresentation(Ty)))
    return nullptr;

  llvm::MDBuilder MDHelper(getLLVMContext());
  return MDHelper.createRange(Min, End);
}

bool CodeGenFunction::EmitScalarRangeCheck(llvm::Value *Value, QualType Ty,
                                           SourceLocation Loc) {
  bool HasBoolCheck = SanOpts.has(SanitizerKind::Bool);
  bool HasEnumCheck = SanOpts.has(SanitizerKind::Enum);
  if (!HasBoolCheck && !HasEnumCheck)
    return false;

  bool IsBool = hasBooleanRepresentation(Ty) ||
                NSAPI(CGM.getContext()).isObjCBOOLType(Ty);
  bool NeedsBoolCheck = HasBoolCheck && IsBool;
  bool NeedsEnumCheck = HasEnumCheck && Ty->getAs<EnumType>();
  if (!NeedsBoolCheck && !NeedsEnumCheck)
    return false;

  // Single-bit booleans don't need to be checked. Special-case this to avoid
  // a bit width mismatch when handling bitfield values. This is handled by
  // EmitFromMemory for the non-bitfield case.
  if (IsBool &&
      cast<llvm::IntegerType>(Value->getType())->getBitWidth() == 1)
    return false;

  llvm::APInt Min, End;
  if (!getRangeForType(*this, Ty, Min, End, /*StrictEnums=*/true, IsBool))
    return true;

  auto &Ctx = getLLVMContext();
  SanitizerScope SanScope(this);
  llvm::Value *Check;
  --End;
  if (!Min) {
    Check = Builder.CreateICmpULE(Value, llvm::ConstantInt::get(Ctx, End));
  } else {
    llvm::Value *Upper =
        Builder.CreateICmpSLE(Value, llvm::ConstantInt::get(Ctx, End));
    llvm::Value *Lower =
        Builder.CreateICmpSGE(Value, llvm::ConstantInt::get(Ctx, Min));
    Check = Builder.CreateAnd(Upper, Lower);
  }
  llvm::Constant *StaticArgs[] = {EmitCheckSourceLocation(Loc),
                                  EmitCheckTypeDescriptor(Ty)};
  SanitizerMask Kind =
      NeedsEnumCheck ? SanitizerKind::Enum : SanitizerKind::Bool;
  EmitCheck(std::make_pair(Check, Kind), SanitizerHandler::LoadInvalidValue,
            StaticArgs, EmitCheckValue(Value));
  return true;
}

llvm::Value *CodeGenFunction::EmitLoadOfScalar(Address Addr, bool Volatile,
                                               QualType Ty,
                                               SourceLocation Loc,
                                               LValueBaseInfo BaseInfo,
                                               TBAAAccessInfo TBAAInfo,
                                               bool isNontemporal) {
  if (!CGM.getCodeGenOpts().PreserveVec3Type) {
    // For better performance, handle vector loads differently.
    if (Ty->isVectorType()) {
      const llvm::Type *EltTy = Addr.getElementType();

      const auto *VTy = cast<llvm::VectorType>(EltTy);

      // Handle vectors of size 3 like size 4 for better performance.
      if (VTy->getNumElements() == 3) {

        // Bitcast to vec4 type.
        llvm::VectorType *vec4Ty =
            llvm::VectorType::get(VTy->getElementType(), 4);
        Address Cast = Builder.CreateElementBitCast(Addr, vec4Ty, "castToVec4");
        // Now load value.
        llvm::Value *V = Builder.CreateLoad(Cast, Volatile, "loadVec4");

        // Shuffle vector to get vec3.
        V = Builder.CreateShuffleVector(V, llvm::UndefValue::get(vec4Ty),
                                        {0, 1, 2}, "extractVec");
        return EmitFromMemory(V, Ty);
      }
    }
  }

  // Atomic operations have to be done on integral types.
  LValue AtomicLValue =
      LValue::MakeAddr(Addr, Ty, getContext(), BaseInfo, TBAAInfo);
  if (Ty->isAtomicType() || LValueIsSuitableForInlineAtomic(AtomicLValue)) {
    return EmitAtomicLoad(AtomicLValue, Loc).getScalarVal();
  }

  llvm::LoadInst *Load = Builder.CreateLoad(Addr, Volatile);
  if (isNontemporal) {
    llvm::MDNode *Node = llvm::MDNode::get(
        Load->getContext(), llvm::ConstantAsMetadata::get(Builder.getInt32(1)));
    Load->setMetadata(CGM.getModule().getMDKindID("nontemporal"), Node);
  }

  CGM.DecorateInstructionWithTBAA(Load, TBAAInfo);

  if (EmitScalarRangeCheck(Load, Ty, Loc)) {
    // In order to prevent the optimizer from throwing away the check, don't
    // attach range metadata to the load.
  } else if (CGM.getCodeGenOpts().OptimizationLevel > 0)
    if (llvm::MDNode *RangeInfo = getRangeForLoadFromType(Ty))
      Load->setMetadata(llvm::LLVMContext::MD_range, RangeInfo);

  return EmitFromMemory(Load, Ty);
}

llvm::Value *CodeGenFunction::EmitToMemory(llvm::Value *Value, QualType Ty) {
  // Bool has a different representation in memory than in registers.
  if (hasBooleanRepresentation(Ty)) {
    // This should really always be an i1, but sometimes it's already
    // an i8, and it's awkward to track those cases down.
    if (Value->getType()->isIntegerTy(1))
      return Builder.CreateZExt(Value, ConvertTypeForMem(Ty), "frombool");
    assert(Value->getType()->isIntegerTy(getContext().getTypeSize(Ty)) &&
           "wrong value rep of bool");
  }

  return Value;
}

llvm::Value *CodeGenFunction::EmitFromMemory(llvm::Value *Value, QualType Ty) {
  // Bool has a different representation in memory than in registers.
  if (hasBooleanRepresentation(Ty)) {
    assert(Value->getType()->isIntegerTy(getContext().getTypeSize(Ty)) &&
           "wrong value rep of bool");
    return Builder.CreateTrunc(Value, Builder.getInt1Ty(), "tobool");
  }

  return Value;
}

void CodeGenFunction::EmitStoreOfScalar(llvm::Value *Value, Address Addr,
                                        bool Volatile, QualType Ty,
                                        LValueBaseInfo BaseInfo,
                                        TBAAAccessInfo TBAAInfo,
                                        bool isInit, bool isNontemporal) {
  if (!CGM.getCodeGenOpts().PreserveVec3Type) {
    // Handle vectors differently to get better performance.
    if (Ty->isVectorType()) {
      llvm::Type *SrcTy = Value->getType();
      auto *VecTy = dyn_cast<llvm::VectorType>(SrcTy);
      // Handle vec3 special.
      if (VecTy && VecTy->getNumElements() == 3) {
        // Our source is a vec3, do a shuffle vector to make it a vec4.
        llvm::Constant *Mask[] = {Builder.getInt32(0), Builder.getInt32(1),
                                  Builder.getInt32(2),
                                  llvm::UndefValue::get(Builder.getInt32Ty())};
        llvm::Value *MaskV = llvm::ConstantVector::get(Mask);
        Value = Builder.CreateShuffleVector(Value, llvm::UndefValue::get(VecTy),
                                            MaskV, "extractVec");
        SrcTy = llvm::VectorType::get(VecTy->getElementType(), 4);
      }
      if (Addr.getElementType() != SrcTy) {
        Addr = Builder.CreateElementBitCast(Addr, SrcTy, "storetmp");
      }
    }
  }

  Value = EmitToMemory(Value, Ty);

  LValue AtomicLValue =
      LValue::MakeAddr(Addr, Ty, getContext(), BaseInfo, TBAAInfo);
  if (Ty->isAtomicType() ||
      (!isInit && LValueIsSuitableForInlineAtomic(AtomicLValue))) {
    EmitAtomicStore(RValue::get(Value), AtomicLValue, isInit);
    return;
  }

  llvm::StoreInst *Store = Builder.CreateStore(Value, Addr, Volatile);
  if (isNontemporal) {
    llvm::MDNode *Node =
        llvm::MDNode::get(Store->getContext(),
                          llvm::ConstantAsMetadata::get(Builder.getInt32(1)));
    Store->setMetadata(CGM.getModule().getMDKindID("nontemporal"), Node);
  }

  CGM.DecorateInstructionWithTBAA(Store, TBAAInfo);
}

void CodeGenFunction::EmitStoreOfScalar(llvm::Value *value, LValue lvalue,
                                        bool isInit) {
  EmitStoreOfScalar(value, lvalue.getAddress(), lvalue.isVolatile(),
                    lvalue.getType(), lvalue.getBaseInfo(),
                    lvalue.getTBAAInfo(), isInit, lvalue.isNontemporal());
}

/// EmitLoadOfLValue - Given an expression that represents a value lvalue, this
/// method emits the address of the lvalue, then loads the result as an rvalue,
/// returning the rvalue.
RValue CodeGenFunction::EmitLoadOfLValue(LValue LV, SourceLocation Loc) {
  if (LV.isObjCWeak()) {
    // load of a __weak object.
    Address AddrWeakObj = LV.getAddress();
    return RValue::get(CGM.getObjCRuntime().EmitObjCWeakRead(*this,
                                                             AddrWeakObj));
  }
  if (LV.getQuals().getObjCLifetime() == Qualifiers::OCL_Weak) {
    // In MRC mode, we do a load+autorelease.
    if (!getLangOpts().ObjCAutoRefCount) {
      return RValue::get(EmitARCLoadWeak(LV.getAddress()));
    }

    // In ARC mode, we load retained and then consume the value.
    llvm::Value *Object = EmitARCLoadWeakRetained(LV.getAddress());
    Object = EmitObjCConsumeObject(LV.getType(), Object);
    return RValue::get(Object);
  }

  if (LV.isSimple()) {
    assert(!LV.getType()->isFunctionType());

    // Everything needs a load.
    return RValue::get(EmitLoadOfScalar(LV, Loc));
  }

  if (LV.isVectorElt()) {
    llvm::LoadInst *Load = Builder.CreateLoad(LV.getVectorAddress(),
                                              LV.isVolatileQualified());
    return RValue::get(Builder.CreateExtractElement(Load, LV.getVectorIdx(),
                                                    "vecext"));
  }

  // If this is a reference to a subset of the elements of a vector, either
  // shuffle the input or extract/insert them as appropriate.
  if (LV.isExtVectorElt())
    return EmitLoadOfExtVectorElementLValue(LV);

  // Global Register variables always invoke intrinsics
  if (LV.isGlobalReg())
    return EmitLoadOfGlobalRegLValue(LV);

  assert(LV.isBitField() && "Unknown LValue type!");
  return EmitLoadOfBitfieldLValue(LV, Loc);
}

RValue CodeGenFunction::EmitLoadOfBitfieldLValue(LValue LV,
                                                 SourceLocation Loc) {
  const CGBitFieldInfo &Info = LV.getBitFieldInfo();

  // Get the output type.
  llvm::Type *ResLTy = ConvertType(LV.getType());

  Address Ptr = LV.getBitFieldAddress();
  llvm::Value *Val = Builder.CreateLoad(Ptr, LV.isVolatileQualified(), "bf.load");

  if (Info.IsSigned) {
    assert(static_cast<unsigned>(Info.Offset + Info.Size) <= Info.StorageSize);
    unsigned HighBits = Info.StorageSize - Info.Offset - Info.Size;
    if (HighBits)
      Val = Builder.CreateShl(Val, HighBits, "bf.shl");
    if (Info.Offset + HighBits)
      Val = Builder.CreateAShr(Val, Info.Offset + HighBits, "bf.ashr");
  } else {
    if (Info.Offset)
      Val = Builder.CreateLShr(Val, Info.Offset, "bf.lshr");
    if (static_cast<unsigned>(Info.Offset) + Info.Size < Info.StorageSize)
      Val = Builder.CreateAnd(Val, llvm::APInt::getLowBitsSet(Info.StorageSize,
                                                              Info.Size),
                              "bf.clear");
  }
  Val = Builder.CreateIntCast(Val, ResLTy, Info.IsSigned, "bf.cast");
  EmitScalarRangeCheck(Val, LV.getType(), Loc);
  return RValue::get(Val);
}

// If this is a reference to a subset of the elements of a vector, create an
// appropriate shufflevector.
RValue CodeGenFunction::EmitLoadOfExtVectorElementLValue(LValue LV) {
  llvm::Value *Vec = Builder.CreateLoad(LV.getExtVectorAddress(),
                                        LV.isVolatileQualified());

  const llvm::Constant *Elts = LV.getExtVectorElts();

  // If the result of the expression is a non-vector type, we must be extracting
  // a single element.  Just codegen as an extractelement.
  const VectorType *ExprVT = LV.getType()->getAs<VectorType>();
  if (!ExprVT) {
    unsigned InIdx = getAccessedFieldNo(0, Elts);
    llvm::Value *Elt = llvm::ConstantInt::get(SizeTy, InIdx);
    return RValue::get(Builder.CreateExtractElement(Vec, Elt));
  }

  // Always use shuffle vector to try to retain the original program structure
  unsigned NumResultElts = ExprVT->getNumElements();

  SmallVector<llvm::Constant*, 4> Mask;
  for (unsigned i = 0; i != NumResultElts; ++i)
    Mask.push_back(Builder.getInt32(getAccessedFieldNo(i, Elts)));

  llvm::Value *MaskV = llvm::ConstantVector::get(Mask);
  Vec = Builder.CreateShuffleVector(Vec, llvm::UndefValue::get(Vec->getType()),
                                    MaskV);
  return RValue::get(Vec);
}

/// Generates lvalue for partial ext_vector access.
Address CodeGenFunction::EmitExtVectorElementLValue(LValue LV) {
  Address VectorAddress = LV.getExtVectorAddress();
  const VectorType *ExprVT = LV.getType()->getAs<VectorType>();
  QualType EQT = ExprVT->getElementType();
  llvm::Type *VectorElementTy = CGM.getTypes().ConvertType(EQT);

  Address CastToPointerElement =
    Builder.CreateElementBitCast(VectorAddress, VectorElementTy,
                                 "conv.ptr.element");

  const llvm::Constant *Elts = LV.getExtVectorElts();
  unsigned ix = getAccessedFieldNo(0, Elts);

  Address VectorBasePtrPlusIx =
    Builder.CreateConstInBoundsGEP(CastToPointerElement, ix,
                                   "vector.elt");

  return VectorBasePtrPlusIx;
}

/// Load of global gamed gegisters are always calls to intrinsics.
RValue CodeGenFunction::EmitLoadOfGlobalRegLValue(LValue LV) {
  assert((LV.getType()->isIntegerType() || LV.getType()->isPointerType()) &&
         "Bad type for register variable");
  llvm::MDNode *RegName = cast<llvm::MDNode>(
      cast<llvm::MetadataAsValue>(LV.getGlobalReg())->getMetadata());

  // We accept integer and pointer types only
  llvm::Type *OrigTy = CGM.getTypes().ConvertType(LV.getType());
  llvm::Type *Ty = OrigTy;
  if (OrigTy->isPointerTy())
    Ty = CGM.getTypes().getDataLayout().getIntPtrType(OrigTy);
  llvm::Type *Types[] = { Ty };

  llvm::Function *F = CGM.getIntrinsic(llvm::Intrinsic::read_register, Types);
  llvm::Value *Call = Builder.CreateCall(
      F, llvm::MetadataAsValue::get(Ty->getContext(), RegName));
  if (OrigTy->isPointerTy())
    Call = Builder.CreateIntToPtr(Call, OrigTy);
  return RValue::get(Call);
}


/// EmitStoreThroughLValue - Store the specified rvalue into the specified
/// lvalue, where both are guaranteed to the have the same type, and that type
/// is 'Ty'.
void CodeGenFunction::EmitStoreThroughLValue(RValue Src, LValue Dst,
                                             bool isInit) {
  if (!Dst.isSimple()) {
    if (Dst.isVectorElt()) {
      // Read/modify/write the vector, inserting the new element.
      llvm::Value *Vec = Builder.CreateLoad(Dst.getVectorAddress(),
                                            Dst.isVolatileQualified());
      Vec = Builder.CreateInsertElement(Vec, Src.getScalarVal(),
                                        Dst.getVectorIdx(), "vecins");
      Builder.CreateStore(Vec, Dst.getVectorAddress(),
                          Dst.isVolatileQualified());
      return;
    }

    // If this is an update of extended vector elements, insert them as
    // appropriate.
    if (Dst.isExtVectorElt())
      return EmitStoreThroughExtVectorComponentLValue(Src, Dst);

    if (Dst.isGlobalReg())
      return EmitStoreThroughGlobalRegLValue(Src, Dst);

    assert(Dst.isBitField() && "Unknown LValue type");
    return EmitStoreThroughBitfieldLValue(Src, Dst);
  }

  // There's special magic for assigning into an ARC-qualified l-value.
  if (Qualifiers::ObjCLifetime Lifetime = Dst.getQuals().getObjCLifetime()) {
    switch (Lifetime) {
    case Qualifiers::OCL_None:
      llvm_unreachable("present but none");

    case Qualifiers::OCL_ExplicitNone:
      // nothing special
      break;

    case Qualifiers::OCL_Strong:
      if (isInit) {
        Src = RValue::get(EmitARCRetain(Dst.getType(), Src.getScalarVal()));
        break;
      }
      EmitARCStoreStrong(Dst, Src.getScalarVal(), /*ignore*/ true);
      return;

    case Qualifiers::OCL_Weak:
      if (isInit)
        // Initialize and then skip the primitive store.
        EmitARCInitWeak(Dst.getAddress(), Src.getScalarVal());
      else
        EmitARCStoreWeak(Dst.getAddress(), Src.getScalarVal(), /*ignore*/ true);
      return;

    case Qualifiers::OCL_Autoreleasing:
      Src = RValue::get(EmitObjCExtendObjectLifetime(Dst.getType(),
                                                     Src.getScalarVal()));
      // fall into the normal path
      break;
    }
  }

  if (Dst.isObjCWeak() && !Dst.isNonGC()) {
    // load of a __weak object.
    Address LvalueDst = Dst.getAddress();
    llvm::Value *src = Src.getScalarVal();
     CGM.getObjCRuntime().EmitObjCWeakAssign(*this, src, LvalueDst);
    return;
  }

  if (Dst.isObjCStrong() && !Dst.isNonGC()) {
    // load of a __strong object.
    Address LvalueDst = Dst.getAddress();
    llvm::Value *src = Src.getScalarVal();
    if (Dst.isObjCIvar()) {
      assert(Dst.getBaseIvarExp() && "BaseIvarExp is NULL");
      llvm::Type *ResultType = IntPtrTy;
      Address dst = EmitPointerWithAlignment(Dst.getBaseIvarExp());
      llvm::Value *RHS = dst.getPointer();
      RHS = Builder.CreatePtrToInt(RHS, ResultType, "sub.ptr.rhs.cast");
      llvm::Value *LHS =
        Builder.CreatePtrToInt(LvalueDst.getPointer(), ResultType,
                               "sub.ptr.lhs.cast");
      llvm::Value *BytesBetween = Builder.CreateSub(LHS, RHS, "ivar.offset");
      CGM.getObjCRuntime().EmitObjCIvarAssign(*this, src, dst,
                                              BytesBetween);
    } else if (Dst.isGlobalObjCRef()) {
      CGM.getObjCRuntime().EmitObjCGlobalAssign(*this, src, LvalueDst,
                                                Dst.isThreadLocalRef());
    }
    else
      CGM.getObjCRuntime().EmitObjCStrongCastAssign(*this, src, LvalueDst);
    return;
  }

  assert(Src.isScalar() && "Can't emit an agg store with this method");
  EmitStoreOfScalar(Src.getScalarVal(), Dst, isInit);
}

void CodeGenFunction::EmitStoreThroughBitfieldLValue(RValue Src, LValue Dst,
                                                     llvm::Value **Result) {
  const CGBitFieldInfo &Info = Dst.getBitFieldInfo();
  llvm::Type *ResLTy = ConvertTypeForMem(Dst.getType());
  Address Ptr = Dst.getBitFieldAddress();

  // Get the source value, truncated to the width of the bit-field.
  llvm::Value *SrcVal = Src.getScalarVal();

  // Cast the source to the storage type and shift it into place.
  SrcVal = Builder.CreateIntCast(SrcVal, Ptr.getElementType(),
                                 /*IsSigned=*/false);
  llvm::Value *MaskedVal = SrcVal;

  // See if there are other bits in the bitfield's storage we'll need to load
  // and mask together with source before storing.
  if (Info.StorageSize != Info.Size) {
    assert(Info.StorageSize > Info.Size && "Invalid bitfield size.");
    llvm::Value *Val =
      Builder.CreateLoad(Ptr, Dst.isVolatileQualified(), "bf.load");

    // Mask the source value as needed.
    if (!hasBooleanRepresentation(Dst.getType()))
      SrcVal = Builder.CreateAnd(SrcVal,
                                 llvm::APInt::getLowBitsSet(Info.StorageSize,
                                                            Info.Size),
                                 "bf.value");
    MaskedVal = SrcVal;
    if (Info.Offset)
      SrcVal = Builder.CreateShl(SrcVal, Info.Offset, "bf.shl");

    // Mask out the original value.
    Val = Builder.CreateAnd(Val,
                            ~llvm::APInt::getBitsSet(Info.StorageSize,
                                                     Info.Offset,
                                                     Info.Offset + Info.Size),
                            "bf.clear");

    // Or together the unchanged values and the source value.
    SrcVal = Builder.CreateOr(Val, SrcVal, "bf.set");
  } else {
    assert(Info.Offset == 0);
  }

  // Write the new value back out.
  Builder.CreateStore(SrcVal, Ptr, Dst.isVolatileQualified());

  // Return the new value of the bit-field, if requested.
  if (Result) {
    llvm::Value *ResultVal = MaskedVal;

    // Sign extend the value if needed.
    if (Info.IsSigned) {
      assert(Info.Size <= Info.StorageSize);
      unsigned HighBits = Info.StorageSize - Info.Size;
      if (HighBits) {
        ResultVal = Builder.CreateShl(ResultVal, HighBits, "bf.result.shl");
        ResultVal = Builder.CreateAShr(ResultVal, HighBits, "bf.result.ashr");
      }
    }

    ResultVal = Builder.CreateIntCast(ResultVal, ResLTy, Info.IsSigned,
                                      "bf.result.cast");
    *Result = EmitFromMemory(ResultVal, Dst.getType());
  }
}

void CodeGenFunction::EmitStoreThroughExtVectorComponentLValue(RValue Src,
                                                               LValue Dst) {
  // This access turns into a read/modify/write of the vector.  Load the input
  // value now.
  llvm::Value *Vec = Builder.CreateLoad(Dst.getExtVectorAddress(),
                                        Dst.isVolatileQualified());
  const llvm::Constant *Elts = Dst.getExtVectorElts();

  llvm::Value *SrcVal = Src.getScalarVal();

  if (const VectorType *VTy = Dst.getType()->getAs<VectorType>()) {
    unsigned NumSrcElts = VTy->getNumElements();
    unsigned NumDstElts = Vec->getType()->getVectorNumElements();
    if (NumDstElts == NumSrcElts) {
      // Use shuffle vector is the src and destination are the same number of
      // elements and restore the vector mask since it is on the side it will be
      // stored.
      SmallVector<llvm::Constant*, 4> Mask(NumDstElts);
      for (unsigned i = 0; i != NumSrcElts; ++i)
        Mask[getAccessedFieldNo(i, Elts)] = Builder.getInt32(i);

      llvm::Value *MaskV = llvm::ConstantVector::get(Mask);
      Vec = Builder.CreateShuffleVector(SrcVal,
                                        llvm::UndefValue::get(Vec->getType()),
                                        MaskV);
    } else if (NumDstElts > NumSrcElts) {
      // Extended the source vector to the same length and then shuffle it
      // into the destination.
      // FIXME: since we're shuffling with undef, can we just use the indices
      //        into that?  This could be simpler.
      SmallVector<llvm::Constant*, 4> ExtMask;
      for (unsigned i = 0; i != NumSrcElts; ++i)
        ExtMask.push_back(Builder.getInt32(i));
      ExtMask.resize(NumDstElts, llvm::UndefValue::get(Int32Ty));
      llvm::Value *ExtMaskV = llvm::ConstantVector::get(ExtMask);
      llvm::Value *ExtSrcVal =
        Builder.CreateShuffleVector(SrcVal,
                                    llvm::UndefValue::get(SrcVal->getType()),
                                    ExtMaskV);
      // build identity
      SmallVector<llvm::Constant*, 4> Mask;
      for (unsigned i = 0; i != NumDstElts; ++i)
        Mask.push_back(Builder.getInt32(i));

      // When the vector size is odd and .odd or .hi is used, the last element
      // of the Elts constant array will be one past the size of the vector.
      // Ignore the last element here, if it is greater than the mask size.
      if (getAccessedFieldNo(NumSrcElts - 1, Elts) == Mask.size())
        NumSrcElts--;

      // modify when what gets shuffled in
      for (unsigned i = 0; i != NumSrcElts; ++i)
        Mask[getAccessedFieldNo(i, Elts)] = Builder.getInt32(i+NumDstElts);
      llvm::Value *MaskV = llvm::ConstantVector::get(Mask);
      Vec = Builder.CreateShuffleVector(Vec, ExtSrcVal, MaskV);
    } else {
      // We should never shorten the vector
      llvm_unreachable("unexpected shorten vector length");
    }
  } else {
    // If the Src is a scalar (not a vector) it must be updating one element.
    unsigned InIdx = getAccessedFieldNo(0, Elts);
    llvm::Value *Elt = llvm::ConstantInt::get(SizeTy, InIdx);
    Vec = Builder.CreateInsertElement(Vec, SrcVal, Elt);
  }

  Builder.CreateStore(Vec, Dst.getExtVectorAddress(),
                      Dst.isVolatileQualified());
}

/// Store of global named registers are always calls to intrinsics.
void CodeGenFunction::EmitStoreThroughGlobalRegLValue(RValue Src, LValue Dst) {
  assert((Dst.getType()->isIntegerType() || Dst.getType()->isPointerType()) &&
         "Bad type for register variable");
  llvm::MDNode *RegName = cast<llvm::MDNode>(
      cast<llvm::MetadataAsValue>(Dst.getGlobalReg())->getMetadata());
  assert(RegName && "Register LValue is not metadata");

  // We accept integer and pointer types only
  llvm::Type *OrigTy = CGM.getTypes().ConvertType(Dst.getType());
  llvm::Type *Ty = OrigTy;
  if (OrigTy->isPointerTy())
    Ty = CGM.getTypes().getDataLayout().getIntPtrType(OrigTy);
  llvm::Type *Types[] = { Ty };

  llvm::Function *F = CGM.getIntrinsic(llvm::Intrinsic::write_register, Types);
  llvm::Value *Value = Src.getScalarVal();
  if (OrigTy->isPointerTy())
    Value = Builder.CreatePtrToInt(Value, Ty);
  Builder.CreateCall(
      F, {llvm::MetadataAsValue::get(Ty->getContext(), RegName), Value});
}

// setObjCGCLValueClass - sets class of the lvalue for the purpose of
// generating write-barries API. It is currently a global, ivar,
// or neither.
static void setObjCGCLValueClass(const ASTContext &Ctx, const Expr *E,
                                 LValue &LV,
                                 bool IsMemberAccess=false) {
  if (Ctx.getLangOpts().getGC() == LangOptions::NonGC)
    return;

  if (isa<ObjCIvarRefExpr>(E)) {
    QualType ExpTy = E->getType();
    if (IsMemberAccess && ExpTy->isPointerType()) {
      // If ivar is a structure pointer, assigning to field of
      // this struct follows gcc's behavior and makes it a non-ivar
      // writer-barrier conservatively.
      ExpTy = ExpTy->getAs<PointerType>()->getPointeeType();
      if (ExpTy->isRecordType()) {
        LV.setObjCIvar(false);
        return;
      }
    }
    LV.setObjCIvar(true);
    auto *Exp = cast<ObjCIvarRefExpr>(const_cast<Expr *>(E));
    LV.setBaseIvarExp(Exp->getBase());
    LV.setObjCArray(E->getType()->isArrayType());
    return;
  }

  if (const auto *Exp = dyn_cast<DeclRefExpr>(E)) {
    if (const auto *VD = dyn_cast<VarDecl>(Exp->getDecl())) {
      if (VD->hasGlobalStorage()) {
        LV.setGlobalObjCRef(true);
        LV.setThreadLocalRef(VD->getTLSKind() != VarDecl::TLS_None);
      }
    }
    LV.setObjCArray(E->getType()->isArrayType());
    return;
  }

  if (const auto *Exp = dyn_cast<UnaryOperator>(E)) {
    setObjCGCLValueClass(Ctx, Exp->getSubExpr(), LV, IsMemberAccess);
    return;
  }

  if (const auto *Exp = dyn_cast<ParenExpr>(E)) {
    setObjCGCLValueClass(Ctx, Exp->getSubExpr(), LV, IsMemberAccess);
    if (LV.isObjCIvar()) {
      // If cast is to a structure pointer, follow gcc's behavior and make it
      // a non-ivar write-barrier.
      QualType ExpTy = E->getType();
      if (ExpTy->isPointerType())
        ExpTy = ExpTy->getAs<PointerType>()->getPointeeType();
      if (ExpTy->isRecordType())
        LV.setObjCIvar(false);
    }
    return;
  }

  if (const auto *Exp = dyn_cast<GenericSelectionExpr>(E)) {
    setObjCGCLValueClass(Ctx, Exp->getResultExpr(), LV);
    return;
  }

  if (const auto *Exp = dyn_cast<ImplicitCastExpr>(E)) {
    setObjCGCLValueClass(Ctx, Exp->getSubExpr(), LV, IsMemberAccess);
    return;
  }

  if (const auto *Exp = dyn_cast<CStyleCastExpr>(E)) {
    setObjCGCLValueClass(Ctx, Exp->getSubExpr(), LV, IsMemberAccess);
    return;
  }

  if (const auto *Exp = dyn_cast<ObjCBridgedCastExpr>(E)) {
    setObjCGCLValueClass(Ctx, Exp->getSubExpr(), LV, IsMemberAccess);
    return;
  }

  if (const auto *Exp = dyn_cast<ArraySubscriptExpr>(E)) {
    setObjCGCLValueClass(Ctx, Exp->getBase(), LV);
    if (LV.isObjCIvar() && !LV.isObjCArray())
      // Using array syntax to assigning to what an ivar points to is not
      // same as assigning to the ivar itself. {id *Names;} Names[i] = 0;
      LV.setObjCIvar(false);
    else if (LV.isGlobalObjCRef() && !LV.isObjCArray())
      // Using array syntax to assigning to what global points to is not
      // same as assigning to the global itself. {id *G;} G[i] = 0;
      LV.setGlobalObjCRef(false);
    return;
  }

  if (const auto *Exp = dyn_cast<MemberExpr>(E)) {
    setObjCGCLValueClass(Ctx, Exp->getBase(), LV, true);
    // We don't know if member is an 'ivar', but this flag is looked at
    // only in the context of LV.isObjCIvar().
    LV.setObjCArray(E->getType()->isArrayType());
    return;
  }
}

static llvm::Value *
EmitBitCastOfLValueToProperType(CodeGenFunction &CGF,
                                llvm::Value *V, llvm::Type *IRType,
                                StringRef Name = StringRef()) {
  unsigned AS = cast<llvm::PointerType>(V->getType())->getAddressSpace();
  return CGF.EmitPointerCast(V, IRType->getPointerTo(AS));
}

static LValue EmitThreadPrivateVarDeclLValue(
    CodeGenFunction &CGF, const VarDecl *VD, QualType T, Address Addr,
    llvm::Type *RealVarTy, SourceLocation Loc) {
  Addr = CGF.CGM.getOpenMPRuntime().getAddrOfThreadPrivate(CGF, VD, Addr, Loc);
  Addr = CGF.Builder.CreateElementBitCast(Addr, RealVarTy);
  return CGF.MakeAddrLValue(Addr, T, AlignmentSource::Decl);
}

static Address emitDeclTargetLinkVarDeclLValue(CodeGenFunction &CGF,
                                               const VarDecl *VD, QualType T) {
  llvm::Optional<OMPDeclareTargetDeclAttr::MapTypeTy> Res =
      OMPDeclareTargetDeclAttr::isDeclareTargetDeclaration(VD);
  if (!Res || *Res == OMPDeclareTargetDeclAttr::MT_To)
    return Address::invalid();
  assert(*Res == OMPDeclareTargetDeclAttr::MT_Link && "Expected link clause");
  QualType PtrTy = CGF.getContext().getPointerType(VD->getType());
  Address Addr = CGF.CGM.getOpenMPRuntime().getAddrOfDeclareTargetLink(VD);
  return CGF.EmitLoadOfPointer(Addr, PtrTy->castAs<PointerType>());
}

Address
CodeGenFunction::EmitLoadOfReference(LValue RefLVal,
                                     LValueBaseInfo *PointeeBaseInfo,
                                     TBAAAccessInfo *PointeeTBAAInfo) {
  llvm::LoadInst *Load = Builder.CreateLoad(RefLVal.getAddress(),
                                            RefLVal.isVolatile());
  CGM.DecorateInstructionWithTBAA(Load, RefLVal.getTBAAInfo());

  CharUnits Align = getNaturalTypeAlignment(RefLVal.getType()->getPointeeType(),
                                            PointeeBaseInfo, PointeeTBAAInfo,
                                            /* forPointeeType= */ true);
  return Address(Load, Align);
}

LValue CodeGenFunction::EmitLoadOfReferenceLValue(LValue RefLVal) {
  LValueBaseInfo PointeeBaseInfo;
  TBAAAccessInfo PointeeTBAAInfo;
  Address PointeeAddr = EmitLoadOfReference(RefLVal, &PointeeBaseInfo,
                                            &PointeeTBAAInfo);
  return MakeAddrLValue(PointeeAddr, RefLVal.getType()->getPointeeType(),
                        PointeeBaseInfo, PointeeTBAAInfo);
}

Address CodeGenFunction::EmitLoadOfPointer(Address Ptr,
                                           const PointerType *PtrTy,
                                           LValueBaseInfo *BaseInfo,
                                           TBAAAccessInfo *TBAAInfo) {
  llvm::Value *Addr = Builder.CreateLoad(Ptr);
  return Address(Addr, getNaturalTypeAlignment(PtrTy->getPointeeType(),
                                               BaseInfo, TBAAInfo,
                                               /*forPointeeType=*/true));
}

LValue CodeGenFunction::EmitLoadOfPointerLValue(Address PtrAddr,
                                                const PointerType *PtrTy) {
  LValueBaseInfo BaseInfo;
  TBAAAccessInfo TBAAInfo;
  Address Addr = EmitLoadOfPointer(PtrAddr, PtrTy, &BaseInfo, &TBAAInfo);
  return MakeAddrLValue(Addr, PtrTy->getPointeeType(), BaseInfo, TBAAInfo);
}

static LValue EmitGlobalVarDeclLValue(CodeGenFunction &CGF,
                                      const Expr *E, const VarDecl *VD) {
  QualType T = E->getType();

  // If it's thread_local, emit a call to its wrapper function instead.
  if (VD->getTLSKind() == VarDecl::TLS_Dynamic &&
      CGF.CGM.getCXXABI().usesThreadWrapperFunction())
    return CGF.CGM.getCXXABI().EmitThreadLocalVarDeclLValue(CGF, VD, T);
  // Check if the variable is marked as declare target with link clause in
  // device codegen.
  if (CGF.getLangOpts().OpenMPIsDevice) {
    Address Addr = emitDeclTargetLinkVarDeclLValue(CGF, VD, T);
    if (Addr.isValid())
      return CGF.MakeAddrLValue(Addr, T, AlignmentSource::Decl);
  }

  llvm::Value *V = CGF.CGM.GetAddrOfGlobalVar(VD);
  llvm::Type *RealVarTy = CGF.getTypes().ConvertTypeForMem(VD->getType());
  V = EmitBitCastOfLValueToProperType(CGF, V, RealVarTy);
  CharUnits Alignment = CGF.getContext().getDeclAlign(VD);
  Address Addr(V, Alignment);
  // Emit reference to the private copy of the variable if it is an OpenMP
  // threadprivate variable.
  if (CGF.getLangOpts().OpenMP && !CGF.getLangOpts().OpenMPSimd &&
      VD->hasAttr<OMPThreadPrivateDeclAttr>()) {
    return EmitThreadPrivateVarDeclLValue(CGF, VD, T, Addr, RealVarTy,
                                          E->getExprLoc());
  }
  LValue LV = VD->getType()->isReferenceType() ?
      CGF.EmitLoadOfReferenceLValue(Addr, VD->getType(),
                                    AlignmentSource::Decl) :
      CGF.MakeAddrLValue(Addr, T, AlignmentSource::Decl);
  setObjCGCLValueClass(CGF.getContext(), E, LV);
  return LV;
}

llvm::Value *CodeGenFunction::FunctionAddressToCapability(CodeGenFunction &CGF,
                                                          llvm::Value *Addr,
                                                          llvm::Type *CapTy,
                                                          bool IsDirectCall) {
  auto* VTy = cast<llvm::PointerType>(Addr->getType());
  unsigned CapAS = CGF.CGM.getTargetCodeGenInfo().getCHERICapabilityAS();
  if (!CapTy)
    CapTy = VTy->getElementType()->getPointerTo(CapAS);
  const bool IsFunction = isa<llvm::FunctionType>(VTy->getPointerElementType());
  if (llvm::MCTargetOptions::cheriUsesCapabilityTable()) {
    if (IsDirectCall) {
      assert(IsFunction);
      return Addr; // Don't add a cast for direct calls
    }
    return CGF.Builder.CreatePointerBitCastOrAddrSpaceCast(Addr, CapTy);
  }

  // Without a cap table we need to get the function address using $pcc
  if (!IsFunction && VTy->getPointerAddressSpace() == CapAS)
    // For data we don't need any special handling:
    return CGF.Builder.CreateBitCast(Addr, CapTy);

  llvm::Value *V = CGF.Builder.CreatePtrToInt(Addr, CGF.Int64Ty);
  llvm::Value *PCC = CGF.Builder.CreateCall(
          CGF.CGM.getIntrinsic(llvm::Intrinsic::cheri_pcc_get), {});
  if (auto *F = dyn_cast<llvm::Function>(Addr->stripPointerCasts())) {
    if (F->hasWeakLinkage() || F->hasExternalWeakLinkage()) {
      V = CGF.Builder.CreateCall(
          CGF.CGM.getIntrinsic(llvm::Intrinsic::cheri_cap_from_pointer,
                               CGF.IntPtrTy),
          {PCC, V});
      return CGF.Builder.CreateBitCast(V, CapTy);
    }
  }
  return CGF.Builder.CreateBitCast(CGF.setPointerOffset(PCC, V), CapTy);
}

static llvm::Value *EmitFunctionDeclPointer(CodeGenFunction &CGF,
                                            const FunctionDecl *FD,
                                            bool IsDirectCall) {
  CodeGenModule &CGM = CGF.CGM;
  if (FD->hasAttr<WeakRefAttr>()) {
    ConstantAddress aliasee = CGM.GetWeakRefReference(FD);
    return aliasee.getPointer();
  }

  llvm::Value *V = CGM.GetAddrOfFunction(FD);
  auto &TI = CGF.getContext().getTargetInfo();
  if (TI.areAllPointersCapabilities())
    V = CodeGenFunction::FunctionAddressToCapability(CGF, V, nullptr,
                                                     IsDirectCall);

  if (!FD->hasPrototype()) {
    if (const FunctionProtoType *Proto =
            FD->getType()->getAs<FunctionProtoType>()) {
      // Ugly case: for a K&R-style definition, the type of the definition
      // isn't the same as the type of a use.  Correct for this with a
      // bitcast.
      QualType NoProtoType =
          CGM.getContext().getFunctionNoProtoType(Proto->getReturnType());
      NoProtoType = CGM.getContext().getPointerType(NoProtoType);
      V = CGF.Builder.CreatePointerBitCastOrAddrSpaceCast(V,
                                      CGM.getTypes().ConvertType(NoProtoType));
    }
  }
  return V;
}

static LValue EmitFunctionDeclLValue(CodeGenFunction &CGF,
                                     const Expr *E, const FunctionDecl *FD) {
  llvm::Value *V = EmitFunctionDeclPointer(CGF, FD, /*IsDirectCall=*/false);
  CharUnits Alignment = CGF.getContext().getDeclAlign(FD);
  return CGF.MakeAddrLValue(V, E->getType(), Alignment,
                            AlignmentSource::Decl);
}

static LValue EmitCapturedFieldLValue(CodeGenFunction &CGF, const FieldDecl *FD,
                                      llvm::Value *ThisValue) {
  QualType TagType = CGF.getContext().getTagDeclType(FD->getParent());
  LValue LV = CGF.MakeNaturalAlignAddrLValue(ThisValue, TagType);
  return CGF.EmitLValueForField(LV, FD);
}

/// Named Registers are named metadata pointing to the register name
/// which will be read from/written to as an argument to the intrinsic
/// @llvm.read/write_register.
/// So far, only the name is being passed down, but other options such as
/// register type, allocation type or even optimization options could be
/// passed down via the metadata node.
static LValue EmitGlobalNamedRegister(const VarDecl *VD, CodeGenModule &CGM) {
  SmallString<64> Name("llvm.named.register.");
  AsmLabelAttr *Asm = VD->getAttr<AsmLabelAttr>();
  assert(Asm->getLabel().size() < 64-Name.size() &&
      "Register name too big");
  Name.append(Asm->getLabel());
  llvm::NamedMDNode *M =
    CGM.getModule().getOrInsertNamedMetadata(Name);
  if (M->getNumOperands() == 0) {
    llvm::MDString *Str = llvm::MDString::get(CGM.getLLVMContext(),
                                              Asm->getLabel());
    llvm::Metadata *Ops[] = {Str};
    M->addOperand(llvm::MDNode::get(CGM.getLLVMContext(), Ops));
  }

  CharUnits Alignment = CGM.getContext().getDeclAlign(VD);

  llvm::Value *Ptr =
    llvm::MetadataAsValue::get(CGM.getLLVMContext(), M->getOperand(0));
  return LValue::MakeGlobalReg(Address(Ptr, Alignment), VD->getType());
}

LValue CodeGenFunction::EmitDeclRefLValue(const DeclRefExpr *E) {
  const NamedDecl *ND = E->getDecl();
  QualType T = E->getType();

  if (const auto *VD = dyn_cast<VarDecl>(ND)) {
    // Global Named registers access via intrinsics only
    if (VD->getStorageClass() == SC_Register &&
        VD->hasAttr<AsmLabelAttr>() && !VD->isLocalVarDecl())
      return EmitGlobalNamedRegister(VD, CGM);

    // A DeclRefExpr for a reference initialized by a constant expression can
    // appear without being odr-used. Directly emit the constant initializer.
    VD->getAnyInitializer(VD);
    if (E->isNonOdrUse() == NOUR_Constant && VD->getType()->isReferenceType()) {
      llvm::Constant *Val =
        ConstantEmitter(*this).emitAbstract(E->getLocation(),
                                            *VD->evaluateValue(),
                                            VD->getType());

      // If this is a CHERI reference to a function then convert the function
      // address to a capability
      const ReferenceType *RT = cast<ReferenceType>(VD->getType().getTypePtr());
      llvm::Value* Result = Val; // use llvm::Value for
                                 // CodeGenFunction::FunctionAddressToCapability
      if (RT->isCHERICapability() && RT->getPointeeType()->isFunctionType()) {
        // First strip the addrspacecast if the ConstantEmitter inserted it
        if (const auto CE = dyn_cast<llvm::ConstantExpr>(Val)) {
          if (CE->getOpcode() == llvm::Instruction::AddrSpaceCast) {
            Result = CE->getOperand(0);
          }
        }

        unsigned CapAS = CGM.getTargetCodeGenInfo().getCHERICapabilityAS();
        llvm::Type *ResTy = Result->getType();
        if (ResTy->getPointerAddressSpace() != CapAS) {
          Result = FunctionAddressToCapability(*this, Result);
        }
      }

      assert(Result && "failed to emit reference constant expression");
      // FIXME: Eventually we will want to emit vector element references.

      // Should we be using the alignment of the constant pointer we emitted?
      CharUnits Alignment = getNaturalTypeAlignment(E->getType(),
                                                    /* BaseInfo= */ nullptr,
                                                    /* TBAAInfo= */ nullptr,
                                                    /* forPointeeType= */ true);
      return MakeAddrLValue(Address(Result, Alignment), T, AlignmentSource::Decl);
    }

    // FIXME: Handle other kinds of non-odr-use DeclRefExprs.

    // Check for captured variables.
    if (E->refersToEnclosingVariableOrCapture()) {
      VD = VD->getCanonicalDecl();
      if (auto *FD = LambdaCaptureFields.lookup(VD))
        return EmitCapturedFieldLValue(*this, FD, CXXABIThisValue);
      else if (CapturedStmtInfo) {
        auto I = LocalDeclMap.find(VD);
        if (I != LocalDeclMap.end()) {
          if (VD->getType()->isReferenceType())
            return EmitLoadOfReferenceLValue(I->second, VD->getType(),
                                             AlignmentSource::Decl);
          return MakeAddrLValue(I->second, T);
        }
        LValue CapLVal =
            EmitCapturedFieldLValue(*this, CapturedStmtInfo->lookup(VD),
                                    CapturedStmtInfo->getContextValue());
        return MakeAddrLValue(
            Address(CapLVal.getPointer(), getContext().getDeclAlign(VD)),
            CapLVal.getType(), LValueBaseInfo(AlignmentSource::Decl),
            CapLVal.getTBAAInfo());
      }

      assert(isa<BlockDecl>(CurCodeDecl));
      Address addr = GetAddrOfBlockDecl(VD);
      return MakeAddrLValue(addr, T, AlignmentSource::Decl);
    }
  }

  // FIXME: We should be able to assert this for FunctionDecls as well!
  // FIXME: We should be able to assert this for all DeclRefExprs, not just
  // those with a valid source location.
  assert((ND->isUsed(false) || !isa<VarDecl>(ND) ||
          !E->getLocation().isValid()) &&
         "Should not use decl without marking it used!");

  if (ND->hasAttr<WeakRefAttr>()) {
    const auto *VD = cast<ValueDecl>(ND);
    ConstantAddress Aliasee = CGM.GetWeakRefReference(VD);
    return MakeAddrLValue(Aliasee, T, AlignmentSource::Decl);
  }

  if (const auto *VD = dyn_cast<VarDecl>(ND)) {
    // Check if this is a global variable.
    if (VD->hasLinkage() || VD->isStaticDataMember())
      return EmitGlobalVarDeclLValue(*this, E, VD);

    Address addr = Address::invalid();

    // The variable should generally be present in the local decl map.
    auto iter = LocalDeclMap.find(VD);
    if (iter != LocalDeclMap.end()) {
      addr = iter->second;

    // Otherwise, it might be static local we haven't emitted yet for
    // some reason; most likely, because it's in an outer function.
    } else if (VD->isStaticLocal()) {
      addr = Address(CGM.getOrCreateStaticVarDecl(
          *VD, CGM.getLLVMLinkageVarDefinition(VD, /*isConstant=*/false)),
                     getContext().getDeclAlign(VD));

    // No other cases for now.
    } else {
      llvm_unreachable("DeclRefExpr for Decl not entered in LocalDeclMap?");
    }


    // Check for OpenMP threadprivate variables.
    if (getLangOpts().OpenMP && !getLangOpts().OpenMPSimd &&
        VD->hasAttr<OMPThreadPrivateDeclAttr>()) {
      return EmitThreadPrivateVarDeclLValue(
          *this, VD, T, addr, getTypes().ConvertTypeForMem(VD->getType()),
          E->getExprLoc());
    }

    // Drill into block byref variables.
    bool isBlockByref = VD->isEscapingByref();
    if (isBlockByref) {
      addr = emitBlockByrefAddress(addr, VD);
    }

    // Drill into reference types.
    LValue LV = VD->getType()->isReferenceType() ?
        EmitLoadOfReferenceLValue(addr, VD->getType(), AlignmentSource::Decl) :
        MakeAddrLValue(addr, T, AlignmentSource::Decl);

    bool isLocalStorage = VD->hasLocalStorage();

    bool NonGCable = isLocalStorage &&
                     !VD->getType()->isReferenceType() &&
                     !isBlockByref;
    if (NonGCable) {
      LV.getQuals().removeObjCGCAttr();
      LV.setNonGC(true);
    }

    bool isImpreciseLifetime =
      (isLocalStorage && !VD->hasAttr<ObjCPreciseLifetimeAttr>());
    if (isImpreciseLifetime)
      LV.setARCPreciseLifetime(ARCImpreciseLifetime);
    setObjCGCLValueClass(getContext(), E, LV);
    return LV;
  }

  if (const auto *FD = dyn_cast<FunctionDecl>(ND))
    return EmitFunctionDeclLValue(*this, E, FD);

  // FIXME: While we're emitting a binding from an enclosing scope, all other
  // DeclRefExprs we see should be implicitly treated as if they also refer to
  // an enclosing scope.
  if (const auto *BD = dyn_cast<BindingDecl>(ND))
    return EmitLValue(BD->getBinding());

  llvm_unreachable("Unhandled DeclRefExpr");
}

LValue CodeGenFunction::EmitUnaryOpLValue(const UnaryOperator *E) {
  // __extension__ doesn't affect lvalue-ness.
  if (E->getOpcode() == UO_Extension)
    return EmitLValue(E->getSubExpr());

  QualType ExprTy = getContext().getCanonicalType(E->getSubExpr()->getType());
  switch (E->getOpcode()) {
  default: llvm_unreachable("Unknown unary operator lvalue!");
  case UO_Deref: {
    QualType T = E->getSubExpr()->getType()->getPointeeType();
    assert(!T.isNull() && "CodeGenFunction::EmitUnaryOpLValue: Illegal type");

    LValueBaseInfo BaseInfo;
    TBAAAccessInfo TBAAInfo;
    Address Addr = EmitPointerWithAlignment(E->getSubExpr(), &BaseInfo,
                                            &TBAAInfo);
    LValue LV = MakeAddrLValue(Addr, T, BaseInfo, TBAAInfo);
    LV.getQuals().setAddressSpace(ExprTy.getAddressSpace());

    // We should not generate __weak write barrier on indirect reference
    // of a pointer to object; as in void foo (__weak id *param); *param = 0;
    // But, we continue to generate __strong write barrier on indirect write
    // into a pointer to object.
    if (getLangOpts().ObjC &&
        getLangOpts().getGC() != LangOptions::NonGC &&
        LV.isObjCWeak())
      LV.setNonGC(!E->isOBJCGCCandidate(getContext()));
    return LV;
  }
  case UO_Real:
  case UO_Imag: {
    LValue LV = EmitLValue(E->getSubExpr());
    assert(LV.isSimple() && "real/imag on non-ordinary l-value");

    // __real is valid on scalars.  This is a faster way of testing that.
    // __imag can only produce an rvalue on scalars.
    if (E->getOpcode() == UO_Real &&
        !LV.getAddress().getElementType()->isStructTy()) {
      assert(E->getSubExpr()->getType()->isArithmeticType());
      return LV;
    }

    QualType T = ExprTy->castAs<ComplexType>()->getElementType();

    Address Component =
      (E->getOpcode() == UO_Real
         ? emitAddrOfRealComponent(LV.getAddress(), LV.getType())
         : emitAddrOfImagComponent(LV.getAddress(), LV.getType()));
    LValue ElemLV = MakeAddrLValue(Component, T, LV.getBaseInfo(),
                                   CGM.getTBAAInfoForSubobject(LV, T));
    ElemLV.getQuals().addQualifiers(LV.getQuals());
    return ElemLV;
  }
  case UO_PreInc:
  case UO_PreDec: {
    LValue LV = EmitLValue(E->getSubExpr());
    bool isInc = E->getOpcode() == UO_PreInc;

    if (E->getType()->isAnyComplexType())
      EmitComplexPrePostIncDec(E, LV, isInc, true/*isPre*/);
    else
      EmitScalarPrePostIncDec(E, LV, isInc, true/*isPre*/);
    return LV;
  }
  }
}

LValue CodeGenFunction::EmitStringLiteralLValue(const StringLiteral *E) {
  return MakeAddrLValue(CGM.GetAddrOfConstantStringFromLiteral(E),
                        E->getType(), AlignmentSource::Decl);
}

LValue CodeGenFunction::EmitObjCEncodeExprLValue(const ObjCEncodeExpr *E) {
  return MakeAddrLValue(CGM.GetAddrOfConstantStringFromObjCEncode(E),
                        E->getType(), AlignmentSource::Decl);
}

LValue CodeGenFunction::EmitPredefinedLValue(const PredefinedExpr *E) {
  auto SL = E->getFunctionName();
  assert(SL != nullptr && "No StringLiteral name in PredefinedExpr");
  StringRef FnName = CurFn->getName();
  if (FnName.startswith("\01"))
    FnName = FnName.substr(1);
  StringRef NameItems[] = {
      PredefinedExpr::getIdentKindName(E->getIdentKind()), FnName};
  std::string GVName = llvm::join(NameItems, NameItems + 2, ".");
  if (auto *BD = dyn_cast_or_null<BlockDecl>(CurCodeDecl)) {
    std::string Name = SL->getString();
    if (!Name.empty()) {
      unsigned Discriminator =
          CGM.getCXXABI().getMangleContext().getBlockId(BD, true);
      if (Discriminator)
        Name += "_" + Twine(Discriminator + 1).str();
      auto C = CGM.GetAddrOfConstantCString(Name, GVName.c_str());
      return MakeAddrLValue(C, E->getType(), AlignmentSource::Decl);
    } else {
      auto C = CGM.GetAddrOfConstantCString(FnName, GVName.c_str());
      return MakeAddrLValue(C, E->getType(), AlignmentSource::Decl);
    }
  }
  auto C = CGM.GetAddrOfConstantStringFromLiteral(SL, GVName);
  return MakeAddrLValue(C, E->getType(), AlignmentSource::Decl);
}

/// Emit a type description suitable for use by a runtime sanitizer library. The
/// format of a type descriptor is
///
/// \code
///   { i16 TypeKind, i16 TypeInfo }
/// \endcode
///
/// followed by an array of i8 containing the type name. TypeKind is 0 for an
/// integer, 1 for a floating point value, and -1 for anything else.
llvm::Constant *CodeGenFunction::EmitCheckTypeDescriptor(QualType T) {
  // Only emit each type's descriptor once.
  if (llvm::Constant *C = CGM.getTypeDescriptorFromMap(T))
    return C;

  uint16_t TypeKind = -1;
  uint16_t TypeInfo = 0;

  if (T->isIntegerType()) {
    TypeKind = 0;
    TypeInfo = (llvm::Log2_32(getContext().getTypeSize(T)) << 1) |
               (T->isSignedIntegerType() ? 1 : 0);
  } else if (T->isFloatingType()) {
    TypeKind = 1;
    TypeInfo = getContext().getTypeSize(T);
  }

  // Format the type name as if for a diagnostic, including quotes and
  // optionally an 'aka'.
  SmallString<32> Buffer;
  CGM.getDiags().ConvertArgToString(DiagnosticsEngine::ak_qualtype,
                                    (intptr_t)T.getAsOpaquePtr(),
                                    StringRef(), StringRef(), None, Buffer,
                                    None);

  llvm::Constant *Components[] = {
    Builder.getInt16(TypeKind), Builder.getInt16(TypeInfo),
    llvm::ConstantDataArray::getString(getLLVMContext(), Buffer)
  };
  llvm::Constant *Descriptor = llvm::ConstantStruct::getAnon(Components);

  auto *GV = new llvm::GlobalVariable(
      CGM.getModule(), Descriptor->getType(),
      /*isConstant=*/true, llvm::GlobalVariable::PrivateLinkage, Descriptor);
  GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
  CGM.getSanitizerMetadata()->disableSanitizerForGlobal(GV);

  // Remember the descriptor for this type.
  CGM.setTypeDescriptorInMap(T, GV);

  return GV;
}

llvm::Value *CodeGenFunction::EmitCheckValue(llvm::Value *V) {
  llvm::Type *TargetTy = IntPtrTy;

  if (V->getType() == TargetTy)
    return V;

  // Floating-point types which fit into intptr_t are bitcast to integers
  // and then passed directly (after zero-extension, if necessary).
  if (V->getType()->isFloatingPointTy()) {
    unsigned Bits = V->getType()->getPrimitiveSizeInBits();
    if (Bits <= TargetTy->getIntegerBitWidth())
      V = Builder.CreateBitCast(V, llvm::Type::getIntNTy(getLLVMContext(),
                                                         Bits));
  }

  // Integers which fit in intptr_t are zero-extended and passed directly.
  if (V->getType()->isIntegerTy() &&
      V->getType()->getIntegerBitWidth() <= TargetTy->getIntegerBitWidth())
    return Builder.CreateZExt(V, TargetTy);

  // Pointers are passed directly, everything else is passed by address.
  if (!V->getType()->isPointerTy()) {
    Address Ptr = CreateDefaultAlignTempAlloca(V->getType());
    Builder.CreateStore(V, Ptr);
    V = Ptr.getPointer();
  }
  return Builder.CreatePtrToInt(V, TargetTy);
}

/// Emit a representation of a SourceLocation for passing to a handler
/// in a sanitizer runtime library. The format for this data is:
/// \code
///   struct SourceLocation {
///     const char *Filename;
///     int32_t Line, Column;
///   };
/// \endcode
/// For an invalid SourceLocation, the Filename pointer is null.
llvm::Constant *CodeGenFunction::EmitCheckSourceLocation(SourceLocation Loc) {
  llvm::Constant *Filename;
  int Line, Column;

  PresumedLoc PLoc = getContext().getSourceManager().getPresumedLoc(Loc);
  if (PLoc.isValid()) {
    StringRef FilenameString = PLoc.getFilename();

    int PathComponentsToStrip =
        CGM.getCodeGenOpts().EmitCheckPathComponentsToStrip;
    if (PathComponentsToStrip < 0) {
      assert(PathComponentsToStrip != INT_MIN);
      int PathComponentsToKeep = -PathComponentsToStrip;
      auto I = llvm::sys::path::rbegin(FilenameString);
      auto E = llvm::sys::path::rend(FilenameString);
      while (I != E && --PathComponentsToKeep)
        ++I;

      FilenameString = FilenameString.substr(I - E);
    } else if (PathComponentsToStrip > 0) {
      auto I = llvm::sys::path::begin(FilenameString);
      auto E = llvm::sys::path::end(FilenameString);
      while (I != E && PathComponentsToStrip--)
        ++I;

      if (I != E)
        FilenameString =
            FilenameString.substr(I - llvm::sys::path::begin(FilenameString));
      else
        FilenameString = llvm::sys::path::filename(FilenameString);
    }

    auto FilenameGV = CGM.GetAddrOfConstantCString(FilenameString, ".src");
    CGM.getSanitizerMetadata()->disableSanitizerForGlobal(
                          cast<llvm::GlobalVariable>(FilenameGV.getPointer()));
    Filename = FilenameGV.getPointer();
    Line = PLoc.getLine();
    Column = PLoc.getColumn();
  } else {
    Filename = llvm::Constant::getNullValue(Int8PtrTy);
    Line = Column = 0;
  }

  llvm::Constant *Data[] = {Filename, Builder.getInt32(Line),
                            Builder.getInt32(Column)};

  return llvm::ConstantStruct::getAnon(Data);
}

namespace {
/// Specify under what conditions this check can be recovered
enum class CheckRecoverableKind {
  /// Always terminate program execution if this check fails.
  Unrecoverable,
  /// Check supports recovering, runtime has both fatal (noreturn) and
  /// non-fatal handlers for this check.
  Recoverable,
  /// Runtime conditionally aborts, always need to support recovery.
  AlwaysRecoverable
};
}

static CheckRecoverableKind getRecoverableKind(SanitizerMask Kind) {
  assert(Kind.countPopulation() == 1);
  if (Kind == SanitizerKind::Vptr)
    return CheckRecoverableKind::AlwaysRecoverable;
  else if (Kind == SanitizerKind::Return || Kind == SanitizerKind::Unreachable)
    return CheckRecoverableKind::Unrecoverable;
  else
    return CheckRecoverableKind::Recoverable;
}

namespace {
struct SanitizerHandlerInfo {
  char const *const Name;
  unsigned Version;
};
}

const SanitizerHandlerInfo SanitizerHandlers[] = {
#define SANITIZER_CHECK(Enum, Name, Version) {#Name, Version},
    LIST_SANITIZER_CHECKS
#undef SANITIZER_CHECK
};

static void emitCheckHandlerCall(CodeGenFunction &CGF,
                                 llvm::FunctionType *FnType,
                                 ArrayRef<llvm::Value *> FnArgs,
                                 SanitizerHandler CheckHandler,
                                 CheckRecoverableKind RecoverKind, bool IsFatal,
                                 llvm::BasicBlock *ContBB) {
  assert(IsFatal || RecoverKind != CheckRecoverableKind::Unrecoverable);
  Optional<ApplyDebugLocation> DL;
  if (!CGF.Builder.getCurrentDebugLocation()) {
    // Ensure that the call has at least an artificial debug location.
    DL.emplace(CGF, SourceLocation());
  }
  bool NeedsAbortSuffix =
      IsFatal && RecoverKind != CheckRecoverableKind::Unrecoverable;
  bool MinimalRuntime = CGF.CGM.getCodeGenOpts().SanitizeMinimalRuntime;
  const SanitizerHandlerInfo &CheckInfo = SanitizerHandlers[CheckHandler];
  const StringRef CheckName = CheckInfo.Name;
  std::string FnName = "__ubsan_handle_" + CheckName.str();
  if (CheckInfo.Version && !MinimalRuntime)
    FnName += "_v" + llvm::utostr(CheckInfo.Version);
  if (MinimalRuntime)
    FnName += "_minimal";
  if (NeedsAbortSuffix)
    FnName += "_abort";
  bool MayReturn =
      !IsFatal || RecoverKind == CheckRecoverableKind::AlwaysRecoverable;

  llvm::AttrBuilder B;
  if (!MayReturn) {
    B.addAttribute(llvm::Attribute::NoReturn)
        .addAttribute(llvm::Attribute::NoUnwind);
  }
  B.addAttribute(llvm::Attribute::UWTable);

  llvm::FunctionCallee Fn = CGF.CGM.CreateRuntimeFunction(
      FnType, FnName,
      llvm::AttributeList::get(CGF.getLLVMContext(),
                               llvm::AttributeList::FunctionIndex, B),
      /*Local=*/true);
  llvm::CallInst *HandlerCall = CGF.EmitNounwindRuntimeCall(Fn, FnArgs);
  if (!MayReturn) {
    HandlerCall->setDoesNotReturn();
    CGF.Builder.CreateUnreachable();
  } else {
    CGF.Builder.CreateBr(ContBB);
  }
}

void CodeGenFunction::EmitCheck(
    ArrayRef<std::pair<llvm::Value *, SanitizerMask>> Checked,
    SanitizerHandler CheckHandler, ArrayRef<llvm::Constant *> StaticArgs,
    ArrayRef<llvm::Value *> DynamicArgs) {
  assert(IsSanitizerScope);
  assert(Checked.size() > 0);
  assert(CheckHandler >= 0 &&
         size_t(CheckHandler) < llvm::array_lengthof(SanitizerHandlers));
  const StringRef CheckName = SanitizerHandlers[CheckHandler].Name;

  llvm::Value *FatalCond = nullptr;
  llvm::Value *RecoverableCond = nullptr;
  llvm::Value *TrapCond = nullptr;
  for (int i = 0, n = Checked.size(); i < n; ++i) {
    llvm::Value *Check = Checked[i].first;
    // -fsanitize-trap= overrides -fsanitize-recover=.
    llvm::Value *&Cond =
        CGM.getCodeGenOpts().SanitizeTrap.has(Checked[i].second)
            ? TrapCond
            : CGM.getCodeGenOpts().SanitizeRecover.has(Checked[i].second)
                  ? RecoverableCond
                  : FatalCond;
    Cond = Cond ? Builder.CreateAnd(Cond, Check) : Check;
  }

  if (TrapCond)
    EmitTrapCheck(TrapCond);
  if (!FatalCond && !RecoverableCond)
    return;

  llvm::Value *JointCond;
  if (FatalCond && RecoverableCond)
    JointCond = Builder.CreateAnd(FatalCond, RecoverableCond);
  else
    JointCond = FatalCond ? FatalCond : RecoverableCond;
  assert(JointCond);

  CheckRecoverableKind RecoverKind = getRecoverableKind(Checked[0].second);
  assert(SanOpts.has(Checked[0].second));
#ifndef NDEBUG
  for (int i = 1, n = Checked.size(); i < n; ++i) {
    assert(RecoverKind == getRecoverableKind(Checked[i].second) &&
           "All recoverable kinds in a single check must be same!");
    assert(SanOpts.has(Checked[i].second));
  }
#endif

  llvm::BasicBlock *Cont = createBasicBlock("cont");
  llvm::BasicBlock *Handlers = createBasicBlock("handler." + CheckName);
  llvm::Instruction *Branch = Builder.CreateCondBr(JointCond, Cont, Handlers);
  // Give hint that we very much don't expect to execute the handler
  // Value chosen to match UR_NONTAKEN_WEIGHT, see BranchProbabilityInfo.cpp
  llvm::MDBuilder MDHelper(getLLVMContext());
  llvm::MDNode *Node = MDHelper.createBranchWeights((1U << 20) - 1, 1);
  Branch->setMetadata(llvm::LLVMContext::MD_prof, Node);
  EmitBlock(Handlers);

  // Handler functions take an i8* pointing to the (handler-specific) static
  // information block, followed by a sequence of intptr_t arguments
  // representing operand values.
  SmallVector<llvm::Value *, 4> Args;
  SmallVector<llvm::Type *, 4> ArgTypes;
  if (!CGM.getCodeGenOpts().SanitizeMinimalRuntime) {
    Args.reserve(DynamicArgs.size() + 1);
    ArgTypes.reserve(DynamicArgs.size() + 1);

    // Emit handler arguments and create handler function type.
    if (!StaticArgs.empty()) {
      llvm::Constant *Info = llvm::ConstantStruct::getAnon(StaticArgs);
      auto *InfoPtr =
          new llvm::GlobalVariable(CGM.getModule(), Info->getType(), false,
                                   llvm::GlobalVariable::PrivateLinkage, Info);
      InfoPtr->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
      CGM.getSanitizerMetadata()->disableSanitizerForGlobal(InfoPtr);
      Args.push_back(Builder.CreateBitCast(InfoPtr, Int8PtrTy));
      ArgTypes.push_back(Int8PtrTy);
    }

    for (size_t i = 0, n = DynamicArgs.size(); i != n; ++i) {
      Args.push_back(EmitCheckValue(DynamicArgs[i]));
      ArgTypes.push_back(IntPtrTy);
    }
  }

  llvm::FunctionType *FnType =
    llvm::FunctionType::get(CGM.VoidTy, ArgTypes, false);

  if (!FatalCond || !RecoverableCond) {
    // Simple case: we need to generate a single handler call, either
    // fatal, or non-fatal.
    emitCheckHandlerCall(*this, FnType, Args, CheckHandler, RecoverKind,
                         (FatalCond != nullptr), Cont);
  } else {
    // Emit two handler calls: first one for set of unrecoverable checks,
    // another one for recoverable.
    llvm::BasicBlock *NonFatalHandlerBB =
        createBasicBlock("non_fatal." + CheckName);
    llvm::BasicBlock *FatalHandlerBB = createBasicBlock("fatal." + CheckName);
    Builder.CreateCondBr(FatalCond, NonFatalHandlerBB, FatalHandlerBB);
    EmitBlock(FatalHandlerBB);
    emitCheckHandlerCall(*this, FnType, Args, CheckHandler, RecoverKind, true,
                         NonFatalHandlerBB);
    EmitBlock(NonFatalHandlerBB);
    emitCheckHandlerCall(*this, FnType, Args, CheckHandler, RecoverKind, false,
                         Cont);
  }

  EmitBlock(Cont);
}

void CodeGenFunction::EmitCfiSlowPathCheck(
    SanitizerMask Kind, llvm::Value *Cond, llvm::ConstantInt *TypeId,
    llvm::Value *Ptr, ArrayRef<llvm::Constant *> StaticArgs) {
  llvm::BasicBlock *Cont = createBasicBlock("cfi.cont");

  llvm::BasicBlock *CheckBB = createBasicBlock("cfi.slowpath");
  llvm::BranchInst *BI = Builder.CreateCondBr(Cond, Cont, CheckBB);

  llvm::MDBuilder MDHelper(getLLVMContext());
  llvm::MDNode *Node = MDHelper.createBranchWeights((1U << 20) - 1, 1);
  BI->setMetadata(llvm::LLVMContext::MD_prof, Node);

  EmitBlock(CheckBB);

  bool WithDiag = !CGM.getCodeGenOpts().SanitizeTrap.has(Kind);

  llvm::CallInst *CheckCall;
  llvm::FunctionCallee SlowPathFn;
  if (WithDiag) {
    llvm::Constant *Info = llvm::ConstantStruct::getAnon(StaticArgs);
    auto *InfoPtr =
        new llvm::GlobalVariable(CGM.getModule(), Info->getType(), false,
                                 llvm::GlobalVariable::PrivateLinkage, Info);
    InfoPtr->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    CGM.getSanitizerMetadata()->disableSanitizerForGlobal(InfoPtr);

    SlowPathFn = CGM.getModule().getOrInsertFunction(
        "__cfi_slowpath_diag",
        llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy, Int8PtrTy},
                                false));
    CheckCall = Builder.CreateCall(
        SlowPathFn, {TypeId, Ptr, Builder.CreateBitCast(InfoPtr, Int8PtrTy)});
  } else {
    SlowPathFn = CGM.getModule().getOrInsertFunction(
        "__cfi_slowpath",
        llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy}, false));
    CheckCall = Builder.CreateCall(SlowPathFn, {TypeId, Ptr});
  }

  CGM.setDSOLocal(
      cast<llvm::GlobalValue>(SlowPathFn.getCallee()->stripPointerCasts()));
  CheckCall->setDoesNotThrow();

  EmitBlock(Cont);
}

// Emit a stub for __cfi_check function so that the linker knows about this
// symbol in LTO mode.
void CodeGenFunction::EmitCfiCheckStub() {
  llvm::Module *M = &CGM.getModule();
  auto &Ctx = M->getContext();
  llvm::Function *F = llvm::Function::Create(
      llvm::FunctionType::get(VoidTy, {Int64Ty, Int8PtrTy, Int8PtrTy}, false),
      llvm::GlobalValue::WeakAnyLinkage, "__cfi_check", M);
  CGM.setDSOLocal(F);
  llvm::BasicBlock *BB = llvm::BasicBlock::Create(Ctx, "entry", F);
  // FIXME: consider emitting an intrinsic call like
  // call void @llvm.cfi_check(i64 %0, i8* %1, i8* %2)
  // which can be lowered in CrossDSOCFI pass to the actual contents of
  // __cfi_check. This would allow inlining of __cfi_check calls.
  llvm::CallInst::Create(
      llvm::Intrinsic::getDeclaration(M, llvm::Intrinsic::trap), "", BB);
  llvm::ReturnInst::Create(Ctx, nullptr, BB);
}

// This function is basically a switch over the CFI failure kind, which is
// extracted from CFICheckFailData (1st function argument). Each case is either
// llvm.trap or a call to one of the two runtime handlers, based on
// -fsanitize-trap and -fsanitize-recover settings.  Default case (invalid
// failure kind) traps, but this should really never happen.  CFICheckFailData
// can be nullptr if the calling module has -fsanitize-trap behavior for this
// check kind; in this case __cfi_check_fail traps as well.
void CodeGenFunction::EmitCfiCheckFail() {
  SanitizerScope SanScope(this);
  FunctionArgList Args;
  ImplicitParamDecl ArgData(getContext(), getContext().VoidPtrTy,
                            ImplicitParamDecl::Other);
  ImplicitParamDecl ArgAddr(getContext(), getContext().VoidPtrTy,
                            ImplicitParamDecl::Other);
  Args.push_back(&ArgData);
  Args.push_back(&ArgAddr);

  const CGFunctionInfo &FI =
    CGM.getTypes().arrangeBuiltinFunctionDeclaration(getContext().VoidTy, Args);

  llvm::Function *F = llvm::Function::Create(
      llvm::FunctionType::get(VoidTy, {VoidPtrTy, VoidPtrTy}, false),
      llvm::GlobalValue::WeakODRLinkage, "__cfi_check_fail", &CGM.getModule());
  F->setVisibility(llvm::GlobalValue::HiddenVisibility);

  StartFunction(GlobalDecl(), CGM.getContext().VoidTy, F, FI, Args,
                SourceLocation());

  // This function should not be affected by blacklist. This function does
  // not have a source location, but "src:*" would still apply. Revert any
  // changes to SanOpts made in StartFunction.
  SanOpts = CGM.getLangOpts().Sanitize;

  llvm::Value *Data =
      EmitLoadOfScalar(GetAddrOfLocalVar(&ArgData), /*Volatile=*/false,
                       CGM.getContext().VoidPtrTy, ArgData.getLocation());
  llvm::Value *Addr =
      EmitLoadOfScalar(GetAddrOfLocalVar(&ArgAddr), /*Volatile=*/false,
                       CGM.getContext().VoidPtrTy, ArgAddr.getLocation());

  // Data == nullptr means the calling module has trap behaviour for this check.
  llvm::Value *DataIsNotNullPtr =
      Builder.CreateICmpNE(Data, llvm::ConstantPointerNull::get(Int8PtrTy));
  EmitTrapCheck(DataIsNotNullPtr);

  llvm::StructType *SourceLocationTy =
      llvm::StructType::get(VoidPtrTy, Int32Ty, Int32Ty);
  llvm::StructType *CfiCheckFailDataTy =
      llvm::StructType::get(Int8Ty, SourceLocationTy, VoidPtrTy);

  llvm::Value *V = Builder.CreateConstGEP2_32(
      CfiCheckFailDataTy,
      Builder.CreatePointerCast(Data, CfiCheckFailDataTy->getPointerTo(0)), 0,
      0);
  Address CheckKindAddr(V, getIntAlign());
  llvm::Value *CheckKind = Builder.CreateLoad(CheckKindAddr);

  llvm::Value *AllVtables = llvm::MetadataAsValue::get(
      CGM.getLLVMContext(),
      llvm::MDString::get(CGM.getLLVMContext(), "all-vtables"));
  llvm::Value *ValidVtable = Builder.CreateZExt(
      Builder.CreateCall(CGM.getIntrinsic(llvm::Intrinsic::type_test),
                         {Addr, AllVtables}),
      IntPtrTy);

  const std::pair<int, SanitizerMask> CheckKinds[] = {
      {CFITCK_VCall, SanitizerKind::CFIVCall},
      {CFITCK_NVCall, SanitizerKind::CFINVCall},
      {CFITCK_DerivedCast, SanitizerKind::CFIDerivedCast},
      {CFITCK_UnrelatedCast, SanitizerKind::CFIUnrelatedCast},
      {CFITCK_ICall, SanitizerKind::CFIICall}};

  SmallVector<std::pair<llvm::Value *, SanitizerMask>, 5> Checks;
  for (auto CheckKindMaskPair : CheckKinds) {
    int Kind = CheckKindMaskPair.first;
    SanitizerMask Mask = CheckKindMaskPair.second;
    llvm::Value *Cond =
        Builder.CreateICmpNE(CheckKind, llvm::ConstantInt::get(Int8Ty, Kind));
    if (CGM.getLangOpts().Sanitize.has(Mask))
      EmitCheck(std::make_pair(Cond, Mask), SanitizerHandler::CFICheckFail, {},
                {Data, Addr, ValidVtable});
    else
      EmitTrapCheck(Cond);
  }

  FinishFunction();
  // The only reference to this function will be created during LTO link.
  // Make sure it survives until then.
  CGM.addUsedGlobal(F);
}

void CodeGenFunction::EmitUnreachable(SourceLocation Loc) {
  if (SanOpts.has(SanitizerKind::Unreachable)) {
    SanitizerScope SanScope(this);
    EmitCheck(std::make_pair(static_cast<llvm::Value *>(Builder.getFalse()),
                             SanitizerKind::Unreachable),
              SanitizerHandler::BuiltinUnreachable,
              EmitCheckSourceLocation(Loc), None);
  }
  Builder.CreateUnreachable();
}

void CodeGenFunction::EmitTrapCheck(llvm::Value *Checked) {
  llvm::BasicBlock *Cont = createBasicBlock("cont");

  // If we're optimizing, collapse all calls to trap down to just one per
  // function to save on code size.
  if (!CGM.getCodeGenOpts().OptimizationLevel || !TrapBB) {
    TrapBB = createBasicBlock("trap");
    Builder.CreateCondBr(Checked, Cont, TrapBB);
    EmitBlock(TrapBB);
    llvm::CallInst *TrapCall = EmitTrapCall(llvm::Intrinsic::trap);
    TrapCall->setDoesNotReturn();
    TrapCall->setDoesNotThrow();
    Builder.CreateUnreachable();
  } else {
    Builder.CreateCondBr(Checked, Cont, TrapBB);
  }

  EmitBlock(Cont);
}

llvm::CallInst *CodeGenFunction::EmitTrapCall(llvm::Intrinsic::ID IntrID) {
  llvm::CallInst *TrapCall = Builder.CreateCall(CGM.getIntrinsic(IntrID));

  if (!CGM.getCodeGenOpts().TrapFuncName.empty()) {
    auto A = llvm::Attribute::get(getLLVMContext(), "trap-func-name",
                                  CGM.getCodeGenOpts().TrapFuncName);
    TrapCall->addAttribute(llvm::AttributeList::FunctionIndex, A);
  }

  return TrapCall;
}

Address CodeGenFunction::EmitArrayToPointerDecay(const Expr *E,
                                                 LValueBaseInfo *BaseInfo,
                                                 TBAAAccessInfo *TBAAInfo) {
  assert(E->getType()->isArrayType() &&
         "Array to pointer decay must have array source type!");

  // Expressions of array type can't be bitfields or vector elements.
  LValue LV = EmitLValue(E);
  Address Addr = LV.getAddress();

  // If the array type was an incomplete type, we need to make sure
  // the decay ends up being the right type.
  llvm::Type *NewTy = ConvertType(E->getType());
  Addr = Builder.CreateElementBitCast(Addr, NewTy);

  // Note that VLA pointers are always decayed, so we don't need to do
  // anything here.
  if (!E->getType()->isVariableArrayType()) {
    assert(isa<llvm::ArrayType>(Addr.getElementType()) &&
           "Expected pointer to array");
    Addr = Builder.CreateConstArrayGEP(Addr, 0, "arraydecay");
  }

  // The result of this decay conversion points to an array element within the
  // base lvalue. However, since TBAA currently does not support representing
  // accesses to elements of member arrays, we conservatively represent accesses
  // to the pointee object as if it had no any base lvalue specified.
  // TODO: Support TBAA for member arrays.
  QualType EltType = E->getType()->castAsArrayTypeUnsafe()->getElementType();
  if (BaseInfo) *BaseInfo = LV.getBaseInfo();
  if (TBAAInfo) *TBAAInfo = CGM.getTBAAAccessInfo(EltType);

  Addr = Builder.CreateElementBitCast(Addr, ConvertTypeForMem(EltType));
  // FIXME-cheri-qual: should getTargetAddressSpace return 200? 
  unsigned AS =
    CGM.getTargetAddressSpace(E->getType().getAddressSpace());
  llvm::PointerType *PtrTy = cast<llvm::PointerType>(Addr.getPointer()->getType());
  if (PtrTy->getPointerAddressSpace() != AS) {
    if (getContext().getTargetInfo().areAllPointersCapabilities()) {
      assert(PtrTy->getPointerAddressSpace() == 
                        CGM.getTargetCodeGenInfo().getCHERICapabilityAS() &&
             "Expected memory capability address space in pure capability ABI");
      assert(E->getType().getAddressSpace() == LangAS::Default &&
             "non-zero address space in pure capability ABI");
      return Addr;
    } else
      Addr = Address(Builder.CreateAddrSpaceCast(Addr.getPointer(),
            llvm::PointerType::get(PtrTy->getElementType(), AS)),
          Addr.getAlignment());
  }
  if (getLangOpts().getCheriBounds() >= LangOptions::CBM_SubObjectsSafe) {
    auto BoundedResult = setCHERIBoundsOnArrayDecay(Addr.getPointer(), E);
    assert(BoundedResult->getType() == Addr.getType());
    Addr = Address(BoundedResult, Addr.getAlignment());
  }
  return Addr;
}

/// isSimpleArrayDecayOperand - If the specified expr is a simple decay from an
/// array to pointer, return the array subexpression.
static const Expr *isSimpleArrayDecayOperand(const Expr *E) {
  // If this isn't just an array->pointer decay, bail out.
  const auto *CE = dyn_cast<CastExpr>(E);
  if (!CE || CE->getCastKind() != CK_ArrayToPointerDecay)
    return nullptr;

  // If this is a decay from variable width array, bail out.
  const Expr *SubExpr = CE->getSubExpr();
  if (SubExpr->getType()->isVariableArrayType())
    return nullptr;

  return SubExpr;
}

static llvm::Value *
emitArraySubscriptGEP(CodeGenFunction &CGF, llvm::Value *ptr,
                      ArrayRef<llvm::Value *> indices, bool inbounds,
                      bool signedIndices, const Expr *E,
                      const llvm::Twine &name = "arrayidx") {

  if (auto *ASE = dyn_cast<ArraySubscriptExpr>(E)) {
    if (CGF.getLangOpts().getCheriBounds() >= LangOptions::CBM_SubObjectsSafe) {
      // CSetBounds must be done before the GEP otherwise we set the base before
      // adding the index!
      auto BoundedResult = CGF.setCHERIBoundsOnArraySubscript(ptr, ASE);
      assert(BoundedResult->getType() == ptr->getType());
      ptr = BoundedResult;
    }
  } else {
    // For now don't setbounds for OMPArraySectionExpr
    assert(isa<OMPArraySectionExpr>(E) && "Called with wrong expr type");
  }
  if (inbounds) {
    return CGF.EmitCheckedInBoundsGEP(ptr, indices, signedIndices,
                                      CodeGenFunction::NotSubtraction,
                                      E->getExprLoc(), name);
  } else {
    return CGF.Builder.CreateGEP(ptr, indices, name);
  }
}

static CharUnits getArrayElementAlign(CharUnits arrayAlign,
                                      llvm::Value *idx,
                                      CharUnits eltSize) {
  // If we have a constant index, we can use the exact offset of the
  // element we're accessing.
  if (auto constantIdx = dyn_cast<llvm::ConstantInt>(idx)) {
    CharUnits offset = constantIdx->getZExtValue() * eltSize;
    return arrayAlign.alignmentAtOffset(offset);

  // Otherwise, use the worst-case alignment for any element.
  } else {
    return arrayAlign.alignmentOfArrayElement(eltSize);
  }
}

static QualType getFixedSizeElementType(const ASTContext &ctx,
                                        const VariableArrayType *vla) {
  QualType eltType;
  do {
    eltType = vla->getElementType();
  } while ((vla = ctx.getAsVariableArrayType(eltType)));
  return eltType;
}

static Address emitArraySubscriptGEP(CodeGenFunction &CGF, Address addr,
                                     ArrayRef<llvm::Value *> indices,
                                     QualType eltType, bool inbounds,
                                     bool signedIndices, const Expr *E,
                                     const llvm::Twine &name = "arrayidx") {
  // All the indices except that last must be zero.
#ifndef NDEBUG
  for (auto idx : indices.drop_back())
    assert(isa<llvm::ConstantInt>(idx) &&
           cast<llvm::ConstantInt>(idx)->isZero());
#endif

  // Determine the element size of the statically-sized base.  This is
  // the thing that the indices are expressed in terms of.
  if (auto vla = CGF.getContext().getAsVariableArrayType(eltType)) {
    eltType = getFixedSizeElementType(CGF.getContext(), vla);
  }

  // We can use that to compute the best alignment of the element.
  CharUnits eltSize = CGF.getContext().getTypeSizeInChars(eltType);
  CharUnits eltAlign =
    getArrayElementAlign(addr.getAlignment(), indices.back(), eltSize);

  llvm::Value *eltPtr = emitArraySubscriptGEP(CGF, addr.getPointer(), indices,
                                              inbounds, signedIndices, E, name);
  return Address(eltPtr, eltAlign);
}

LValue CodeGenFunction::EmitArraySubscriptExpr(const ArraySubscriptExpr *E,
                                               bool Accessed) {
  // The index must always be an integer, which is not an aggregate.  Emit it
  // in lexical order (this complexity is, sadly, required by C++17).
  llvm::Value *IdxPre =
      (E->getLHS() == E->getIdx()) ? EmitScalarExpr(E->getIdx()) : nullptr;
  bool SignedIndices = false;
  auto EmitIdxAfterBase = [&, IdxPre](bool Promote) -> llvm::Value * {
    auto *Idx = IdxPre;
    if (E->getLHS() != E->getIdx()) {
      assert(E->getRHS() == E->getIdx() && "index was neither LHS nor RHS");
      Idx = EmitScalarExpr(E->getIdx());
    }

    QualType IdxTy = E->getIdx()->getType();
    bool IdxSigned = IdxTy->isSignedIntegerOrEnumerationType();
    SignedIndices |= IdxSigned;

    if (SanOpts.has(SanitizerKind::ArrayBounds))
      EmitBoundsCheck(E, E->getBase(), Idx, IdxTy, Accessed);

    if (Idx->getType()->isPointerTy())
      Idx = getCapabilityIntegerValue(Idx);

    // Extend or truncate the index type to 32 or 64-bits.
    if (Promote && Idx->getType() != IntPtrTy)
      Idx = Builder.CreateIntCast(Idx, IntPtrTy, IdxSigned, "idxprom");

    return Idx;
  };
  IdxPre = nullptr;

  // If the base is a vector type, then we are forming a vector element lvalue
  // with this subscript.
  if (E->getBase()->getType()->isVectorType() &&
      !isa<ExtVectorElementExpr>(E->getBase())) {
    // Emit the vector as an lvalue to get its address.
    LValue LHS = EmitLValue(E->getBase());
    auto *Idx = EmitIdxAfterBase(/*Promote*/false);
    assert(LHS.isSimple() && "Can only subscript lvalue vectors here!");
    assert(getLangOpts().getCheriBounds() < LangOptions::CBM_SubObjectsSafe &&
           "vector subobject bounds not implemented yet");
    return LValue::MakeVectorElt(LHS.getAddress(), Idx, E->getBase()->getType(),
                                 LHS.getBaseInfo(), TBAAAccessInfo());
  }

  // All the other cases basically behave like simple offsetting.

  // Handle the extvector case we ignored above.
  if (isa<ExtVectorElementExpr>(E->getBase())) {
    LValue LV = EmitLValue(E->getBase());
    auto *Idx = EmitIdxAfterBase(/*Promote*/true);
    Address Addr = EmitExtVectorElementLValue(LV);

    QualType EltType = LV.getType()->castAs<VectorType>()->getElementType();
    Addr = emitArraySubscriptGEP(*this, Addr, Idx, EltType, /*inbounds*/ true,
                                 SignedIndices, E);
    assert(getLangOpts().getCheriBounds() < LangOptions::CBM_SubObjectsSafe &&
           "ExtVectorElementExpr subobject bounds not implemented yet");
    return MakeAddrLValue(Addr, EltType, LV.getBaseInfo(),
                          CGM.getTBAAInfoForSubobject(LV, EltType));
  }

  LValueBaseInfo EltBaseInfo;
  TBAAAccessInfo EltTBAAInfo;
  Address Addr = Address::invalid();
  if (const VariableArrayType *vla =
           getContext().getAsVariableArrayType(E->getType())) {
    // The base must be a pointer, which is not an aggregate.  Emit
    // it.  It needs to be emitted first in case it's what captures
    // the VLA bounds.
    Addr = EmitPointerWithAlignment(E->getBase(), &EltBaseInfo, &EltTBAAInfo);
    auto *Idx = EmitIdxAfterBase(/*Promote*/true);

    // The element count here is the total number of non-VLA elements.
    llvm::Value *numElements = getVLASize(vla).NumElts;

    // Effectively, the multiply by the VLA size is part of the GEP.
    // GEP indexes are signed, and scaling an index isn't permitted to
    // signed-overflow, so we use the same semantics for our explicit
    // multiply.  We suppress this if overflow is not undefined behavior.
    if (getLangOpts().isSignedOverflowDefined()) {
      Idx = Builder.CreateMul(Idx, numElements);
    } else {
      Idx = Builder.CreateNSWMul(Idx, numElements);
    }

    Addr = emitArraySubscriptGEP(*this, Addr, Idx, vla->getElementType(),
                                 !getLangOpts().isSignedOverflowDefined(),
                                 SignedIndices, E);

  } else if (const ObjCObjectType *OIT = E->getType()->getAs<ObjCObjectType>()){
    // Indexing over an interface, as in "NSString *P; P[4];"

    // Emit the base pointer.
    Addr = EmitPointerWithAlignment(E->getBase(), &EltBaseInfo, &EltTBAAInfo);
    auto *Idx = EmitIdxAfterBase(/*Promote*/true);

    CharUnits InterfaceSize = getContext().getTypeSizeInChars(OIT);
    llvm::Value *InterfaceSizeVal =
        llvm::ConstantInt::get(Idx->getType(), InterfaceSize.getQuantity());

    llvm::Value *ScaledIdx = Builder.CreateMul(Idx, InterfaceSizeVal);

    // We don't necessarily build correct LLVM struct types for ObjC
    // interfaces, so we can't rely on GEP to do this scaling
    // correctly, so we need to cast to i8*.  FIXME: is this actually
    // true?  A lot of other things in the fragile ABI would break...
    llvm::Type *OrigBaseTy = Addr.getType();
    Addr = Builder.CreateElementBitCast(Addr, Int8Ty);

    // Do the GEP.
    CharUnits EltAlign =
      getArrayElementAlign(Addr.getAlignment(), Idx, InterfaceSize);
    llvm::Value *EltPtr = emitArraySubscriptGEP(
        *this, Addr.getPointer(), ScaledIdx, false, SignedIndices, E);
    Addr = Address(EltPtr, EltAlign);

    // Cast back.
    Addr = Builder.CreateBitCast(Addr, OrigBaseTy);
  } else if (const Expr *Array = isSimpleArrayDecayOperand(E->getBase())) {
    // If this is A[i] where A is an array, the frontend will have decayed the
    // base to be a ArrayToPointerDecay implicit cast.  While correct, it is
    // inefficient at -O0 to emit a "gep A, 0, 0" when codegen'ing it, then a
    // "gep x, i" here.  Emit one "gep A, 0, i".
    assert(Array->getType()->isArrayType() &&
           "Array to pointer decay must have array source type!");
    LValue ArrayLV;
    // For simple multidimensional array indexing, set the 'accessed' flag for
    // better bounds-checking of the base expression.
    if (const auto *ASE = dyn_cast<ArraySubscriptExpr>(Array))
      ArrayLV = EmitArraySubscriptExpr(ASE, /*Accessed*/ true);
    else
      ArrayLV = EmitLValue(Array);
    auto *Idx = EmitIdxAfterBase(/*Promote*/true);

    // Propagate the alignment from the array itself to the result.
    Addr = emitArraySubscriptGEP(
        *this, ArrayLV.getAddress(), {CGM.getSize(CharUnits::Zero()), Idx},
        E->getType(), !getLangOpts().isSignedOverflowDefined(), SignedIndices,
        E);
    EltBaseInfo = ArrayLV.getBaseInfo();
    EltTBAAInfo = CGM.getTBAAInfoForSubobject(ArrayLV, E->getType());
  } else {
    // The base must be a pointer; emit it with an estimate of its alignment.
    Addr = EmitPointerWithAlignment(E->getBase(), &EltBaseInfo, &EltTBAAInfo);
    auto *Idx = EmitIdxAfterBase(/*Promote*/true);
    Addr = emitArraySubscriptGEP(*this, Addr, Idx, E->getType(),
                                 !getLangOpts().isSignedOverflowDefined(),
                                 SignedIndices, E);
  }

  LValue LV = MakeAddrLValue(Addr, E->getType(), EltBaseInfo, EltTBAAInfo);

  if (getLangOpts().ObjC &&
      getLangOpts().getGC() != LangOptions::NonGC) {
    LV.setNonGC(!E->isOBJCGCCandidate(getContext()));
    setObjCGCLValueClass(getContext(), E, LV);
  }
  return LV;
}

static Address emitOMPArraySectionBase(CodeGenFunction &CGF, const Expr *Base,
                                       LValueBaseInfo &BaseInfo,
                                       TBAAAccessInfo &TBAAInfo,
                                       QualType BaseTy, QualType ElTy,
                                       bool IsLowerBound) {
  LValue BaseLVal;
  if (auto *ASE = dyn_cast<OMPArraySectionExpr>(Base->IgnoreParenImpCasts())) {
    BaseLVal = CGF.EmitOMPArraySectionExpr(ASE, IsLowerBound);
    if (BaseTy->isArrayType()) {
      Address Addr = BaseLVal.getAddress();
      BaseInfo = BaseLVal.getBaseInfo();

      // If the array type was an incomplete type, we need to make sure
      // the decay ends up being the right type.
      llvm::Type *NewTy = CGF.ConvertType(BaseTy);
      Addr = CGF.Builder.CreateElementBitCast(Addr, NewTy);

      // Note that VLA pointers are always decayed, so we don't need to do
      // anything here.
      if (!BaseTy->isVariableArrayType()) {
        assert(isa<llvm::ArrayType>(Addr.getElementType()) &&
               "Expected pointer to array");
        Addr = CGF.Builder.CreateConstArrayGEP(Addr, 0, "arraydecay");
      }

      return CGF.Builder.CreateElementBitCast(Addr,
                                              CGF.ConvertTypeForMem(ElTy));
    }
    LValueBaseInfo TypeBaseInfo;
    TBAAAccessInfo TypeTBAAInfo;
    CharUnits Align = CGF.getNaturalTypeAlignment(ElTy, &TypeBaseInfo,
                                                  &TypeTBAAInfo);
    BaseInfo.mergeForCast(TypeBaseInfo);
    TBAAInfo = CGF.CGM.mergeTBAAInfoForCast(TBAAInfo, TypeTBAAInfo);
    return Address(CGF.Builder.CreateLoad(BaseLVal.getAddress()), Align);
  }
  return CGF.EmitPointerWithAlignment(Base, &BaseInfo, &TBAAInfo);
}

LValue CodeGenFunction::EmitOMPArraySectionExpr(const OMPArraySectionExpr *E,
                                                bool IsLowerBound) {
  QualType BaseTy = OMPArraySectionExpr::getBaseOriginalType(E->getBase());
  QualType ResultExprTy;
  if (auto *AT = getContext().getAsArrayType(BaseTy))
    ResultExprTy = AT->getElementType();
  else
    ResultExprTy = BaseTy->getPointeeType();
  llvm::Value *Idx = nullptr;
  if (IsLowerBound || E->getColonLoc().isInvalid()) {
    // Requesting lower bound or upper bound, but without provided length and
    // without ':' symbol for the default length -> length = 1.
    // Idx = LowerBound ?: 0;
    if (auto *LowerBound = E->getLowerBound()) {
      Idx = Builder.CreateIntCast(
          EmitScalarExpr(LowerBound), IntPtrTy,
          LowerBound->getType()->hasSignedIntegerRepresentation());
    } else
      Idx = llvm::ConstantInt::getNullValue(IntPtrTy);
  } else {
    // Try to emit length or lower bound as constant. If this is possible, 1
    // is subtracted from constant length or lower bound. Otherwise, emit LLVM
    // IR (LB + Len) - 1.
    auto &C = CGM.getContext();
    auto *Length = E->getLength();
    llvm::APSInt ConstLength;
    if (Length) {
      // Idx = LowerBound + Length - 1;
      if (Length->isIntegerConstantExpr(ConstLength, C)) {
        ConstLength = ConstLength.zextOrTrunc(PointerWidthInBits);
        Length = nullptr;
      }
      auto *LowerBound = E->getLowerBound();
      llvm::APSInt ConstLowerBound(PointerWidthInBits, /*isUnsigned=*/false);
      if (LowerBound && LowerBound->isIntegerConstantExpr(ConstLowerBound, C)) {
        ConstLowerBound = ConstLowerBound.zextOrTrunc(PointerWidthInBits);
        LowerBound = nullptr;
      }
      if (!Length)
        --ConstLength;
      else if (!LowerBound)
        --ConstLowerBound;

      if (Length || LowerBound) {
        auto *LowerBoundVal =
            LowerBound
                ? Builder.CreateIntCast(
                      EmitScalarExpr(LowerBound), IntPtrTy,
                      LowerBound->getType()->hasSignedIntegerRepresentation())
                : llvm::ConstantInt::get(IntPtrTy, ConstLowerBound);
        auto *LengthVal =
            Length
                ? Builder.CreateIntCast(
                      EmitScalarExpr(Length), IntPtrTy,
                      Length->getType()->hasSignedIntegerRepresentation())
                : llvm::ConstantInt::get(IntPtrTy, ConstLength);
        Idx = Builder.CreateAdd(LowerBoundVal, LengthVal, "lb_add_len",
                                /*HasNUW=*/false,
                                !getLangOpts().isSignedOverflowDefined());
        if (Length && LowerBound) {
          Idx = Builder.CreateSub(
              Idx, llvm::ConstantInt::get(IntPtrTy, /*V=*/1), "idx_sub_1",
              /*HasNUW=*/false, !getLangOpts().isSignedOverflowDefined());
        }
      } else
        Idx = llvm::ConstantInt::get(IntPtrTy, ConstLength + ConstLowerBound);
    } else {
      // Idx = ArraySize - 1;
      QualType ArrayTy = BaseTy->isPointerType()
                             ? E->getBase()->IgnoreParenImpCasts()->getType()
                             : BaseTy;
      if (auto *VAT = C.getAsVariableArrayType(ArrayTy)) {
        Length = VAT->getSizeExpr();
        if (Length->isIntegerConstantExpr(ConstLength, C))
          Length = nullptr;
      } else {
        auto *CAT = C.getAsConstantArrayType(ArrayTy);
        ConstLength = CAT->getSize();
      }
      if (Length) {
        auto *LengthVal = Builder.CreateIntCast(
            EmitScalarExpr(Length), IntPtrTy,
            Length->getType()->hasSignedIntegerRepresentation());
        Idx = Builder.CreateSub(
            LengthVal, llvm::ConstantInt::get(IntPtrTy, /*V=*/1), "len_sub_1",
            /*HasNUW=*/false, !getLangOpts().isSignedOverflowDefined());
      } else {
        ConstLength = ConstLength.zextOrTrunc(PointerWidthInBits);
        --ConstLength;
        Idx = llvm::ConstantInt::get(IntPtrTy, ConstLength);
      }
    }
  }
  assert(Idx);

  Address EltPtr = Address::invalid();
  LValueBaseInfo BaseInfo;
  TBAAAccessInfo TBAAInfo;
  if (auto *VLA = getContext().getAsVariableArrayType(ResultExprTy)) {
    // The base must be a pointer, which is not an aggregate.  Emit
    // it.  It needs to be emitted first in case it's what captures
    // the VLA bounds.
    Address Base =
        emitOMPArraySectionBase(*this, E->getBase(), BaseInfo, TBAAInfo,
                                BaseTy, VLA->getElementType(), IsLowerBound);
    // The element count here is the total number of non-VLA elements.
    llvm::Value *NumElements = getVLASize(VLA).NumElts;

    // Effectively, the multiply by the VLA size is part of the GEP.
    // GEP indexes are signed, and scaling an index isn't permitted to
    // signed-overflow, so we use the same semantics for our explicit
    // multiply.  We suppress this if overflow is not undefined behavior.
    if (getLangOpts().isSignedOverflowDefined())
      Idx = Builder.CreateMul(Idx, NumElements);
    else
      Idx = Builder.CreateNSWMul(Idx, NumElements);
    EltPtr = emitArraySubscriptGEP(*this, Base, Idx, VLA->getElementType(),
                                   !getLangOpts().isSignedOverflowDefined(),
                                   /*SignedIndices=*/false, E);
  } else if (const Expr *Array = isSimpleArrayDecayOperand(E->getBase())) {
    // If this is A[i] where A is an array, the frontend will have decayed the
    // base to be a ArrayToPointerDecay implicit cast.  While correct, it is
    // inefficient at -O0 to emit a "gep A, 0, 0" when codegen'ing it, then a
    // "gep x, i" here.  Emit one "gep A, 0, i".
    assert(Array->getType()->isArrayType() &&
           "Array to pointer decay must have array source type!");
    LValue ArrayLV;
    // For simple multidimensional array indexing, set the 'accessed' flag for
    // better bounds-checking of the base expression.
    if (const auto *ASE = dyn_cast<ArraySubscriptExpr>(Array))
      ArrayLV = EmitArraySubscriptExpr(ASE, /*Accessed*/ true);
    else
      ArrayLV = EmitLValue(Array);

    // Propagate the alignment from the array itself to the result.
    EltPtr = emitArraySubscriptGEP(
        *this, ArrayLV.getAddress(), {CGM.getSize(CharUnits::Zero()), Idx},
        ResultExprTy, !getLangOpts().isSignedOverflowDefined(),
        /*SignedIndices=*/false, E);
    BaseInfo = ArrayLV.getBaseInfo();
    TBAAInfo = CGM.getTBAAInfoForSubobject(ArrayLV, ResultExprTy);
  } else {
    Address Base = emitOMPArraySectionBase(*this, E->getBase(), BaseInfo,
                                           TBAAInfo, BaseTy, ResultExprTy,
                                           IsLowerBound);
    EltPtr = emitArraySubscriptGEP(*this, Base, Idx, ResultExprTy,
                                   !getLangOpts().isSignedOverflowDefined(),
                                   /*SignedIndices=*/false, E);
  }

  return MakeAddrLValue(EltPtr, ResultExprTy, BaseInfo, TBAAInfo);
}

LValue CodeGenFunction::
EmitExtVectorElementExpr(const ExtVectorElementExpr *E) {
  // Emit the base vector as an l-value.
  LValue Base;

  // ExtVectorElementExpr's base can either be a vector or pointer to vector.
  if (E->isArrow()) {
    // If it is a pointer to a vector, emit the address and form an lvalue with
    // it.
    LValueBaseInfo BaseInfo;
    TBAAAccessInfo TBAAInfo;
    Address Ptr = EmitPointerWithAlignment(E->getBase(), &BaseInfo, &TBAAInfo);
    const PointerType *PT = E->getBase()->getType()->getAs<PointerType>();
    Base = MakeAddrLValue(Ptr, PT->getPointeeType(), BaseInfo, TBAAInfo);
    Base.getQuals().removeObjCGCAttr();
  } else if (E->getBase()->isGLValue()) {
    // Otherwise, if the base is an lvalue ( as in the case of foo.x.x),
    // emit the base as an lvalue.
    assert(E->getBase()->getType()->isVectorType());
    Base = EmitLValue(E->getBase());
  } else {
    // Otherwise, the base is a normal rvalue (as in (V+V).x), emit it as such.
    assert(E->getBase()->getType()->isVectorType() &&
           "Result must be a vector");
    llvm::Value *Vec = EmitScalarExpr(E->getBase());

    // Store the vector to memory (because LValue wants an address).
    Address VecMem = CreateMemTemp(E->getBase()->getType());
    Builder.CreateStore(Vec, VecMem);
    Base = MakeAddrLValue(VecMem, E->getBase()->getType(),
                          AlignmentSource::Decl);
  }

  QualType type =
    E->getType().withCVRQualifiers(Base.getQuals().getCVRQualifiers());

  // Encode the element access list into a vector of unsigned indices.
  SmallVector<uint32_t, 4> Indices;
  E->getEncodedElementAccess(Indices);

  if (Base.isSimple()) {
    llvm::Constant *CV =
        llvm::ConstantDataVector::get(getLLVMContext(), Indices);
    return LValue::MakeExtVectorElt(Base.getAddress(), CV, type,
                                    Base.getBaseInfo(), TBAAAccessInfo());
  }
  assert(Base.isExtVectorElt() && "Can only subscript lvalue vec elts here!");

  llvm::Constant *BaseElts = Base.getExtVectorElts();
  SmallVector<llvm::Constant *, 4> CElts;

  for (unsigned i = 0, e = Indices.size(); i != e; ++i)
    CElts.push_back(BaseElts->getAggregateElement(Indices[i]));
  llvm::Constant *CV = llvm::ConstantVector::get(CElts);
  return LValue::MakeExtVectorElt(Base.getExtVectorAddress(), CV, type,
                                  Base.getBaseInfo(), TBAAAccessInfo());
}

LValue CodeGenFunction::EmitMemberExpr(const MemberExpr *E) {
  if (DeclRefExpr *DRE = tryToConvertMemberExprToDeclRefExpr(*this, E)) {
    EmitIgnoredExpr(E->getBase());
    return EmitDeclRefLValue(DRE);
  }

  Expr *BaseExpr = E->getBase();
  // If this is s.x, emit s as an lvalue.  If it is s->x, emit s as a scalar.
  LValue BaseLV;
  if (E->isArrow()) {
    LValueBaseInfo BaseInfo;
    TBAAAccessInfo TBAAInfo;
    Address Addr = EmitPointerWithAlignment(BaseExpr, &BaseInfo, &TBAAInfo);
    QualType PtrTy = BaseExpr->getType()->getPointeeType();
    SanitizerSet SkippedChecks;
    bool IsBaseCXXThis = IsWrappedCXXThis(BaseExpr);
    if (IsBaseCXXThis)
      SkippedChecks.set(SanitizerKind::Alignment, true);
    if (IsBaseCXXThis || isa<DeclRefExpr>(BaseExpr))
      SkippedChecks.set(SanitizerKind::Null, true);
    EmitTypeCheck(TCK_MemberAccess, E->getExprLoc(), Addr.getPointer(), PtrTy,
                  /*Alignment=*/CharUnits::Zero(), SkippedChecks);
    BaseLV = MakeAddrLValue(Addr, PtrTy, BaseInfo, TBAAInfo);
  } else
    BaseLV = EmitCheckedLValue(BaseExpr, TCK_MemberAccess);

  NamedDecl *ND = E->getMemberDecl();
  if (auto *Field = dyn_cast<FieldDecl>(ND)) {
    LValue LV = EmitLValueForField(BaseLV, Field);
    setObjCGCLValueClass(getContext(), E, LV);
    return LV;
  }

  if (const auto *FD = dyn_cast<FunctionDecl>(ND))
    return EmitFunctionDeclLValue(*this, E, FD);

  llvm_unreachable("Unhandled member declaration!");
}

/// Given that we are currently emitting a lambda, emit an l-value for
/// one of its members.
LValue CodeGenFunction::EmitLValueForLambdaField(const FieldDecl *Field) {
  assert(cast<CXXMethodDecl>(CurCodeDecl)->getParent()->isLambda());
  assert(cast<CXXMethodDecl>(CurCodeDecl)->getParent() == Field->getParent());
  QualType LambdaTagType =
    getContext().getTagDeclType(Field->getParent());
  LValue LambdaLV = MakeNaturalAlignAddrLValue(CXXABIThisValue, LambdaTagType);
  return EmitLValueForField(LambdaLV, Field);
}

/// Drill down to the storage of a field without walking into
/// reference types.
///
/// The resulting address doesn't necessarily have the right type.
static Address emitAddrOfFieldStorage(CodeGenFunction &CGF, Address base,
                                      const FieldDecl *field) {
  const RecordDecl *rec = field->getParent();

  unsigned idx =
    CGF.CGM.getTypes().getCGRecordLayout(rec).getLLVMFieldNo(field);

  return CGF.Builder.CreateStructGEP(base, idx, field->getName());
}

static bool hasAnyVptr(const QualType Type, const ASTContext &Context) {
  const auto *RD = Type.getTypePtr()->getAsCXXRecordDecl();
  if (!RD)
    return false;

  if (RD->isDynamicClass())
    return true;

  for (const auto &Base : RD->bases())
    if (hasAnyVptr(Base.getType(), Context))
      return true;

  for (const FieldDecl *Field : RD->fields())
    if (hasAnyVptr(Field->getType(), Context))
      return true;

  return false;
}

LValue CodeGenFunction::EmitLValueForField(LValue base,
                                           const FieldDecl *field) {
  LValueBaseInfo BaseInfo = base.getBaseInfo();

  if (field->isBitField()) {
    const CGRecordLayout &RL =
      CGM.getTypes().getCGRecordLayout(field->getParent());
    const CGBitFieldInfo &Info = RL.getBitFieldInfo(field);
    Address Addr = base.getAddress();
    unsigned Idx = RL.getLLVMFieldNo(field);
    if (Idx != 0)
      // For structs, we GEP to the field that the record layout suggests.
      Addr = Builder.CreateStructGEP(Addr, Idx, field->getName());
    // Get the access type.
    llvm::Type *FieldIntTy =
      llvm::Type::getIntNTy(getLLVMContext(), Info.StorageSize);
    if (Addr.getElementType() != FieldIntTy)
      Addr = Builder.CreateElementBitCast(Addr, FieldIntTy);

    QualType fieldType =
      field->getType().withCVRQualifiers(base.getVRQualifiers());
    // TODO: Support TBAA for bit fields.
    LValueBaseInfo FieldBaseInfo(BaseInfo.getAlignmentSource());
    return LValue::MakeBitfield(Addr, Info, fieldType, FieldBaseInfo,
                                TBAAAccessInfo());
  }

  // Fields of may-alias structures are may-alias themselves.
  // FIXME: this should get propagated down through anonymous structs
  // and unions.
  QualType FieldType = field->getType();
  const RecordDecl *rec = field->getParent();
  AlignmentSource BaseAlignSource = BaseInfo.getAlignmentSource();
  LValueBaseInfo FieldBaseInfo(getFieldAlignmentSource(BaseAlignSource));
  TBAAAccessInfo FieldTBAAInfo;
  if (base.getTBAAInfo().isMayAlias() ||
          rec->hasAttr<MayAliasAttr>() || FieldType->isVectorType()) {
    FieldTBAAInfo = TBAAAccessInfo::getMayAliasInfo();
  } else if (rec->isUnion()) {
    // TODO: Support TBAA for unions.
    FieldTBAAInfo = TBAAAccessInfo::getMayAliasInfo();
  } else {
    // If no base type been assigned for the base access, then try to generate
    // one for this base lvalue.
    FieldTBAAInfo = base.getTBAAInfo();
    if (!FieldTBAAInfo.BaseType) {
        FieldTBAAInfo.BaseType = CGM.getTBAABaseTypeInfo(base.getType());
        assert(!FieldTBAAInfo.Offset &&
               "Nonzero offset for an access with no base type!");
    }

    // Adjust offset to be relative to the base type.
    const ASTRecordLayout &Layout =
        getContext().getASTRecordLayout(field->getParent());
    unsigned CharWidth = getContext().getCharWidth();
    if (FieldTBAAInfo.BaseType)
      FieldTBAAInfo.Offset +=
          Layout.getFieldOffset(field->getFieldIndex()) / CharWidth;

    // Update the final access type and size.
    FieldTBAAInfo.AccessType = CGM.getTBAATypeInfo(FieldType);
    FieldTBAAInfo.Size =
        getContext().getTypeSizeInChars(FieldType).getQuantity();
  }

  Address addr = base.getAddress();
  if (auto *ClassDef = dyn_cast<CXXRecordDecl>(rec)) {
    if (CGM.getCodeGenOpts().StrictVTablePointers &&
        ClassDef->isDynamicClass()) {
      // Getting to any field of dynamic object requires stripping dynamic
      // information provided by invariant.group.  This is because accessing
      // fields may leak the real address of dynamic object, which could result
      // in miscompilation when leaked pointer would be compared.
      auto *stripped = Builder.CreateStripInvariantGroup(addr.getPointer());
      addr = Address(stripped, addr.getAlignment());
    }
  }

  unsigned RecordCVR = base.getVRQualifiers();
  if (rec->isUnion()) {
    // For unions, there is no pointer adjustment.
    assert(!FieldType->isReferenceType() && "union has reference member");
    if (CGM.getCodeGenOpts().StrictVTablePointers &&
        hasAnyVptr(FieldType, getContext()))
      // Because unions can easily skip invariant.barriers, we need to add
      // a barrier every time CXXRecord field with vptr is referenced.
      addr = Address(Builder.CreateLaunderInvariantGroup(addr.getPointer()),
                     addr.getAlignment());
  } else {
    // For structs, we GEP to the field that the record layout suggests.
    addr = emitAddrOfFieldStorage(*this, addr, field);

    // If this is a reference field, load the reference right now.
    if (FieldType->isReferenceType()) {
      LValue RefLVal = MakeAddrLValue(addr, FieldType, FieldBaseInfo,
                                      FieldTBAAInfo);
      if (RecordCVR & Qualifiers::Volatile)
        RefLVal.getQuals().addVolatile();
      addr = EmitLoadOfReference(RefLVal, &FieldBaseInfo, &FieldTBAAInfo);

      // Qualifiers on the struct don't apply to the referencee.
      RecordCVR = 0;
      FieldType = FieldType->getPointeeType();
    }
  }

  // Make sure that the address is pointing to the right type.  This is critical
  // for both unions and structs.  A union needs a bitcast, a struct element
  // will need a bitcast if the LLVM type laid out doesn't match the desired
  // type.
  addr = Builder.CreateElementBitCast(
      addr, CGM.getTypes().ConvertTypeForMem(FieldType), field->getName());

  if (field->hasAttr<AnnotateAttr>())
    addr = EmitFieldAnnotations(field, addr);

  LValue LV = MakeAddrLValue(addr, FieldType, FieldBaseInfo, FieldTBAAInfo);
  LV.getQuals().addCVRQualifiers(RecordCVR);

  // __weak attribute on a field is ignored.
  if (LV.getQuals().getObjCGCAttr() == Qualifiers::Weak)
    LV.getQuals().removeObjCGCAttr();

  return LV;
}

LValue
CodeGenFunction::EmitLValueForFieldInitialization(LValue Base,
                                                  const FieldDecl *Field) {
  QualType FieldType = Field->getType();

  if (!FieldType->isReferenceType())
    return EmitLValueForField(Base, Field);

  Address V = emitAddrOfFieldStorage(*this, Base.getAddress(), Field);

  // Make sure that the address is pointing to the right type.
  llvm::Type *llvmType = ConvertTypeForMem(FieldType);
  V = Builder.CreateElementBitCast(V, llvmType, Field->getName());

  // TODO: Generate TBAA information that describes this access as a structure
  // member access and not just an access to an object of the field's type. This
  // should be similar to what we do in EmitLValueForField().
  LValueBaseInfo BaseInfo = Base.getBaseInfo();
  AlignmentSource FieldAlignSource = BaseInfo.getAlignmentSource();
  LValueBaseInfo FieldBaseInfo(getFieldAlignmentSource(FieldAlignSource));
  return MakeAddrLValue(V, FieldType, FieldBaseInfo,
                        CGM.getTBAAInfoForSubobject(Base, FieldType));
}

LValue CodeGenFunction::EmitCompoundLiteralLValue(const CompoundLiteralExpr *E){
  if (E->isFileScope()) {
    ConstantAddress GlobalPtr = CGM.GetAddrOfConstantCompoundLiteral(E);
    return MakeAddrLValue(GlobalPtr, E->getType(), AlignmentSource::Decl);
  }
  if (E->getType()->isVariablyModifiedType())
    // make sure to emit the VLA size.
    EmitVariablyModifiedType(E->getType());

  Address DeclPtr = CreateMemTemp(E->getType(), ".compoundliteral");
  const Expr *InitExpr = E->getInitializer();
  LValue Result = MakeAddrLValue(DeclPtr, E->getType(), AlignmentSource::Decl);

  EmitAnyExprToMem(InitExpr, DeclPtr, E->getType().getQualifiers(),
                   /*Init*/ true);

  return Result;
}

LValue CodeGenFunction::EmitInitListLValue(const InitListExpr *E) {
  if (!E->isGLValue())
    // Initializing an aggregate temporary in C++11: T{...}.
    return EmitAggExprToLValue(E);

  // An lvalue initializer list must be initializing a reference.
  assert(E->isTransparent() && "non-transparent glvalue init list");
  return EmitLValue(E->getInit(0));
}

/// Emit the operand of a glvalue conditional operator. This is either a glvalue
/// or a (possibly-parenthesized) throw-expression. If this is a throw, no
/// LValue is returned and the current block has been terminated.
static Optional<LValue> EmitLValueOrThrowExpression(CodeGenFunction &CGF,
                                                    const Expr *Operand) {
  if (auto *ThrowExpr = dyn_cast<CXXThrowExpr>(Operand->IgnoreParens())) {
    CGF.EmitCXXThrowExpr(ThrowExpr, /*KeepInsertionPoint*/false);
    return None;
  }

  return CGF.EmitLValue(Operand);
}

LValue CodeGenFunction::
EmitConditionalOperatorLValue(const AbstractConditionalOperator *expr) {
  if (!expr->isGLValue()) {
    // ?: here should be an aggregate.
    assert(hasAggregateEvaluationKind(expr->getType()) &&
           "Unexpected conditional operator!");
    return EmitAggExprToLValue(expr);
  }

  OpaqueValueMapping binding(*this, expr);

  const Expr *condExpr = expr->getCond();
  bool CondExprBool;
  if (ConstantFoldsToSimpleInteger(condExpr, CondExprBool)) {
    const Expr *live = expr->getTrueExpr(), *dead = expr->getFalseExpr();
    if (!CondExprBool) std::swap(live, dead);

    if (!ContainsLabel(dead)) {
      // If the true case is live, we need to track its region.
      if (CondExprBool)
        incrementProfileCounter(expr);
      return EmitLValue(live);
    }
  }

  llvm::BasicBlock *lhsBlock = createBasicBlock("cond.true");
  llvm::BasicBlock *rhsBlock = createBasicBlock("cond.false");
  llvm::BasicBlock *contBlock = createBasicBlock("cond.end");

  ConditionalEvaluation eval(*this);
  EmitBranchOnBoolExpr(condExpr, lhsBlock, rhsBlock, getProfileCount(expr));

  // Any temporaries created here are conditional.
  EmitBlock(lhsBlock);
  incrementProfileCounter(expr);
  eval.begin(*this);
  Optional<LValue> lhs =
      EmitLValueOrThrowExpression(*this, expr->getTrueExpr());
  eval.end(*this);

  if (lhs && !lhs->isSimple())
    return EmitUnsupportedLValue(expr, "conditional operator");

  lhsBlock = Builder.GetInsertBlock();
  if (lhs)
    Builder.CreateBr(contBlock);

  // Any temporaries created here are conditional.
  EmitBlock(rhsBlock);
  eval.begin(*this);
  Optional<LValue> rhs =
      EmitLValueOrThrowExpression(*this, expr->getFalseExpr());
  eval.end(*this);
  if (rhs && !rhs->isSimple())
    return EmitUnsupportedLValue(expr, "conditional operator");
  rhsBlock = Builder.GetInsertBlock();

  EmitBlock(contBlock);

  if (lhs && rhs) {
    llvm::PHINode *phi = Builder.CreatePHI(lhs->getPointer()->getType(),
                                           2, "cond-lvalue");
    phi->addIncoming(lhs->getPointer(), lhsBlock);
    phi->addIncoming(rhs->getPointer(), rhsBlock);
    Address result(phi, std::min(lhs->getAlignment(), rhs->getAlignment()));
    AlignmentSource alignSource =
      std::max(lhs->getBaseInfo().getAlignmentSource(),
               rhs->getBaseInfo().getAlignmentSource());
    TBAAAccessInfo TBAAInfo = CGM.mergeTBAAInfoForConditionalOperator(
        lhs->getTBAAInfo(), rhs->getTBAAInfo());
    return MakeAddrLValue(result, expr->getType(), LValueBaseInfo(alignSource),
                          TBAAInfo);
  } else {
    assert((lhs || rhs) &&
           "both operands of glvalue conditional are throw-expressions?");
    return lhs ? *lhs : *rhs;
  }
}

/// EmitCastLValue - Casts are never lvalues unless that cast is to a reference
/// type. If the cast is to a reference, we can have the usual lvalue result,
/// otherwise if a cast is needed by the code generator in an lvalue context,
/// then it must mean that we need the address of an aggregate in order to
/// access one of its members.  This can happen for all the reasons that casts
/// are permitted with aggregate result, including noop aggregate casts, and
/// cast from scalar to union.
LValue CodeGenFunction::EmitCastLValue(const CastExpr *E) {
  switch (E->getCastKind()) {
  case CK_ToVoid:
  case CK_BitCast:
  case CK_ArrayToPointerDecay:
  case CK_FunctionToPointerDecay:
  case CK_NullToMemberPointer:
  case CK_NullToPointer:
  case CK_IntegralToPointer:
  case CK_PointerToIntegral:
  case CK_PointerToBoolean:
  case CK_VectorSplat:
  case CK_IntegralCast:
  case CK_BooleanToSignedIntegral:
  case CK_IntegralToBoolean:
  case CK_IntegralToFloating:
  case CK_FloatingToIntegral:
  case CK_FloatingToBoolean:
  case CK_FloatingCast:
  case CK_FloatingRealToComplex:
  case CK_FloatingComplexToReal:
  case CK_FloatingComplexToBoolean:
  case CK_FloatingComplexCast:
  case CK_FloatingComplexToIntegralComplex:
  case CK_IntegralRealToComplex:
  case CK_IntegralComplexToReal:
  case CK_IntegralComplexToBoolean:
  case CK_IntegralComplexCast:
  case CK_IntegralComplexToFloatingComplex:
  case CK_DerivedToBaseMemberPointer:
  case CK_BaseToDerivedMemberPointer:
  case CK_MemberPointerToBoolean:
  case CK_ReinterpretMemberPointer:
  case CK_AnyPointerToBlockPointerCast:
  case CK_ARCProduceObject:
  case CK_ARCConsumeObject:
  case CK_ARCReclaimReturnedObject:
  case CK_ARCExtendBlockObject:
  case CK_CopyAndAutoreleaseBlockObject:
  case CK_CHERICapabilityToPointer:
  case CK_PointerToCHERICapability:
  case CK_CHERICapabilityToOffset:
  case CK_CHERICapabilityToAddress:
  case CK_IntToOCLSampler:
  case CK_FixedPointCast:
  case CK_FixedPointToBoolean:
  case CK_FixedPointToIntegral:
  case CK_IntegralToFixedPoint:
    return EmitUnsupportedLValue(E, "unexpected cast lvalue");

  case CK_Dependent:
    llvm_unreachable("dependent cast kind in IR gen!");

  case CK_BuiltinFnToFnPtr:
    llvm_unreachable("builtin functions are handled elsewhere");

  // These are never l-values; just use the aggregate emission code.
  case CK_NonAtomicToAtomic:
  case CK_AtomicToNonAtomic:
    return EmitAggExprToLValue(E);

  case CK_Dynamic: {
    LValue LV = EmitLValue(E->getSubExpr());
    Address V = LV.getAddress();
    const auto *DCE = cast<CXXDynamicCastExpr>(E);
    return MakeNaturalAlignAddrLValue(EmitDynamicCast(V, DCE), E->getType());
  }

  case CK_ConstructorConversion:
  case CK_UserDefinedConversion:
  case CK_CPointerToObjCPointerCast:
  case CK_BlockPointerToObjCPointerCast:
  case CK_NoOp:
  case CK_LValueToRValue:
    return EmitLValue(E->getSubExpr());

  case CK_UncheckedDerivedToBase:
  case CK_DerivedToBase: {
    const RecordType *DerivedClassTy =
      E->getSubExpr()->getType()->getAs<RecordType>();
    auto *DerivedClassDecl = cast<CXXRecordDecl>(DerivedClassTy->getDecl());

    LValue LV = EmitLValue(E->getSubExpr());
    Address This = LV.getAddress();

    // Perform the derived-to-base conversion
    Address Base = GetAddressOfBaseClass(
        This, DerivedClassDecl, E->path_begin(), E->path_end(),
        /*NullCheckValue=*/false, E->getExprLoc());

    // TODO: Support accesses to members of base classes in TBAA. For now, we
    // conservatively pretend that the complete object is of the base class
    // type.
    return MakeAddrLValue(Base, E->getType(), LV.getBaseInfo(),
                          CGM.getTBAAInfoForSubobject(LV, E->getType()));
  }
  case CK_ToUnion:
    return EmitAggExprToLValue(E);
  case CK_BaseToDerived: {
    const RecordType *DerivedClassTy = E->getType()->getAs<RecordType>();
    auto *DerivedClassDecl = cast<CXXRecordDecl>(DerivedClassTy->getDecl());

    LValue LV = EmitLValue(E->getSubExpr());

    // Perform the base-to-derived conversion
    Address Derived =
      GetAddressOfDerivedClass(LV.getAddress(), DerivedClassDecl,
                               E->path_begin(), E->path_end(),
                               /*NullCheckValue=*/false);

    // C++11 [expr.static.cast]p2: Behavior is undefined if a downcast is
    // performed and the object is not of the derived type.
    if (sanitizePerformTypeCheck())
      EmitTypeCheck(TCK_DowncastReference, E->getExprLoc(),
                    Derived.getPointer(), E->getType());

    if (SanOpts.has(SanitizerKind::CFIDerivedCast))
      EmitVTablePtrCheckForCast(E->getType(), Derived.getPointer(),
                                /*MayBeNull=*/false, CFITCK_DerivedCast,
                                E->getBeginLoc());

    return MakeAddrLValue(Derived, E->getType(), LV.getBaseInfo(),
                          CGM.getTBAAInfoForSubobject(LV, E->getType()));
  }
  case CK_LValueBitCast: {
    // This must be a reinterpret_cast (or c-style equivalent).
    const auto *CE = cast<ExplicitCastExpr>(E);

    CGM.EmitExplicitCastExprType(CE, this);
    LValue LV = EmitLValue(E->getSubExpr());
    Address V = Builder.CreateBitCast(LV.getAddress(),
                                      ConvertType(CE->getTypeAsWritten()));

    if (SanOpts.has(SanitizerKind::CFIUnrelatedCast))
      EmitVTablePtrCheckForCast(E->getType(), V.getPointer(),
                                /*MayBeNull=*/false, CFITCK_UnrelatedCast,
                                E->getBeginLoc());

    return MakeAddrLValue(V, E->getType(), LV.getBaseInfo(),
                          CGM.getTBAAInfoForSubobject(LV, E->getType()));
  }
  case CK_AddressSpaceConversion: {
    LValue LV = EmitLValue(E->getSubExpr());
    QualType DestTy = getContext().getPointerType(E->getType());
    llvm::Value *V = getTargetHooks().performAddrSpaceCast(
        *this, LV.getPointer(), E->getSubExpr()->getType().getAddressSpace(),
        E->getType().getAddressSpace(), ConvertType(DestTy));
    return MakeAddrLValue(Address(V, LV.getAddress().getAlignment()),
                          E->getType(), LV.getBaseInfo(), LV.getTBAAInfo());
  }
  case CK_ObjCObjectLValueCast: {
    LValue LV = EmitLValue(E->getSubExpr());
    Address V = Builder.CreateElementBitCast(LV.getAddress(),
                                             ConvertType(E->getType()));
    return MakeAddrLValue(V, E->getType(), LV.getBaseInfo(),
                          CGM.getTBAAInfoForSubobject(LV, E->getType()));
  }
  case CK_ZeroToOCLOpaqueType:
    llvm_unreachable("NULL to OpenCL opaque type lvalue cast is not valid");
  }

  llvm_unreachable("Unhandled lvalue cast kind?");
}

LValue CodeGenFunction::EmitOpaqueValueLValue(const OpaqueValueExpr *e) {
  assert(OpaqueValueMappingData::shouldBindAsLValue(e));
  return getOrCreateOpaqueLValueMapping(e);
}

LValue
CodeGenFunction::getOrCreateOpaqueLValueMapping(const OpaqueValueExpr *e) {
  assert(OpaqueValueMapping::shouldBindAsLValue(e));

  llvm::DenseMap<const OpaqueValueExpr*,LValue>::iterator
      it = OpaqueLValues.find(e);

  if (it != OpaqueLValues.end())
    return it->second;

  assert(e->isUnique() && "LValue for a nonunique OVE hasn't been emitted");
  return EmitLValue(e->getSourceExpr());
}

RValue
CodeGenFunction::getOrCreateOpaqueRValueMapping(const OpaqueValueExpr *e) {
  assert(!OpaqueValueMapping::shouldBindAsLValue(e));

  llvm::DenseMap<const OpaqueValueExpr*,RValue>::iterator
      it = OpaqueRValues.find(e);

  if (it != OpaqueRValues.end())
    return it->second;

  assert(e->isUnique() && "RValue for a nonunique OVE hasn't been emitted");
  return EmitAnyExpr(e->getSourceExpr());
}

RValue CodeGenFunction::EmitRValueForField(LValue LV,
                                           const FieldDecl *FD,
                                           SourceLocation Loc) {
  QualType FT = FD->getType();
  LValue FieldLV = EmitLValueForField(LV, FD);
  switch (getEvaluationKind(FT)) {
  case TEK_Complex:
    return RValue::getComplex(EmitLoadOfComplex(FieldLV, Loc));
  case TEK_Aggregate:
    return FieldLV.asAggregateRValue();
  case TEK_Scalar:
    // This routine is used to load fields one-by-one to perform a copy, so
    // don't load reference fields.
    if (FD->getType()->isReferenceType())
      return RValue::get(FieldLV.getPointer(),
                         FieldLV.getAlignment().getQuantity());
    return EmitLoadOfLValue(FieldLV, Loc);
  }
  llvm_unreachable("bad evaluation kind");
}

//===--------------------------------------------------------------------===//
//                             Expression Emission
//===--------------------------------------------------------------------===//

RValue CodeGenFunction::EmitCallExpr(const CallExpr *E,
                                     ReturnValueSlot ReturnValue) {
  // Builtins never have block type.
  if (E->getCallee()->getType()->isBlockPointerType())
    return EmitBlockCallExpr(E, ReturnValue);

  if (const auto *CE = dyn_cast<CXXMemberCallExpr>(E))
    return EmitCXXMemberCallExpr(CE, ReturnValue);

  if (const auto *CE = dyn_cast<CUDAKernelCallExpr>(E))
    return EmitCUDAKernelCallExpr(CE, ReturnValue);

  if (const auto *CE = dyn_cast<CXXOperatorCallExpr>(E))
    if (const CXXMethodDecl *MD =
          dyn_cast_or_null<CXXMethodDecl>(CE->getCalleeDecl()))
      return EmitCXXOperatorMemberCallExpr(CE, MD, ReturnValue);

  CGCallee callee = EmitCallee(E->getCallee());

  if (callee.isBuiltin()) {
    return EmitBuiltinExpr(callee.getBuiltinDecl(), callee.getBuiltinID(),
                           E, ReturnValue);
  }

  if (callee.isPseudoDestructor()) {
    return EmitCXXPseudoDestructorExpr(callee.getPseudoDestructorExpr());
  }

  return EmitCall(E->getCallee()->getType(), callee, E, ReturnValue);
}

/// Emit a CallExpr without considering whether it might be a subclass.
RValue CodeGenFunction::EmitSimpleCallExpr(const CallExpr *E,
                                           ReturnValueSlot ReturnValue) {
  CGCallee Callee = EmitCallee(E->getCallee());
  return EmitCall(E->getCallee()->getType(), Callee, E, ReturnValue);
}

static CGCallee EmitDirectCallee(CodeGenFunction &CGF, const FunctionDecl *FD) {
  if (auto builtinID = FD->getBuiltinID()) {
    return CGCallee::forBuiltin(builtinID, FD);
  }

  llvm::Value *calleePtr = EmitFunctionDeclPointer(CGF, FD, /*IsDirectCall=*/true);
  return CGCallee(GlobalDecl(FD), calleePtr);
}

CGCallee CodeGenFunction::EmitCallee(const Expr *E) {
  E = E->IgnoreParens();

  // Look through function-to-pointer decay.
  if (auto ICE = dyn_cast<ImplicitCastExpr>(E)) {
    if (ICE->getCastKind() == CK_FunctionToPointerDecay ||
        ICE->getCastKind() == CK_BuiltinFnToFnPtr) {
      return EmitCallee(ICE->getSubExpr());
    }

  // Resolve direct calls.
  } else if (auto DRE = dyn_cast<DeclRefExpr>(E)) {
    if (auto FD = dyn_cast<FunctionDecl>(DRE->getDecl())) {
      return EmitDirectCallee(*this, FD);
    }
  } else if (auto ME = dyn_cast<MemberExpr>(E)) {
    if (auto FD = dyn_cast<FunctionDecl>(ME->getMemberDecl())) {
      EmitIgnoredExpr(ME->getBase());
      return EmitDirectCallee(*this, FD);
    }

  // Look through template substitutions.
  } else if (auto NTTP = dyn_cast<SubstNonTypeTemplateParmExpr>(E)) {
    return EmitCallee(NTTP->getReplacement());

  // Treat pseudo-destructor calls differently.
  } else if (auto PDE = dyn_cast<CXXPseudoDestructorExpr>(E)) {
    return CGCallee::forPseudoDestructor(PDE);
  }

  // Otherwise, we have an indirect reference.
  llvm::Value *calleePtr;
  QualType functionType;
  if (auto ptrType = E->getType()->getAs<PointerType>()) {
    calleePtr = EmitScalarExpr(E);
    functionType = ptrType->getPointeeType();
  } else {
    functionType = E->getType();
    calleePtr = EmitLValue(E).getPointer();
  }
  assert(functionType->isFunctionType());

  GlobalDecl GD;
  if (const auto *VD =
          dyn_cast_or_null<VarDecl>(E->getReferencedDeclOfCallee()))
    GD = GlobalDecl(VD);

  CGCalleeInfo calleeInfo(functionType->getAs<FunctionProtoType>(), GD);
  CGCallee callee(calleeInfo, calleePtr);
  return callee;
}

LValue CodeGenFunction::EmitBinaryOperatorLValue(const BinaryOperator *E) {
  // Comma expressions just emit their LHS then their RHS as an l-value.
  if (E->getOpcode() == BO_Comma) {
    EmitIgnoredExpr(E->getLHS());
    EnsureInsertPoint();
    return EmitLValue(E->getRHS());
  }

  if (E->getOpcode() == BO_PtrMemD ||
      E->getOpcode() == BO_PtrMemI)
    return EmitPointerToDataMemberBinaryExpr(E);

  assert(E->getOpcode() == BO_Assign && "unexpected binary l-value");

  // Note that in all of these cases, __block variables need the RHS
  // evaluated first just in case the variable gets moved by the RHS.

  switch (getEvaluationKind(E->getType())) {
  case TEK_Scalar: {
    switch (E->getLHS()->getType().getObjCLifetime()) {
    case Qualifiers::OCL_Strong:
      return EmitARCStoreStrong(E, /*ignored*/ false).first;

    case Qualifiers::OCL_Autoreleasing:
      return EmitARCStoreAutoreleasing(E).first;

    // No reason to do any of these differently.
    case Qualifiers::OCL_None:
    case Qualifiers::OCL_ExplicitNone:
    case Qualifiers::OCL_Weak:
      break;
    }

    RValue RV = EmitAnyExpr(E->getRHS());
    LValue LV = EmitCheckedLValue(E->getLHS(), TCK_Store);
    if (RV.isScalar())
      EmitNullabilityCheck(LV, RV.getScalarVal(), E->getExprLoc());
    EmitStoreThroughLValue(RV, LV);
    return LV;
  }

  case TEK_Complex:
    return EmitComplexAssignmentLValue(E);

  case TEK_Aggregate:
    return EmitAggExprToLValue(E);
  }
  llvm_unreachable("bad evaluation kind");
}

LValue CodeGenFunction::EmitCallExprLValue(const CallExpr *E) {
  RValue RV = EmitCallExpr(E);

  if (!RV.isScalar())
    return MakeAddrLValue(RV.getAggregateAddress(), E->getType(),
                          AlignmentSource::Decl);

  assert(E->getCallReturnType(getContext())->isReferenceType() &&
         "Can't have a scalar return unless the return type is a "
         "reference type!");

  return MakeNaturalAlignPointeeAddrLValue(RV.getScalarVal(), E->getType());
}

LValue CodeGenFunction::EmitVAArgExprLValue(const VAArgExpr *E) {
  // FIXME: This shouldn't require another copy.
  return EmitAggExprToLValue(E);
}

LValue CodeGenFunction::EmitCXXConstructLValue(const CXXConstructExpr *E) {
  assert(E->getType()->getAsCXXRecordDecl()->hasTrivialDestructor()
         && "binding l-value to type which needs a temporary");
  AggValueSlot Slot = CreateAggTemp(E->getType());
  EmitCXXConstructExpr(E, Slot);
  return MakeAddrLValue(Slot.getAddress(), E->getType(), AlignmentSource::Decl);
}

LValue
CodeGenFunction::EmitCXXTypeidLValue(const CXXTypeidExpr *E) {
  return MakeNaturalAlignAddrLValue(EmitCXXTypeidExpr(E), E->getType());
}

Address CodeGenFunction::EmitCXXUuidofExpr(const CXXUuidofExpr *E) {
  return Builder.CreateElementBitCast(CGM.GetAddrOfUuidDescriptor(E),
                                      ConvertType(E->getType()));
}

LValue CodeGenFunction::EmitCXXUuidofLValue(const CXXUuidofExpr *E) {
  return MakeAddrLValue(EmitCXXUuidofExpr(E), E->getType(),
                        AlignmentSource::Decl);
}

LValue
CodeGenFunction::EmitCXXBindTemporaryLValue(const CXXBindTemporaryExpr *E) {
  AggValueSlot Slot = CreateAggTemp(E->getType(), "temp.lvalue");
  Slot.setExternallyDestructed();
  EmitAggExpr(E->getSubExpr(), Slot);
  EmitCXXTemporary(E->getTemporary(), E->getType(), Slot.getAddress());
  return MakeAddrLValue(Slot.getAddress(), E->getType(), AlignmentSource::Decl);
}

LValue CodeGenFunction::EmitObjCMessageExprLValue(const ObjCMessageExpr *E) {
  RValue RV = EmitObjCMessageExpr(E);

  if (!RV.isScalar())
    return MakeAddrLValue(RV.getAggregateAddress(), E->getType(),
                          AlignmentSource::Decl);

  assert(E->getMethodDecl()->getReturnType()->isReferenceType() &&
         "Can't have a scalar return unless the return type is a "
         "reference type!");

  return MakeNaturalAlignPointeeAddrLValue(RV.getScalarVal(), E->getType());
}

LValue CodeGenFunction::EmitObjCSelectorLValue(const ObjCSelectorExpr *E) {
  Address V =
    CGM.getObjCRuntime().GetAddrOfSelector(*this, E->getSelector());
  return MakeAddrLValue(V, E->getType(), AlignmentSource::Decl);
}

llvm::Value *CodeGenFunction::EmitIvarOffset(const ObjCInterfaceDecl *Interface,
                                             const ObjCIvarDecl *Ivar) {
  return CGM.getObjCRuntime().EmitIvarOffset(*this, Interface, Ivar);
}

LValue CodeGenFunction::EmitLValueForIvar(QualType ObjectTy,
                                          llvm::Value *BaseValue,
                                          const ObjCIvarDecl *Ivar,
                                          unsigned CVRQualifiers) {
  return CGM.getObjCRuntime().EmitObjCValueForIvar(*this, ObjectTy, BaseValue,
                                                   Ivar, CVRQualifiers);
}

LValue CodeGenFunction::EmitObjCIvarRefLValue(const ObjCIvarRefExpr *E) {
  // FIXME: A lot of the code below could be shared with EmitMemberExpr.
  llvm::Value *BaseValue = nullptr;
  const Expr *BaseExpr = E->getBase();
  Qualifiers BaseQuals;
  QualType ObjectTy;
  if (E->isArrow()) {
    BaseValue = EmitScalarExpr(BaseExpr);
    ObjectTy = BaseExpr->getType()->getPointeeType();
    BaseQuals = ObjectTy.getQualifiers();
  } else {
    LValue BaseLV = EmitLValue(BaseExpr);
    BaseValue = BaseLV.getPointer();
    ObjectTy = BaseExpr->getType();
    BaseQuals = ObjectTy.getQualifiers();
  }

  LValue LV =
    EmitLValueForIvar(ObjectTy, BaseValue, E->getDecl(),
                      BaseQuals.getCVRQualifiers());
  setObjCGCLValueClass(getContext(), E, LV);
  return LV;
}

LValue CodeGenFunction::EmitStmtExprLValue(const StmtExpr *E) {
  // Can only get l-value for message expression returning aggregate type
  RValue RV = EmitAnyExprToTemp(E);
  return MakeAddrLValue(RV.getAggregateAddress(), E->getType(),
                        AlignmentSource::Decl);
}

RValue CodeGenFunction::EmitCall(QualType CalleeType, const CGCallee &OrigCallee,
                                 const CallExpr *E, ReturnValueSlot ReturnValue,
                                 llvm::Value *Chain) {
  // Get the actual function type. The callee type will always be a pointer to
  // function type or a block pointer type.
  assert(CalleeType->isFunctionPointerType() &&
         "Call must have function pointer type!");

  const Decl *TargetDecl =
      OrigCallee.getAbstractInfo().getCalleeDecl().getDecl();

  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(TargetDecl))
    // We can only guarantee that a function is called from the correct
    // context/function based on the appropriate target attributes,
    // so only check in the case where we have both always_inline and target
    // since otherwise we could be making a conditional call after a check for
    // the proper cpu features (and it won't cause code generation issues due to
    // function based code generation).
    if (TargetDecl->hasAttr<AlwaysInlineAttr>() &&
        TargetDecl->hasAttr<TargetAttr>())
      checkTargetFeatures(E, FD);

  CalleeType = getContext().getCanonicalType(CalleeType);

  auto PointeeType = cast<PointerType>(CalleeType)->getPointeeType();

  CGCallee Callee = OrigCallee;

  if (getLangOpts().CPlusPlus && SanOpts.has(SanitizerKind::Function) &&
      (!TargetDecl || !isa<FunctionDecl>(TargetDecl))) {
    if (llvm::Constant *PrefixSig =
            CGM.getTargetCodeGenInfo().getUBSanFunctionSignature(CGM)) {
      SanitizerScope SanScope(this);
      // Remove any (C++17) exception specifications, to allow calling e.g. a
      // noexcept function through a non-noexcept pointer.
      auto ProtoTy =
        getContext().getFunctionTypeWithExceptionSpec(PointeeType, EST_None);
      llvm::Constant *FTRTTIConst =
          CGM.GetAddrOfRTTIDescriptor(ProtoTy, /*ForEH=*/true);
      llvm::Type *PrefixStructTyElems[] = {PrefixSig->getType(), Int32Ty};
      llvm::StructType *PrefixStructTy = llvm::StructType::get(
          CGM.getLLVMContext(), PrefixStructTyElems, /*isPacked=*/true);

      llvm::Value *CalleePtr = Callee.getFunctionPointer();

      llvm::Value *CalleePrefixStruct = Builder.CreateBitCast(
          CalleePtr, CGM.getPointerInDefaultAS(PrefixStructTy));
      llvm::Value *CalleeSigPtr =
          Builder.CreateConstGEP2_32(PrefixStructTy, CalleePrefixStruct, 0, 0);
      llvm::Value *CalleeSig =
          Builder.CreateAlignedLoad(CalleeSigPtr, getIntAlign());
      llvm::Value *CalleeSigMatch = Builder.CreateICmpEQ(CalleeSig, PrefixSig);

      llvm::BasicBlock *Cont = createBasicBlock("cont");
      llvm::BasicBlock *TypeCheck = createBasicBlock("typecheck");
      Builder.CreateCondBr(CalleeSigMatch, TypeCheck, Cont);

      EmitBlock(TypeCheck);
      llvm::Value *CalleeRTTIPtr =
          Builder.CreateConstGEP2_32(PrefixStructTy, CalleePrefixStruct, 0, 1);
      llvm::Value *CalleeRTTIEncoded =
          Builder.CreateAlignedLoad(CalleeRTTIPtr, getPointerAlign());
      llvm::Value *CalleeRTTI =
          DecodeAddrUsedInPrologue(CalleePtr, CalleeRTTIEncoded);
      llvm::Value *CalleeRTTIMatch =
          Builder.CreateICmpEQ(CalleeRTTI, FTRTTIConst);
      llvm::Constant *StaticData[] = {EmitCheckSourceLocation(E->getBeginLoc()),
                                      EmitCheckTypeDescriptor(CalleeType)};
      EmitCheck(std::make_pair(CalleeRTTIMatch, SanitizerKind::Function),
                SanitizerHandler::FunctionTypeMismatch, StaticData,
                {CalleePtr, CalleeRTTI, FTRTTIConst});

      Builder.CreateBr(Cont);
      EmitBlock(Cont);
    }
  }

  const auto *FnType = cast<FunctionType>(PointeeType);

  // If we are checking indirect calls and this call is indirect, check that the
  // function pointer is a member of the bit set for the function type.
  if (SanOpts.has(SanitizerKind::CFIICall) &&
      (!TargetDecl || !isa<FunctionDecl>(TargetDecl))) {
    SanitizerScope SanScope(this);
    EmitSanitizerStatReport(llvm::SanStat_CFI_ICall);

    llvm::Metadata *MD;
    if (CGM.getCodeGenOpts().SanitizeCfiICallGeneralizePointers)
      MD = CGM.CreateMetadataIdentifierGeneralized(QualType(FnType, 0));
    else
      MD = CGM.CreateMetadataIdentifierForType(QualType(FnType, 0));

    llvm::Value *TypeId = llvm::MetadataAsValue::get(getLLVMContext(), MD);

    llvm::Value *CalleePtr = Callee.getFunctionPointer();
    llvm::Value *CastedCallee = Builder.CreateBitCast(CalleePtr, Int8PtrTy);
    llvm::Value *TypeTest = Builder.CreateCall(
        CGM.getIntrinsic(llvm::Intrinsic::type_test), {CastedCallee, TypeId});

    auto CrossDsoTypeId = CGM.CreateCrossDsoCfiTypeId(MD);
    llvm::Constant *StaticData[] = {
        llvm::ConstantInt::get(Int8Ty, CFITCK_ICall),
        EmitCheckSourceLocation(E->getBeginLoc()),
        EmitCheckTypeDescriptor(QualType(FnType, 0)),
    };
    if (CGM.getCodeGenOpts().SanitizeCfiCrossDso && CrossDsoTypeId) {
      EmitCfiSlowPathCheck(SanitizerKind::CFIICall, TypeTest, CrossDsoTypeId,
                           CastedCallee, StaticData);
    } else {
      EmitCheck(std::make_pair(TypeTest, SanitizerKind::CFIICall),
                SanitizerHandler::CFICheckFail, StaticData,
                {CastedCallee, llvm::UndefValue::get(IntPtrTy)});
    }
  }

  CallArgList Args;
  if (Chain)
    Args.add(RValue::get(Builder.CreateBitCast(Chain, CGM.VoidPtrTy)),
             CGM.getContext().VoidPtrTy);

  // C++17 requires that we evaluate arguments to a call using assignment syntax
  // right-to-left, and that we evaluate arguments to certain other operators
  // left-to-right. Note that we allow this to override the order dictated by
  // the calling convention on the MS ABI, which means that parameter
  // destruction order is not necessarily reverse construction order.
  // FIXME: Revisit this based on C++ committee response to unimplementability.
  EvaluationOrder Order = EvaluationOrder::Default;
  if (auto *OCE = dyn_cast<CXXOperatorCallExpr>(E)) {
    if (OCE->isAssignmentOp())
      Order = EvaluationOrder::ForceRightToLeft;
    else {
      switch (OCE->getOperator()) {
      case OO_LessLess:
      case OO_GreaterGreater:
      case OO_AmpAmp:
      case OO_PipePipe:
      case OO_Comma:
      case OO_ArrowStar:
        Order = EvaluationOrder::ForceLeftToRight;
        break;
      default:
        break;
      }
    }
  }

  EmitCallArgs(Args, dyn_cast<FunctionProtoType>(FnType), E->arguments(),
               E->getDirectCallee(), /*ParamsToSkip*/ 0, Order);

  bool CallCHERIInvoke = false;
  // For CHERI callbacks, the function 'pointer' is actually a struct
  // containing all of the information required for a cross domain call.
  // Note: It doesn't actually matter what the order of the number and class
  // are, as they will be in a different category of register.  This is *not*
  // necessarily the case for implementations that have a merged register file!
  if (FnType->getCallConv() == CC_CHERICCallback) {
    CallCHERIInvoke = true;
    SmallVector<QualType, 16> NewParams;
    // Add the method number
    auto *MethodNum = Builder.CreateExtractValue(Callee.getFunctionPointer(), {1});
    auto NumTy = getContext().UnsignedLongLongTy;
    CallArg MethodNumArg(RValue::get(MethodNum), NumTy);
    NewParams.push_back(NumTy);
    Args.insert(Args.begin(), MethodNumArg);
    // Add the CHERI object
    auto *Obj = Builder.CreateExtractValue(Callee.getFunctionPointer(), {0});
    auto ObjTy = getContext().getCHERIClassType();
    CallArg ObjArg(RValue::get(Obj), ObjTy);
    NewParams.push_back(ObjTy);
    Args.insert(Args.begin(), ObjArg);

    auto *FnPType = cast<FunctionProtoType>(FnType);
    auto Params = FnPType->getParamTypes();
    FunctionProtoType::ExtProtoInfo EPI = FnPType->getExtProtoInfo();
    NewParams.insert(NewParams.end(), Params.begin(), Params.end());
    FnType = getContext().getFunctionType(FnPType->getReturnType(),
        NewParams, EPI)->getAs<FunctionType>();
  } else if (FnType->getCallConv() == CC_CHERICCall &&
             TargetDecl->hasAttr<CHERIMethodClassAttr>()) {
    assert(TargetDecl);
    StringRef Suffix = TargetDecl->hasAttr<CHERIMethodSuffixAttr>() ?
      TargetDecl->getAttr<CHERIMethodSuffixAttr>()->getSuffix() : "";
    CallCHERIInvoke = true;
    SmallVector<QualType, 16> NewParams;
    auto NumTy = getContext().UnsignedLongLongTy;
    auto *ClsAttr = TargetDecl->getAttr<CHERIMethodClassAttr>();
    StringRef FunctionBaseName = cast<NamedDecl>(TargetDecl)->getName();
    FunctionBaseName = FunctionBaseName.substr(0, FunctionBaseName.size() -
        Suffix.size());
    auto *MethodNumVar =
        CGM.EmitSandboxRequiredMethod(ClsAttr->getDefaultClass()->getName(),
                                      FunctionBaseName);
    // Load the global and use it in the call
    // FIXME: EmitSandboxRequiredMethod should return an Address so that we
    // don't have to know the alignment here.
    auto *MethodNum = Builder.CreateLoad(Address(MethodNumVar, CharUnits::fromQuantity(8)));
    MethodNum->setMetadata(CGM.getModule().getMDKindID("invariant.load"),
        llvm::MDNode::get(getLLVMContext(), None));
    CallArg MethodNumArg(RValue::get(MethodNum), NumTy);
    NewParams.push_back(NumTy);
    // If we have a non-empty suffix, then we're not the version of the method
    // that takes an explicit class, we're the version with the same explicit
    // parameters and extra implicit ones for the default class, so add the
    // default class.
    if (Suffix.size() == 0) {
      // FIXME: We should silently insert a common-linkage global if the class
      // isn't declared.
      DeclarationName DN(ClsAttr->getDefaultClass());
      auto *TU = CGM.getContext().getTranslationUnitDecl();
      auto Cls = cast<VarDecl>(TU->lookup(DN)[0]);
      llvm::Value *V = CGM.GetAddrOfGlobalVar(Cls);
      auto ClsTy = Cls->getType();
      llvm::Type *RealVarTy = getTypes().ConvertTypeForMem(ClsTy);
      V = EmitBitCastOfLValueToProperType(*this, V, RealVarTy);
      CharUnits Alignment = getContext().getDeclAlign(Cls);
      LValue LV = MakeAddrLValue(V, ClsTy, Alignment);
      Address A1 = Builder.CreateStructGEP(LV.getAddress(), 0, "arg1");
      Address A2 = Builder.CreateStructGEP(LV.getAddress(), 1, "arg2");
      auto Fields = ClsTy->getAs<RecordType>()->getDecl()->fields();
      auto FieldsIt = Fields.begin();
      assert(std::distance(Fields.begin(), Fields.end()) == 2);
      QualType Ty1 = FieldsIt->getType();
      QualType Ty2 = (++FieldsIt)->getType();
      CallArg Arg1(RValue::get(Builder.CreateLoad(A1)), Ty1);
      CallArg Arg2(RValue::get(Builder.CreateLoad(A2)), Ty2);
      Args.insert(Args.begin(), MethodNumArg);
      Args.insert(Args.begin(), Arg2);
      Args.insert(Args.begin(), Arg1);
      NewParams.push_back(ClsTy);
    } else {
      Args.insert(Args.begin()+1, MethodNumArg);
    }
    auto *FnPType = cast<FunctionProtoType>(FnType);
    auto Params = FnPType->getParamTypes();
    FunctionProtoType::ExtProtoInfo EPI = FnPType->getExtProtoInfo();
    NewParams.insert(NewParams.end(), Params.begin(), Params.end());
    FnType = getContext().getFunctionType(FnPType->getReturnType(),
        NewParams, EPI)->getAs<FunctionType>();
  }
  const CGFunctionInfo &FnInfo = CGM.getTypes().arrangeFreeFunctionCall(
      Args, FnType, /*isChainCall=*/Chain);

  if (CallCHERIInvoke)
    Callee.setFunctionPointer(CGM.getModule().getOrInsertFunction("cheri_invoke",
        getTypes().GetFunctionType(FnInfo)).getCallee());

  // C99 6.5.2.2p6:
  //   If the expression that denotes the called function has a type
  //   that does not include a prototype, [the default argument
  //   promotions are performed]. If the number of arguments does not
  //   equal the number of parameters, the behavior is undefined. If
  //   the function is defined with a type that includes a prototype,
  //   and either the prototype ends with an ellipsis (, ...) or the
  //   types of the arguments after promotion are not compatible with
  //   the types of the parameters, the behavior is undefined. If the
  //   function is defined with a type that does not include a
  //   prototype, and the types of the arguments after promotion are
  //   not compatible with those of the parameters after promotion,
  //   the behavior is undefined [except in some trivial cases].
  // That is, in the general case, we should assume that a call
  // through an unprototyped function type works like a *non-variadic*
  // call.  The way we make this work is to cast to the exact type
  // of the promoted arguments.
  //
  // Chain calls use this same code path to add the invisible chain parameter
  // to the function type.
  if (isa<FunctionNoProtoType>(FnType) || Chain) {
    llvm::Type *CalleeTy = getTypes().GetFunctionType(FnInfo);

    llvm::Value *CalleePtr = Callee.getFunctionPointer();
    CalleeTy =
        CalleeTy->getPointerTo(CalleePtr->getType()->getPointerAddressSpace());
    CalleePtr = Builder.CreateBitCast(CalleePtr, CalleeTy, "callee.knr.cast");
    Callee.setFunctionPointer(CalleePtr);
  }

  auto ret = EmitCall(FnInfo, Callee, ReturnValue, Args, nullptr, E->getExprLoc());
  // If we're doing a CHERI ccall in C++ mode and have exceptions enabled, then
  // translate the CHERI errno value into an exception.
  if (CallCHERIInvoke && getLangOpts().CPlusPlus && getLangOpts().Exceptions &&
      TargetDecl && !TargetDecl->hasAttr<NoThrowAttr>()) {
    auto &M = CGM.getModule();
    auto *CHERIErrno = M.getNamedGlobal("cheri_errno");
    if (!CHERIErrno) {
      CHERIErrno = new llvm::GlobalVariable(M, IntTy,
          /*isConstant*/false, llvm::GlobalValue::ExternalLinkage,
          nullptr, "cheri_errno", nullptr, llvm::GlobalValue::NotThreadLocal,
          CGM.getTargetCodeGenInfo().getTlsAddressSpace());
      CHERIErrno->setThreadLocal(true);
    }
    // FIXME: Don't hard code 4-byte alignment for int!
    auto *ErrVal = Builder.CreateLoad(Address(CHERIErrno, CharUnits::fromQuantity(4)));
    auto *IsZero = Builder.CreateICmpEQ(ErrVal,
        llvm::Constant::getNullValue(ErrVal->getType()));
    auto *Continue = createBasicBlock("cheri_invoke_continue");
    auto *Error = createBasicBlock("cheri_invoke_fail");
    Builder.CreateCondBr(IsZero, Continue, Error);
    llvm::FunctionType *FTy = llvm::FunctionType::get(VoidTy, IntTy, false);
    llvm::AttrBuilder B;
    B.addAttribute(llvm::Attribute::NoReturn);
    llvm::FunctionCallee ErrorFn =
        CGM.CreateRuntimeFunction(FTy, "__cxa_cheri_sandbox_invoke_failure",
          llvm::AttributeList::get(getLLVMContext(),
            llvm::AttributeList::FunctionIndex, B));
    EmitBlock(Error);
    Builder.CreateCall(ErrorFn, ErrVal);
    Builder.CreateUnreachable();
    EmitBlock(Continue);
  }
  return ret;
}

LValue CodeGenFunction::
EmitPointerToDataMemberBinaryExpr(const BinaryOperator *E) {
  Address BaseAddr = Address::invalid();
  if (E->getOpcode() == BO_PtrMemI) {
    BaseAddr = EmitPointerWithAlignment(E->getLHS());
  } else {
    BaseAddr = EmitLValue(E->getLHS()).getAddress();
  }

  llvm::Value *OffsetV = EmitScalarExpr(E->getRHS());

  const MemberPointerType *MPT
    = E->getRHS()->getType()->getAs<MemberPointerType>();

  LValueBaseInfo BaseInfo;
  TBAAAccessInfo TBAAInfo;
  Address MemberAddr =
    EmitCXXMemberDataPointerAddress(E, BaseAddr, OffsetV, MPT, &BaseInfo,
                                    &TBAAInfo);

  return MakeAddrLValue(MemberAddr, MPT->getPointeeType(), BaseInfo, TBAAInfo);
}

/// Given the address of a temporary variable, produce an r-value of
/// its type.
RValue CodeGenFunction::convertTempToRValue(Address addr,
                                            QualType type,
                                            SourceLocation loc) {
  LValue lvalue = MakeAddrLValue(addr, type, AlignmentSource::Decl);
  switch (getEvaluationKind(type)) {
  case TEK_Complex:
    return RValue::getComplex(EmitLoadOfComplex(lvalue, loc));
  case TEK_Aggregate:
    return lvalue.asAggregateRValue();
  case TEK_Scalar:
    return RValue::get(EmitLoadOfScalar(lvalue, loc));
  }
  llvm_unreachable("bad evaluation kind");
}

void CodeGenFunction::SetFPAccuracy(llvm::Value *Val, float Accuracy) {
  assert(Val->getType()->isFPOrFPVectorTy());
  if (Accuracy == 0.0 || !isa<llvm::Instruction>(Val))
    return;

  llvm::MDBuilder MDHelper(getLLVMContext());
  llvm::MDNode *Node = MDHelper.createFPMath(Accuracy);

  cast<llvm::Instruction>(Val)->setMetadata(llvm::LLVMContext::MD_fpmath, Node);
}

namespace {
  struct LValueOrRValue {
    LValue LV;
    RValue RV;
  };
}

static LValueOrRValue emitPseudoObjectExpr(CodeGenFunction &CGF,
                                           const PseudoObjectExpr *E,
                                           bool forLValue,
                                           AggValueSlot slot) {
  SmallVector<CodeGenFunction::OpaqueValueMappingData, 4> opaques;

  // Find the result expression, if any.
  const Expr *resultExpr = E->getResultExpr();
  LValueOrRValue result;

  for (PseudoObjectExpr::const_semantics_iterator
         i = E->semantics_begin(), e = E->semantics_end(); i != e; ++i) {
    const Expr *semantic = *i;

    // If this semantic expression is an opaque value, bind it
    // to the result of its source expression.
    if (const auto *ov = dyn_cast<OpaqueValueExpr>(semantic)) {
      // Skip unique OVEs.
      if (ov->isUnique()) {
        assert(ov != resultExpr &&
               "A unique OVE cannot be used as the result expression");
        continue;
      }

      // If this is the result expression, we may need to evaluate
      // directly into the slot.
      typedef CodeGenFunction::OpaqueValueMappingData OVMA;
      OVMA opaqueData;
      if (ov == resultExpr && ov->isRValue() && !forLValue &&
          CodeGenFunction::hasAggregateEvaluationKind(ov->getType())) {
        CGF.EmitAggExpr(ov->getSourceExpr(), slot);
        LValue LV = CGF.MakeAddrLValue(slot.getAddress(), ov->getType(),
                                       AlignmentSource::Decl);
        opaqueData = OVMA::bind(CGF, ov, LV);
        result.RV = slot.asRValue();

      // Otherwise, emit as normal.
      } else {
        opaqueData = OVMA::bind(CGF, ov, ov->getSourceExpr());

        // If this is the result, also evaluate the result now.
        if (ov == resultExpr) {
          if (forLValue)
            result.LV = CGF.EmitLValue(ov);
          else
            result.RV = CGF.EmitAnyExpr(ov, slot);
        }
      }

      opaques.push_back(opaqueData);

    // Otherwise, if the expression is the result, evaluate it
    // and remember the result.
    } else if (semantic == resultExpr) {
      if (forLValue)
        result.LV = CGF.EmitLValue(semantic);
      else
        result.RV = CGF.EmitAnyExpr(semantic, slot);

    // Otherwise, evaluate the expression in an ignored context.
    } else {
      CGF.EmitIgnoredExpr(semantic);
    }
  }

  // Unbind all the opaques now.
  for (unsigned i = 0, e = opaques.size(); i != e; ++i)
    opaques[i].unbind(CGF);

  return result;
}

RValue CodeGenFunction::EmitPseudoObjectRValue(const PseudoObjectExpr *E,
                                               AggValueSlot slot) {
  return emitPseudoObjectExpr(*this, E, false, slot).RV;
}

LValue CodeGenFunction::EmitPseudoObjectLValue(const PseudoObjectExpr *E) {
  return emitPseudoObjectExpr(*this, E, true, AggValueSlot::ignored()).LV;
}
