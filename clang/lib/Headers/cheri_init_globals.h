/*===---- cheri_init_globals.h - Utility function to initialize global
 *                               capabilities in the c startup code
 *
 * Copyright (c) 2017 Alex Richardson
 * Copyright (c) 2014 Robert N. M. Watson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
\*===----------------------------------------------------------------------===*/

#pragma once
#if !__has_feature(capabilities)
#error "This header requires CHERI capability support"
#endif

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

/* Bump this on every incompatible change */
#define CHERI_INIT_GLOBALS_VERSION 4
#define CHERI_INIT_GLOBALS_NUM_ARGS 7
#define CHERI_INIT_GLOBALS_SUPPORTS_CONSTANT_FLAG 1

struct capreloc {
    __SIZE_TYPE__ capability_location;
    __SIZE_TYPE__ object;
    __SIZE_TYPE__ offset;
    __SIZE_TYPE__ size;
    __SIZE_TYPE__ permissions;
};
static const __SIZE_TYPE__ function_reloc_flag =
   (__SIZE_TYPE__)1 << (__SIZE_WIDTH__ - 1);
static const __SIZE_TYPE__ function_pointer_permissions_mask =
    ~(__CHERI_CAP_PERMISSION_PERMIT_SEAL__ |
      __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ |
      __CHERI_CAP_PERMISSION_PERMIT_STORE__);
static const __SIZE_TYPE__ constant_reloc_flag =
   (__SIZE_TYPE__)1 << (__SIZE_WIDTH__ - 2);
static const __SIZE_TYPE__ constant_pointer_permissions_mask =
    ~(__CHERI_CAP_PERMISSION_PERMIT_SEAL__ |
      __CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ |
      __CHERI_CAP_PERMISSION_PERMIT_STORE_LOCAL__ |
      __CHERI_CAP_PERMISSION_PERMIT_STORE__ |
      __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__);
static const __SIZE_TYPE__ global_pointer_permissions_mask =
    ~(__CHERI_CAP_PERMISSION_PERMIT_SEAL__|
      __CHERI_CAP_PERMISSION_PERMIT_EXECUTE__);

__attribute__((weak))
extern struct capreloc __start___cap_relocs;
__attribute__((weak))
extern struct capreloc __stop___cap_relocs;

__attribute__((weak)) extern void *__capability
__cap_table_start;
__attribute__((weak)) extern void *__capability
__cap_table_end;

/*
 * Sandbox data segments are relocated by moving DDC, since they're compiled as
 * position-dependent executables.
 */
#ifdef CHERI_INIT_GLOBALS_USE_OFFSET
#define cgetaddr_or_offset "cgetoffset"
#define csetaddr_or_offset "csetoffset"
#define cheri_address_or_offset_set(_cap, _val)                                \
  __builtin_cheri_offset_set((_cap), (_val))
#else
#define cgetaddr_or_offset "cgetaddr"
#define csetaddr_or_offset "csetaddr"
#define cheri_address_or_offset_set(_cap, _val)                                \
  __builtin_cheri_address_set((_cap), (_val))
#endif

#define __STRINGIFY2(x) #x
#define __STRINGIFY(x) __STRINGIFY2(x)
#define CGP_PERMISSIONS __STRINGIFY((__CHERI_CAP_PERMISSION_PERMIT_LOAD_CAPABILITY__ | __CHERI_CAP_PERMISSION_PERMIT_LOAD__))

/* By default derive $cgp from $pcc on startup */
#ifndef GET_GCP_BASE_CAPABILITY
/* The initial PCC should have load+load_cap and span the current binary */
#define GET_GCP_BASE_CAPABILITY "cgetpcc $cgp\n\t"
#endif

#if defined(__mips__)

#if __CHERI_CAPABILITY_TABLE__ == 3
/* No need to setup $cgp for pc-relative ABI: */
#define INIT_CGP_REGISTER_ASM
#else
#define INIT_CGP_REGISTER_ASM                                                  \
  ".option pic0\n\t"                                                           \
  /* If we already have a tagged $cgp skip the setup (e.g. called by RTLD) */  \
  "cbts $cgp, .Lskip_cgp_setup\n\t"                                            \
  "nop\n\t"                                                                    \
  GET_GCP_BASE_CAPABILITY                                                      \
  "dla $2, __cap_table_start\n\t"                                              \
  "dla $3, __cap_table_end\n\t"                                                \
  "dsubu $1, $3, $2\n\t"                                                       \
  csetaddr_or_offset " $cgp, $cgp, $2\n\t"                                     \
  "csetbounds $cgp, $cgp, $1\n\t"                                              \
  /* Clear all permissions except LOAD+LOAD_CAP */                             \
  "dli $1, " CGP_PERMISSIONS "\n\t"                                            \
  "candperm $cgp, $cgp, $1\n\t"                                                \
  ".Lskip_cgp_setup: \n\t"
#endif

/*
 * Defines a __start function that sets up $cgp and then branches to
 * c_startup_fn which does the real startup
 */
#define DEFINE_CHERI_START_FUNCTION(c_startup_fn)                              \
  __asm__(".text\n\t"                                                          \
          ".set noreorder\n\t"                                                 \
          ".global __start\n\t"                                                \
          ".ent __start\n\t"                                                   \
          "__start:\n\t"                                                       \
          INIT_CGP_REGISTER_ASM                                                \
          ".set noat\n\t"                                                      \
          /* Setup $c12 correctly in case we are inferring $cgp from $c12 */   \
          ".protected "  #c_startup_fn "\n\t"                                  \
          "lui $1, %pcrel_hi("  #c_startup_fn " - 8)\n\t"                      \
          "daddiu $1, $1, %pcrel_lo("  #c_startup_fn " - 4)\n\t"               \
          "cgetpcc $c12\n\t"                                                   \
          "cincoffset $c12, $c12, $1\n\t"                                      \
          "cjr $c12\n\t"                                                       \
          "nop\n\t"                                                            \
          ".end __start");

#elif defined(__riscv)
/* No DEFINE_CHERI_START_FUNCTION needed; everything is currently PC-relative
 * using AUIPCC. */
#else
#error Unknown architecture
#endif

static __attribute__((always_inline)) void
cheri_init_globals_impl(const struct capreloc *start_relocs,
                        const struct capreloc *stop_relocs, void *data_cap,
                        const void *code_cap, const void *rodata_cap,
                        bool tight_code_bounds, __SIZE_TYPE__ base_addr) {
  data_cap =
      __builtin_cheri_perms_and(data_cap, global_pointer_permissions_mask);
  code_cap =
      __builtin_cheri_perms_and(code_cap, function_pointer_permissions_mask);
  rodata_cap =
      __builtin_cheri_perms_and(rodata_cap, constant_pointer_permissions_mask);
  for (const struct capreloc *reloc = start_relocs; reloc < stop_relocs; reloc++) {
    const void **dest = (const void **)cheri_address_or_offset_set(
        data_cap, reloc->capability_location + base_addr);
    if (reloc->object == 0) {
      /* XXXAR: clang fills uninitialized capabilities with 0xcacaca..., so we
       * we need to explicitly write NULL here */
      *dest = (void*)0;
      continue;
    }
    const void *base_cap;
    bool can_set_bounds = true;
    if ((reloc->permissions & function_reloc_flag) == function_reloc_flag) {
      base_cap = code_cap; /* code pointer */
      /* Do not set tight bounds for functions (unless we are in the plt ABI) */
      can_set_bounds = tight_code_bounds;
    } else if ((reloc->permissions & constant_reloc_flag) ==
               constant_reloc_flag) {
      base_cap = rodata_cap; /* read-only data pointer */
    } else {
      base_cap = data_cap; /* read-write data */
    }
    const void *src =
        cheri_address_or_offset_set(base_cap, reloc->object + base_addr);
    if (can_set_bounds && (reloc->size != 0)) {
      src = __builtin_cheri_bounds_set(src, reloc->size);
    }
    src = __builtin_cheri_offset_increment(src, reloc->offset);
    if ((reloc->permissions & function_reloc_flag) == function_reloc_flag) {
      /* Convert function pointers to sentries: */
      src = __builtin_cheri_seal_entry(src);
    }
    *dest = src;
  }
}

static __attribute__((always_inline)) void
cheri_init_globals_3(void *data_cap, const void *code_cap,
                     const void *rodata_cap) {
  const struct capreloc *start_relocs;
  const struct capreloc *stop_relocs;
  __SIZE_TYPE__ start_addr, end_addr;
#if defined(__mips__)
  __asm__ (".option pic0\n\t"
       "dla %0, __start___cap_relocs\n\t"
       "dla %1, __stop___cap_relocs\n\t"
       :"=r"(start_addr), "=r"(end_addr));
#elif defined(__riscv)
  void * __capability tmp;
  __asm__ (
       "1: auipcc %2, %%pcrel_hi(__start___cap_relocs)\n\t"
       "cincoffset %2, %2, %%pcrel_lo(1b)\n\t"
       cgetaddr_or_offset " %0, %2\n\t"
       "2: auipcc %2, %%pcrel_hi(__stop___cap_relocs)\n\t"
       "cincoffset %2, %2, %%pcrel_lo(2b)\n\t"
       cgetaddr_or_offset " %1, %2\n\t"
       :"=r"(start_addr), "=r"(end_addr), "=&C"(tmp));
#else
#error Unknown architecture
#endif
  __SIZE_TYPE__ relocs_size = end_addr - start_addr;
  /*
   * Always get __cap_relocs relative to the initial $pcc. This should span
   * rodata and rw data, too so we can access __cap_relocs, no matter where it
   * was placed.
   */
  start_relocs = (const struct capreloc *)cheri_address_or_offset_set(
      __builtin_cheri_program_counter_get(), start_addr);
  start_relocs = __builtin_cheri_bounds_set(start_relocs, relocs_size);
  /*
   * Note: with imprecise capabilities start_relocs could have a non-zero offset
   * so we must not use setoffset!
   * TODO: use csetboundsexact and teach the linker to align __cap_relocs.
   */
  stop_relocs =
      (const struct capreloc *)((const char *)start_relocs + relocs_size);

#if __CHERI_CAPABILITY_TABLE__ == 3
  /* pc-relative ABI -> need large bounds on $pcc */
  bool can_set_code_bounds = false;
#else
  bool can_set_code_bounds = true; /* fn-desc/plt ABI -> tight bounds okay */
#endif
  /*
   * We can assume that all relocations in the __cap_relocs section have already
   * been processed so we don't need to add a relocation base address to the
   * location of the capreloc.
   */
  cheri_init_globals_impl(start_relocs, stop_relocs, data_cap, code_cap,
                          rodata_cap, can_set_code_bounds, /*relocbase=*/0);
}

static __attribute__((always_inline, unused)) void
cheri_init_globals_gdc(void *gdc) {
  cheri_init_globals_3(gdc, __builtin_cheri_program_counter_get(), gdc);
}

#ifndef CHERI_INIT_GLOBALS_GDC_ONLY
static __attribute__((always_inline, unused)) void cheri_init_globals(void) {
  cheri_init_globals_gdc(__builtin_cheri_global_data_get());
}
#endif

#ifdef __cplusplus
}
#endif
