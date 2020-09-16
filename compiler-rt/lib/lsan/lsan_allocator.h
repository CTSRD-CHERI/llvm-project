//=-- lsan_allocator.h ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of LeakSanitizer.
// Allocator for standalone LSan.
//
//===----------------------------------------------------------------------===//

#ifndef LSAN_ALLOCATOR_H
#define LSAN_ALLOCATOR_H

#include "sanitizer_common/sanitizer_allocator.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_internal_defs.h"
#include "lsan_common.h"

namespace __lsan {

void *Allocate(const StackTrace &stack, usize size, usize alignment,
               bool cleared);
void Deallocate(void *p);
void *Reallocate(const StackTrace &stack, void *p, usize new_size,
                 usize alignment);
usize GetMallocUsableSize(const void *p);

template<typename Callable>
void ForEachChunk(const Callable &callback);

void GetAllocatorCacheRange(uptr *begin, uptr *end);
void AllocatorThreadFinish();
void InitializeAllocator();

const bool kAlwaysClearMemory = true;

struct ChunkMetadata {
  u8 allocated : 8;  // Must be first.
  ChunkTag tag : 2;
#if SANITIZER_WORDSIZE == 64
  uptr requested_size : 54;
#else
  uptr requested_size : 32;
  uptr padding : 22;
#endif
  u32 stack_trace_id;
};

#if SANITIZER_CAN_USE_ALLOCATOR64
template <typename AddressSpaceViewTy>
struct AP64 {  // Allocator64 parameters. Deliberately using a short name.
  static const uptr kSpaceBeg = __sanitizer::kAllocatorSpace;
  static const uptr kSpaceSize = __sanitizer::kAllocatorSize;
  static const uptr kMetadataSize = sizeof(ChunkMetadata);
  typedef __sanitizer::AllocatorSizeClassMap SizeClassMap;
  typedef NoOpMapUnmapCallback MapUnmapCallback;
  static const uptr kFlags = 0;
  using AddressSpaceView = AddressSpaceViewTy;
};
template <typename AddressSpaceView>
using PrimaryAllocatorASVT = SizeClassAllocator64<AP64<AddressSpaceView>>;
using PrimaryAllocator = PrimaryAllocatorASVT<LocalAddressSpaceView>;
#else  // !SANITIZER_CAN_USE_ALLOCATOR64
template <typename AddressSpaceViewTy>
struct AP32 {
  static const vaddr kSpaceBeg = 0;
  static const u64 kSpaceSize = SANITIZER_MMAP_RANGE_SIZE;
  static const usize kMetadataSize = sizeof(ChunkMetadata);
  typedef __sanitizer::CompactSizeClassMap SizeClassMap;
  static const usize kRegionSizeLog = 20;
  using AddressSpaceView = AddressSpaceViewTy;
  typedef NoOpMapUnmapCallback MapUnmapCallback;
  static const usize kFlags = 0;
};
template <typename AddressSpaceView>
using PrimaryAllocatorASVT = SizeClassAllocator32<AP32<AddressSpaceView>>;
using PrimaryAllocator = PrimaryAllocatorASVT<LocalAddressSpaceView>;
#endif  // SANITIZER_CAN_USE_ALLOCATOR64

template <typename AddressSpaceView>
using AllocatorASVT = CombinedAllocator<PrimaryAllocatorASVT<AddressSpaceView>>;
using Allocator = AllocatorASVT<LocalAddressSpaceView>;
using AllocatorCache = Allocator::AllocatorCache;

Allocator::AllocatorCache *GetAllocatorCache();

int lsan_posix_memalign(void **memptr, usize alignment, usize size,
                        const StackTrace &stack);
void *lsan_aligned_alloc(usize alignment, usize size, const StackTrace &stack);
void *lsan_memalign(usize alignment, usize size, const StackTrace &stack);
void *lsan_malloc(usize size, const StackTrace &stack);
void lsan_free(void *p);
void *lsan_realloc(void *p, usize size, const StackTrace &stack);
void *lsan_reallocarray(void *p, usize nmemb, usize size,
                        const StackTrace &stack);
void *lsan_calloc(usize nmemb, usize size, const StackTrace &stack);
void *lsan_valloc(usize size, const StackTrace &stack);
void *lsan_pvalloc(usize size, const StackTrace &stack);
usize lsan_mz_size(const void *p);

}  // namespace __lsan

#endif  // LSAN_ALLOCATOR_H
