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
#include "llvm/Analysis/Utils/Local.h"
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
#include "llvm/Pass.h"
#include "llvm/Transforms/CHERICap.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

namespace {
class CHERICapDirectCalls : public FunctionPass,
                          public InstVisitor<CHERICapDirectCalls> {
  bool Modified = false;
  SmallVector<Value*,32> DeadInstructions;
  StringRef getPassName() const override {
    return "Contract CHERI pure-capability calls";
  }
public:
  static char ID;
  CHERICapDirectCalls() : FunctionPass(ID) {}
  void visitCallSite(CallSite CS) {
    Value *Callee = CS.getCalledValue()->stripPointerCasts();
    if (Callee->getType()->getPointerAddressSpace() != 200)
      return;
    auto *SetOffset = dyn_cast<IntrinsicInst>(Callee);
    if (!SetOffset)
      return;
    if (SetOffset->getIntrinsicID() != llvm::Intrinsic::cheri_cap_offset_set)
      return;
    auto *PtrToInt = dyn_cast<PtrToIntOperator>(SetOffset->getOperand(1));
    if (!PtrToInt)
      return;
    auto *Func =
        dyn_cast<Function>(PtrToInt->getOperand(0)->stripPointerCasts());
    if (!Func)
      return;
    auto *CalledValue = CS.getCalledValue();
    // insert a bitcast if the type of CalledValue and Func are different
    if (CalledValue->getType() != Func->getType()) {
      if (auto *CalledValPtrTy = dyn_cast<PointerType>(CalledValue->getType())) {
        IRBuilder<> B(CS.getInstruction());
        // XXXAR: the legacy ABI expects all functions to be in AS0 even though program AS is 200!
        auto *NewCalledValue = B.CreateBitCast(Func, PointerType::get(CalledValPtrTy->getElementType(), 0));
        CS.setCalledFunction(NewCalledValue);
      }
    }
    else {
      CS.setCalledFunction(Func);
      CS.mutateFunctionType(cast<FunctionType>(Func->getType()->getElementType()));
    }
    if (CalledValue->use_begin() == CalledValue->use_end()) {
      DeadInstructions.push_back(CalledValue);
    } else {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      llvm::errs() << "CALLEE: "; CalledValue->dump();
      llvm::errs() << "Func: "; Func->dump();
      llvm::errs() << "NUM USES: " << CalledValue->getNumUses() << "\n";
#endif
      assert(CalledValue->getNumUses() == 0 && "Unexpected uses");
    }
  }
  bool runOnFunction(Function &F) override {
    Modified = false;
    DeadInstructions.clear();
    visit(F);
    for (auto *V : DeadInstructions)
      RecursivelyDeleteTriviallyDeadInstructions(V);
    return Modified;
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }
};
}

char CHERICapDirectCalls::ID = 0;
INITIALIZE_PASS(CHERICapDirectCalls, "cheri-direct-calls",
                "Eliminate PCC-relative calls", false, false)

Pass *llvm::createCHERICapDirectCallsPass() { return new CHERICapDirectCalls(); }
