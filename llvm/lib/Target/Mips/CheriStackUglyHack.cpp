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
  std::unordered_map<PHINode*, PHINode*> ModifiedPHIs;

  Value *InitialBitCast;
  Value *CastToCap;

  PointerType *getCapType(PointerType *Ty) {
    assert(Ty->getAddressSpace() == 0);
    return PointerType::get(Ty->getElementType(), 200);
  }
  PointerType *getCapType(Value *V) {
    return getCapType(cast<PointerType>(V->getType()));
  }

  virtual const char *getPassName() const {
    return "CHERI capability stack transform";
  }

  void replaceGEP(GetElementPtrInst *GEP, const Replacement &R) {
    SmallVector<Value*, 8> Idxs;
    for (auto I=GEP->idx_begin(),E=GEP->idx_end() ; I!=E ; I++)
      Idxs.push_back(I->get());
    Value *NewGEP;
    Instruction *NewGEPAsPtr;

    APInt Offset(64, 0, true);
    if (GEP->accumulateConstantOffset(*DL, Offset) && Offset.isNegative()) {
      // If the GEP is negative then a capability offset will fail.  We must
      // re-derive the capability version from the pointer.
      // FIXME: This becomes obsolete with cursors.
      NewGEPAsPtr = GetElementPtrInst::Create(R.Ptr, Idxs, GEP->getName(),
          GEP);

      LLVMContext &C = M->getContext();
      IRBuilder<> B(C);
      ilist_iterator<Instruction> I(NewGEPAsPtr);
      B.SetInsertPoint(GEP->getParent(), ++I);

      Function *CastFn =
        Intrinsic::getDeclaration(M, Intrinsic::mips_stack_to_cap);
      Value *I8GEP = B.CreateBitCast(NewGEPAsPtr, Type::getInt8PtrTy(C, 0));
      Value *I8CapGEP  = B.CreateCall(CastFn, I8GEP);
      NewGEP = B.CreateBitCast(I8CapGEP,
          getCapType(cast<PointerType>(NewGEPAsPtr->getType())));
    } else {
      // The result of a GEP is another pointer, so we must do the replacement
      // here.
      NewGEP = GetElementPtrInst::Create(R.Cap, Idxs, GEP->getName(), GEP);
      NewGEPAsPtr = new AddrSpaceCastInst(NewGEP, GEP->getType(),
          GEP->getName(), GEP);
    }


    Replacement GEPR = { GEP, NewGEP, NewGEPAsPtr };
    replaceUsers(GEP, GEPR);
  }
  void replaceStore(StoreInst *SI, const Replacement &R) {
    Value *Ptr = SI->getPointerOperand();
    Value *Val = SI->getValueOperand();
    if (Ptr == R.Alloca)
      Ptr = R.Cap;
    if (Val == R.Alloca)
      Val = R.Ptr;
    new StoreInst(Val, Ptr, SI->isVolatile(),
        SI->getAlignment(), SI->getOrdering(), SI->getSynchScope(), SI);
    // Stores can't have users...
    SI->eraseFromParent();
  }
  void replacePHI(PHINode *Phi, const Replacement &R) {
    unsigned Count = Phi->getNumIncomingValues();
    Phi->replaceUsesOfWith(R.Alloca, R.Ptr);
    PHINode *NewPhi = ModifiedPHIs[Phi];
    // If the replacement (capability) PHI node has already been created, then
    // just add this use
    if (NewPhi) {
      for (unsigned i=0 ; i<Count ; i++)
        if (Phi->getIncomingValue(i) == R.Ptr)
          NewPhi->setIncomingValue(i, R.Cap);
      return;
    }
    Type *CapTy = getCapType(Phi);
    NewPhi = PHINode::Create(CapTy, Count, Phi->getName(), Phi);
    ModifiedPHIs[Phi] = NewPhi;
    for (unsigned i=0 ; i<Count ; i++) {
      // We need a placeholder value for a 'value not yet known', but it can't
      // be null and must have the same type as the Phi, so we use the Phi
      // itself.  This results in an invalid module (which the verifier will
      // spot), but the verifier shouldn't be run for an under-construction
      // module and we want it to complain if we haven't fixed it by the time
      // this pass finishes.
      NewPhi->addIncoming(NewPhi, Phi->getIncomingBlock(i));
      if (Phi->getIncomingValue(i) == R.Ptr)
        NewPhi->setIncomingValue(i, R.Cap);
    }
    Replacement NewR = { Phi, NewPhi, Phi };
    replaceUsers(Phi, NewR);
    return;
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
  void replaceICmp(ICmpInst *II, const Replacement &R) {
    II->replaceUsesOfWith(R.Alloca, R.Ptr);
  }

  void replaceBitCast(BitCastInst *BI, const Replacement &R) {
    // If the bitcast isn't a pointer cast, then just use the pointer version
    if (!BI->getType()->isPointerTy()) {
      BI->replaceUsesOfWith(R.Alloca, R.Ptr);
      return;
    }
    // If it is a pointer, then we want to replace it with a capability version
    // and then propagate this to users.
    PointerType *BaseType = cast<PointerType>(BI->getType());
    // If it's not address space, then just reuse the original
    if (BaseType->getAddressSpace() != 0) {
      BI->replaceUsesOfWith(R.Alloca, R.Ptr);
      return;
    }
    Type *CapTy = PointerType::get(BaseType->getElementType(), 200);
    Value *CapBitcast = new BitCastInst(R.Cap, CapTy, BI->getName(), BI);
    Value *PtrBitcast = new BitCastInst(R.Ptr, BI->getType(), BI->getName(),
        BI);

    Replacement BCR = { BI, CapBitcast, PtrBitcast };
    replaceUsers(BI, BCR);
  }

  void replaceUsers(Instruction *Inst, const Replacement &R) {
    // We're going to delete users as we go, so we need to make sure that we
    // finish iterating before we start deleting
    SmallVector<Value *, 16> Users;
    for (auto U=Inst->user_begin(),E=Inst->user_end() ; U!=E ; ++U)
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
      } else if (PHINode *P = dyn_cast<PHINode>(U))
        replacePHI(P, R);
      else if (ICmpInst *I = dyn_cast<ICmpInst>(U))
        replaceICmp(I, R);
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
      ModifiedPHIs.clear();
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
      }
      // Some of the PHI nodes may not have had a capability propagated to all
      // of their values.  For these, we just construct a capability that we
      // can then use from the pointer available at the end of the incoming
      // block.
      for (auto &I : ModifiedPHIs) {
        PHINode *PtrPhi = I.first;
        PHINode *CapPhi = I.second;
        for (unsigned i=0, e=PtrPhi->getNumIncomingValues() ; i<e ; i++)
          if (CapPhi == CapPhi->getIncomingValue(i)) {
            Instruction *Insert = CapPhi->getIncomingBlock(i)->getTerminator();
            Value *Cap = new AddrSpaceCastInst(PtrPhi->getIncomingValue(i),
                CapPhi->getType(), "", Insert);
            CapPhi->setIncomingValue(i, Cap);
          }
      }
      for (BasicBlock &BB : F) {
        SimplifyInstructionsInBlock(&BB, DL);
      }
      removeUnreachableBlocks(F);
#ifndef NDEBUG
      verifyModule(*F.getParent());
#endif
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
