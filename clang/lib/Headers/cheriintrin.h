/*-------------- cheriintrin.h - CHERI intrinsics -----------------------
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2018-2020 Alex Richardson
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract FA8750-10-C-0237
 * ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory (Department of Computer Science and
 * Technology) under DARPA contract HR0011-18-C-0016 ("ECATS"), as part of the
 * DARPA SSITH research programme.
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

#ifndef __CHERIINTRIN_H
#define __CHERIINTRIN_H

#if !__has_feature(capabilities)
#error "<cheriintrin.h> should only be included when CHERI is supported"
#endif

/*
 * Provide shorter names for CHERI builtins that are portable across all
 * CHERI-enabled architectures:
 */
#define cheri_address_get(x) __builtin_cheri_address_get(x)
#define cheri_address_set(x, y) __builtin_cheri_address_set((x), (y))
#define cheri_base_get(x) __builtin_cheri_base_get(x)
#define cheri_length_get(x) __builtin_cheri_length_get(x)
#define cheri_offset_get(x) __builtin_cheri_offset_get(x)
#define cheri_offset_set(x, y) __builtin_cheri_offset_set((x), (y))
#define cheri_tag_clear(x) __builtin_cheri_tag_clear(x)
#define cheri_tag_get(x) __builtin_cheri_tag_get(x)
#define cheri_is_valid(x) __builtin_cheri_tag_get(x)
#define cheri_is_invalid(x) (!__builtin_cheri_tag_get(x))
#define cheri_is_equal_exact(x, y) __builtin_cheri_equal_exact((x), (y))
#define cheri_is_subset(x, y) __builtin_cheri_subset_test((x), (y))

/* Bounds setting intrinsics: */
#define cheri_representable_length(x)                                          \
  __builtin_cheri_round_representable_length(x)
#define cheri_representable_alignment_mask(x)                                  \
  __builtin_cheri_representable_alignment_mask(x)
#define cheri_bounds_set(x, y) __builtin_cheri_bounds_set((x), (y))
#define cheri_bounds_set_exact(x, y) __builtin_cheri_bounds_set_exact((x), (y))

/* Object types, sealing and unsealing: */
typedef long cheri_otype_t;
#if defined(__mips__) || defined(__riscv)
/* CHERI-MIPS and CHERI-RISC-V use negative numbers for hardware-interpreted
 * otypes */
#define CHERI_OTYPE_UNSEALED ((cheri_otype_t)-1)
#define CHERI_OTYPE_SENTRY ((cheri_otype_t)-2)
#elif defined(__aarch64__)
#define CHERI_OTYPE_UNSEALED ((cheri_otype_t)0)
#define CHERI_OTYPE_SENTRY ((cheri_otype_t)1)
#else
#error "Unknown architecture"
#endif
#define cheri_type_get(x) __builtin_cheri_type_get(x)
#define cheri_is_sealed(x) __builtin_cheri_sealed_get(x)
#define cheri_is_sentry(x) (__builtin_cheri_type_get(x) == CHERI_OTYPE_SENTRY)
#define cheri_is_unsealed(x) (!__builtin_cheri_sealed_get(x))
/* TODO: builtins for indirect sentries */
#define cheri_sentry_create(x) __builtin_cheri_seal_entry(x)
#define cheri_seal(x, y) __builtin_cheri_seal((x), (y))
#define cheri_unseal(x, y) __builtin_cheri_unseal((x), (y))

/* Reconstruct capabilities from raw data: */
#define cheri_cap_build(x, y) __builtin_cheri_cap_build((x), (y))
#define cheri_seal_conditionally(x, y)                                         \
  __builtin_cheri_conditional_seal((x), (y))
#define cheri_type_copy(x, y) __builtin_cheri_cap_type_copy((x), (y))

/* Capability permissions: */
typedef enum __attribute__((flag_enum, enum_extensibility(open))) {
  CHERI_PERM_GLOBAL = __CHERI_CAP_PERMISSION_GLOBAL__,
  CHERI_PERM_EXECUTE = __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__,
  CHERI_PERM_LOAD = __CHERI_CAP_PERMISSION_PERMIT_LOAD__,
  CHERI_PERM_STORE = __CHERI_CAP_PERMISSION_PERMIT_STORE__,
  CHERI_PERM_LOAD_CAP = __CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__,
  CHERI_PERM_STORE_CAP = __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__,
  CHERI_PERM_STORE_LOCAL_CAP = __CHERI_CAP_PERMISSION_PERMIT_STORE_LOCAL__,
  CHERI_PERM_SEAL = __CHERI_CAP_PERMISSION_PERMIT_SEAL__,
  CHERI_PERM_INVOKE = __CHERI_CAP_PERMISSION_PERMIT_INVOKE__,
  CHERI_PERM_UNSEAL = __CHERI_CAP_PERMISSION_PERMIT_UNSEAL__,
  CHERI_PERM_SYSTEM_REGS = __CHERI_CAP_PERMISSION_ACCESS_SYSTEM_REGISTERS__,
  /* TODO: architecture-dependent permissions */
} cheri_perms_t;
#define cheri_perms_get(x) ((cheri_perms_t)(__builtin_cheri_perms_get(x)))
#define cheri_perms_and(x, y) __builtin_cheri_perms_and((x), (__SIZE_TYPE__)(y))
#define cheri_perms_clear(x, y)                                                \
  __builtin_cheri_perms_and((x), ~(__SIZE_TYPE__)(y))

/* Accessors for capability registers. Currently exposes DDC and PCC. */
#define cheri_ddc_get() __builtin_cheri_global_data_get()
#define cheri_pcc_get() __builtin_cheri_program_counter_get()

/* Partially portable builtins: */
/* Note: {get,set}flags does nothing for MIPS, but can still be used. */
#define cheri_flags_get(x) __builtin_cheri_flags_get(x)
#define cheri_flags_set(x, y) __builtin_cheri_flags_set((x), (y))
#define cheri_tags_load(x) __builtin_cheri_cap_load_tags(x)

/*
 * Alignment builtins: Not CHERI-specific, but motivated by CHERI.
 * https://clang.llvm.org/docs/LanguageExtensions.html#alignment-builtins
 */
#define cheri_align_up(x, alignment) __builtin_align_up((x), (alignment))
#define cheri_align_down(x, alignment) __builtin_align_down((x), (alignment))
#define cheri_is_aligned(x, alignment) __builtin_is_aligned((x), (alignment))

#endif /* __CHERIINTRIN_H */
