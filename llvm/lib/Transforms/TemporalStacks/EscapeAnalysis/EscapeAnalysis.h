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
#ifndef LLVM_ESCAPEANALYSIS_H
#define LLVM_ESCAPEANALYSIS_H

#include "BitTree.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/IR/Instructions.h"

namespace llvm {
namespace cheri {

enum SNODE {
  S_SAFE = 0,
  S_UNSAFE = 1,
};

// This is the safety state of a pointer. For every store /load / store to /
// load to / derive from / derive by we impose a constraint on aother pointers.
// This class also provides those constraints.

class SafetyState {
  // A tree representing safety. LHS = when loaded from. RHS = when stored to. 0
  // = directly safe. 1 = directly unsafe
  // In/Out control which direction we allow propagation of constraints
  BitTree In;
  BitTree Out;

  explicit SafetyState(const BitTree I, const BitTree O) : In(I), Out(O) {}

public:
  static const SafetyState SafeState;
  static const SafetyState UnsafeState;
  static const SafetyState UnsafeToDeriveFrom;

  inline SNODE IsDirectlySafe() const {
    return (SNODE)BitTree::OrNodes(In, Out).getRoot();
  }

  void dump() const {
    auto &d = dbgs();
    d << "State: In = ";
    In.dump();
    d << ", Out = ";
    Out.dump();
    d << "\t\t\t";
  }

  inline static SafetyState getMaximum(const SafetyState A,
                                       const SafetyState B) {
    return SafetyState(BitTree::OrNodes(A.In, B.In),
                       BitTree::OrNodes(A.Out, B.Out));
  }

  inline SafetyState getPropagationForDerivedFrom() const {
    return SafetyState(Out, Out);
  }

  inline SafetyState getPropagationForDerivedBy() const {
    return SafetyState(In, BitTrees::AllZeros);
  }

  inline SafetyState getPropagationForLoaded() const {
    BitTree LHS = Out.asLHS();
    BitTree RHS = LHS.pivotLTOR();
    BitTree T = BitTree::OrNodes(LHS, RHS);
    return SafetyState(T, T);
  }

  inline SafetyState getPropagationForStored() const {
    BitTree T = In.asLHS();
    return SafetyState(T, T);
  }

  inline SafetyState getPropagationForStoredTo() const {
    BitTree RHS = In.getRHS();
    return SafetyState(RHS, RHS);
  }

  inline SafetyState getPropagationForLoadedFrom() const {
    BitTree LHS = In.getLHS();
    return SafetyState(LHS, BitTrees::AllZeros);
  }

  inline bool operator==(const SafetyState &rhs) const {
    return (this->In == rhs.In) && (this->Out == rhs.Out);
  }
};

enum CallSort {
  SORT_SAFE = 0x00,
  LEAKS_ARGUMENTS = 0x01,
  UNSAFE_RETURN = 0x02,
  DERIVES_RETURN_FROM_ARGUMENTS = 0x04,
  // acts like a memcpy/memmove. Copies from second argument to first. Return
  // derives from first arg.
  MEMCPY = 0x08,
  // acts like VA_START. The first argument is a pointer to a pointer to a
  // vector of arguments
  VA_START = 0x10,
};

class EscapeAnalyser {

  static const int max_static_function_expansions = 64;

  class StaticFuncionAnalysis {
  public:
    // This is a static instantiation of this function
    const Function *CopyOf = nullptr;
    // Which call instruction expanded this
    const Instruction *fromCall = nullptr;
    // and its analysis state
    StaticFuncionAnalysis *fromAnalysis = nullptr;
    bool needsMore = true;
    // The expanded calls this instantiation makes. If an instruction can't be
    // found, then it is a leaf
    DenseMap<const Instruction *, StaticFuncionAnalysis *> callMap;
    // All the return instructions. Used to quickly propagate between FAs
    SmallVector<const ReturnInst *, 4> Rets;
    const Value *va_vector_ptr;
    // Cache of results for this function
    DenseMap<const Value *, SafetyState> resultCache;
  };

  StaticFuncionAnalysis expandedFunctions[max_static_function_expansions];
  size_t expandedFunctionsCount = 0;

  bool readOnlyEscapes; // Do read only functions escape their arguments

  bool analysisDone = false; // Is the result cache complete for this function

  CallSort getSort(const Instruction &I, const Use *Use = nullptr);

  // Constrain the value V to be at least newConstraint. May have knock on
  // effects. Returns true if any changes were made
  bool DecreaseSafety(StaticFuncionAnalysis *FA, const Value *V,
                      SafetyState newConstraint);

  bool deepenAllocateFunctionAnalysis(const Function *F,
                                      StaticFuncionAnalysis *Expanded,
                                      StaticFuncionAnalysis *fromAnalysis,
                                      const Instruction *fromCall, int depth,
                                      size_t expandLimit);

  void addRootSet(StaticFuncionAnalysis *funcionAnalysis);

  void dumpFA(StaticFuncionAnalysis *FA) {
    for (auto Entry : FA->resultCache) {
      Entry.getFirst()->dump();
      Entry.getSecond().dump();
    }
  }

public:
  explicit EscapeAnalyser(bool ReadOnlyEscapes)
      : readOnlyEscapes(ReadOnlyEscapes) {}

  // Reset the cache
  void reset();

  void addRootFunction(const Function &F,
                       size_t expandLimit = max_static_function_expansions);

  void dump() { dumpFA(&expandedFunctions[0]); }

  // Get the safety state for a value in the root
  SafetyState GetSafetyState(const Value *V);
  // Returns true if the value does not escape the root
  bool IsValueDirectlySafe(const Value *V);
};

} // namespace cheri
} // namespace llvm

#endif // LLVM_ESCAPEANALYSIS_H
