//===- CheriSetBounds.h - Functions to log information on CSetBounds ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This family of functions perform various local transformations to the
// program.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_CHERISETBOUNDS_H
#define LLVM_TRANSFORMS_UTILS_CHERISETBOUNDS_H

#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils.h"

namespace llvm {

class Value;
class Instruction;

namespace cheri {

// TODO: Remove these when I merge / cherry pick.

/// This function can be used if there is no DataLayout available and will
/// in that case assume that AS200 is only used for CHERI
inline bool isCheriPointer(unsigned AddrSpace) {
  return AddrSpace == 200 /* TODO: || AddrSpace == 201 */;
}

inline bool isCheriPointer(const Type *Ty) {
  return Ty->isPointerTy() && isCheriPointer(Ty->getPointerAddressSpace());
}

inline bool isCheriPointer(const Value *I) {
  return isCheriPointer(I->getType());
}

// This probably exists somewhere else
inline Optional<uint64_t> inferConstantValue(Value *V) {
  Optional<uint64_t> Result;
  if (auto CI = dyn_cast_or_null<ConstantInt>(V)) {
    Result = CI->getSExtValue();
  } else {
    // TODO: use KnownBits to infer something?
  }
  return Result;
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