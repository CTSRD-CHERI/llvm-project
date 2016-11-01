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
#include "llvm/Transforms/MemCap.h"

using namespace llvm;

namespace {
class MemCapDirectCalls : public FunctionPass,
                          public InstVisitor<MemCapDirectCalls> {
  bool Modified = false;

public:
  static char ID;
  MemCapDirectCalls() : FunctionPass(ID) {}
  void visitCallSite(CallSite CS) {
    Value *Callee = CS.getCalledValue()->stripPointerCasts();
    if (Callee->getType()->getPointerAddressSpace() != 200)
      return;
    auto *SetOffset = dyn_cast<IntrinsicInst>(Callee);
    if (!SetOffset)
      return;
    if (SetOffset->getIntrinsicID() != llvm::Intrinsic::memcap_cap_offset_set)
      return;
    auto *PtrToInt = dyn_cast<PtrToIntOperator>(SetOffset->getOperand(1));
    if (!PtrToInt)
      return;
    auto *Func =
        dyn_cast<Function>(PtrToInt->getOperand(0)->stripPointerCasts());
    if (!Func)
      return;
    CS.setCalledFunction(Func);
    CS.mutateFunctionType(
        cast<FunctionType>(Func->getType()->getElementType()));
  }
  bool runOnFunction(Function &F) override {
    Modified = false;
    visit(F);
    return Modified;
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }
};
}

char MemCapDirectCalls::ID = 0;
INITIALIZE_PASS(MemCapDirectCalls, "memcap-direct-calls",
                "Eliminate PCC-relative calls", false, false)

Pass *llvm::createMemCapDirectCallsPass() { return new MemCapDirectCalls(); }
