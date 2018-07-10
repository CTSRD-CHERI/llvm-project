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

// FIXME: this shouldn't be here
#pragma once
#if !__has_feature(capabilities)
#error "This header requires CHERI capability support"
#endif

struct capreloc {
    __UINT64_TYPE__ capability_location;
    __UINT64_TYPE__ object;
    __UINT64_TYPE__ offset;
    __UINT64_TYPE__ size;
    __UINT64_TYPE__ permissions;
};
static const __UINT64_TYPE__ function_reloc_flag = 1ULL << 63;
static const __UINT64_TYPE__ function_pointer_permissions =
  ~0 &
  ~__CHERI_CAP_PERMISSION_PERMIT_STORE_CAPABILITY__ &
  ~__CHERI_CAP_PERMISSION_PERMIT_STORE__;
static const __UINT64_TYPE__ global_pointer_permissions =
  ~0 & ~__CHERI_CAP_PERMISSION_PERMIT_EXECUTE__;


__attribute__((weak))
extern struct capreloc __start___cap_relocs;
__attribute__((weak))
extern struct capreloc __stop___cap_relocs;

__attribute__((weak)) extern void *__capability
__cap_table_start;
__attribute__((weak)) extern void *__capability
__cap_table_end;

#define INIT_CGP_REGISTER_ASM                                                  \
  ".option pic0\n\t"                                                           \
  "dla $2, __cap_table_start\n\t"                                              \
  "beqz $2, .Lskip_cgp_setup\n\t"                                              \
  "nop\n\t"                                                                    \
  "dla $3, __cap_table_end\n\t"                                                \
  "cgetdefault $cgp\n\t"                                                       \
  "csetoffset $cgp, $cgp, $2\n\t"                                              \
  "dsubu $1, $3, $2\n\t"                                                       \
  "csetbounds $cgp, $cgp, $1\n\t"                                              \
  ".Lskip_cgp_setup: \n\t"

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

static __attribute__((always_inline))
void cheri_init_globals_impl(const struct capreloc *start_relocs,
                             const struct capreloc * stop_relocs,
                             void* gdc, void* pcc, __UINT64_TYPE__ relocbase) {
  gdc = __builtin_cheri_perms_and(gdc, global_pointer_permissions);
  pcc = __builtin_cheri_perms_and(pcc, function_pointer_permissions);
  for (const struct capreloc *reloc = start_relocs; reloc < stop_relocs; reloc++) {
    _Bool isFunction = (reloc->permissions & function_reloc_flag) == function_reloc_flag;
    void **dest = __builtin_cheri_offset_set(gdc, reloc->capability_location + relocbase);
    if (reloc->object == 0) {
      /* XXXAR: clang fills uninitialized capabilities with 0xcacaca..., so we
       * we need to explicitly write NULL here */
      *dest = (void*)0;
      continue;
    }
    void *base = isFunction ? pcc : gdc;
    void *src = __builtin_cheri_offset_set(base, reloc->object);
    if (!isFunction && (reloc->size != 0)) {
      src = __builtin_cheri_bounds_set(src, reloc->size);
    }
    src = __builtin_cheri_offset_increment(src, reloc->offset);
    *dest = src;
  }
}

#ifndef ADDITIONAL_CAPRELOC_PROCESSING
#define ADDITIONAL_CAPRELOC_PROCESSING
#endif
static __attribute__((always_inline)) void cheri_init_globals(void) {
  struct capreloc *start_relocs;
  struct capreloc *stop_relocs;
#ifndef __CHERI_CAPABILITY_TABLE__

  /* If we are not using the CHERI capability table we can just synthesize
   * the capabilities for these using the GOT and $ddc */
  start_relocs = &__start___cap_relocs;
  stop_relocs = &__stop___cap_relocs;
#else
  __UINT64_TYPE__ start_addr, end_addr;
  __asm__ (".option pic0\n\t"
       "dla %0, __start___cap_relocs\n\t"
       "dla %1, __stop___cap_relocs\n\t"
       :"=r"(start_addr), "=r"(end_addr));
  long relocs_size = end_addr - start_addr;
  start_relocs = __builtin_cheri_offset_set(__builtin_cheri_global_data_get(), start_addr);
  start_relocs = __builtin_cheri_bounds_set(start_relocs, relocs_size);
  stop_relocs = __builtin_cheri_offset_set(start_relocs, relocs_size);
#endif
  void *gdc = __builtin_cheri_global_data_get();
  void *pcc = __builtin_cheri_program_counter_get();
  /*
   * We can assume that all relocations in the __cap_relocs section have already
   * been processed so we don't need to a a relocation base address to the
   * location of the capreloc.
   */
  cheri_init_globals_impl(start_relocs, stop_relocs, gdc, pcc, /*relocbase=*/0);
}
