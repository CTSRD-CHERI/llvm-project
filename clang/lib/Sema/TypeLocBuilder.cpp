//===--- TypeLocBuilder.cpp - Type Source Info collector ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This files defines TypeLocBuilder, a class for building TypeLocs
//  bottom-up.
//
//===----------------------------------------------------------------------===//

#include "TypeLocBuilder.h"

using namespace clang;

void TypeLocBuilder::pushFullCopy(TypeLoc L) {
  size_t Size = L.getFullDataSize();
  reserve(Size);

  SmallVector<TypeLoc, 4> TypeLocs;
  TypeLoc CurTL = L;
  while (CurTL) {
    TypeLocs.push_back(CurTL);
    CurTL = CurTL.getNextTypeLoc();
  }

  for (unsigned i = 0, e = TypeLocs.size(); i < e; ++i) {
    TypeLoc CurTL = TypeLocs[e-i-1];
    switch (CurTL.getTypeLocClass()) {
#define ABSTRACT_TYPELOC(CLASS, PARENT)
#define TYPELOC(CLASS, PARENT) \
    case TypeLoc::CLASS: { \
      CLASS##TypeLoc NewTL = push<class CLASS##TypeLoc>(CurTL.getType()); \
      memcpy(NewTL.getOpaqueData(), CurTL.getOpaqueData(), NewTL.getLocalDataSize()); \
      break; \
    }
#include "clang/AST/TypeLocNodes.def"
    }
  }
}

void TypeLocBuilder::grow(size_t NewCapacity) {
  assert(NewCapacity > Capacity);

  // Allocate the new buffer and copy the old data into it.
  char *NewBuffer = new char[NewCapacity];
  unsigned NewIndex = Index + NewCapacity - Capacity;
  memcpy(&NewBuffer[NewIndex],
         &Buffer[Index],
         Capacity - Index);

  if (Buffer != InlineBuffer)
    delete[] Buffer;

  Buffer = NewBuffer;
  Capacity = NewCapacity;
  Index = NewIndex;
}

TypeLoc TypeLocBuilder::pushImpl(QualType T, size_t LocalSize, llvm::Align LocalAlignment) {
#ifndef NDEBUG
  QualType TLast = TypeLoc(T, nullptr).getNextTypeLoc().getType();
  assert(TLast == LastTy &&
         "mismatch between last type and new type's inner type");
  LastTy = T;
#endif

  assert(LocalAlignment <= BufferMaxAlignment && "Unexpected alignment");

  // Because we're adding elements to the TypeLoc backwards, we have to
  // do some extra work to keep everything aligned appropriately.
  // FIXME: This algorithm is a absolute mess because every TypeLoc returned
  // needs to be valid.  Partial TypeLocs are a terrible idea.

  // Recalculate layout, but don't update LayoutBlocks yet
  llvm::SmallVector<size_t, MaxLayoutBlocks> Deltas;
  size_t Offset = LocalSize;
  for (auto &Block : llvm::reverse(LayoutBlocks)) {
    Offset = llvm::alignTo(Offset, Block.Align);
    Deltas.push_back(Offset - Block.Offset);
    Offset += Block.Size;
  }
  llvm::Align NewAlign = std::max(LocalAlignment, LayoutBlocks[0].Align);
  size_t NewSize = llvm::alignTo(Offset, NewAlign);

  assert(NewSize == TypeLoc::getFullDataSizeForType(T) &&
         "incorrect data size provided to CreateTypeSourceInfo!");

  // If we need to grow, grow by a factor of 2.
  if (NewSize > Capacity) {
    size_t NewCapacity = Capacity * 2;
    while (NewSize > NewCapacity)
      NewCapacity *= 2;
    grow(NewCapacity);
  }

  // Move blocks for the new layout and update LayoutBlocks to match.
  //
  // Within the buffer we have a prefix of blocks moving left, a prefix of
  // blocks moving right or no blocks moving at all. We treat no blocks moving
  // at all as a special case of moving left.
  //
  // Note that LayoutBlocks and Deltas are stored in opposite orders; the
  // left-most block is last in LayoutBlocks but first in Deltas.
  size_t NewIndex = Capacity - NewSize;
  size_t SizeDelta = Index - NewIndex;
  if (Deltas[0] <= SizeDelta) {
    // Move blocks left (from left to right)
    auto BI = LayoutBlocks.rbegin();
    auto BE = LayoutBlocks.rend();
    auto DI = Deltas.begin();
    for (; BI != BE && *DI < SizeDelta; ++BI, ++DI) {
      size_t NewOffset = BI->Offset + *DI;
      memmove(&Buffer[NewIndex + NewOffset], &Buffer[Index + BI->Offset],
              BI->Size);
      BI->Offset = NewOffset;
    }
    for (; BI != BE; ++BI, ++DI) {
      assert(*DI == SizeDelta && "Unexpected block delta after left prefix");
      size_t NewOffset = BI->Offset + *DI;
      BI->Offset = NewOffset;
    }
  } else {
    // Move blocks right (from right to left)
    auto BI = LayoutBlocks.begin();
    auto BE = LayoutBlocks.end();
    auto DI = Deltas.rbegin();
    for (; BI != BE && *DI == SizeDelta; ++BI, ++DI) {
      size_t NewOffset = BI->Offset + *DI;
      BI->Offset = NewOffset;
    }
    for (; BI != BE; ++BI, ++DI) {
      assert(*DI > SizeDelta && "Unexpected block delta after right prefix");
      size_t NewOffset = BI->Offset + *DI;
      memmove(&Buffer[NewIndex + NewOffset], &Buffer[Index + BI->Offset],
              BI->Size);
      BI->Offset = NewOffset;
    }
  }

  // Update TypeLoc buffer start index to match
  Index = NewIndex;

  // Record this element's block, merging with existing blocks subsumed by it.
  LayoutBlock NewBlock(0, LocalSize, LocalAlignment);
  while (!LayoutBlocks.empty() && LayoutBlocks.back().Align <= NewBlock.Align) {
    NewBlock.Size = LayoutBlocks.back().Offset + LayoutBlocks.back().Size;
    LayoutBlocks.pop_back();
  }
  LayoutBlocks.push_back(NewBlock);

  return getTemporaryTypeLoc(T);
}
