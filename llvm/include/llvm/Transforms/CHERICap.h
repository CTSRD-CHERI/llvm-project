//===-- CHERICap.h - CHERICap Scalar Transformations --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose passes
// in the CHERICap Scalar Transformations library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_CHERICAP_H
#define LLVM_TRANSFORMS_CHERICAP_H

namespace llvm {

class Pass;

//===----------------------------------------------------------------------===//
//
// CHERICapDirectCalls - Transform PCC-relative calls into direct calls to allow
// inlining.
//
Pass *createCHERICapDirectCallsPass();
//===----------------------------------------------------------------------===//
//
// createCHERICapFoldCapIntrinsics - Fold redundant capability operations.
//
Pass *createCHERICapFoldIntrinsicsPass();

Pass *createCHERISafeStacksPass();

} // End llvm namespace

#endif
