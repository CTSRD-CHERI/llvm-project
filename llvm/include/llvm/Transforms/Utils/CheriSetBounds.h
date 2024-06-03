//===- CheriSetBounds.h - Functions to log information on CSetBounds ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_UTILS_CHERISETBOUNDS_H
#define LLVM_TRANSFORMS_UTILS_CHERISETBOUNDS_H

#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CheriSetBounds.h"
#include "llvm/Transforms/Utils.h"

namespace llvm {

class Value;
class Instruction;

namespace cheri {

// This probably exists somewhere else
inline std::optional<uint64_t> inferConstantValue(Value *V) {
  std::optional<uint64_t> Result;
  if (auto CI = dyn_cast_or_null<ConstantInt>(V)) {
    Result = CI->getSExtValue();
  } else {
    // TODO: use KnownBits to infer something?
  }
  return Result;
}

inline void addSetBoundsStats(Align KnownAlignment, Value *Length,
                              StringRef Pass, SetBoundsPointerSource Kind,
                              const Twine &Details, std::string SourceLoc,
                              std::optional<uint64_t> SizeMultipleOf = None) {
  CSetBoundsStats->add(KnownAlignment, inferConstantValue(Length), Pass, Kind,
                       Details, std::move(SourceLoc), SizeMultipleOf);
}

inline SetBoundsPointerSource inferPointerSource(const Value *V) {
  // look through casts and find out if they are global/alloca/unknown
  return SetBoundsPointerSource::Unknown;
}

// These 3 helpers should probably be somewhere else

// Look at the attached debug info to get the name of the local variable or if
// not known return the name of the allocainst
std::string inferLocalVariableName(AllocaInst *AI);

// returns a source file + line if debug info is present valid, otherwise falls
// back to "<somewhere in $FUNCTION_NAME>"
std::string inferSourceLocation(Instruction *Inst);
std::string inferSourceLocation(const DebugLoc &DL, StringRef Function);

} // end namespace cheri

} // end namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_CHERISETBOUNDS_H
