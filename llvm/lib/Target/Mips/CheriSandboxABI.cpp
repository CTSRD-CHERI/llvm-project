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
class CheriSandboxABI : public ModulePass, public InstVisitor<CheriSandboxABI> {
  DataLayout *DL;
  Module *M;
  llvm::SmallVector<AllocaInst *, 16> Allocas;
  bool IsCheri128;

  virtual const char *getPassName() const { return "CHERI sandbox ABI setup"; }

public:
  static char ID;
  CheriSandboxABI() : ModulePass(ID), DL(nullptr) {}
  virtual ~CheriSandboxABI() {
    if (DL)
      delete DL;
  }
  void visitAllocaInst(AllocaInst &AI) { Allocas.push_back(&AI); }
  virtual bool runOnModule(Module &Mod) {
    M = &Mod;
    DL = new DataLayout(M);
    // Early abort if we aren't using capabilities on the stack
    if (DL->getAllocaAS() != 200)
      return false;
    // We're going to replace all allocas (in address space 200) with ones in
    // AS 0, followed by an intrinsic that expands to a
    // stack-capability-relative access and setting the length.  Make sure
    // that the AS for new allocas is 0.
    DL->setAllocaAS(0);
    M->setDataLayout(*DL);
    IsCheri128 = DL->getPointerSizeInBits(200) == 128;
    bool Modified = false;
    for (Function &F : Mod)
      Modified |= runOnFunction(F);
    // Now we're going to fix up all va_start calls so that they have the
    // address space cast of the alloca directly in front of them.  This
    // makes the cast visible to SelectionDAG and allows it to look through
    // and find the original.
    if (Function *Fn = M->getFunction("llvm.va_start"))
      for (Value *V : Fn->users()) {
        CallInst *Call = cast<CallInst>(V->stripPointerCasts());
        Instruction *Cast = cast<Instruction>(Call->getOperand(0));
        if (Cast->getParent() != Call->getParent()) {
          AddrSpaceCastInst *NewCast = new llvm::AddrSpaceCastInst(
              Cast->getOperand(0), Cast->getType(), "va_cast", Call);
          Call->setOperand(0, NewCast);
        }
      }
    return Modified;
  }
  int RoundUpToPowerOfTwo(int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
  }
  bool runOnFunction(Function &F) {
    LLVMContext &C = M->getContext();
    Allocas.clear();
    visit(F);
    // Give up if this function has no allocas
    if (Allocas.empty())
      return false;

    Intrinsic::ID SetLength = Intrinsic::memcap_cap_bounds_set;
    Function *CastFn =
        Intrinsic::getDeclaration(M, Intrinsic::mips_stack_to_cap);
    Function *SetLenFun = Intrinsic::getDeclaration(M, SetLength);

    IRBuilder<> B(C);

    for (AllocaInst *AI : Allocas) {
      assert(AI->getType()->getPointerAddressSpace() == 200);
      B.SetInsertPoint(AI->getParent(), AI);
      unsigned ForcedAlignment = 0;

      PointerType *AllocaTy = AI->getType();
      Type *AllocationTy = AllocaTy->getElementType();
      Value *ArraySize = AI->getArraySize();

      // Create a new (AS 0) alloca
      Value *Alloca = B.CreateAlloca(AllocationTy, ArraySize);
      assert(Alloca->getType()->getPointerAddressSpace() == 0);
      // For imprecise capabilities, we need to increase the alignment for
      // on-stack allocations to ensure that we can create precise bounds.
      if (IsCheri128) {
        uint64_t AllocaSize = DL->getTypeAllocSize(AllocationTy);
        if (ConstantInt *CI = dyn_cast<ConstantInt>(ArraySize))
          AllocaSize *= CI->getValue().getLimitedValue();
        else
          AllocaSize *= 1048576;
        ForcedAlignment = AllocaSize / (1 << 13);
        ForcedAlignment = RoundUpToPowerOfTwo(ForcedAlignment);
        // MIPS doesn't support stack alignments greater than 2^16
        ForcedAlignment = std::min(ForcedAlignment, 0x4000U);
      }
      cast<AllocaInst>(Alloca)
          ->setAlignment(std::max(AI->getAlignment(), ForcedAlignment));

      // Convert the new alloca into a stack-cap relative capability
      Alloca = B.CreateBitCast(Alloca, Type::getInt8PtrTy(C, 0));
      Alloca = B.CreateCall(CastFn, Alloca);

      // Get the size of the alloca
      unsigned ElementSize = DL->getTypeAllocSize(AllocationTy);
      Value *Size = ConstantInt::get(Type::getInt64Ty(C), ElementSize);
      if (AI->isArrayAllocation())
        Size = B.CreateMul(Size, AI->getArraySize());
      Alloca = B.CreateCall(SetLenFun, {Alloca, Size});
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
ModulePass *createCheriSandboxABI(void) { return new CheriSandboxABI(); }
}
