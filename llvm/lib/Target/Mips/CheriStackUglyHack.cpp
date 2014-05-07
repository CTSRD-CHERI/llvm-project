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
#include "llvm/Support/CallSite.h"
#include "llvm/InstVisitor.h"
#include "llvm/Transforms/Utils/Local.h"

#include <string>
#include <utility>

#include "llvm/IR/Verifier.h"

using namespace llvm;
using std::pair;

namespace 
{
class CheriStackHack : public FunctionPass,
                       public InstVisitor<CheriStackHack> {
  DataLayout *DL;
  Module *M;
  /// Structure used to track an alloca and its possible replacements.
  struct Replacement {
    /// The original alloca instruction
    Value  *Alloca;
    /// The capability to use for capability references to this
    Value *Cap;
    /// The C pointer value for this alloca
    Value *Ptr;
  };
  llvm::SmallVector<AllocaInst*, 16> Allocas;

  Value *InitialBitCast;
  Value *CastToCap;

  void replaceGEP(GetElementPtrInst *GEP, const Replacement &R) {
    SmallVector<Value*, 8> Idxs;
    for (auto I=GEP->idx_begin(),E=GEP->idx_end() ; I!=E ; I++)
      Idxs.push_back(I->get());
    // The result of a GEP is another pointer, so we must do the replacement
    // here.
    Value *NewGEP = GetElementPtrInst::Create(R.Cap, Idxs, GEP->getName(),
        GEP);
    Value *NewGEPAsPtr = new AddrSpaceCastInst(NewGEP, GEP->getType(),
        GEP->getName(), GEP);

    Replacement GEPR = { GEP, NewGEP, NewGEPAsPtr };
    replaceUsers(GEP, GEPR);
    RecursivelyDeleteTriviallyDeadInstructions(GEP);
    RecursivelyDeleteTriviallyDeadInstructions(NewGEPAsPtr);
  }
  void replaceStore(StoreInst *SI, const Replacement &R) {
    new StoreInst(SI->getOperand(0), R.Cap, SI->isVolatile(),
        SI->getAlignment(), SI->getOrdering(), SI->getSynchScope(), SI);
    // Stores can't have users...
    SI->eraseFromParent();
  }
  void replaceLoad(LoadInst *LI, const Replacement &R) {
    LoadInst *CapLoad = new LoadInst(R.Cap, LI->getName(), LI->isVolatile(),
        LI->getAlignment(), LI->getOrdering(), LI->getSynchScope(), LI);
    // We shouldn't need to visit users of this, because once we've loaded the
    // value then it will all work correctly.  Note that this isn't the case if
    // we have an on-stack pointer array and we store a stack address in it.
    // In that case, everything will break.
    LI->replaceAllUsesWith(CapLoad);
  }
  void replaceAddrSpaceCast(AddrSpaceCastInst *AI, const Replacement &R) {
    if (AI->getType() == R.Cap->getType())
      AI->replaceAllUsesWith(R.Cap);
    else {
      BitCastInst *BI = new BitCastInst(R.Cap, AI->getType(), AI->getName(), AI);
      AI->replaceAllUsesWith(BI);
    }
    AI->eraseFromParent();
  }

  void replaceCall(CallInst *CI, const Replacement &R) {
    if (CI == CastToCap)
      return;
    // We can safely ignore some intrinsics: they are only used for mid-level
    // optimisers.
    if (Function *F = CI->getCalledFunction())
      switch (F->getIntrinsicID()) {
        case Intrinsic::lifetime_start:
        case Intrinsic::lifetime_end:
        case Intrinsic::invariant_start:
        case Intrinsic::invariant_end:
          CI->eraseFromParent();
          return;
      }
    CI->replaceUsesOfWith(R.Alloca, R.Ptr);
  }

  void replacePtrToInt(PtrToIntInst *PI, const Replacement &R) {
    PI->replaceUsesOfWith(R.Alloca, R.Ptr);
  }
  void replaceBitCast(BitCastInst *BI, const Replacement &R) {
    BI->replaceUsesOfWith(R.Alloca, R.Ptr);
  }

  void replaceUsers(Instruction *Inst, const Replacement &R) {
    // We're going to delete users as we go, so we need to make sure that we
    // finish iterating before we start deleting
    SmallVector<Value *, 16> Users;
    for (auto U=Inst->use_begin(),E=Inst->use_end() ; U!=E ; ++U)
      Users.push_back(*U);
    for (Value *U : Users) {
      if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(U))
        replaceGEP(GEP, R);
      else if (LoadInst *LI = dyn_cast<LoadInst>(U))
        replaceLoad(LI, R);
      else if (StoreInst *LI = dyn_cast<StoreInst>(U))
        replaceStore(LI, R);
      else if (CallInst *CI = dyn_cast<CallInst>(U))
        replaceCall(CI, R);
      else if (AddrSpaceCastInst *AI = dyn_cast<AddrSpaceCastInst>(U)) {
        if (AI != R.Cap)
          replaceAddrSpaceCast(AI, R);
      }
      else if (PtrToIntInst *PI = dyn_cast<PtrToIntInst>(U))
        replacePtrToInt(PI, R);
      else if (BitCastInst *BI = dyn_cast<BitCastInst>(U)) {
        if (BI != InitialBitCast)
          replaceBitCast(BI, R);
      }
      else {
        U->dump();
        llvm_unreachable("Unknown instruction using alloca!");
      }
    }
  }

  public:
    static char ID;
    CheriStackHack() : FunctionPass(ID) {}
    virtual bool doInitialization(Module &Mod) {
      M = &Mod;
      DL = new DataLayout(M);
      return true;
    }
    virtual ~CheriStackHack() {
      delete DL;
    }
    void visitAllocaInst(AllocaInst &AI) {
      Allocas.push_back(&AI);
    }
    virtual bool runOnFunction(Function &F) {
      Allocas.clear();
      visit(F);
      if (Allocas.empty())
        return false;

      LLVMContext &C = M->getContext();
      Function *CastFn =
        Intrinsic::getDeclaration(M, Intrinsic::mips_stack_to_cap);
      Function *SetLenFun =
        Intrinsic::getDeclaration(M, Intrinsic::mips_set_cap_length);

      IRBuilder<> B(C);

      for (AllocaInst *AI : Allocas) {
        ilist_iterator<Instruction> I(AI);
        B.SetInsertPoint(AI->getParent(), ++I);

        PointerType *AllocaTy = AI->getType();
        Type *AllocationTy = AllocaTy->getElementType();
        unsigned ElementSize = DL->getTypeAllocSize(AllocationTy);
        Value *Size = ConstantInt::get(Type::getInt64Ty(C), ElementSize);
        if (AI->isArrayAllocation())
          Size = B.CreateMul(Size, AI->getArraySize());

        PointerType *CapTy = PointerType::get(AllocationTy, 200);
        InitialBitCast = B.CreateBitCast(AI, Type::getInt8PtrTy(C, 0));
        CastToCap = B.CreateCall(CastFn, InitialBitCast);
        Value *AllocaAsCap = B.CreateCall2(SetLenFun, CastToCap, Size);
        AllocaAsCap = B.CreateBitCast(AllocaAsCap, CapTy);
        Value *AllocaAsPtr = B.CreateAddrSpaceCast(CastToCap, AllocaTy);
        Replacement R = { AI, AllocaAsCap, AllocaAsPtr };
        replaceUsers(AI, R);
        RecursivelyDeleteTriviallyDeadInstructions(AllocaAsPtr);
      }
      return true;
    }
};

} // anonymous namespace

char CheriStackHack::ID;

namespace llvm {
FunctionPass *createCheriStackHack(void) {
  return new CheriStackHack();
}
}
