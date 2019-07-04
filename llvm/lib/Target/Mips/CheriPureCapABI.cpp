#include "Mips.h"
#include "llvm/ADT/Statistic.h"
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

#include "cheri-compressed-cap/cheri_compressed_cap.h"

#define DEBUG_TYPE "cheri-purecap-alloca"


using namespace llvm;
using std::pair;

STATISTIC(NumProcessed,  "Number of allocas that were analyzed for CHERI bounds");
STATISTIC(NumDynamicAllocas,  "Number of dyanmic allocas that were analyzed"); // TODO: skip them
STATISTIC(NumUsesProcessed, "Total number of alloca uses that were analyzed");
STATISTIC(NumCompletelyUnboundedAllocas, "Number of allocas where CHERI bounds were completely unncessary");
STATISTIC(NumUsesWithBounds, "Number of alloca uses that had CHERI bounds added");
STATISTIC(NumUsesWithoutBounds, "Number of alloca uses that did not needed CHERI bounds");
STATISTIC(NumSingleIntrin, "Number of times that a single intrinisic was used instead of per-use");

// FIXME: if the stack variable itself is stored this will break
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
    bool Result = useNeedsBounds(U, CurrentGEPOffset, 1);
    if (Result) {
      DBG_MESSAGE("Found alloca use that needs bounds: "; U.getUser()->dump());
    }
    return Result;
  }

  void findUsesThatNeedBounds(SmallVectorImpl<const Use *> *UsesThatNeedBounds,
                              bool BoundAllUses, bool *MustUseSingleIntrinsic) {
    // TODO: don't replace load/store instructions with the bounded value
    for (const Use &U : AI->uses()) {
      if (BoundAllUses || check(U)) {
        UsesThatNeedBounds->push_back(&U);
#if 0  // This should now be fixed
        // See comment further down as to why we must reuse a single intrinsic
        // if one of the bounded users is a phi node.
        if (isa<PHINode>(U.getUser()))
          *MustUseSingleIntrinsic = true;
#endif
      }
    }
  }

private:
  bool useNeedsBounds(const Use &U, const APInt &CurrentGEPOffset, unsigned Depth) {
    const Instruction *I = cast<Instruction>(U.getUser());
    // Value *V = U->get();
    if (Depth > 10) {
      DBG_INDENTED("reached max depth, assuming bounds needed.\n");
      return true;
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
      case Intrinsic::memset_element_unordered_atomic:
      case Intrinsic::memcpy:
      case Intrinsic::memcpy_element_unordered_atomic:
      case Intrinsic::memmove:
      case Intrinsic::memmove_element_unordered_atomic:
        // TODO: an inline-expanded memset/memcpy/memmove doesn't need bounds!
        DBG_INDENTED("Adding stack bounds for memset/memcpy/memmove: ";
                     I->dump());
        return true;

      case Intrinsic::vastart:
      case Intrinsic::vacopy:
      case Intrinsic::vaend:
        // TODO: are bounds always needed?
        DBG_INDENTED("Adding stack bounds for va_start/va_copy/va_end: ";
                     I->dump());
        return true;

      case Intrinsic::cheri_bounded_stack_cap:
      case Intrinsic::cheri_cap_address_get:
      case Intrinsic::cheri_cap_address_set:
      case Intrinsic::cheri_cap_base_get:
      case Intrinsic::cheri_cap_bounds_set:
      case Intrinsic::cheri_cap_bounds_set_exact:
      case Intrinsic::cheri_cap_build:
      case Intrinsic::cheri_cap_conditional_seal:
      case Intrinsic::cheri_cap_diff:
      case Intrinsic::cheri_cap_from_ddc:
      case Intrinsic::cheri_cap_from_pointer:
      case Intrinsic::cheri_cap_length_get:
      case Intrinsic::cheri_cap_load_tags:
      case Intrinsic::cheri_cap_offset_get:
      case Intrinsic::cheri_cap_offset_increment:
      case Intrinsic::cheri_cap_offset_set:
      case Intrinsic::cheri_cap_perms_and:
      case Intrinsic::cheri_cap_perms_check:
      case Intrinsic::cheri_cap_perms_get:
      case Intrinsic::cheri_cap_flags_set:
      case Intrinsic::cheri_cap_flags_get:
      case Intrinsic::cheri_cap_seal:
      case Intrinsic::cheri_cap_sealed_get:
      case Intrinsic::cheri_cap_subset_test:
      case Intrinsic::cheri_cap_tag_clear:
      case Intrinsic::cheri_cap_tag_get:
      case Intrinsic::cheri_cap_to_pointer:
      case Intrinsic::cheri_cap_type_check:
      case Intrinsic::cheri_cap_type_copy:
      case Intrinsic::cheri_cap_type_get:
      case Intrinsic::cheri_cap_unseal:
        // CHERI intrinsics might depend on the bounds
        DBG_INDENTED("Adding stack bounds for alloca used in CHERI intrinsic:";
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
      // Loading from a stack slot does not need bounds if the offset is known to
      // be within the alloca. We can just use cl* offset($csp) istead of adding
      // a cincoffset+csetbouds.
      return canLoadStoreBeOutOfBounds(I, U, CurrentGEPOffset, Depth);
    case Instruction::Store:
      // Storing to a stack slot does not need bounds if the offset is known to
      // be within the alloca. We can just use cs* offset($csp) istead of adding
      // a cincoffset+csetbouds.
      return canLoadStoreBeOutOfBounds(I, U, CurrentGEPOffset, Depth);

    case Instruction::AtomicRMW:
    case Instruction::AtomicCmpXchg:
      // cmpxchg and rmw are the same as load/store: if it is a fixed stack
      // slot we can omit the bounds
      return canLoadStoreBeOutOfBounds(I, U, CurrentGEPOffset, Depth);

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
        return true;
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

  bool canLoadStoreBeOutOfBounds(const Instruction *I, const Use& U,
                                 const APInt &CurrentGEPOffset, unsigned Depth) {
    DBG_INDENTED("Checking if load/store needs bounds (GEP offset is "
                    << CurrentGEPOffset << "): ";
                I->dump());
    Depth++; // indent the next debug message more
    assert(AllocaSize &&
           "dynamic size alloca should have been checked earlier");
    Type *LoadStoreType = nullptr;
    if (auto CmpXchg = dyn_cast<AtomicCmpXchgInst>(I)) {
      if (U.get() != CmpXchg->getPointerOperand()) {
        DBG_INDENTED("Stack slot used as value and not pointer -> must set bounds\n";);
        return true;
      }
      LoadStoreType = CmpXchg->getNewValOperand()->getType();
    } else if (auto RMW = dyn_cast<AtomicRMWInst>(I)) {
      LoadStoreType = RMW->getValOperand()->getType();
      if (U.get() != RMW->getPointerOperand()) {
        DBG_INDENTED("Stack slot used as value and not pointer -> must set bounds\n";);
        return true;
      }
    } else if (const auto *Store = dyn_cast<StoreInst>(I)) {
      if (U.get() != Store->getPointerOperand()) {
        DBG_INDENTED("Stack slot used as value and not pointer -> must set bounds\n";);
        return true;
      }
      LoadStoreType = Store->getValueOperand()->getType();
    } else if (const auto *Load = dyn_cast<LoadInst>(I)) {
      assert(U.get() == Load->getPointerOperand() && "Invalid load?");
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

#if 0
    auto &DL = Mod.getDataLayout();
    if (IsCheri128) {
      for (auto &G : Mod.globals()) {
        // Adjust the alignment for CHERI128 global variable definitions to
        // ensure that we can precisely represent bounds.
        // We can increase alignment for exact definitions and common
        // definitions:
        if (G.canIncreaseAlignment() || G.hasCommonLinkage()) {
          auto Size = DL.getTypeAllocSize(G.getValueType());
          uint64_t Alignment = G.getAlignment();
          uint64_t ReqAlignment = cc128_get_required_alignment(Size);
          if (ReqAlignment > Alignment) {
            ReqAlignment = std::min(ReqAlignment, UINT64_C(1) << 31);
            LLVM_DEBUG(dbgs() << "\nIncreased alignment for global from "
                              << Alignment << " to " << ReqAlignment << ": ";
                       G.dump(););
            G.setAlignment(ReqAlignment);
          }
        }
      }
    }
#endif
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
    const DataLayout &DL = F.getParent()->getDataLayout();
    Type *SizeTy = Type::getIntNTy(C, DL.getIndexSizeInBits(200));
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
        ForcedAlignment = cc128_get_required_alignment(AllocaSize);
        // MIPS doesn't support stack alignments greater than 2^16
        ForcedAlignment = std::min(ForcedAlignment, 0x4000U);
      }
      AI->setAlignment(std::max(AI->getAlignment(), ForcedAlignment));
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
        StackAllocNeedBoundsChecker BoundsChecker(AI, DL);
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
      Value *Size = ConstantInt::get(Type::getInt64Ty(C), ElementSize);
      if (AI->isArrayAllocation())
        Size = B.CreateMul(Size, AI->getArraySize());

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
        NumSingleIntrin++;
        // We need to convert it to an i8* for the intrinisic:
        Instruction *AllocaI8 =
          cast<Instruction>(B.CreateBitCast(AI, Type::getInt8PtrTy(C, 200)));
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
            cast<Instruction>(B.CreateBitCast(AI, Type::getInt8PtrTy(C, 200)));
          auto WithBounds = B.CreateCall(SetBoundsIntrin, {AllocaI8, Size});
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
