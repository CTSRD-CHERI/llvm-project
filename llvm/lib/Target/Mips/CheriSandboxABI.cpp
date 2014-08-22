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
#include "llvm/Transforms/Utils/Local.h"

#include <string>
#include <utility>
#include <unordered_map>

#include "llvm/IR/Verifier.h"

using namespace llvm;
using std::pair;

namespace 
{
class CheriSandboxABI : public ModulePass,
                       public InstVisitor<CheriSandboxABI> {
  DataLayout *DL;
  Module *M;
  llvm::SmallVector<AllocaInst*, 16> Allocas;

  virtual const char *getPassName() const {
    return "CHERI sandbox ABI setup";
  }

  public:
    static char ID;
    CheriSandboxABI() : ModulePass(ID), DL(nullptr) {}
    virtual ~CheriSandboxABI() {
      if (DL)
        delete DL;
    }
    void visitAllocaInst(AllocaInst &AI) {
      Allocas.push_back(&AI);
    }
    virtual bool runOnModule(Module &Mod) {
      LLVMContext &C = Mod.getContext();
      // Early abort if we aren't using capabilities on the stack
      if (C.getAllocaAddressSpace() != 200)
        return false;
      C.setAllocaAddressSpace(0);
      M = &Mod;
      DL = new DataLayout(M);
      bool Modified = false;
      for (Function &F : Mod)
        Modified |= runOnFunction(F);
      if (Modified)

#ifndef NDEBUG
      verifyModule(Mod);
#endif
      return Modified;
    }
    bool runOnFunction(Function &F) {
      LLVMContext &C = M->getContext();
      Allocas.clear();
      visit(F);
      // Give up if this function has no allocas
      if (Allocas.empty())
        return false;

      // We're going to replace these allocas (in address space 200) with ones
      // in AS 0, followed by an intrinsic that expands to a
      // stack-capability-relative access and setting the length)
      C.setAllocaAddressSpace(0);

      Function *CastFn =
        Intrinsic::getDeclaration(M, Intrinsic::mips_stack_to_cap);
      Function *SetLenFun =
        Intrinsic::getDeclaration(M, Intrinsic::mips_set_cap_length);

      IRBuilder<> B(C);

      for (AllocaInst *AI : Allocas) {
        B.SetInsertPoint(AI->getParent(), AI);

        PointerType *AllocaTy = AI->getType();
        Type *AllocationTy = AllocaTy->getElementType();

        // Create a new (AS 0) alloca
        Value *Alloca = B.CreateAlloca(AllocationTy, AI->getArraySize());

        // Convert the new alloca into a stack-cap relative capability
        Alloca = B.CreateBitCast(Alloca, Type::getInt8PtrTy(C, 0));
        Alloca = B.CreateCall(CastFn, Alloca);

        // Get the size of the alloca
        unsigned ElementSize = DL->getTypeAllocSize(AllocationTy);
        Value *Size = ConstantInt::get(Type::getInt64Ty(C), ElementSize);
        if (AI->isArrayAllocation())
          Size = B.CreateMul(Size, AI->getArraySize());
        Alloca = B.CreateCall2(SetLenFun, Alloca, Size);
        Alloca = B.CreateBitCast(Alloca, AllocaTy);
        AI->replaceAllUsesWith(Alloca);
        AI->eraseFromParent();
      }
      return true;
    }
};

} // anonymous namespace

char CheriSandboxABI::ID;

namespace llvm {
ModulePass *createCheriSandboxABI(void) {
  return new CheriSandboxABI();
}
}
