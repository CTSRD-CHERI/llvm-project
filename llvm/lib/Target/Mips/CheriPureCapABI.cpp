#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/PtrUseVisitor.h"
#include "llvm/Analysis/Utils/Local.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Cheri.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/CheriSetBounds.h"
#include "llvm/Transforms/Utils/Local.h"

#include <string>
#include <utility>
#include <unordered_map>

#include "llvm/IR/Verifier.h"

#define DEBUG_TYPE "cheri-purecap-alloca"


using namespace llvm;
using std::pair;

namespace {

class CheckPtrEscape : public PtrUseVisitor<CheckPtrEscape> {
  using Base = PtrUseVisitor<CheckPtrEscape>;
  AllocaInst* Alloca = nullptr;
public:
  CheckPtrEscape(const DataLayout &DL) : PtrUseVisitor<CheckPtrEscape>(DL) {}

  PtrInfo checkAlloca(AllocaInst* AI) {
    Alloca = AI;
    return visitPtr(*AI);
  }

  void visitGetElementPtrInst(GetElementPtrInst &GEPI) {
    Base::visitGetElementPtrInst(GEPI);
#if 1
    // We also treat a GetElementPtrInst with a non-constant offset as escaped
    // since this means we need to set bounds.
    // Example:
    // int foo(int n) { int buffer[128]; /* ... */ return buffer[n] }
    // The basic PtrUseVisitor does not treat GEPs as escaped so without
    // this check we would not be setting bounds.
    if (!GEPI.isInBounds() && !GEPI.hasAllConstantIndices()) {
      PI.setEscaped(&GEPI);
    } else if (IsOffsetKnown) {
      unsigned BitWidth = DL.getIndexTypeSizeInBits(GEPI.getType());
      auto AllocaSize = Alloca->getAllocationSizeInBits(DL);
      APInt Zero(BitWidth, 0);
      APInt Max(BitWidth, AllocaSize ? *AllocaSize / 8: 0);
      LLVM_DEBUG(dbgs() << GEPI.getFunction()->getName() << ": MAX IS" << Max; GEPI.dump(););
      LLVM_DEBUG(dbgs() << GEPI.getFunction()->getName() << ": OFFSET IS: " << Offset; GEPI.dump(););
      if (Offset.slt(Zero) || Offset.sge(Max)) {
        LLVM_DEBUG(dbgs() << GEPI.getFunction()->getName() << ": OFFSET IS OUT OF BOUNDS: " <<  Offset; GEPI.dump());
        PI.setEscaped(&GEPI);
      }
    } else {
       LLVM_DEBUG(dbgs() << "OFFSET IS UNKNOWN in " << GEPI.getFunction()->getName(); GEPI.dump(););
       PI.setEscaped(&GEPI);
    }
#else
    // for now just always treat a GEP as needing bounds.
    // TODO: be a bit more smart about this to allow struct member loads/stores
    PI.setEscaped(&GEPI);
#endif
  }

  void visitReturnInst(ReturnInst &R) {
    // Returning the alloca also causes it to escape (even if that shouldn't
    // happen in practice we should still be setting bounds here.
    // Example:
    // int foo(int n) { int stack_var; /* ... */ return &stack_var; }
    PI.setEscaped(&R);
    Base::visitReturnInst(R);
  }
#if 0
  // TODO: do I need this for visitGetElementPtr too?
  void visitGetElementPtr(GetElementPtrInst &GEPI) {
    if (!GEPI.hasAllConstantIndices())
      PI.setEscaped(&GEPI);

    PtrUseVisitor<CheckPtrEscape>::visitGetElementPtr(GEPI);
  }
#endif
};

class CheriPureCapABI : public ModulePass, public InstVisitor<CheriPureCapABI> {
  Module *M;
  llvm::SmallVector<AllocaInst *, 16> Allocas;
  bool IsCheri128;


public:
  static char ID;
  CheriPureCapABI() : ModulePass(ID) {}
  virtual StringRef getPassName() const { return "CHERI sandbox ABI setup"; }
  void visitAllocaInst(AllocaInst &AI) { Allocas.push_back(&AI); }
  virtual bool runOnModule(Module &Mod) {
    M = &Mod;
    // Early abort if we aren't using capabilities on the stack
    if (Mod.getDataLayout().getAllocaAddrSpace() != 200)
      return false;
    IsCheri128 = Mod.getDataLayout().getPointerSizeInBits(200) == 128;
    bool Modified = false;
    for (Function &F : Mod)
      Modified |= runOnFunction(F);
    return Modified;
  }

  bool runOnFunction(Function &F) {
    LLVMContext &C = M->getContext();
    Allocas.clear();
    visit(F);
    // Give up if this function has no allocas
    if (Allocas.empty())
      return false;

    Intrinsic::ID SetLength = Intrinsic::cheri_cap_bounds_set;
    Function *SetLenFun = Intrinsic::getDeclaration(M, SetLength);
    Intrinsic::ID StackToCap = Intrinsic::mips_stack_to_cap;
    Function *StackToCapFn = Intrinsic::getDeclaration(M, StackToCap);

    IRBuilder<> B(C);
    const DataLayout &DL = F.getParent()->getDataLayout();
    CheckPtrEscape CPE(DL);

    for (AllocaInst *AI : Allocas) {
      // Insert immediately after the alloca
      B.SetInsertPoint(AI);
      B.SetInsertPoint(&*++B.GetInsertPoint());
      unsigned ForcedAlignment = 0;

      PointerType *AllocaTy = AI->getType();
      assert(isCheriPointer(AllocaTy, &DL));
      Type *AllocationTy = AllocaTy->getElementType();
      Value *ArraySize = AI->getArraySize();

      // Create a new (AS 0) alloca
      // For imprecise capabilities, we need to increase the alignment for
      // on-stack allocations to ensure that we can create precise bounds.
      if (IsCheri128) {
        uint64_t AllocaSize = DL.getTypeAllocSize(AllocationTy);
        if (ConstantInt *CI = dyn_cast<ConstantInt>(ArraySize))
          AllocaSize *= CI->getValue().getLimitedValue();
        else
          AllocaSize *= 1048576;
        ForcedAlignment = AllocaSize / (1 << 13);
        ForcedAlignment = PowerOf2Ceil(ForcedAlignment);
        // MIPS doesn't support stack alignments greater than 2^16
        ForcedAlignment = std::min(ForcedAlignment, 0x4000U);
      }
      AI->setAlignment(std::max(AI->getAlignment(), ForcedAlignment));
      CheckPtrEscape::PtrInfo PI = CPE.checkAlloca(AI);
      // Only set bounds for allocas that escape this function
      const bool NeedBounds = PI.isEscaped();

      Instruction *BitCast =
          cast<Instruction>(B.CreateBitCast(AI, Type::getInt8PtrTy(C, 200)));

      // Get the size of the alloca
      unsigned ElementSize = DL.getTypeAllocSize(AllocationTy);
      Value *Size = ConstantInt::get(Type::getInt64Ty(C), ElementSize);
      if (AI->isArrayAllocation())
        Size = B.CreateMul(Size, AI->getArraySize());

      Value *Alloca = B.CreateCall(StackToCapFn, BitCast);
      if (BitCast == AI)
         BitCast = cast<Instruction>(Alloca);

      if (NeedBounds) {
        LLVM_DEBUG(dbgs() << "No need to set bounds on stack alloca";
                   AI->dump());

        if (cheri::ShouldCollectCSetBoundsStats) {
          cheri::addSetBoundsStats(AI->getAlignment(), Size, getPassName(),
                                   cheri::SetBoundsPointerSource::Stack,
                                   "set bounds on " +
                                       cheri::inferLocalVariableName(AI),
                                   cheri::inferSourceLocation(AI));
        }
        LLVM_DEBUG(auto S = cheri::inferConstantValue(Size);
                   dbgs() << AI->getFunction()->getName()
                          << ": setting bounds on stack alloca to "
                          << (S ? Twine(*S) : Twine("<unknown>"));
                   AI->dump());
        Alloca = B.CreateCall(SetLenFun, {Alloca, Size});
      }
      Alloca = B.CreateBitCast(Alloca, AllocaTy);
      AI->replaceNonMetadataUsesWith(Alloca);
      // If we didn't create a bitcast because the alloca has the right type
      // we need to set the @llvm.cheri.cap.bounds.set() argument to be the
      // alloca instruction!
      // Otherwise we need to update the bitcast operand to be the alloca again
      // since it was replaced with the setbounds result by replaceNonMetadataUsesWith()
      // This is invalid IR so we need to undo it.
      assert(BitCast != AI);
      BitCast->setOperand(0, AI);
    }
    return true;
  }
};

} // anonymous namespace

char CheriPureCapABI::ID;

namespace llvm {
ModulePass *createCheriPureCapABI(void) { return new CheriPureCapABI(); }
}
