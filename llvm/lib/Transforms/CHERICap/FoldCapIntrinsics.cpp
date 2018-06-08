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

  Type* I8CapTy;
  Type* CapOffsetTy;
  const DataLayout *DL;
  bool Modified;

  template <typename Infer>
  void foldGet(Module *M, Intrinsic::ID ID, Infer infer, int NullValue = 0) {
    Function *Func = M->getFunction(Intrinsic::getName(ID));
    if (!Func)
      return;
    // Calling eraseFromParent() inside the following loop causes iterators
    // to be invalidated and crashes -> collect and erase instead
    std::vector<CallInst *> ToErase;
    for (Value *Use : Func->users()) {
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

  void foldGetIntrinisics(Module *M) {
    foldGet(M, Intrinsic::cheri_cap_offset_get,
            [this](Value *V, CallInst *CI, int) {
              return inferCapabilityOffset(V, CI, CI->getType());
            });
    foldGet(M, Intrinsic::cheri_cap_address_get,
            [this](Value *V, CallInst *CI, int) {
              return inferCapabilityAddress(V, CI, CI->getType());
            });
    // For all other capability fields we can only infer a value when the
    // argument is a null capability (potentially with an offset)
    auto inferOther = [this](Value *V, CallInst *CI, int NullValue) {
      // TODO: Can we ignore capabilities becoming unrepresentable for any of
      // these intrinsics?
      return inferCapabilityNonOffsetField(V, CI, NullValue);
    };
    foldGet(M, Intrinsic::cheri_cap_base_get, inferOther);
    foldGet(M, Intrinsic::cheri_cap_perms_get, inferOther);
    foldGet(M, Intrinsic::cheri_cap_tag_get, inferOther);
    foldGet(M, Intrinsic::cheri_cap_sealed_get, inferOther);
    // CGetType and CGetLen on a null capability now return -1
    foldGet(M, Intrinsic::cheri_cap_length_get, inferOther, -1);
    foldGet(M, Intrinsic::cheri_cap_type_get, inferOther, -1);
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

  Value *inferCapabilityNonOffsetField(Value *V, CallInst *Call, int NullValue,
                                       bool OnlyIfRootWasNull = false) {
    // Calling an llvm.cheri.cap.$INTRIN.get() on a null value or
    // an integer stored in a capability always returns 0 or -1 (for CGetLen and
    // CGetType) unless $INTRIN is offset/address (in which case it returns the
    // int value)
    if (isa<ConstantPointerNull>(V) || getIntToPtrSourceValue(V)) {
      Type *Ty = Call->getType();
      return NullValue == 0 ? llvm::Constant::getNullValue(Ty)
                            : llvm::Constant::getAllOnesValue(Ty);
    }
    Value *Arg = nullptr;
    // We can ignore all setoffset/incoffset/setaddr operations if called on
    // null -> set OnlyIfRootWasNull=true when recursing. This currently doesn't
    // change anything but if we were to also infer for non-null values we
    // would need to abort any further analysis after a setoffset/incoffset or
    // setaddr since the intrinsic could cause the capability to become
    // unrepresentable (which is not an issue if it is already null)
    if (match(V, m_Intrinsic<Intrinsic::cheri_cap_offset_set>(m_Value(Arg)))) {
      return inferCapabilityNonOffsetField(Arg, Call, NullValue, true);
    } else if (match(V, m_Intrinsic<Intrinsic::cheri_cap_offset_increment>(
                            m_Value(Arg)))) {
      return inferCapabilityNonOffsetField(Arg, Call, NullValue, true);
    } else if (match(V, m_Intrinsic<Intrinsic::cheri_cap_address_set>(
                            m_Value(Arg)))) {
      return inferCapabilityNonOffsetField(Arg, Call, NullValue, true);
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

  template <Intrinsic::ID Intrin, typename InferFunc>
  Value *inferCapabilityOffsetOrAddr(InferFunc &&InferOffsetOrAddr,
                                            Value *V, CallInst *Call,
                                            Type *IntTy,
                                            Value **BaseCap = nullptr) {
    // For null values both getoffset and getaddr return zero
    if (isa<ConstantPointerNull>(V)) {
      if (BaseCap)
        *BaseCap = V;
      return llvm::Constant::getNullValue(IntTy);
    }
    // For inttoptr values both getoffset and getaddr return the integer value
    if (Constant* IntToPtr = getIntToPtrSourceValue(V)) {
      if (BaseCap)
        *BaseCap = V;
      return IntToPtr;
    }
    Value *Arg = nullptr;
    Value *IntrinArg = nullptr;
    // For a setoffset we can infer the offset (and the address for setoffset on
    // NULL)
    if (match(V, m_Intrinsic<Intrinsic::cheri_cap_offset_set>(
                     m_Value(Arg), m_Value(IntrinArg)))) {
      // If we are inferring the offset a setoffset can always be used. For
      // the address we can only infer the value if the setoffset call is on
      // NULL
      if (Intrin == Intrinsic::cheri_cap_offset_get ||
          isa<ConstantPointerNull>(Arg)) {
        if (BaseCap)
          *BaseCap = Arg;
        return IntrinArg;
      }
    }

    if (match(V, m_Intrinsic<Intrinsic::cheri_cap_address_set>(
                     m_Value(Arg), m_Value(IntrinArg)))) {
      // If we are inferring the offset a we can only infer the value of the
      // setaddr intrinsic was used on NULL. For getaddr the value will always
      // be the argumennt of the setaddr intrinsic.
      if (Intrin == Intrinsic::cheri_cap_address_get ||
          isa<ConstantPointerNull>(Arg)) {
        if (BaseCap)
          *BaseCap = Arg;
        return IntrinArg;
      }
    }
    // Finally we can fold inc-offset calls into the result as adds
    if (Value *Increment = getOffsetIncrement(V, &Arg)) {
      if (Value *LHS = (this->*InferOffsetOrAddr)(Arg, Call, IntTy, BaseCap)) {
        IRBuilder<> B(cast<Instruction>(V));
        return B.CreateAdd(LHS, Increment);
      }
    }
    // TODO: is there anything else we can infer?
    return nullptr;
  }

  Value *inferCapabilityOffset(Value *V, CallInst *Call, Type *IntTy,
                                      Value **BaseCap = nullptr) {
    return inferCapabilityOffsetOrAddr<Intrinsic::cheri_cap_offset_get>(
        &CHERICapFoldIntrinsics::inferCapabilityOffset, V, Call, IntTy, BaseCap);
  }

  Value *inferCapabilityAddress(Value *V, CallInst *Call, Type *IntTy,
                                       Value **BaseCap = nullptr) {
    return inferCapabilityOffsetOrAddr<Intrinsic::cheri_cap_address_get>(
        &CHERICapFoldIntrinsics::inferCapabilityAddress, V, Call, IntTy, BaseCap);
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
      // Also convert a setoffset on null to a incoffset on null since we have
      // an immediate version of incoffset but not setoffset
      // FIXME: this should be done in the MIPS backend instead...
      if (isa<ConstantPointerNull>(CI->getOperand(0))) {
        IRBuilder<> B(CI);
        CallInst *Replacement = B.CreateCall(
            IncOffset, {CI->getOperand(0), CI->getOperand(1)});
        // Replacement->setTailCall(true);
        CI->replaceAllUsesWith(Replacement);
        Modified = true;
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
    // Don't add these intrinsics to the module if none of them are used:
    IncOffset = M.getFunction(
        Intrinsic::getName(Intrinsic::cheri_cap_offset_increment));
    SetOffset =
        M.getFunction(Intrinsic::getName(Intrinsic::cheri_cap_offset_set));
    GetOffset =
        M.getFunction(Intrinsic::getName(Intrinsic::cheri_cap_offset_get));
    // at least one intrinsic was used -> we need to run the fold
    // setoffset/incoffset pass
    if (IncOffset || SetOffset || GetOffset) {
      if (!IncOffset)
        IncOffset = Intrinsic::getDeclaration(
            &M, Intrinsic::cheri_cap_offset_increment);
      if (!SetOffset)
        SetOffset =
            Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_set);
      if (!GetOffset)
        GetOffset =
            Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_get);
      // Ensure that all the intrinsics exist in the module
      I8CapTy = IncOffset->getReturnType();
      CapOffsetTy = GetOffset->getReturnType();
      // TODO: does the order here matter?
      foldIncOffset();
      foldSetOffset();
    }
    foldGetIntrinisics(&M);
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
