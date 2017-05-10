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

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Transforms/MemCap.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace PatternMatch;

namespace {
class MemCapFoldIntrinsics : public ModulePass {
  Function *IncOffset;
  Function *SetOffset;
  Function *GetOffset;
  Function *GetBase;
  Function *GetLength;
  Function *GetType;
  Function *GetPerms;
  Function *GetTag;
  Function *GetSealed;

  bool Modified;

  void foldNullGet(std::initializer_list<Function*> Intrinsics) {
    // This is probably quite inefficient...
    for (Function* Intrin : Intrinsics) {
      for (Value *Use : Intrin->users()) {
        CallInst* CI = cast<CallInst>(Use);
        if (isa<ConstantPointerNull>(CI->getOperand(0))) {
          CI->replaceAllUsesWith(llvm::Constant::getNullValue(CI->getType()));
          CI->eraseFromParent();
          Modified = true;
        }
      }
    }
  }

  /// Replace get-offset, add, set-offset sequences with inc-offset
  void foldGetAddSetToInc() {
    std::vector<CallInst *> SetOffsets;
    for (Value *V : SetOffset->users())
      SetOffsets.push_back(cast<CallInst>(V));
    for (CallInst *CI : SetOffsets) {
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
          Value *Replacement = B.CreateCall(IncOffset, {BaseCap, Add});
          CI->replaceAllUsesWith(Replacement);
          Modified = true;
        }
      }
    }
  }
  /// Replace get-offset immediately following a set-offset with the offset
  void forwardGetFromSet() {
    std::vector<CallInst *> GetOffsets;
    for (Value *V : GetOffset->users())
      GetOffsets.push_back(cast<CallInst>(V));
    for (CallInst *CI : GetOffsets) {
      Value *Offset;
      Value *BaseCap = CI->getOperand(0);
      if (match(BaseCap, m_Intrinsic<Intrinsic::cheri_cap_offset_set>(
                             m_Value(), m_Value(Offset)))) {
        CI->replaceAllUsesWith(Offset);
        Modified = true;
      } else if (isa<ConstantPointerNull>(BaseCap)) {
        CI->replaceAllUsesWith(llvm::Constant::getNullValue(CI->getType()));
        Modified = true;
      }
    }
  }

public:
  static char ID;
  MemCapFoldIntrinsics() : ModulePass(ID) {}
  bool runOnModule(Module &M) override {
    Modified = false;
    IncOffset =
        Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_increment);
    SetOffset = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_set);
    GetOffset = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_get);
    GetBase = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_base_get);
    GetLength = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_length_get);
    GetType = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_type_get);
    GetPerms = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_perms_get);
    GetSealed = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_sealed_get);
    GetTag = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_tag_get);
    // XXXAR: revisit this in case GetType were to return -1 for invalid types
    foldNullGet({ GetOffset, GetBase, GetLength, GetType, GetPerms, GetSealed,
                  GetTag });
    foldGetAddSetToInc();
    forwardGetFromSet();
    return Modified;
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }
};
}

char MemCapFoldIntrinsics::ID = 0;
INITIALIZE_PASS(MemCapFoldIntrinsics, "cheri-fold-intrisics",
                "Remove redundant capability instructions", false, false)

Pass *llvm::createMemCapFoldIntrinsicsPass() {
  return new MemCapFoldIntrinsics();
}
