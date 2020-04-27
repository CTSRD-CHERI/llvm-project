//===- llvm/IR/Cheri.h - various CHERI related utility functions     ----*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains various CHERI utilities. This is a separate file to avoid
// changes to headers that will require a full LLVM recompile.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_CHERI_INTRINSICS_H
#define LLVM_IR_CHERI_INTRINSICS_H

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"

namespace llvm {
namespace cheri {

Intrinsic::ID correspondingSetIntrinsic(Intrinsic::ID GetIntrin) {
  switch (GetIntrin) {
  case Intrinsic::cheri_cap_offset_get:
    return Intrinsic::cheri_cap_offset_set;
  case Intrinsic::cheri_cap_address_get:
    return Intrinsic::cheri_cap_address_set;
  case Intrinsic::cheri_cap_flags_get:
    return Intrinsic::cheri_cap_flags_set;
  default:
    llvm_unreachable("No matching set intrinsic");
  }
}
Intrinsic::ID correspondingGetIntrinsic(Intrinsic::ID SetIntrin) {
  switch (SetIntrin) {
  case Intrinsic::cheri_cap_offset_set:
    return Intrinsic::cheri_cap_offset_get;
  case Intrinsic::cheri_cap_address_set:
    return Intrinsic::cheri_cap_address_get;
  case Intrinsic::cheri_cap_flags_set:
    return Intrinsic::cheri_cap_flags_get;
  default:
    llvm_unreachable("No matching get intrinsic");
  }
}

} // namespace cheri
} // end namespace llvm

#endif // LLVM_IR_CHERI_INTRINSICS_H
