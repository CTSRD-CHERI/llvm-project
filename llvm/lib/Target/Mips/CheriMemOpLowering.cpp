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

namespace 
{
class CheriMemOpLowering : public FunctionPass,
                           public InstVisitor<CheriMemOpLowering> {
  DataLayout *DL;
  Module *M;
  Constant *Memset_c;
  bool Modified;

  virtual const char *getPassName() const {
    return "CHERI memory operation lowering";
  }
  Constant *getMemSet_c() {
    if (!Memset_c) {
      LLVMContext &C = M->getContext();
      Memset_c = M->getOrInsertFunction("memset_c", Type::getVoidTy(C),
          Type::getInt8PtrTy(C, 200), Type::getInt32Ty(C),
          Type::getInt64Ty(C), nullptr);
    }
    return Memset_c;
  }

  public:
    static char ID;
    CheriMemOpLowering() : FunctionPass(ID) {}
    virtual bool doInitialization(Module &Mod) {
      M = &Mod;
      Memset_c = nullptr;
      DL = new DataLayout(M);
      return true;
    }
    virtual ~CheriMemOpLowering() {
      Memset_c = nullptr;
      delete DL;
    }
    void visitCallInst(CallInst &CI) {
      if (MemSetInst *MSI = dyn_cast<MemSetInst>(&CI))
        if (MSI->getDestAddressSpace() == 200) {
          IRBuilder<> B(&CI);
          Modified = true;
          Value *Args[3];
          Args[0] = CI.getOperand(0);
          Args[1] = B.CreateZExt(CI.getOperand(1),
              Type::getInt32Ty(M->getContext()));
          Args[2] = CI.getOperand(2);
          CallInst *Memset = CallInst::Create(getMemSet_c(), Args,
              CI.getName(), &CI);
          Memset->setDebugLoc(CI.getDebugLoc());
          CI.eraseFromParent();
        }
    }
    virtual bool runOnFunction(Function &F) {
      Modified = false;
      visit(F);
#ifndef NDEBUG
      verifyModule(*F.getParent());
#endif
      return Modified;
    }
};

} // anonymous namespace

char CheriMemOpLowering::ID;

namespace llvm {
FunctionPass *createCheriMemOpLowering(void) {
  return new CheriMemOpLowering();
}
}
