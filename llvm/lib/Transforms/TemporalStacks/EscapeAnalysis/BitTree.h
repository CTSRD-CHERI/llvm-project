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
#ifndef LLVM_BITTREE_H
#define LLVM_BITTREE_H

#include "cstdint"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

// A mask of all 1s from bit START, N bits long

#define BIT_RANGE(START, N) (((1ULL << N) - 1) << START)

// The bit pattern of these are in alternating blocks of 1 bit, 2 bits, 4 bits,
// 8 bits etc...
static const uint64_t ZIP32_MASKS[] = {0x5555555555555555, 0x3333333333333333,
                                       0x0F0F0F0F0F0F0F0F, 0x00FF00FF00FF00FF,
                                       0x0000FFFF0000FFFF, 0x00000000FFFFFFFF};

// Takes a 32 bit value (in little end) and zips it with 0s
static uint64_t zip32(uint64_t v) {
  v &= ZIP32_MASKS[5];
  for (int i = 4; i >= 0; i--) {
    v = (v | (v << (1 << i))) & ZIP32_MASKS[i];
  }
  return v;
}

// Takes a 64 bit value and returns the even bits packed into little 32
static uint64_t unzip32(uint64_t v) {
  for (int i = 0; i != 5; i++) {
    v &= ZIP32_MASKS[i];
    v = (v | (v >> (1 << i)));
  }
  v &= ZIP32_MASKS[5];
  return v;
}

class BitTree {
  // The tree is conceptually infinite, the "leaf" codes indicate what every
  // node under them is. a 1 is always followed by all 1s (so the most unsafe
  // tree is all 1s) The tree is made by recursively zipping the bits of
  // sub-trees. The root of the tree is the lowest bit. The idea behind
  // arranging the tree this way is so that the difference between a node and
  // its children is the same for every node at that level The upshot of that is
  // we can make every parent in a row a function of its children (or vice
  // versa) with one operation. Snazy. If two nodes would fall out
  // representability, we make the parent the logical or of the children (which
  // over approximating unsafeness). (TODO: Maybe control this with a flag to be
  // an and for minimum?)
  uint64_t packed;

  static BitTree subTree(uint64_t pack, uint8_t shift) {
    // Shift so the tree we want has its root in bit 0
    pack >>= shift;
    // unzip to discard other tree
    pack = unzip32(pack);
    // now propagate the last layer

    // Bits 15 to 30 copied (and duplicated once) into bits 31 62 to extend tree
    uint64_t penultimateRow = pack & BIT_RANGE(15, 16);
    uint64_t lastRow = (penultimateRow << 16); // left child
    lastRow |= lastRow << 16;                  // right child

    return BitTree(pack | lastRow);
  }

  static BitTree mergeWithSafe(uint64_t pack, uint8_t shift) {
    // We are going to lose the leaves, make the parent node a 1 if either of
    // its children are

    // Or the leaves
    uint64_t ordLeaf = (pack | (pack >> 16));

    // Move in position
    ordLeaf >>= 16;

    // mask
    ordLeaf &= BIT_RANGE(15, 16);

    // set parent
    pack |= ordLeaf;

    pack &= ~(1 << 31); // This is the first bit of the layer that should
                        // disappear. It might accidentally go into bit 63.

    // Now zip with tree of 0s
    pack = zip32(pack);

    // Now add a new root
    return BitTree(pack << shift);
  }

public:
  explicit BitTree(uint64_t pack) : packed(pack) {}

  void dump() const { llvm::dbgs().write_hex(packed); }

  inline uint64_t getRoot() const { return (packed & 1); }

  inline BitTree getLHS() const { return subTree(packed, 1); }

  inline BitTree getRHS() const { return subTree(packed, 2); }

  // Gives another tree with this as a LHS, and a safe tree as a RHS
  inline BitTree asLHS() const { return mergeWithSafe(packed, 1); }

  // Gives another tree with this as a RHS, and a safe tree as a LHS
  inline BitTree asRHS() const { return mergeWithSafe(packed, 2); }

  // Makes a LHS the RHS (assumes the root and RHS is a 0, otherwise mask it
  // first!)
  inline BitTree pivotLTOR() const { return BitTree(packed << 1); }

  inline static BitTree OrNodes(const BitTree A, const BitTree B) {
    return BitTree(A.packed | B.packed);
  }

  inline bool operator==(const BitTree &rhs) const {
    return this->packed == rhs.packed;
  }
};

namespace BitTrees {
static const BitTree AllZeros = BitTree(0);
static const BitTree AllOnes = BitTree(BIT_RANGE(0, 63));
} // namespace BitTrees

} // namespace llvm
#endif // LLVM_BITTREE_H
