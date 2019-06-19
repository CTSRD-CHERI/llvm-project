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

#include <llvm/IR/Operator.h>
#include "llvm/IR/Function.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Support/Debug.h"
#include "BitTree.h"
#include "EscapeAnalysis.h"
#include "llvm/Transforms/Utils/CheriSetBounds.h"
#include "llvm/IR/Cheri.h"

namespace llvm {
namespace cheri {

const SafetyState SafetyState::SafeState = SafetyState(BitTrees::AllZeros,BitTrees::AllZeros);
const SafetyState SafetyState::UnsafeState = SafetyState(BitTrees::AllOnes,BitTrees::AllOnes);
const SafetyState SafetyState::UnsafeToDeriveFrom = SafetyState(BitTrees::AllOnes,BitTrees::AllZeros);

inline bool isUnsafeTrackable(const Value *I) {
  return isCheriPointer(I->getType(), nullptr) && !isa<Function>(I);
}

CallSort EscapeAnalyser::getSort(const Instruction &I, const Use* Use) {

  ImmutableCallSite CS(&I);

  bool safeIntrinsic;

  CallSort result = SORT_SAFE;

  switch(CS.getIntrinsicID()) {
    // FIXME: Go through and make a bunch of these safe. Then make the default unsafe.
  default:
    safeIntrinsic = true;
    break;
  case Intrinsic::memcpy:
  case Intrinsic::memmove:
    return MEMCPY;
  case Intrinsic::vastart:
    return VA_START;
  case Intrinsic::not_intrinsic:
    safeIntrinsic = false;
  }

  // I want memcpy to be considered memcpy even in freestanding. Use the name.
  auto CF = CS.getCalledFunction();
  if(CF && CF->hasName()) {
    StringRef name = CF->getName();
    if(name.equals("memcpy") || name.equals("memmove")) return MEMCPY;
  }

  bool retCap = isUnsafeTrackable(&I);

  if(retCap && (Use == nullptr || (CS.isArgOperand(Use) && !CS.doesNotCapture(CS.getArgumentNo(Use))))) {
    result = (CallSort)(result | DERIVES_RETURN_FROM_ARGUMENTS);
  }

  if(!safeIntrinsic) {

    if((readOnlyEscapes || !CS.onlyReadsMemory()) && !isIntrinsicReturningPointerAliasingArgumentWithoutCapturing(CS))
      result = (CallSort)(result | LEAKS_ARGUMENTS);

    if(retCap)
      result = (CallSort)(result | UNSAFE_RETURN);

  }

  return result;
}

bool cheri::EscapeAnalyser::DecreaseSafety(StaticFuncionAnalysis *FA, const Value *V, SafetyState newConstraint) {
  if(newConstraint == SafetyState::SafeState) return false; // everything not in the map by the end of the pass is safe, don't add things explicitly
  assert(V);

  if(!isUnsafeTrackable(V)) {
    V->dump();
    assert(0 && "Only for CHERI pointers");
  }

  auto cache = &FA->resultCache;
  auto lookup = cache->insert(std::pair<const Value*, SafetyState>(V, newConstraint));


  if(!lookup.second) {
    // We have already seen this value. The new result might still be stronger.

    SafetyState current = lookup.first->second;

    newConstraint = SafetyState::getMaximum(newConstraint, current);

    if(newConstraint == current) return false;

    // Otherwise update and then re-process
    lookup.first->second = newConstraint;
  }

  // All pointer types are capabilities, and non capabilities cannot leak. Therefore for all uses that do not store/load/derive permissions we can stop tracking

  // We use the over-aproximation that every load/store via the same pointer alias. e.g. if any capability is loaded and then leaked, every store to that location is unsafe

  const Value * storeVal;
  const Value * storePtr;
  const Value * loadPtr;
  const Value * loadVal;

  // These are the implied safeties of the OTHER pointers

  // If we use this as a pointer
  SafetyState safetyLoadedFrom = newConstraint.getPropagationForLoadedFrom();
  SafetyState safetyStoredTo = newConstraint.getPropagationForStoredTo();

  // If we use this as a value
  SafetyState safetyIfStored = newConstraint.getPropagationForStored();
  SafetyState safetyIfLoaded = newConstraint.getPropagationForLoaded();

  // For things this derives from
  SafetyState safetyDerivedFrom = newConstraint.getPropagationForDerivedFrom();

  // And things that derive from this
  SafetyState safetyDerivedBy = newConstraint.getPropagationForDerivedBy();

  // When we recurse we may get a stronger result for this value. If we do, we will already have processed it in a stronger way so can stop
#define TRY_RETURN_EARLY do {SafetyState newerResult = cache->find(V)->second; if(!(newerResult == newConstraint)) {return true;}} while(0)

  bool isRoot = FA->fromCall == nullptr;

  StaticFuncionAnalysis* CallerFA = FA->fromAnalysis;
  const Instruction* fromCall = FA->fromCall;

  // First look for uses that are loads and stores.

  for (const Use &U : V->uses()) {

    const Value* User = U.getUser();

    const Instruction *I = dyn_cast<Instruction>(User);

    if(!I) {
      if(isa<Operator>(User) && isUnsafeTrackable(User) && DecreaseSafety(FA, User, safetyDerivedBy))
        TRY_RETURN_EARLY;
      continue;
    }


    if(I->getFunction() != FA->CopyOf)
      continue;

    bool IsCap = isUnsafeTrackable(I);

    storePtr = nullptr;
    storeVal = nullptr;
    loadPtr = nullptr;
    loadVal = nullptr;

    switch (I->getOpcode()) {

    case Instruction::Store:
      storeVal = I->getOperand(0);
      storePtr = I->getOperand(1);
      break;
    case Instruction::AtomicCmpXchg: {
      // This is a store and a load
      auto *ACXI = cast<AtomicCmpXchgInst>(I);
      storeVal = ACXI->getNewValOperand();
      loadVal = I;
      loadPtr = storePtr = ACXI->getPointerOperand();
      break;
    }
    case Instruction::AtomicRMW: {
      // Also a store and load
      auto *ARMWI = cast<AtomicRMWInst>(I);
      storeVal = ARMWI->getValOperand();
      loadVal = I;
      loadPtr = storePtr = ARMWI->getPointerOperand();
      break;
    }
    case Instruction::Load:
      loadVal =  I;
      loadPtr = I->getOperand(0);
      break;

    case Instruction::Ret: {
      StaticFuncionAnalysis *CallingFA = FA->fromAnalysis;
      if(CallingFA && isUnsafeTrackable(FA->fromCall)) {
        if(DecreaseSafety(CallingFA, FA->fromCall, safetyDerivedBy))
          TRY_RETURN_EARLY;
      }
      break;
    }

    case Instruction::Call:
    case Instruction::Invoke:
      // Calls are either a derived by OR expanded and therefore a derives by but for the expanded function
    {
      auto entry = FA->callMap.find(I);
      if(entry != FA->callMap.end()) {
        StaticFuncionAnalysis * CalleeFA = entry->getSecond();

        // Get argument index this is
        ImmutableCallSite CS(I);

        unsigned ndx = CS.getArgumentNo(&U);

        // Normal argument
        if(ndx < CalleeFA->CopyOf->arg_size()) {
          auto Arg = &CalleeFA->CopyOf->args().begin()[ndx];

          assert(isUnsafeTrackable(Arg));

          // Impose constraint on CalleeFA input argument
          if(DecreaseSafety(CalleeFA, Arg, safetyDerivedBy))
            TRY_RETURN_EARLY;

        } else {
          // All arguments are considered stored^2 the argument of va_start in the target
          if(CalleeFA->va_vector_ptr) {
            SafetyState SS = safetyIfStored.getPropagationForStored();
            if(DecreaseSafety(CalleeFA, CalleeFA->va_vector_ptr, SS))
              TRY_RETURN_EARLY;
          }
        }

        break; // ONLY break if in map, otherwise fall into next case
      }

      CallSort Sort = getSort(*I, &U);

      if(Sort == MEMCPY) {
        // Memcpy semantics are better known
        const Value* Ret = I;
        const Value* Dst = I->getOperand(0);
        const Value* Src = I->getOperand(1);

        assert(isUnsafeTrackable(Dst));
        assert(isUnsafeTrackable(Src));

        if(Dst == V) {
          // Store to V, that was loaded from Src.
          if(DecreaseSafety(FA, Src, safetyStoredTo.getPropagationForLoaded()))
            TRY_RETURN_EARLY;
          // Also Ret derives from this V
          if(isUnsafeTrackable(Ret) && DecreaseSafety(FA, Ret, safetyDerivedBy))
            TRY_RETURN_EARLY;
        }

        if(Src == V) {
          // Load from V, that is stored to src.
          if(DecreaseSafety(FA, Dst, safetyLoadedFrom.getPropagationForStored()))
            TRY_RETURN_EARLY;
        }

        break;
      } else if(Sort == VA_START) {
        if(fromCall) {
          // All arguments are (stored to)^2 this ptr
          SafetyState SS = safetyStoredTo.getPropagationForStoredTo();
          ImmutableCallSite CS(fromCall);

          // We need to propagate back to ALL arguments after the formal ones
          for(size_t ndx = FA->CopyOf->arg_size(); ndx != CS.getNumArgOperands(); ndx++) {
            auto Arg = CS.getArgument((int)ndx);
            if(isUnsafeTrackable(Arg) && DecreaseSafety(CallerFA, Arg, SS))
              TRY_RETURN_EARLY;
          }
        }

        break;
      }
      if((Sort & DERIVES_RETURN_FROM_ARGUMENTS) != DERIVES_RETURN_FROM_ARGUMENTS)
        break;

      // Otherwise fall through
    }

      LLVM_FALLTHROUGH;
    case Instruction::BitCast:
    case Instruction::GetElementPtr:
    case Instruction::PHI:
    case Instruction::Select:
    case Instruction::AddrSpaceCast:
      // I could be derived from this pointer
      if(IsCap && DecreaseSafety(FA, I, safetyDerivedBy))
        TRY_RETURN_EARLY;
      break;
    default:
    {}
    }

    // We stored via something that was somewhat unsafe
    if(storePtr == V && isUnsafeTrackable(storeVal) && DecreaseSafety(FA, storeVal, safetyStoredTo)) {
      TRY_RETURN_EARLY;
    }

    // We loaded via something unsafe
    if(loadPtr == V && isUnsafeTrackable(loadVal) && DecreaseSafety(FA, loadVal, safetyLoadedFrom)) {
      TRY_RETURN_EARLY;
    }

    // We stored something unsafe. Things that come out of the pointer are therefore unsafe
    if(storeVal == V && DecreaseSafety(FA,storePtr, safetyIfStored)) {
      TRY_RETURN_EARLY;
    }
  }

  const Argument* AsArg;

  // Now look at if this capability was derived from another (which will be as unsafe as this is), or is loaded
  if(auto I = dyn_cast<Instruction>(V)) {

    loadPtr = nullptr;

    switch(I->getOpcode()) {
    case Instruction::AtomicCmpXchg: {
      auto *ACXI = cast<AtomicCmpXchgInst>(I);
      loadPtr = ACXI->getPointerOperand();
      break;
    }
    case Instruction::AtomicRMW: {
      auto *ARMWI = cast<AtomicRMWInst>(I);
      loadPtr = ARMWI->getPointerOperand();
      break;
    }
    case Instruction::Load:
      loadPtr = I->getOperand(0);
      break;
    case Instruction::Ret:
      assert(0);
      /*
      if(!isRoot && DecreaseSafety(CallerFA, fromCall, safetyDerivedBy)) {
        // Statically calling FA derives from this things argument
        TRY_RETURN_EARLY;
      }
      break;
       */
    case Instruction::Call:
    case Instruction::Invoke: {
      // This might be derived from any ret instruction
      auto entry = FA->callMap.find(I);
      if (entry != FA->callMap.end()) {
        StaticFuncionAnalysis *CalleeFA = entry->getSecond();
        SmallVector<const ReturnInst *, 4> &rets = CalleeFA->Rets;
        for (const ReturnInst *RI: rets) {
          if (DecreaseSafety(CalleeFA, RI->getReturnValue(), safetyDerivedFrom))
            TRY_RETURN_EARLY;
        }
        break;
      }
      CallSort Sort = getSort(*I);
      if (Sort == MEMCPY) {
        // Memcpy semantics are better known
        const Value *Dst = I->getOperand(0);
        // This derives from Dst
        DecreaseSafety(FA, Dst, safetyDerivedFrom);
        break;
      }
      assert(Sort != VA_START);
      if ((Sort & DERIVES_RETURN_FROM_ARGUMENTS) != DERIVES_RETURN_FROM_ARGUMENTS)
        break;

      ImmutableCallSite CS(I);

      // This capability can derive from the captured arguments
      auto B = CS.data_operands_begin();
      auto E = CS.data_operands_end();
      for (auto A = B; A != E; ++A)
        if (isUnsafeTrackable(A->get()) && !CS.doesNotCapture(A - B)
            && DecreaseSafety(FA, A->get(), safetyDerivedFrom))
              TRY_RETURN_EARLY;

      break;
    }

    case Instruction::BitCast:
    case Instruction::GetElementPtr:
    case Instruction::PHI:
    case Instruction::Select:
    case Instruction::AddrSpaceCast:
      // This pointer could be derived from its operands. Unsafety is a bi-implication, so also mark all all operands as unade
      for(const Value* O : I->operands()) {
        if(isUnsafeTrackable(O)) {
          DecreaseSafety(FA,O, safetyDerivedFrom);
          TRY_RETURN_EARLY;
        }
      }
      break;
    default:
    {}
    }

    // We loaded an unsafe value from a pointer, so that pointers contents are unsafe
    if(loadPtr) {
      DecreaseSafety(FA,loadPtr, safetyIfLoaded);
    }

  } else if(!isRoot && (AsArg = dyn_cast<Argument>(V))) {
    unsigned ndx = AsArg->getArgNo();
    ImmutableCallSite CS(fromCall);
    const Value* callersArg = CS.getArgument(ndx);
    if(isUnsafeTrackable(callersArg))
        DecreaseSafety(CallerFA, callersArg, safetyDerivedFrom);
  }

  // This might be different from newConstraint if a subsequent recursive call made it stronger.
  return true;
}

void EscapeAnalyser::addRootSet(StaticFuncionAnalysis* funcionAnalysis) {

  // Our root set of unsafe things consists of cheri ptrs that are:
  //    globals, arguments (to the root), or passed to leaking functions (not in the tree), or returned (from the root)
  // "Leaking functions" depends on use cases. For eliding bounds its any use but an uncapturing intrinsic
  // For escape analysis we can also include read-only functions in those that do not leak

  bool isRoot = funcionAnalysis->fromCall == nullptr;
  const Function& F = *funcionAnalysis->CopyOf;
  if(isRoot) {
    // Arguments
    for (auto &arg: F.args()) {
      if (isUnsafeTrackable(&arg)) {
        DecreaseSafety(funcionAnalysis, &arg, SafetyState::UnsafeState);
      }
    }

    if(funcionAnalysis->va_vector_ptr) {
      SafetyState varSafety = SafetyState::UnsafeState.getPropagationForStored().getPropagationForStored();
      DecreaseSafety(funcionAnalysis, funcionAnalysis->va_vector_ptr, varSafety);
    }
  }

  for (auto &BB : F) {
    for (auto &I : BB) {

      bool allOperandsUnsafe = false;
      bool valueUnsafeToDeriveFrom = false;

      switch (I.getOpcode()) {
      case Instruction::Call:
      case Instruction::Invoke: {
        bool inMap = funcionAnalysis->callMap.find(&I) != funcionAnalysis->callMap.end();

        if(!inMap) {
          // We don't inspect this function. Instead just use metadata
          CallSort sort = getSort(I);
          valueUnsafeToDeriveFrom = (sort & UNSAFE_RETURN) == UNSAFE_RETURN;
          if((sort & LEAKS_ARGUMENTS) == LEAKS_ARGUMENTS) {
            ImmutableCallSite CS(&I);
            auto B = CS.data_operands_begin();
            auto E = CS.data_operands_end();
            for (auto A = B; A != E; ++A)
              if(isUnsafeTrackable(A->get()) && !CS.doesNotCapture(A-B)) {
                DecreaseSafety(funcionAnalysis, A->get(), SafetyState::UnsafeState);
              }
          }
        }

        break;
      }

      case Instruction::Ret:
        if(isRoot) allOperandsUnsafe = true;
        break;
      default:{}
      }

      if(valueUnsafeToDeriveFrom) {
        DecreaseSafety(funcionAnalysis, &I, SafetyState::UnsafeToDeriveFrom);
      }

      // Globals, or things passed to ret/leaking functions
      for(const Value* V: I.operands()) {
        if(isUnsafeTrackable(V)) {
          if(allOperandsUnsafe || isa<GlobalValue>(V)) DecreaseSafety(funcionAnalysis, V, SafetyState::UnsafeState);
          else {
            const Operator* O;
            while((O = dyn_cast<llvm::Operator>(V)) && !isa<llvm::Instruction>(V)) {
              V = O->getOperand(0);
              if(!isUnsafeTrackable(V))
                break;
              if(isa<GlobalValue>(V)) {
                DecreaseSafety(funcionAnalysis, V, SafetyState::UnsafeState);
                break;
              }
            }
          }
        }
      }
    }
  }

  analysisDone = true;
}

SafetyState EscapeAnalyser::GetSafetyState(const Value *V) {
  assert(analysisDone);
  auto result = expandedFunctions[0].resultCache.find(V);
  return result == expandedFunctions[0].resultCache.end() ? SafetyState::SafeState : (*result).second;
}


void EscapeAnalyser::reset() {
  if(analysisDone) {
    expandedFunctionsCount = 0;
  }
  analysisDone = false;
}

bool EscapeAnalyser::IsValueDirectlySafe(const Value *V) {
  return (GetSafetyState(V).IsDirectlySafe() == S_SAFE);
}
bool EscapeAnalyser::deepenAllocateFunctionAnalysis(const Function *F,
                                                    EscapeAnalyser::StaticFuncionAnalysis *Expanded,
                                                    EscapeAnalyser::StaticFuncionAnalysis *fromAnalysis,
                                                    const Instruction *fromCall,
                                                    int depth,
                                                    size_t expandLimit) {
  bool needMore = false;
  if(depth == 0) {
    // Create here
    Expanded->CopyOf = F;
    Expanded->fromCall = fromCall;
    Expanded->fromAnalysis = fromAnalysis;
    Expanded->callMap.clear();
    Expanded->resultCache.clear();
    Expanded->Rets.clear();
    Expanded->va_vector_ptr = nullptr;
    needMore = true;
  } else if(depth == 1) {


    // Allocate new children and find ret instructions

    for(auto& BB : *F) {
      for(auto& I: BB) {

        if(I.getOpcode() == Instruction::Ret) {
          Expanded->Rets.push_back(&cast<ReturnInst>(I));
        } else if(I.getOpcode() == Instruction::Call || I.getOpcode() == Instruction::Invoke) {
          ImmutableCallSite CS(&I);

          if((expandedFunctionsCount != expandLimit)) {
              const Function* child = CS.getCalledFunction();

              bool worthExpand = false;
              auto B = CS.data_operands_begin();
              auto E = CS.data_operands_end();
              for (auto A = B; A != E; ++A)
                  if(isUnsafeTrackable(A->get()) && !CS.doesNotCapture(A-B))
                      worthExpand = true;

              // Is in same compilation unit. Expand.

              if(worthExpand && child != nullptr && child->isDefinitionExact() && !child->isDeclaration()) {
                StaticFuncionAnalysis* ExpandedChild = &expandedFunctions[expandedFunctionsCount++];
                Expanded->callMap.insert(std::pair<const Instruction*, StaticFuncionAnalysis*>(&I,ExpandedChild));
                deepenAllocateFunctionAnalysis(child, ExpandedChild, Expanded, &I, depth-1, expandLimit);
                needMore = true;
              }
          }

          if(CS.getIntrinsicID() == Intrinsic::vastart) {
            Expanded->va_vector_ptr = CS.getArgument(0);
          }
        }
      }
    }

  } else {
    // Else loop though children already made
    for(auto child : Expanded->callMap) {
      if(child.getSecond()->needsMore) {
        needMore |= deepenAllocateFunctionAnalysis(child.getSecond()->CopyOf, child.getSecond(), Expanded, child.getFirst(),
            depth-1,expandLimit);
      }
    }
  }

  Expanded->needsMore = needMore;
  return needMore;
}

void EscapeAnalyser::addRootFunction(const Function &F, size_t expandLimit) {

  assert(!analysisDone);

  if(!(expandLimit > 0 && expandLimit <= max_static_function_expansions))
    report_fatal_error(Twine("Expand limit of ") + Twine(expandLimit) + "is invalid.  Must be between 1 and " + Twine(max_static_function_expansions),false);

  // First create a tree of statically expanded functions
  int depth = 0;
  expandedFunctionsCount = 1;
  do {
    deepenAllocateFunctionAnalysis(&F, &expandedFunctions[0], nullptr, nullptr, depth++, expandLimit);
  } while(expandedFunctions[0].needsMore);

  // Then add escaping values to the root set
  for(size_t i = 0; i != expandedFunctionsCount; i++) {
    addRootSet(&expandedFunctions[i]);
  }

  analysisDone = true;
}

}
}

