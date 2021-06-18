#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/RevocationStrategy.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <string>

#define SHADOW_STACK_CAP_GLOBAL_NAME "StackShadowStackCap"

#define DEBUG_TYPE "cheri-cap-derived-lifetimes"
#define PASS_NAME "CHERI use capability-derived lifetimes"

#define CAPREVOKE_SHADOW_FLAGS 0x07

using namespace llvm;
using namespace llvm::cheri;

namespace {

cl::opt<bool> UnconditionalRevocationAfterAllFunctions(
    "unconditional-revoke-after-all-functions",
    cl::desc("Issue an unconditional revocation sweep after all functions"),
    cl::init(false));

cl::opt<bool> UnconditionalRevocationAfterAllEscapingFunctions(
    "unconditional-revoke-after-all-escaping-functions",
    cl::desc("Issue an unconditional revocation sweep after all functions"),
    cl::init(false));

/**
 * Inserts lifetime checks to ensure temporal stack safety.
 */
class CheriCapDerivedLifetimes : public ModulePass {
private:
  FunctionCallee CaprevokeShadowFunction;
  FunctionCallee CaprevokeStackFunction;

public:
  static char ID;
  CheriCapDerivedLifetimes();
  StringRef getPassName() const override;
  bool runOnModule(Module &Mod) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
  }

private:
  /// Find the caprevoke_shadow() and caprevoke_stack() syscalls and set up the
  /// pointers to them.
  void initialiseCaprevokeFunctions(Module *M);

  /// Run the pass on a function.
  bool runOnFunction(Function &F, Module *M, const LoopInfo &LI) const;

  /// Inserts lifetime check instructions before all pointer-type stores.
  bool insertLifetimeChecks(Function &F, Module *M) const;

  /// Inserts a stack lifetime check directly before a pointer-type store
  /// instruction.
  void insertLifetimeCheckForStore(StoreInst &I, Module *M) const;

  /// Determines the revocation strategy to be used for the function.
  RevocationStrategy determineRevocationStrategy(Function &F, Module *M,
                                                 const LoopInfo &LI) const;

  /// Inserts a conditional revocation call before all terminators in the
  /// function.
  void insertConditionalRevocation(Function &F, Module *M) const;

  /// Inserts an unconditional revocation call before all terminators in the
  /// function.
  void insertUnconditionalRevocation(Function &F, Module *M) const;

  /// Determines whether a basic block terminates with a return statement.
  bool blockIsReturnBlock(const BasicBlock &BB) const;

  /// Insert a revocation call to revoke everything in the current stack frame.
  void insertRevocation(IRBuilder<> &Builder, Module *M) const;

  /// Inserts code to perform a revocation sweep before the given Instruction.
  void insertTestAndRevokeBefore(Instruction &I, Module *M) const;

  /// Returns the frame pointer for the current function.
  Value *getFramePointer(LLVMContext &Context, IRBuilder<> &Builder,
                         Type *DesiredType) const;

  /// Determines whether the function has any properties that make it
  /// incompatible with the capability-derived lifetimes scheme. Functions that
  /// are larger than 4 KiB, contain dynamically-sized stack allocations, or use
  /// address-taken parameters, are currently incompatible.
  bool functionIncompatibleWithCDL(Function &F, const Module *M,
                                   const LoopInfo &LI) const;

  /// Estimates the frame size of a function. If there is any uncertainty, this
  /// will always overestimate.
  unsigned estimateStaticFrameSize(Function &F, const DataLayout &DL) const;

  /// Determines whether a function contains any dynamically-sized stack
  /// allocations.
  bool containsDynamicStackAllocation(Function &F, const LoopInfo &LI) const;

  /// Determines whether any of the parameters to the function are
  /// address-taken.
  bool containsAddressTakenParameter(Function &F) const;
};

} // anonymous namespace

CheriCapDerivedLifetimes::CheriCapDerivedLifetimes() : ModulePass(ID) {
  initializeCheriCapDerivedLifetimesPass(*PassRegistry::getPassRegistry());
}

StringRef CheriCapDerivedLifetimes::getPassName() const { return PASS_NAME; }

bool CheriCapDerivedLifetimes::runOnModule(Module &Mod) {
  initialiseCaprevokeFunctions(&Mod);
  bool modified = false;
  for (Function &F : Mod) {
    if (F.isIntrinsic() || F.empty())
      continue;
    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    modified |= runOnFunction(F, &Mod, LI);
  }
  return modified;
}

void CheriCapDerivedLifetimes::initialiseCaprevokeFunctions(Module *M) {
  LLVMContext &Context = M->getContext();
  DataLayout DL(M);
  Type *IntTy = IntegerType::getInt32Ty(Context);
  Type *CharPtrTy =
      IntegerType::getInt8PtrTy(Context, DL.getProgramAddressSpace());
  Type *CharPtrPtrTy = CharPtrTy->getPointerTo(DL.getProgramAddressSpace());

  CaprevokeShadowFunction = M->getOrInsertFunction(
      "caprevoke_shadow", IntTy, IntTy, CharPtrTy, CharPtrPtrTy);
  CaprevokeStackFunction = M->getOrInsertFunction("caprevoke_stack", IntTy,
                                                  CharPtrTy, CharPtrTy);
}

bool CheriCapDerivedLifetimes::runOnFunction(Function &F, Module *M,
                                             const LoopInfo &LI) const {
  bool Changed = false;

  // Insert CCSC instructions
  Changed |= insertLifetimeChecks(F, M);

  // Determine which revocation strategy to use and insert revocation calls
  // where appropriate
  RevocationStrategy RevStrategy = determineRevocationStrategy(F, M, LI);
  setFunctionRevocationStrategy(F, RevStrategy);
  switch (RevStrategy) {
  case RevocationStrategy::NoRevocation:
    break;
  case RevocationStrategy::ConditionalRevocation:
    insertConditionalRevocation(F, M);
    Changed = true;
    break;
  case RevocationStrategy::UnconditionalRevocation:
    insertUnconditionalRevocation(F, M);
    Changed = true;
  }

  return Changed;
}

bool CheriCapDerivedLifetimes::insertLifetimeChecks(Function &F,
                                                    Module *M) const {
  bool ContainsChecks = false;
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {

      // We only want to check pointer-type stores
      if (I.getOpcode() != Instruction::Store || !isa<StoreInst>(I))
        continue;
      auto &Store = cast<StoreInst>(I);
      if (!Store.getValueOperand()->getType()->isPointerTy())
        continue;

      insertLifetimeCheckForStore(Store, M);
      ContainsChecks = true;
    }
  }
  return ContainsChecks;
}

void CheriCapDerivedLifetimes::insertLifetimeCheckForStore(StoreInst &I,
                                                         Module *M) const {
  assert(I.getValueOperand()->getType()->isPointerTy());

  LLVMContext &Context = I.getContext();
  DataLayout DL(M);

  // Address spaces of the cap used to store and the cap being stored
  unsigned ValueAS = I.getValueOperand()->getType()->getPointerAddressSpace();

  // Types of the pointers and offsets
  Type *SizeTy = Type::getIntNTy(Context, DL.getIndexSizeInBits(ValueAS));

  // Insert the ccsc instruction
  IRBuilder<> Builder(I.getNextNode());
  // TODO: Handle offsets better (it's surprisingly complex!)
  Builder.CreateIntrinsic(Intrinsic::cheri_check_cap_store_cap,
                          {I.getValueOperand()->getType(),
                           I.getPointerOperand()->getType(), SizeTy},
                          {I.getValueOperand(), I.getPointerOperand(),
                           ConstantInt::get(SizeTy, 0)});
}

RevocationStrategy CheriCapDerivedLifetimes::determineRevocationStrategy(
    Function &F, Module *M, const LoopInfo &LI) const {

  bool UnconditionalRevocationRequired =
      UnconditionalRevocationAfterAllFunctions ||
      (UnconditionalRevocationAfterAllEscapingFunctions &&
       F.containsPossiblyEscapingLocals()) ||
      functionIncompatibleWithCDL(F, M, LI);

  if (UnconditionalRevocationRequired) {
    return RevocationStrategy::UnconditionalRevocation;
  } else if (F.containsPossiblyEscapingLocals()) {
    return RevocationStrategy::ConditionalRevocation;
  } else {
    return RevocationStrategy::NoRevocation;
  }
}

void CheriCapDerivedLifetimes::insertConditionalRevocation(Function &F,
                                                           Module *M) const {
  /*
   * Inserting a revocation call requires splitting a basic block into
   * instructions before the return statement, and a new block containing just
   * the return statement. If we just naively loop over all return blocks then
   * we get stuck in an infinite loop until we run out of memory!
   */
  SmallVector<BasicBlock *, 16> TerminatingBlocks;
  for (BasicBlock &BB : F) {
    if (blockIsReturnBlock(BB))
      TerminatingBlocks.push_back(&BB);
  }
  for (BasicBlock *BB : TerminatingBlocks) {
    auto &ReturnStatement = BB->back();
    insertTestAndRevokeBefore(ReturnStatement, M);
  }
}

void CheriCapDerivedLifetimes::insertUnconditionalRevocation(Function &F,
                                                             Module *M) const {
  // See insertConditionalRevocation for explanation of double loop
  SmallVector<BasicBlock *, 16> TerminatingBlocks;
  for (BasicBlock &BB : F) {
    if (blockIsReturnBlock(BB))
      TerminatingBlocks.push_back(&BB);
  }
  for (BasicBlock *BB : TerminatingBlocks) {
    auto &ReturnStatement = BB->back();
    IRBuilder<> Builder(&ReturnStatement);
    insertRevocation(Builder, M);
  }
}

bool CheriCapDerivedLifetimes::blockIsReturnBlock(const BasicBlock &BB) const {
  return !BB.empty() && BB.back().getOpcode() == Instruction::Ret;
}

void CheriCapDerivedLifetimes::insertRevocation(IRBuilder<> &Builder,
                                                     Module *M) const {
  LLVMContext &Context = M->getContext();
  DataLayout DL(M);
  Type *CharPtrTy =
      IntegerType::getInt8PtrTy(Context, DL.getProgramAddressSpace());
  Value *FramePointer = getFramePointer(Context, Builder, CharPtrTy);
  Type *Int32Ty = IntegerType::getInt32Ty(Context);

  Constant *CaprevokeShadowFlags =
      ConstantInt::get(Int32Ty, CAPREVOKE_SHADOW_FLAGS, true);
  Value *ShadowPtrPtr =
      Builder.CreateAlloca(CharPtrTy, DL.getProgramAddressSpace());
  Builder.CreateCall(CaprevokeShadowFunction,
                     {CaprevokeShadowFlags, FramePointer, ShadowPtrPtr});

  Value *ShadowPtr = Builder.CreateLoad(ShadowPtrPtr);
  Builder.CreateCall(CaprevokeStackFunction, {FramePointer, ShadowPtr});
}

void CheriCapDerivedLifetimes::insertTestAndRevokeBefore(Instruction &I,
                                                         Module *M) const {
  LLVMContext &Context = M->getContext();
  DataLayout DL(M);
  Type *Int32Ty = IntegerType::getInt32Ty(Context);
  Type *Int32PtrTy =
      IntegerType::getInt32PtrTy(Context, DL.getProgramAddressSpace());
  IRBuilder<> Builder(&I);

  // Get the frame address
  Value *FramePointer = getFramePointer(Context, Builder, Int32PtrTy);
  Value *StartOfFrame =
      Builder.CreateIntrinsic(Intrinsic::cheri_cap_get_frame_base,
                              {Int32PtrTy, Int32PtrTy}, {FramePointer});

  // Load the value stored there to determine whether revocation is required
  Value *RevocationRequiredFlag =
      Builder.CreateLoad(Int32Ty, StartOfFrame, "revocationRequired");
  Value *RevocationRequired = Builder.CreateICmpEQ(
      RevocationRequiredFlag, ConstantInt::get(Int32Ty, 1));

  // Add a conditional revocation
  Instruction *NewTerminator =
      SplitBlockAndInsertIfThen(RevocationRequired, &I, false);
  Builder.SetInsertPoint(NewTerminator);
  insertRevocation(Builder, M);
}

Value *CheriCapDerivedLifetimes::getFramePointer(LLVMContext &Context,
                                                 IRBuilder<> &Builder,
                                                 Type *DesiredType) const {
  assert(DesiredType->isPointerTy());
  return Builder.CreateIntrinsic(Intrinsic::cheri_cap_get_csp, {DesiredType},
                                 {});
}

bool CheriCapDerivedLifetimes::functionIncompatibleWithCDL(
    Function &F, const Module *M, const LoopInfo &LI) const {
  DataLayout DL(M);

  bool FrameTooBig = estimateStaticFrameSize(F, DL) > 4192;
  bool ContainsDynamicStackAllocation = containsDynamicStackAllocation(F, LI);

  return FrameTooBig || ContainsDynamicStackAllocation;
}

unsigned
CheriCapDerivedLifetimes::estimateStaticFrameSize(Function &F,
                                                  const DataLayout &DL) const {
  unsigned FrameSizeBits = 0;
  for (const BasicBlock &BB : F) {
    for (const Instruction &I : BB) {
      if (I.getOpcode() != Instruction::Alloca)
        continue;
      const auto &Allocation = reinterpret_cast<const AllocaInst &>(I);
      if (!Allocation.getAllocatedType()->isSized())
        continue;
      auto Size = Allocation.getAllocationSizeInBits(DL);
      if (Size)
        FrameSizeBits += *Size;
    }
  }
  return (FrameSizeBits + 7) / 8;
}

bool CheriCapDerivedLifetimes::containsDynamicStackAllocation(
    Function &F, const LoopInfo &LI) const {
  for (const BasicBlock &BB : F) {
    for (const Instruction &I : BB) {
      if (I.getOpcode() != Instruction::Alloca)
        continue;
      const auto &Allocation = reinterpret_cast<const AllocaInst &>(I);

      // Dynamically-sized allocation
      if (!Allocation.isStaticAlloca())
        return true;

      // Allocation that might be in a loop
      if (LI.getLoopDepth(&BB) > 0)
        return true;
    }
  }
  return false;
}

char CheriCapDerivedLifetimes::ID;

INITIALIZE_PASS_BEGIN(CheriCapDerivedLifetimes, DEBUG_TYPE, PASS_NAME, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(CheriCapDerivedLifetimes, DEBUG_TYPE, PASS_NAME, false,
                    false)

ModulePass *llvm::createCheriCapDerivedLifetimesPass() {
  return new CheriCapDerivedLifetimes();
}
