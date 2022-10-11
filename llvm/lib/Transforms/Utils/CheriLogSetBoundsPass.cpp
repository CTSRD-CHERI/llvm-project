//===-- llvm/CodeGen/CheriLogSetBoundsPass.cpp ------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/CheriLogSetBounds.h"

#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Transforms/Utils/CheriSetBounds.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

namespace {

/// This should really be in Analysis/ but it depends on
/// getOrEnforceKnownAlignment which is in Transforms/Scalar.
class CheriLogSetBoundsLegacyPass : public FunctionPass {
public:
  static char ID;

  CheriLogSetBoundsLegacyPass() : FunctionPass(ID) {
    assert(cheri::ShouldCollectCSetBoundsStats &&
           "Pass should only be created when we are logging bouds");
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    AssumptionCache &AC =
        getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
    runImpl(F, DT, AC);
    return false;
  }
  static void runImpl(Function &F, DominatorTree &DT, AssumptionCache &AC) {
    assert(cheri::ShouldCollectCSetBoundsStats &&
           "Should not have been created otherwise");
    // errs() << "Logging bounds for " << F.getName() << "\n";
    // For MIPS we can guess the size here by multiplying by 4:
    const DataLayout &DL = F.getParent()->getDataLayout();
    for (auto &BB : F) {
      for (Instruction &I : BB) {
        const CallBase *CB = dyn_cast<CallBase>(&I);
        if (!CB)
          continue; // Not a call
        Function *CalledFunc = CB->getCalledFunction();
        const auto LogAllocSize =
            [&](std::pair<unsigned, Optional<unsigned>> AllocSize) {
              // TODO: should we also log this for non-capabilities?
              if (!DL.isFatPointer(I.getType()))
                return;
              // Without the assumption cache we don't get any benefit from
              // assume_aligned attributes and getKnownAlignment will return 1.
              // In order to get a context instruction that can be used by the
              // llvm.assume, we simply use the last instruction in the same
              // basic block since the asssume will always dominated it.
              // See llvm::isValidAssumeForContext()
              Instruction *Ctx = &BB.back();
              Align Alignment = getKnownAlignment(&I, DL, Ctx, &AC, &DT);
              Optional<uint64_t> KnownSize;
              Optional<uint64_t> SizeMultipleOf;

              Optional<uint64_t> FirstConstant;
              Optional<uint64_t> SecondConstant;
              if (I.getNumOperands() > AllocSize.first)
                FirstConstant =
                    cheri::inferConstantValue(I.getOperand(AllocSize.first));
              if (AllocSize.second && I.getNumOperands() > *AllocSize.second)
                SecondConstant =
                    cheri::inferConstantValue(I.getOperand(*AllocSize.second));
              if (FirstConstant) {
                if (!AllocSize.second) {
                  // If we used the one-argument version of allocsize this is
                  // the total known size
                  KnownSize = *FirstConstant;
                } else {
                  // If there is a second argument that also needs to be a
                  // constant so that we can infer the size
                  if (SecondConstant) {
                    KnownSize = *FirstConstant * *SecondConstant;
                  } else {
                    // Otherwise can only infer that it is a multiple of N
                    SizeMultipleOf = FirstConstant;
                  }
                }
              } else if (SecondConstant) {
                // First argument is not a constant, but if the second one is we
                // can at least infer the known multiple
                SizeMultipleOf = SecondConstant;
              }
              // Assume zero for now, parse assume_aligned bits?
              // Assume that all alloc_size functions allocate on the heap.
              // This may not be quite true since some might use shared memory
              // but shouldn't really matter for analysis purposes
              cheri::CSetBoundsStats->add(
                  Alignment, KnownSize, "function with alloc_size",
                  cheri::SetBoundsPointerSource::Heap,
                  "call to " +
                      (CalledFunc ? CalledFunc->getName() : "function pointer"),
                  cheri::inferSourceLocation(&I), SizeMultipleOf);
            };
        // Check both the target function and the CallBase for alloc_size attrs:
        if (CalledFunc &&
            CalledFunc->hasFnAttribute(Attribute::AttrKind::AllocSize)) {
          Attribute Attr =
              CalledFunc->getFnAttribute(Attribute::AttrKind::AllocSize);
          LogAllocSize(Attr.getAllocSizeArgs());
          continue;
        }
        // no attr on the called func, try call site too
        if (CB->hasFnAttr(Attribute::AttrKind::AllocSize)) {
          LogAllocSize(CB->getAttributes().getFnAttrs().getAllocSizeArgs().value());
        }
      }
    }
    // TODO: get machine function and count instructions?
    // Probably better to get the code bounds from the ELF writer instead
    // MF.getInstructionCount();
  }

  StringRef getPassName() const override { return "Log CHERI SetBounds Stats"; }
};

} // end anonymous namespace

char CheriLogSetBoundsLegacyPass::ID;

FunctionPass *llvm::createLogCheriSetBoundsPass() {
  static bool created = false;
  assert(!created && "Should only be created once");
  created = true;
  return new CheriLogSetBoundsLegacyPass();
}

PreservedAnalyses CheriLogSetBoundsPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  auto &AC = AM.getResult<AssumptionAnalysis>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  CheriLogSetBoundsLegacyPass::runImpl(F, DT, AC);
  return PreservedAnalyses::all();
}
