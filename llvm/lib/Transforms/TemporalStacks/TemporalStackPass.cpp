/*-
 * Copyright (c) 2019 Lawrence Esswood
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract FA8750-10-C-0237
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Transforms/Utils/CheriSetBounds.h"
#include "llvm/IR/Cheri.h"
#include "EscapeAnalysis/EscapeAnalysis.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Transforms/CHERICap.h"

using namespace llvm;
using namespace llvm::cheri;

#define DEBUG_TYPE "temporal"

namespace llvm {

namespace cheri {

STATISTIC(RAN_ON_FUNCS, "Number of functions the temporal pass was ran on");
STATISTIC(FUNCS_ALL_SAFE, "Number of functions that won't need any unsafe space due to analysis");
STATISTIC(TOTAL_SAFE, "Total number of safe allocations");
STATISTIC(TOTAL_UNSAFE, "Total number of unsafe allocations");
STATISTIC(FUNCS_HAVE_ALLOCS, "Total number of functions that have allocas");

struct Temporal : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  EscapeAnalyser analyser;
  analysis_type type;
  size_t expandLimit;
  Temporal(bool ReadOnlyEscapes, analysis_type Type, int ExpandLimit) :  FunctionPass(ID), analyser(ReadOnlyEscapes), type(Type),expandLimit(ExpandLimit)
  {}

  bool runOnFunction(Function &F) override;

  // Only attaches meta-data
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

public:
  static llvm::MDNode* GetNodeForSafety(AllocaInst *Inst, bool safe);

  bool GetSafetyOf(AllocaInst *Inst);


};

}
}

char llvm::cheri::Temporal::ID = 0;

bool Temporal::runOnFunction(Function &F) {

  RAN_ON_FUNCS++;

  if(type == NONE) return false;

  if(type >= ANALYSIS) {
    analyser.reset();
    analyser.addRootFunction(F, expandLimit);
  }

  bool all_safe = true;
  bool any = false;
  for (auto &BB : F) {
    for (Instruction &I : BB) {
      if (auto allocaInst = dyn_cast<AllocaInst>(&I)) {
        llvm::MDNode *tagged = I.getMetadata("temporal");

        if(!any) {
          any = true;
          FUNCS_HAVE_ALLOCS++;
        }

        if(!tagged) {
          bool safe = GetSafetyOf(allocaInst);

          (safe ? TOTAL_SAFE : TOTAL_UNSAFE)++;

          all_safe &= safe;

          if(!safe && type == ANALYSIS_ALL_SAFE) {
            // Unless the user over-rides, if something is found unsafe in this mode it is an error
            auto var = cheri::inferLocalVariableName(allocaInst);
            auto loc = cheri::inferSourceLocation(&I);
            report_fatal_error(Twine("Local Variable ") + var + " at " + loc + " was found to be unsafe in analysis_all_safe mode. Either tag it safe, or don't let it escape.", false);
          }
          llvm::MDNode *analysis = GetNodeForSafety(allocaInst, safe);
          I.setMetadata("temporal", analysis);

        }
      }
    }
  }

  if(all_safe && any) FUNCS_ALL_SAFE++;

  return false;
}

bool Temporal::GetSafetyOf(AllocaInst *Inst) {

  bool Safe;

  if (type == ALL_SAFE) {
    Safe = true;
  } else if (type == ALL_UNSAFE) {
    Safe = false;
  } else if (type == USE_CAPTURE_TRACK) {
    Safe = !PointerMayBeCaptured(Inst, true, true);
  } else {
    Safe = analyser.IsValueDirectlySafe(Inst);
  }

  return Safe;
}

llvm::MDNode *Temporal::GetNodeForSafety(AllocaInst *Inst, bool safe) {
  LLVMContext &C = Inst->getContext();
  llvm::MDNode *auto_node = llvm::MDNode::get(C, llvm::MDString::get(C, safe ? "safe" : "unsafe"));
  return auto_node;
}

Pass *llvm::createCHERISafeStacksPass() {
  if(get_temporal_analysis_type() == NONE) return nullptr;
  return new Temporal(false, llvm::get_temporal_analysis_type(), llvm::get_temporal_expand_limit());
}


static void loadPass(const PassManagerBuilder &Builder, legacy::PassManagerBase &PM) {
  PM.add(createCHERISafeStacksPass());
}

/* If this is loaded
static RegisterStandardPasses clangtoolLoader_Ox(PassManagerBuilder::EP_OptimizerLast, loadPass);
static RegisterStandardPasses clangtoolLoader_O0(PassManagerBuilder::EP_EnabledOnOptLevel0, loadPass);
*/