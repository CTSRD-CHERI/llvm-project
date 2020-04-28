//===-- llvm/CodeGen/CheriLogSetBoundsPass.cpp ------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Transforms/Utils/CheriSetBounds.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

namespace {

/// This pass frees the MachineFunction object associated with a Function.
class LogCheriSetBounds : public FunctionPass {
public:
  static char ID;

  LogCheriSetBounds() : FunctionPass(ID) {
    assert(cheri::ShouldCollectCSetBoundsStats &&
           "Pass should only be created when we are logging bouds");
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    assert(cheri::ShouldCollectCSetBoundsStats &&
           "Should not have been created otherwise");
    // errs() << "Logging bounds for " << F.getName() << "\n";
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    AssumptionCache *AC =
        &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
    assert(AC && DT && "Cannot be non-null");
    // For MIPS we can guess the size here by multiplying by 4:
    const DataLayout &DL = F.getParent()->getDataLayout();
    for (auto &BB : F) {
      for (Instruction &I : BB) {
        // TODO: CallBase
        Function *CalledFunc = nullptr;
        AttributeList AL;
        // TODO: use CallBase when we merge upstream
        if (const CallInst *CI = dyn_cast<CallInst>(&I)) {
          CalledFunc = CI->getCalledFunction();
          AL = CI->getAttributes();
        } else if (const InvokeInst *II = dyn_cast<InvokeInst>(&I)) {
          CalledFunc = II->getCalledFunction();
          AL = II->getAttributes();
        }
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
              Instruction* Ctx = &BB.back();
              uint64_t Alignment = getKnownAlignment(&I, DL, Ctx, AC, DT);
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
                  Align(Alignment), KnownSize, "function with alloc_size",
                  cheri::SetBoundsPointerSource::Heap,
                  "call to " +
                      (CalledFunc ? CalledFunc->getName() : "function pointer"),
                  cheri::inferSourceLocation(&I), SizeMultipleOf);
            };
        // We can only do this analysis on calls that return pointers
        if (!I.getType()->isPointerTy())
          continue;

        // Check both the target function and the CallInst for alloc_size attrs:
        if (CalledFunc) {
          if (CalledFunc->hasFnAttribute(Attribute::AttrKind::AllocSize)) {
            Attribute Attr =
                CalledFunc->getFnAttribute(Attribute::AttrKind::AllocSize);
            LogAllocSize(Attr.getAllocSizeArgs());
            continue;
          }
        }
        // no attr on the called func, try call site too
        if (AL.hasFnAttribute(Attribute::AttrKind::AllocSize)) {
          LogAllocSize(AL.getFnAttributes().getAllocSizeArgs());
        }
      }
    }
    // TODO: get machine function and count instructions?
    // Probably better to get the code bounds from the ELF writer instead
    // MF.getInstructionCount();
    return false;
  }

  StringRef getPassName() const override { return "Log CHERI SetBounds Stats"; }
};

} // end anonymous namespace

char LogCheriSetBounds::ID;

FunctionPass *llvm::createLogCheriSetBoundsPass() {
  static bool created = false;
  assert(!created && "Should only be created once");
  created = true;
  return new LogCheriSetBounds();
}
