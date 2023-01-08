//===--- TypeLocBuilder.h - Type Source Info collector ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines TypeLocBuilder, a class for building TypeLocs
//  bottom-up.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SEMA_TYPELOCBUILDER_H
#define LLVM_CLANG_LIB_SEMA_TYPELOCBUILDER_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/TypeLoc.h"

namespace clang {

class TypeLocBuilder {
  /// The underlying location-data buffer.  Data grows from the end
  /// of the buffer backwards.
  char *Buffer;

  /// The capacity of the current buffer.
  size_t Capacity;

  /// The index of the first occupied byte in the buffer.
  size_t Index;

#ifndef NDEBUG
  /// The last type pushed on this builder.
  QualType LastTy;
#endif

  /// The inline buffer.
  enum { BufferMaxAlignment = alignof(max_align_t) };
  enum { InlineCapacity = llvm::alignTo<BufferMaxAlignment>(8 * sizeof(SourceLocation)) };
  alignas(BufferMaxAlignment) char InlineBuffer[InlineCapacity];

  /// Represents a block of elements in the TypeLoc's layout that are fixed
  /// relative to each other due to alignment requirements (i.e. all elements
  /// have alignment no greater than the first element in the block). Note
  /// that Size is the unpadded size.
  struct LayoutBlock {
    size_t Offset;
    size_t Size;
    llvm::Align Align;

    LayoutBlock(size_t Offset, size_t Size, llvm::Align Align)
        : Offset(Offset), Size(Size), Align(Align) {}
  };

  /// Represents the entire layout of a TypeLoc as a list of LayoutBlock
  /// elements. Stored in reverse order for more efficient insertion and
  /// deletion, i.e. entries are in decreasing offset and alignment order.
  /// To avoid handling the empty edge-case, this is never empty; empty is
  /// instead represented with a byte-aligned 0-byte block.
  enum { MaxLayoutBlocks = llvm::CTLog2<BufferMaxAlignment>() };
  SmallVector<LayoutBlock, MaxLayoutBlocks> LayoutBlocks;

public:
  TypeLocBuilder()
      : Buffer(InlineBuffer), Capacity(InlineCapacity), Index(InlineCapacity),
        LayoutBlocks({LayoutBlock(0, 0, llvm::Align())}) {}

  ~TypeLocBuilder() {
    if (Buffer != InlineBuffer)
      delete[] Buffer;
  }

  /// Ensures that this buffer has at least as much capacity as described.
  void reserve(size_t Requested) {
    if (Requested > Capacity)
      // For now, match the request exactly.
      grow(Requested);
  }

  /// Pushes a copy of the given TypeLoc onto this builder.  The builder
  /// must be empty for this to work.
  void pushFullCopy(TypeLoc L);

  /// Pushes space for a typespec TypeLoc.  Invalidates any TypeLocs
  /// previously retrieved from this builder.
  TypeSpecTypeLoc pushTypeSpec(QualType T) {
    size_t LocalSize = TypeSpecTypeLoc::LocalDataSize;
    llvm::Align LocalAlign = llvm::Align(TypeSpecTypeLoc::LocalDataAlignment);
    return pushImpl(T, LocalSize, LocalAlign).castAs<TypeSpecTypeLoc>();
  }

  /// Resets this builder to the newly-initialized state.
  void clear() {
#ifndef NDEBUG
    LastTy = QualType();
#endif
    Index = Capacity;
    LayoutBlocks = {LayoutBlock(0, 0, llvm::Align())};
  }

  /// Tell the TypeLocBuilder that the type it is storing has been
  /// modified in some safe way that doesn't affect type-location information.
  void TypeWasModifiedSafely(QualType T) {
#ifndef NDEBUG
    LastTy = T;
#endif
  }

  /// Pushes space for a new TypeLoc of the given type.  Invalidates
  /// any TypeLocs previously retrieved from this builder.
  template <class TyLocType> TyLocType push(QualType T) {
    TyLocType Loc = TypeLoc(T, nullptr).castAs<TyLocType>();
    size_t LocalSize = Loc.getLocalDataSize();
    llvm::Align LocalAlign = llvm::Align(Loc.getLocalDataAlignment());
    return pushImpl(T, LocalSize, LocalAlign).castAs<TyLocType>();
  }

  /// Creates a TypeSourceInfo for the given type.
  TypeSourceInfo *getTypeSourceInfo(ASTContext& Context, QualType T) {
#ifndef NDEBUG
    assert(T == LastTy && "type doesn't match last type pushed!");
#endif

    size_t FullDataSize = Capacity - Index;
    TypeSourceInfo *DI = Context.CreateTypeSourceInfo(T, FullDataSize);
    memcpy(DI->getTypeLoc().getOpaqueData(), &Buffer[Index], FullDataSize);
    return DI;
  }

  /// Copies the type-location information to the given AST context and
  /// returns a \c TypeLoc referring into the AST context.
  TypeLoc getTypeLocInContext(ASTContext &Context, QualType T) {
#ifndef NDEBUG
    assert(T == LastTy && "type doesn't match last type pushed!");
#endif

    size_t FullDataSize = Capacity - Index;
    void *Mem = Context.Allocate(FullDataSize);
    memcpy(Mem, &Buffer[Index], FullDataSize);
    return TypeLoc(T, Mem);
  }

private:

  TypeLoc pushImpl(QualType T, size_t LocalSize, llvm::Align LocalAlignment);

  /// Grow to the given capacity.
  void grow(size_t NewCapacity);

  /// Retrieve a temporary TypeLoc that refers into this \c TypeLocBuilder
  /// object.
  ///
  /// The resulting \c TypeLoc should only be used so long as the
  /// \c TypeLocBuilder is active and has not had more type information
  /// pushed into it.
  TypeLoc getTemporaryTypeLoc(QualType T) {
#ifndef NDEBUG
    assert(LastTy == T && "type doesn't match last type pushed!");
#endif
    return TypeLoc(T, &Buffer[Index]);
  }
};

}

#endif
