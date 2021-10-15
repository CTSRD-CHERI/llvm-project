#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/CheriBounds.h"
#include "llvm/Analysis/Utils/Local.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/InitializePasses.h"
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
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/CheriSetBounds.h"
#include "llvm/Transforms/Utils/Local.h"

#include <string>
#include <utility>
#include <unordered_map>

#include "llvm/IR/Verifier.h"

#define DEBUG_TYPE "cheri-bound-allocas"
using namespace llvm;
#define DBG_MESSAGE(...) LLVM_DEBUG(dbgs() << DEBUG_TYPE ": " << __VA_ARGS__)

namespace {

// TODO: remove this option after eval
static cl::opt<bool> UseRematerializableIntrinsic(
    "cheri-stack-bounds-allow-remat", cl::init(true),
    cl::desc("Use bounded.stack.cap() instead of bounds.set() for stack "
             "allocations (allows rematerialization)"),
    cl::Hidden);

// Loading/storing from constant stack indices does not need to use a small
// tightly bounded capability and can use $csp instead
// TODO: remove these options once we know what the best stragegy is?
// TODO: change this to an integer threshold (more than N uses -> reuse the same one)
static cl::opt<unsigned> SingleIntrinsicThreshold(
    "cheri-stack-bounds-single-intrinsic-threshold", cl::init(5),
    cl::desc("Reuse the result of a single CHERI bounds intrinsic if there are "
             "more than N uses (default=5). A value of 0 means always."),
    cl::Hidden);

// single option instead of the booleans?
enum class StackBoundsMethod {
  Never,
  ForAllUsesIfOneNeedsBounds, // This is not particularly useful, just for
                              // comparison
  IfNeeded,
  AllUses,
};

static cl::opt<StackBoundsMethod> BoundsSettingMode(
    "cheri-stack-bounds",
    cl::desc("Strategy for setting bounds on stack capabilities:"),
    cl::init(StackBoundsMethod::IfNeeded),
    cl::values(clEnumValN(StackBoundsMethod::Never, "never",
                          "Do not add bounds on stack allocations (UNSAFE!)"),
               clEnumValN(StackBoundsMethod::ForAllUsesIfOneNeedsBounds,
                          "all-or-none",
                          "Set stack allocation bounds for all uses if at "
                          "least one use neededs bounds, otherwise omit"),
               clEnumValN(StackBoundsMethod::IfNeeded, "if-needed",
                          "Set stack allocation bounds for all uses except for "
                          "loads/stores to statically known in-bounds offsets"),
               clEnumValN(StackBoundsMethod::AllUses, "all-uses",
                          "Set stack allocation bounds even for loads/stores "
                          "to statically known in-bounds offset")));

STATISTIC(NumProcessed,  "Number of allocas that were analyzed for CHERI bounds");
STATISTIC(NumDynamicAllocas,  "Number of dyanmic allocas that were analyzed"); // TODO: skip them
STATISTIC(NumUsesProcessed, "Total number of alloca uses that were analyzed");
STATISTIC(NumCompletelyUnboundedAllocas, "Number of allocas where CHERI bounds were completely unncessary");
STATISTIC(NumUsesWithBounds, "Number of alloca uses that had CHERI bounds added");
STATISTIC(NumUsesWithoutBounds, "Number of alloca uses that did not needed CHERI bounds");
STATISTIC(NumSingleIntrin, "Number of times that a single intrinisic was used instead of per-use");

class CheriBoundAllocas : public ModulePass, public InstVisitor<CheriBoundAllocas> {
  Module *M;
  llvm::SmallVector<AllocaInst *, 16> Allocas;
  Type *I8CapTy;
  Type *SizeTy;

public:
  static char ID;
  CheriBoundAllocas() : ModulePass(ID) {
    initializeCheriBoundAllocasPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return "CHERI bound stack allocations"; }
  void visitAllocaInst(AllocaInst &AI) { Allocas.push_back(&AI); }
  bool runOnModule(Module &Mod) override {
    M = &Mod;
    const DataLayout &DL = Mod.getDataLayout();
    unsigned AllocaAS = DL.getAllocaAddrSpace();

    // Early abort if we aren't using capabilities on the stack
    if (!DL.isFatPointer(AllocaAS))
      return false;

    LLVMContext &C = M->getContext();
    I8CapTy = Type::getInt8PtrTy(C, AllocaAS);
    SizeTy = Type::getIntNTy(C, DL.getIndexSizeInBits(AllocaAS));

    bool Modified = false;
    for (Function &F : Mod)
      Modified |= runOnFunction(F);

    return Modified;
  }

  bool runOnFunction(Function &F) {
    // always set bounds with optnone
    bool IsOptNone = F.hasFnAttribute(Attribute::OptimizeNone);
    // FIXME: should still ignore lifetime-start + lifetime-end intrinsics even at -O0
    const TargetPassConfig &TPC = getAnalysis<TargetPassConfig>();
    const TargetMachine &TM = TPC.getTM<TargetMachine>();
    const TargetLowering *TLI = TM.getSubtargetImpl(F)->getTargetLowering();

    LLVMContext &C = M->getContext();
    Allocas.clear();
    visit(F);
    // Give up if this function has no allocas
    if (Allocas.empty())
      return false;

    LLVM_DEBUG(dbgs() << "\nChecking function " << F.getName() << "\n");

    // TODO: csetboundsexact and round up sizes
    // Keep around the non-rematerializable cheri_cap_bounds_set code path to
    // compare how much rematerialization can help
    Intrinsic::ID BoundedStackCap = UseRematerializableIntrinsic
                                        ? Intrinsic::cheri_bounded_stack_cap
                                        : Intrinsic::cheri_cap_bounds_set;
    const DataLayout &DL = M->getDataLayout();
    Function *BoundedStackFn =
        Intrinsic::getDeclaration(M, BoundedStackCap, SizeTy);
    StackBoundsMethod BoundsMode = BoundsSettingMode;

    IRBuilder<> B(C);

    for (AllocaInst *AI : Allocas) {
      const uint64_t TotalUses = AI->getNumUses();
      NumProcessed++;
      Function *SetBoundsIntrin = BoundedStackFn;
      // Insert immediately after the alloca
      B.SetInsertPoint(AI);
      B.SetInsertPoint(&*++B.GetInsertPoint());
      Align ForcedAlignment;

      PointerType *AllocaTy = AI->getType();
      assert(isCheriPointer(AllocaTy, &DL));
      Type *AllocationTy = AllocaTy->getElementType();
      Value *ArraySize = B.CreateZExtOrTrunc(AI->getArraySize(), SizeTy);

      // Create a new (AS 0) alloca
      // For imprecise capabilities, we need to increase the alignment for
      // on-stack allocations to ensure that we can create precise bounds.
      if (!TLI->cheriCapabilityTypeHasPreciseBounds()) {
        // If not a constant then definitely a DYNAMIC_STACKALLOC; alignment
        // requirements will be added later during legalisation.
        if (ConstantInt *CI = dyn_cast<ConstantInt>(ArraySize)) {
          uint64_t AllocaSize = DL.getTypeAllocSize(AllocationTy);
          AllocaSize *= CI->getValue().getLimitedValue();
          ForcedAlignment = TLI->getAlignmentForPreciseBounds(AllocaSize);
        }
      }
      AI->setAlignment(max(MaybeAlign(AI->getAlignment()), ForcedAlignment));
      // Only set bounds for allocas that escape this function
      bool NeedBounds = true;
      // Always set bounds if the function has the optnone attribute
      SmallVector<const Use *, 32> UsesThatNeedBounds;
      // If one of the bounded alloca users is a PHI we must reuse the single
      // intrinsic since PHIs must be the first instruction in the basic block
      // and we can't insert anything before. Theoretically we could still
      // use separate intrinsics for the other users but if we are already
      // saving a bounded stack slot we might as well reuse it.
      bool MustUseSingleIntrinsic = false;
      if (BoundsMode == StackBoundsMethod::Never) {
        NeedBounds = false;
      } else {
        CheriNeedBoundsChecker BoundsChecker(AI, DL);
        // With -O0 or =always we set bounds on every stack allocation even
        // if it is not necessary
        bool BoundAll = IsOptNone || BoundsMode == StackBoundsMethod::AllUses;
        BoundsChecker.findUsesThatNeedBounds(&UsesThatNeedBounds, BoundAll,
                                             &MustUseSingleIntrinsic);
        NeedBounds = !UsesThatNeedBounds.empty();
        NumUsesProcessed += TotalUses;
        DBG_MESSAGE(F.getName()
                        << ": " << UsesThatNeedBounds.size() << " of "
                        << TotalUses << " users need bounds for ";
                    AI->dump());
        // TODO: remove the all-or-nothing case
        if (NeedBounds &&
            BoundsMode == StackBoundsMethod::ForAllUsesIfOneNeedsBounds) {
          // We are compiling with the all-or-nothing case and found at least
          // one use that needs bounds -> set bounds on all uses
          UsesThatNeedBounds.clear();
          LLVM_DEBUG(dbgs() << "Checking if alloca needs bounds: "; AI->dump());

          BoundsChecker.findUsesThatNeedBounds(&UsesThatNeedBounds,
                                               /*BoundAllUses=*/true,
                                               &MustUseSingleIntrinsic);
        }
      }
      if (!NeedBounds) {
        NumCompletelyUnboundedAllocas++;
        DBG_MESSAGE("No need to set bounds on stack alloca"; AI->dump());
        continue;
      }

      if (!AI->isStaticAlloca()) {
        NumDynamicAllocas++;
        // TODO: skip bounds on dynamic allocas (maybe add a TLI hook to check
        // whether the backend already adds bounds to the dynamic_stackalloc)
        DBG_MESSAGE("Found dynamic alloca: must use single intrinisic and "
                    "bounds.set intrinisic");
        MustUseSingleIntrinsic = true;
        SetBoundsIntrin =
            Intrinsic::getDeclaration(M, Intrinsic::cheri_cap_bounds_set,
                                      SizeTy);
      }

      // Reuse the result of a single csetbounds intrinisic if we are at -O0 or
      // there are more than N users of this bounded stack capability.
      const bool ReuseSingleIntrinsicCall =
          MustUseSingleIntrinsic || IsOptNone ||
          UsesThatNeedBounds.size() >= SingleIntrinsicThreshold;

      NumUsesWithBounds += UsesThatNeedBounds.size();
      NumUsesWithoutBounds += TotalUses - UsesThatNeedBounds.size();
      // Get the size of the alloca
      unsigned ElementSize = DL.getTypeAllocSize(AllocationTy);
      Value *Size = ConstantInt::get(SizeTy, ElementSize);
      if (AI->isArrayAllocation())
        Size = B.CreateMul(Size, ArraySize);

      if (AI->isStaticAlloca() && ForcedAlignment != Align()) {
        // Pad to ensure bounds don't overlap adjacent objects
        uint64_t AllocaSize =
            cast<ConstantInt>(Size)->getValue().getLimitedValue();
        TailPaddingAmount TailPadding =
          TLI->getTailPaddingForPreciseBounds(AllocaSize);
        if (TailPadding != TailPaddingAmount::None) {
          Type *AllocatedType =
              AI->isArrayAllocation()
                  ? ArrayType::get(
                        AI->getAllocatedType(),
                        cast<ConstantInt>(ArraySize)->getZExtValue())
                  : AI->getAllocatedType();
          Type *PaddingType =
            ArrayType::get(Type::getInt8Ty(F.getContext()),
                           static_cast<uint64_t>(TailPadding));
          Type *TypeWithPadding = StructType::get(AllocatedType, PaddingType);
          auto *NewAI =
            new AllocaInst(TypeWithPadding, AI->getType()->getAddressSpace(),
                           nullptr, "", AI);
          NewAI->takeName(AI);
          NewAI->setAlignment(AI->getAlign());
          NewAI->setUsedWithInAlloca(AI->isUsedWithInAlloca());
          NewAI->setSwiftError(AI->isSwiftError());
          NewAI->copyMetadata(*AI);

          auto *NewPtr = new BitCastInst(NewAI, AI->getType(), "", AI);
          AI->replaceAllUsesWith(NewPtr);
          AI->eraseFromParent();
          AI = NewAI;
          Size =
            ConstantInt::get(SizeTy,
                             AllocaSize + static_cast<uint64_t>(TailPadding));
        }
      }

      if (cheri::ShouldCollectCSetBoundsStats) {
        cheri::addSetBoundsStats(AI->getAlign(), Size, getPassName(),
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

      Value *SingleBoundedAlloc = nullptr;
      if (ReuseSingleIntrinsicCall) {
        NumSingleIntrin++;
        // We need to convert it to an i8* for the intrinisic:
        Instruction *AllocaI8 =
          cast<Instruction>(B.CreateBitCast(AI, I8CapTy));
        SingleBoundedAlloc = B.CreateCall(SetBoundsIntrin, {AllocaI8, Size});
        SingleBoundedAlloc = B.CreateBitCast(SingleBoundedAlloc, AllocaTy);
      }
      for (const Use *U : UsesThatNeedBounds) {
        Instruction *I = cast<Instruction>(U->getUser());
        if (ReuseSingleIntrinsicCall) {
          const_cast<Use *>(U)->set(SingleBoundedAlloc);
        } else {
          if (auto PHI = dyn_cast<PHINode>(I)) {
            // For PHI nodes we can't insert just before the PHI, instead we
            // must insert it just before
            auto BB = PHI->getIncomingBlock(*U);
            // LLVM_DEBUG(dbgs() << "PHI use coming from"; BB->dump());
            B.SetInsertPoint(BB->getTerminator());
          } else {
            // insert a new intrinsic call before every use. This can avoid
            // stack spills but will result in additional instructions.
            // This should avoid spilling registers when using an alloca in a
            // different basic block.
            // TODO: there should be a MIR pass to merge unncessary calls
            B.SetInsertPoint(I);
          }
          // We need to convert it to an i8* for the intrinisic. Note: we must
          // not reuse a bitcast since otherwise that results in spilling the
          // register that was incremented and doing a setbounds in a different
          // basic block. This is stupid and we should be either using the
          // bounded capability everywhere or be doing inc+setoffset in the
          // other block.
          Instruction *AllocaI8 =
            cast<Instruction>(B.CreateBitCast(AI, I8CapTy));
          auto WithBounds = B.CreateCall(SetBoundsIntrin, {AllocaI8, Size});
          const_cast<Use *>(U)->set(B.CreateBitCast(WithBounds, AllocaTy));
        }
      }
    }
    return true;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetPassConfig>();
    AU.setPreservesCFG();
  }
};

} // anonymous namespace

char CheriBoundAllocas::ID;
INITIALIZE_PASS(CheriBoundAllocas, DEBUG_TYPE,
                "CHERI add bounds to alloca instructions", false, false)

ModulePass *llvm::createCheriBoundAllocasPass(void) {
  return new CheriBoundAllocas();
}
