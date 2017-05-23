#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/InstVisitor.h"

#include <string>

#ifndef NDEBUG
#include "llvm/IR/Verifier.h"
#endif

using namespace llvm;

/// Expand the @llvm.cheri.cap.address.get() instrinsic to base + offset
namespace {
class CheriExpandIntrinsicsPass : public ModulePass {
  Function* GetBase;
  Function* GetOffset;

  StringRef getPassName() const override {
    return "CHERI expand instrinsics";
  }

public:
  static char ID;
  CheriExpandIntrinsicsPass() : ModulePass(ID) {}

  void expandAddrGet(Module &M, bool* Modified) {
    Function* GetAddr = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_address_get);
    std::vector<CallInst*> ToErase;
    for (Value* V : GetAddr->users()) {
      CallInst *CI = cast<CallInst>(V);
      Value* Cap = CI->getOperand(0);
      IRBuilder<> B(CI);
      Value *Replacement = B.CreateAdd(B.CreateCall(GetBase, {Cap}),
                                       B.CreateCall(GetOffset, {Cap}));
      CI->replaceAllUsesWith(Replacement);
      // Seems like this causes the users() iterator to be invalidated
      // CI->eraseFromParent();
      // Let's collect the users and erase them in separate loop instead
      ToErase.push_back(CI);
      *Modified = true;
    }
    for (CallInst* CI : ToErase) {
      CI->eraseFromParent();
    }
  }

  bool runOnModule(Module &M) override {
    bool Modified = false;
    GetBase = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_base_get);
    GetOffset = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_get);
    expandAddrGet(M, &Modified);
    return Modified;
  }
};

} // anonymous namespace

char CheriExpandIntrinsicsPass::ID;

namespace llvm {
ModulePass *createCheriExpandIntrinsicsPass(void) {
  return new CheriExpandIntrinsicsPass();
}
}
