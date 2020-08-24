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

//
// createCHERISafeStacksPass - Attach safe/unsafe metadata annotations to
// allocas.
//

Pass *createCHERISafeStacksPass();

} // End llvm namespace

#endif
