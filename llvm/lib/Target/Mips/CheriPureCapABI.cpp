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

namespace {
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

    for (AllocaInst *AI : Allocas) {
      assert(AI->getType()->getPointerAddressSpace() == 200);
      // Insert immediately after the alloca
      B.SetInsertPoint(AI);
      B.SetInsertPoint(&*++B.GetInsertPoint());
      unsigned ForcedAlignment = 0;

      PointerType *AllocaTy = AI->getType();
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
      Alloca = B.CreateCall(SetLenFun, {Alloca, Size});
      Alloca = B.CreateBitCast(Alloca, AllocaTy);
      AI->replaceAllUsesWith(Alloca);
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
