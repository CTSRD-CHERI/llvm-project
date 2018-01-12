//===-- PureCapABICalls.cpp -----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the passes that transform PCC-relative calls into
// direct calls (so that they can be inlined) and then back again (so that we
// generate the correct code).
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/CHERICap.h"

using namespace llvm;
using namespace PatternMatch;

namespace {
class CHERICapFoldIntrinsics : public ModulePass {
  Function *IncOffset;
  Function *SetOffset;
  Function *GetOffset;
  Function *GetBase;
  Function *GetAddress;
  Function *GetLength;
  Function *GetType;
  Function *GetPerms;
  Function *GetTag;
  Function *GetSealed;

  Type* I8CapTy;
  Type* CapOffsetTy;
  const DataLayout *DL;
  bool Modified;

  template <typename Infer>
  void foldGet(Function *Intrinsic, Infer infer, int NullValue = 0) {
    // Calling eraseFromParent() inside the following loop causes iterators
    // to be invalidated and crashes -> collect and erase instead
    std::vector<CallInst *> ToErase;
    for (Value *Use : Intrinsic->users()) {
      CallInst *CI = cast<CallInst>(Use);
      if (Value *Replacement = infer(CI->getOperand(0), CI, NullValue)) {
        CI->replaceAllUsesWith(Replacement);
        // CI->eraseFromParent();
        ToErase.push_back(CI);
        Modified = true;
      }
    }
    for (CallInst *CI : ToErase) {
      CI->eraseFromParent();
    }
  }

  void foldGetIntrinisics() {
    foldGet(GetOffset, [this](Value *V, CallInst *CI, int) {
      return inferCapabilityOffset(V, CI, CI->getType());
    });
    foldGet(GetAddress, [this](Value *V, CallInst *CI, int) {
      return inferCapabilityAddress(V, CI);
    });
    // For all other capability fields we can only infer a value when the
    // argument is a null capability (potentially with an offset)
    auto inferOther = [this](Value *V, CallInst *CI, int NullValue) {
      return inferCapabilityNonOffsetField(V, CI, NullValue);
    };
    foldGet(GetBase, inferOther);
    foldGet(GetPerms, inferOther);
    foldGet(GetTag, inferOther);
    foldGet(GetSealed, inferOther);
    // CGetType and CGetLen on a null capability now return -1
    foldGet(GetLength, inferOther, -1);
    foldGet(GetType, inferOther, -1);
  }

  static Constant* getIntToPtrSourceValue(Value* V) {
    if (ConstantExpr* CE = dyn_cast<ConstantExpr>(V)) {
      if (CE->isCast() && CE->getOpcode() == Instruction::IntToPtr) {
        assert(CE->getNumOperands() == 1);
        Constant *Src = CE->getOperand(0);
        return Src;
      }
    }
    return nullptr;
  }

  Value *inferCapabilityNonOffsetField(Value *V, CallInst *Call,
                                       int NullValue) {
    // Calling an llv.cheri.cap.$INTRIN.get() on a null value or
    // and integer stored in capability always returns 0 or -1 (for CGetLen and
    // CGetType) unless $INTRIN is offset (in which case it returns the int value)
    if (isa<ConstantPointerNull>(V) || getIntToPtrSourceValue(V)) {
      Type *Ty = Call->getType();
      return NullValue == 0 ? llvm::Constant::getNullValue(Ty)
                            : llvm::Constant::getAllOnesValue(Ty);
    }
    Value *Arg = nullptr;
    // ignore all setoffset/incoffset operations:
    if (match(V, m_Intrinsic<Intrinsic::cheri_cap_offset_set>(m_Value(Arg)))) {
      return inferCapabilityNonOffsetField(Arg, Call, NullValue);
    } else if (match(V, m_Intrinsic<Intrinsic::cheri_cap_offset_increment>(
                            m_Value(Arg)))) {
      return inferCapabilityNonOffsetField(Arg, Call, NullValue);
    }
    // TODO: is there anything else we can infer?
    return nullptr;
  }

  // Returns the offset increment if V is a GEP instruction of
  Value *getOffsetIncrement(Value *V, Value **Arg) {
    if (!V)
      return nullptr;
    Value *Offset = nullptr;
    if (match(V, m_Intrinsic<Intrinsic::cheri_cap_offset_increment>(
                     m_Value(*Arg), m_Value(Offset)))) {
      return Offset;
    } else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V)) {
      *Arg = GEP->getOperand(0);

      // XXXAR: do I need the inbounds check here?
      if (GEP->isInBounds() && GEP->getType() == I8CapTy) {
        APInt Offset(CapOffsetTy->getIntegerBitWidth(), 0, true);
        if (GEP->accumulateConstantOffset(*DL, Offset)) {
          return ConstantInt::get(CapOffsetTy, Offset);
        } else if (GEP->getOperand(1)->getType() == CapOffsetTy) {
          // also handle non-constant GEPs:
          // XXXAR: not sure this is always profitable, sometimes a CIncOffset
          // might be is better
          return GEP->getOperand(1);
        }
      }
    }
    return nullptr;
  }

  Value *inferCapabilityOffset(Value *V, CallInst *Call, Type *IntTy,
                               Value **BaseCap = nullptr) {
    if (isa<ConstantPointerNull>(V)) {
      if (BaseCap)
        *BaseCap = V;
      return llvm::Constant::getNullValue(IntTy);
    }
    if (Constant* IntToPtr = getIntToPtrSourceValue(V)) {
      if (BaseCap)
        *BaseCap = V;
      return IntToPtr;
    }
    Value *Arg = nullptr;
    Value *Offset = nullptr;
    if (match(V, m_Intrinsic<Intrinsic::cheri_cap_offset_set>(
                     m_Value(Arg), m_Value(Offset)))) {
      if (BaseCap)
        *BaseCap = Arg;
      return Offset;
    } else if (Value *Increment = getOffsetIncrement(V, &Arg)) {
      if (Value *LHS = inferCapabilityOffset(Arg, Call, IntTy, BaseCap)) {
        IRBuilder<> B(cast<Instruction>(V));
        return B.CreateAdd(LHS, Increment);
      }
    }
    // TODO: is there anything else we can infer?
    return nullptr;
  }

  Value *inferCapabilityAddress(Value *V, CallInst *Call) {
    Type* IntTy = Call->getType();
    Value *Offset = inferCapabilityOffset(V, Call, IntTy);
    Value *Base = inferCapabilityNonOffsetField(V, Call, 0);
    if (Offset && Base) {
      IRBuilder<> B(Call);
      return B.CreateAdd(Base, Offset);
    }
    return nullptr;
  }

  void foldIncOffsetSetOffsetOnlyUserIncrement(CallInst* CI, SmallPtrSet<Instruction*, 8> &ToErase) {
    // errs() << __func__ << ": num users=" << CI->getNumUses() << ": "; CI->dump();
    User* OnlyUser = CI->hasOneUse() ? *CI->user_begin() : nullptr;
    while (OnlyUser) {
      // If there is only a single use of the setoffset result, we might be
      // able to fold it into a single setoffset (for a GEP or incoffset)
      Value* Arg = nullptr;
      if (Value *Increment = getOffsetIncrement(OnlyUser, &Arg)) {
#if 0
        errs() << "CI before folding: "; CI->dump();
        errs() << "Block before folding: "; CI->getParent()->dump();
        errs() << "OnlyUser: "; OnlyUser->dump();
        errs() << "Increment: "; Increment->dump();
#endif
        assert(Arg == CI);
        Instruction *ReplacedInstr = cast<Instruction>(OnlyUser);
        IRBuilder<> B(ReplacedInstr);
        Value *NewOffset = B.CreateAdd(CI->getOperand(1), Increment);
        CI->setOperand(1, NewOffset);
        // We have to move this instruction after the offset instruction
        // because otherwise we use a value that has not yet been defined
        CI->moveAfter(ReplacedInstr);
        // Keep doing this transformation for incoffset chains with only a
        // single user:
        OnlyUser = ReplacedInstr->hasOneUse() ? *ReplacedInstr->user_begin() : nullptr;
        ReplacedInstr->replaceAllUsesWith(CI);
#if 0
        errs() << "ReplacedInstr (" << ReplacedInstr->getNumUses() << " uses): "; ReplacedInstr->dump();
        errs() << "ReplacedInstr: "; ReplacedInstr->dump();
        errs() << "New OnlyUser: "; if (OnlyUser) OnlyUser->dump(); else errs() << "nullptr\n";
        errs() << "New CI (" << CI->getNumUses() << " uses): "; CI->dump();
        errs() << "New Block: "; CI->getParent()->dump();
#endif
        // erasing here can cause a crash -> add to list so that caller can remove it
        // ReplacedInstr->eraseFromParent();
        ToErase.insert(ReplacedInstr);
        assert(OnlyUser != ReplacedInstr && "Should not cause an infinite loop!");
        Modified = true;
        if (!OnlyUser)
          break;
      } else {
        break;
      }
    }
  }

  /// Replace get-offset, add, set-offset sequences with inc-offset
  void foldSetOffset() {
    std::vector<CallInst *> SetOffsets;
    SmallPtrSet<Instruction*, 8> ToErase;
    for (Value *V : SetOffset->users())
      SetOffsets.push_back(cast<CallInst>(V));
    for (CallInst *CI : SetOffsets) {
      if (ToErase.count(CI)) {
        assert(CI->hasNUses(0));
        continue;
      }
      // fold chains of set-offset, (inc-offset/GEP)+ into a single set-offset
      foldIncOffsetSetOffsetOnlyUserIncrement(CI, ToErase);

      Value *LHS, *RHS;
      if (match(CI->getOperand(1), m_Add(m_Value(LHS), m_Value(RHS)))) {
        Value *BaseCap = CI->getOperand(0);
        Value *Add = nullptr;
        if (match(LHS, m_Intrinsic<Intrinsic::cheri_cap_offset_get>(
                           m_Specific(BaseCap))))
          Add = RHS;
        else if (match(RHS, m_Intrinsic<Intrinsic::cheri_cap_offset_get>(
                                m_Specific(BaseCap))))
          Add = LHS;
        if (Add) {
          IRBuilder<> B(CI);
          CallInst *Replacement = B.CreateCall(IncOffset, {BaseCap, Add});
          Replacement->setTailCall(true);
          CI->replaceAllUsesWith(Replacement);
          Modified = true;
        }
      }
    }
    for (Instruction *I : ToErase)
      I->eraseFromParent();
  }

  /// Replace set-offset, inc-offset sequences with a single set-offset
  /// Also fold multiple inc-offsets into a single on if possible
  void foldIncOffset() {
    std::vector<CallInst *> IncOffsets;
    SmallPtrSet<Instruction*, 8> ToErase;
    for (Value *V : IncOffset->users())
      IncOffsets.push_back(cast<CallInst>(V));
    for (CallInst *CI : IncOffsets) {
      if (ToErase.count(CI)) {
        assert(CI->hasNUses(0));
        continue;
      }
      // fold chains of inc-offset, (inc-offset/GEP)+ into a single inc-offset
      foldIncOffsetSetOffsetOnlyUserIncrement(CI, ToErase);

      Value *Inc = CI->getOperand(1);
      Value *BaseCap = nullptr;
      // TODO: how to delete any dead instructions?

      // Also convert a incoffset on null to a setoffset on null
      if (Value *Offset =
              inferCapabilityOffset(CI->getOperand(0), CI, Inc->getType(), &BaseCap)) {
        assert(BaseCap);
        IRBuilder<> B(CI);
        CallInst *Replacement = B.CreateCall(
            SetOffset, {BaseCap, B.CreateAdd(Offset, Inc)});
        Replacement->setTailCall(true);
        CI->replaceAllUsesWith(Replacement);
        Modified = true;
      }
    }
    for (Instruction *I : ToErase)
      I->eraseFromParent();
  }

public:
  static char ID;
  CHERICapFoldIntrinsics() : ModulePass(ID) {}
  StringRef getPassName() const override {
    return "CHERI fold capability intrinsics";
  }
  bool runOnModule(Module &M) override {
    Modified = false;
    DL = &M.getDataLayout();
    IncOffset =
        Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_increment);
    SetOffset = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_set);
    GetOffset = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_get);
    GetBase = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_base_get);
    GetAddress =
        Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_address_get);
    GetLength = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_length_get);
    GetType = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_type_get);
    GetPerms = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_perms_get);
    GetSealed = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_sealed_get);
    GetTag = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_tag_get);
    I8CapTy = IncOffset->getReturnType();
    CapOffsetTy = GetOffset->getReturnType();

    // TODO: does the order here matter?
    foldIncOffset();
    foldSetOffset();
    foldGetIntrinisics();
    return Modified;
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }
};
} // namespace

char CHERICapFoldIntrinsics::ID = 0;
INITIALIZE_PASS(CHERICapFoldIntrinsics, "cheri-fold-intrisics",
                "Remove redundant capability instructions", false, false)

Pass *llvm::createCHERICapFoldIntrinsicsPass() {
  return new CHERICapFoldIntrinsics();
}
