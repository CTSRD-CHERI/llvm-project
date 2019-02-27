#include "Mips.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/PtrUseVisitor.h"
#include "llvm/Analysis/TargetTransformInfo.h"
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
static cl::opt<bool> ReuseSingleIntrinsicCall(
    "cheri-stack-bounds-single-intrinsic-call", cl::init(true),
    cl::desc("Reuse the result of a single CHERI bounds intrinsic"),
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
    "cheri-stack-bounds", cl::desc("Strategy for setting bounds on stack capabilities:"),
    cl::init(StackBoundsMethod::ForAllUsesIfOneNeedsBounds), // TODO: IfNeeded
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

// This is similar to CaptureTracking but we treat way more cases as "captured"

#define DBG_MESSAGE(...) LLVM_DEBUG(dbgs() << DEBUG_TYPE ": " << __VA_ARGS__)
#define DBG_INDENTED(...) DBG_MESSAGE(llvm::right_justify("-", Depth + 1) << __VA_ARGS__)


class StackAllocNeedBoundsChecker {
public:
  StackAllocNeedBoundsChecker(AllocaInst *AI, const DataLayout &DL)
      : AI(AI), DL(DL) {

    AllocaSize = AI->getAllocationSizeInBits(DL);
  }
  bool check(const Use &U) {
    if (!AllocaSize) {
      DBG_MESSAGE("dyanmic size alloca always needs bounds: ";
                  U.getUser()->dump());
      return true;
    }
    APInt CurrentGEPOffset =
        APInt(DL.getIndexSizeInBits(AI->getType()->getAddressSpace()), 0);
    bool Result = useNeedsBounds(U, CurrentGEPOffset, 0);
    if (Result) {
      DBG_MESSAGE("Found alloca use that needs bounds: "; U.getUser()->dump());
    }
    return Result;
  }

  void findUsesThatNeedBounds(SmallVectorImpl<const Use *> *UsesThatNeedBounds,
                              bool BoundAllUses) {
    // TODO: don't replace load/store instructions with the bounded value
    for (const Use &U : AI->uses()) {
      if (BoundAllUses || check(U))
        UsesThatNeedBounds->push_back(&U);
    }
  }

private:
  bool useNeedsBounds(const Use &U, const APInt &CurrentGEPOffset, unsigned Depth) {
    const Instruction *I = cast<Instruction>(U.getUser());
    // Value *V = U->get();
    if (Depth > 10) {
      DBG_INDENTED("reached max depth, assuming bounds needed.");
    }

    switch (I->getOpcode()) {
    case Instruction::Call:
    case Instruction::Invoke: {
      auto CI = cast<CallBase>(I);
      switch (CI->getIntrinsicID()) {
      case Intrinsic::not_intrinsic:
          // not an intrinsic call -> always need bounds:
          DBG_INDENTED("Adding stack bounds since it is passed to call: ";
                    I->dump());
          return true;
      case Intrinsic::lifetime_start:
      case Intrinsic::lifetime_end:
        DBG_INDENTED("No need for stack bounds for lifetime_{start,end}: ";
                     I->dump());
        return false;
      case Intrinsic::dbg_declare:
      case Intrinsic::dbg_value:
      case Intrinsic::dbg_label:
        DBG_INDENTED("No need for stack bounds for dbg_{declare,value,label}: ";
                     I->dump());
        return false;
      case Intrinsic::memset:
      case Intrinsic::memcpy:
      case Intrinsic::memmove:
        // TODO: an inline-expanded memset/memcpy/memmove doesn't need bounds!
        DBG_INDENTED("Adding stack bounds for memset/memcpy/memmove: ";
                     I->dump());
        return true;
      default:
        errs() << DEBUG_TYPE << ": Don't know how to handle intrinsic. Assuming bounds needed"; I->dump();
        DBG_INDENTED("Adding stack bounds for unknown intrinsic call: ";
                     I->dump());
        return true;
      }
    }
    case Instruction::Load:
      // Storing to a stack slot does not need bounds if the offset is known to
      // be within the alloca. We can just use cs* offset($csp) istead of adding
      // a cincoffset+csetbouds.
      return canLoadStoreBeOutOfBounds(I, CurrentGEPOffset, Depth);
    case Instruction::Store:
      // Storing to a stack slot does not need bounds if the offset is known to
      // be within the alloca. We can just use cs* offset($csp) istead of adding
      // a cincoffset+csetbouds.
      return canLoadStoreBeOutOfBounds(I, CurrentGEPOffset, Depth);

    case Instruction::AtomicRMW:
    case Instruction::AtomicCmpXchg:
      // cmpxchg and rmw are the same as load/store: if it is a fixed stack
      // slot we can omit the bounds
      return canLoadStoreBeOutOfBounds(I, CurrentGEPOffset, Depth);

    case Instruction::GetElementPtr: {
      auto GEPI = cast<GetElementPtrInst>(I);
      if (!GEPI->isInBounds() && !GEPI->hasAllConstantIndices()) {
        DBG_INDENTED("Adding stack bounds since GEP is not all constants: ";
                    I->dump());
        return true;
      }
      APInt NewGepOffset(CurrentGEPOffset);
      if (!GEPI->accumulateConstantOffset(DL, NewGepOffset)) {
        DBG_INDENTED("Could not accumulate constant GEP Offset -> need bounds: ";
                    I->dump());
      }
      return anyUserNeedsBounds(GEPI, NewGepOffset, Depth);
    }
    case Instruction::PHI:
    case Instruction::Select:
    case Instruction::BitCast:
    case Instruction::AddrSpaceCast:
      // If any use of the cast/phi/selects needs bounds then we must add bounds
      // for the initial value.
      return anyUserNeedsBounds(I, CurrentGEPOffset, Depth);
    case Instruction::ICmp:
      // comparisons need bounds since cexeq takes bounds into account.
      // TODO: when only comparing addresses we might not need them
      DBG_INDENTED("Adding stack bounds since comparisons need bounds: ";
                  I->dump());
      return true;
    case Instruction::Ret:
      DBG_INDENTED("Adding stack bounds for alloca that is returned: ";
                  I->dump());
      return true;
    default:
      // Something else - be conservative and say it needs bounds.
      errs() << "DON'T know how to handle "; I->dump();
      DBG_INDENTED("Adding stack bounds since don't know how to handle: ";
                  I->dump());
      return true;
    }
  }

  bool anyUserNeedsBounds(const Instruction *I, const APInt &CurrentGEPOffset, unsigned Depth) {
    DBG_INDENTED("Checking if " << I->getOpcodeName() << " needs stack bounds: ";
                I->dump());
    for (const Use &U : I->uses()) {
      if (useNeedsBounds(U, CurrentGEPOffset, Depth + 1)) {
        DBG_INDENTED("Adding stack bounds since " << I->getOpcodeName()
                                                 << " user needs bounds: ";
                    U.getUser()->dump());
        return true;
      }
    }
    DBG_INDENTED("no " << I->getOpcodeName() << " users need bounds: ";
                I->dump());
    return false;
  }

  bool canLoadStoreBeOutOfBounds(const Instruction *I,
                                 const APInt &CurrentGEPOffset, unsigned Depth) {
    DBG_INDENTED("Checking if load/store needs bounds (GEP offset is "
                    << CurrentGEPOffset << "): ";
                I->dump());
    assert(AllocaSize &&
           "dynamic size alloca should have been checked earlier");
    Type *LoadStoreType = nullptr;
    if (auto CmpXchg = dyn_cast<AtomicCmpXchgInst>(I)) {
      LoadStoreType = CmpXchg->getNewValOperand()->getType();
    } else if (auto RMW = dyn_cast<AtomicRMWInst>(I)) {
      LoadStoreType = RMW->getValOperand()->getType();
    } else if (const auto *Store = dyn_cast<StoreInst>(I)) {
      LoadStoreType = Store->getValueOperand()->getType();
    } else if (const auto *Load = dyn_cast<LoadInst>(I)) {
      LoadStoreType = Load->getType();
    } else {
      llvm_unreachable("Invalid load/store type");
    }
    auto Size = DL.getTypeStoreSize(LoadStoreType);
    assert((*AllocaSize % 8) == 0 && "AllocaSize must be a multiple of 8 bits");
    uint64_t AllocaSizeInBytes = (*AllocaSize / 8);
    DBG_INDENTED("Load/store size="
                    << Size << ", alloca size=" << AllocaSizeInBytes
                    << ", current GEP offset=" << CurrentGEPOffset << " for ";
                LoadStoreType->dump(););
    APInt Zero(CurrentGEPOffset.getBitWidth(), 0);
    APInt Max(CurrentGEPOffset.getBitWidth(), AllocaSizeInBytes);
    APInt LastAddr = CurrentGEPOffset + Size;
    if (CurrentGEPOffset.slt(Zero) || LastAddr.sgt(Max)) {
      DBG_INDENTED(I->getFunction()->getName()
                      << ": stack load/store offset OUT OF BOUNDS -> adding "
                         "csetbounds: ";
                  I->dump());
      return true;
    }
    DBG_INDENTED("Load/store is in bounds -> can reuse $csp for "; I->dump(););
    return false;
  }

  const AllocaInst *AI;
  const DataLayout &DL;
  Optional<uint64_t> AllocaSize;
};

class CheriPureCapABI : public ModulePass, public InstVisitor<CheriPureCapABI> {
  Module *M;
  llvm::SmallVector<AllocaInst *, 16> Allocas;
  bool IsCheri128;


public:
  static char ID;
  CheriPureCapABI() : ModulePass(ID) {}
  StringRef getPassName() const override { return "CHERI sandbox ABI setup"; }
  void visitAllocaInst(AllocaInst &AI) { Allocas.push_back(&AI); }
  bool runOnModule(Module &Mod) override {
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
    // always set bounds with optnone
    bool IsOptNone = F.hasFnAttribute(Attribute::OptimizeNone);
    // FIXME: should still ignore lifetime-start + lifetime-end intrinsics even at -O0


    // TargetTransformInfo& TTI = getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

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
    Function *BoundedStackFn = Intrinsic::getDeclaration(M, BoundedStackCap);
    StackBoundsMethod BoundsMode = BoundsSettingMode;

    IRBuilder<> B(C);
    const DataLayout &DL = F.getParent()->getDataLayout();

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
        // TODO: do a sane calculation based on capability precision
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
      // Only set bounds for allocas that escape this function
      bool NeedBounds = true;
      // Always set bounds if the function has the optnone attribute
      SmallVector<const Use *, 32> UsesThatNeedBounds;
      if (BoundsMode == StackBoundsMethod::Never) {
        NeedBounds = false;
      } else {
        StackAllocNeedBoundsChecker BoundsChecker(AI, DL);
        // With -O0 or =always we set bounds on every stack allocation even
        // if it is not necessary
        bool BoundAll = IsOptNone || BoundsMode == StackBoundsMethod::AllUses;
        BoundsChecker.findUsesThatNeedBounds(&UsesThatNeedBounds, BoundAll);
        NeedBounds = !UsesThatNeedBounds.empty();
        DBG_MESSAGE(F.getName()
                        << ": " << UsesThatNeedBounds.size() << " of "
                        << AI->getNumUses() << " users need bounds for ";
                    AI->dump());
        // TODO: remove the all-or-nothing case
        if (NeedBounds &&
            BoundsMode == StackBoundsMethod::ForAllUsesIfOneNeedsBounds) {
          // We are compiling with the all-or-nothing case and found at least
          // one use that needs bounds -> set bounds on all uses
          UsesThatNeedBounds.clear();
          LLVM_DEBUG(dbgs() << "Checking if alloca needs bounds: "; AI->dump());

          BoundsChecker.findUsesThatNeedBounds(&UsesThatNeedBounds,
                                               /*BoundAllUses=*/true);
        }
      }
      if (!NeedBounds) {
        DBG_MESSAGE("No need to set bounds on stack alloca"; AI->dump());
        continue;
      }

      // We need at least one setbounds -> create the bitcast
      Instruction *BitCast =
          cast<Instruction>(B.CreateBitCast(AI, Type::getInt8PtrTy(C, 200)));
      // Get the size of the alloca
      unsigned ElementSize = DL.getTypeAllocSize(AllocationTy);
      Value *Size = ConstantInt::get(Type::getInt64Ty(C), ElementSize);
      if (AI->isArrayAllocation())
        Size = B.CreateMul(Size, AI->getArraySize());
      Value *AllocaI8 = BitCast;

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

      Value *SingleBoundedAlloc = nullptr;
      if (ReuseSingleIntrinsicCall) {
        SingleBoundedAlloc = B.CreateCall(BoundedStackFn, {AllocaI8, Size});
        SingleBoundedAlloc = B.CreateBitCast(SingleBoundedAlloc, AllocaTy);
      }
      for (const Use *U : UsesThatNeedBounds) {
        Instruction *I = cast<Instruction>(U->getUser());
        assert(I != BitCast);
        if (ReuseSingleIntrinsicCall) {
          const_cast<Use *>(U)->set(SingleBoundedAlloc);
        } else {
          // insert a new intrinsic call before every use. This can avoid
          // stack spills but will result in additional instructions.
          // This should avoid spilling registers when using an alloca in a
          // different basic block
          // TODO: there should be a MIR pass to merge unncessary calls
          B.SetInsertPoint(I);
          auto WithBounds = B.CreateCall(BoundedStackFn, {AllocaI8, Size});
          const_cast<Use *>(U)->set(B.CreateBitCast(WithBounds, AllocaTy));
        }
      }
    }
    return true;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.setPreservesCFG();
  }
};

} // anonymous namespace

char CheriPureCapABI::ID;
INITIALIZE_PASS(CheriPureCapABI, DEBUG_TYPE,
                "CHERI add bounds to alloca instructions", false, false)

namespace llvm {
ModulePass *createCheriPureCapABI(void) { return new CheriPureCapABI(); }
}
