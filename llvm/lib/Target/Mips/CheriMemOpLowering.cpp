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

namespace {
class CheriMemOpLowering : public FunctionPass,
                           public InstVisitor<CheriMemOpLowering> {
  DataLayout *DL;
  Module *M;
  Constant *Memset_c;
  Constant *Memcpy_c;
  Constant *Memmove_c;
  Type *CapTy;
  Type *Int32Ty;
  Type *Int64Ty;
  Type *VoidTy;
  bool Modified;

  virtual StringRef getPassName() const {
    return "CHERI memory operation lowering";
  }
  Constant *getMemCpy_c() {
    if (!Memcpy_c)
      Memcpy_c = M->getOrInsertFunction("memcpy_c", CapTy, CapTy, CapTy,
                                        Int64Ty, nullptr);
    return Memcpy_c;
  }
  Constant *getMemMove_c() {
    if (!Memmove_c)
      Memmove_c = M->getOrInsertFunction("memmove_c", CapTy, CapTy, CapTy,
                                         Int64Ty, nullptr);
    return Memmove_c;
  }
  Constant *getMemSet_c() {
    if (!Memset_c)
      Memset_c = M->getOrInsertFunction("memset_c", VoidTy, CapTy, Int32Ty,
                                        Int64Ty, nullptr);
    return Memset_c;
  }

public:
  static char ID;
  CheriMemOpLowering() : FunctionPass(ID) {}
  virtual bool doInitialization(Module &Mod) {
    M = &Mod;
    LLVMContext &C = M->getContext();
    CapTy = Type::getInt8PtrTy(C, 200);
    VoidTy = Type::getVoidTy(C);
    Int32Ty = Type::getInt32Ty(C);
    Int64Ty = Type::getInt64Ty(C);
    Memset_c = nullptr;
    Memcpy_c = nullptr;
    Memmove_c = nullptr;
    DL = new DataLayout(M);
    return true;
  }
  virtual ~CheriMemOpLowering() {
    Memset_c = nullptr;
    delete DL;
  }
  void visitCallInst(CallInst &CI) {
    if (MemSetInst *MSI = dyn_cast<MemSetInst>(&CI)) {
      if (MSI->getDestAddressSpace() == 200) {
        IRBuilder<> B(&CI);
        Modified = true;
        Value *Args[3];
        Args[0] = CI.getOperand(0);
        Args[1] =
            B.CreateZExt(CI.getOperand(1), Type::getInt32Ty(M->getContext()));
        Args[2] = CI.getOperand(2);
        CallInst *Memset =
            CallInst::Create(getMemSet_c(), Args, CI.getName(), &CI);
        Memset->setDebugLoc(CI.getDebugLoc());
        CI.eraseFromParent();
      }
    } else if (MemTransferInst *MTI = dyn_cast<MemTransferInst>(&CI)) {
      if (MTI->getDestAddressSpace() == 200 ||
          MTI->getSourceAddressSpace() == 200) {
        IRBuilder<> B(&CI);
        Modified = true;
        Value *Args[3];
        Args[0] = CI.getOperand(0);
        Args[1] = CI.getOperand(1);
        Args[2] = CI.getOperand(2);
        if (MTI->getDestAddressSpace() != 200)
          Args[0] = B.CreateAddrSpaceCast(Args[0], CapTy);
        if (MTI->getSourceAddressSpace() != 200)
          Args[1] = B.CreateAddrSpaceCast(Args[1], CapTy);
        Constant *Fn = isa<MemCpyInst>(CI) ? getMemCpy_c() : getMemMove_c();
        CallInst *Memset = CallInst::Create(Fn, Args, CI.getName(), &CI);
        Memset->setDebugLoc(CI.getDebugLoc());
        CI.eraseFromParent();
      }
    }
  }
  virtual bool runOnFunction(Function &F) {
    Modified = false;
    visit(F);
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
