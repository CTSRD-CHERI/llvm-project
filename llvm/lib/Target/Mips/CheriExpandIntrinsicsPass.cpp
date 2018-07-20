#define DEBUG_TYPE "cheri-expand-intrinsics"   // This goes before any #includes.

#include "llvm/Pass.h"
#include "llvm/ADT/Statistic.h"
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

using namespace llvm;

/// Expand the @llvm.cheri.cap.address.set() instrinsic to getaddr + sub + cincoffset
namespace {

STATISTIC(NumSetAddrCalls, "Number of CHERI SetAddr uses");

class CHERIExpandCapIntrinsics : public ModulePass {

  StringRef getPassName() const override {
    return "CHERI expand instrinsics";
  }

public:
  static char ID;
  CHERIExpandCapIntrinsics() : ModulePass(ID) {}

  void expandAddrSet(Module &M, bool *Modified) {

    // SetAddr($cap, $addr) -> CIncOffset($cap, $addr - GetAddr($cap))

    Function *SetAddr =
        M.getFunction(Intrinsic::getName(Intrinsic::cheri_cap_address_set));
    if (!SetAddr)
      return;

    Function* GetAddr = Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_address_get);
    Function *IncOffset =
        Intrinsic::getDeclaration(&M, Intrinsic::cheri_cap_offset_increment);

    std::vector<CallInst*> ToErase;
    for (Value* V : SetAddr->users()) {
      NumSetAddrCalls += 1;

      CallInst *CI = cast<CallInst>(V);
      Value* Cap = CI->getOperand(0);
      Value* NewAddr = CI->getOperand(1);
      IRBuilder<> B(CI);
      Value *Difference = B.CreateSub(NewAddr, B.CreateCall(GetAddr, {Cap}));
      Value *Replacement = B.CreateCall(IncOffset, {Cap, Difference});
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
    expandAddrSet(M, &Modified);
    return Modified;
  }
};

} // anonymous namespace

char CHERIExpandCapIntrinsics::ID = 0;
INITIALIZE_PASS(CHERIExpandCapIntrinsics, "cheri-expand-intrinsics",
                "Expand CHERI intrinsics", false, false)

namespace llvm {
ModulePass *createCheriExpandIntrinsicsPass(void) {
  return new CHERIExpandCapIntrinsics();
}
}
