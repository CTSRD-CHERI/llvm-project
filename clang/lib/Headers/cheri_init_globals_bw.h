#ifndef CHERI_INIT_GLOBALS_BW_H_INCLUDED
#define CHERI_INIT_GLOBALS_BW_H_INCLUDED

#if !__has_feature(capabilities)
#error "This header requires CHERI capability support"
#endif

#if !defined(__riscv)
#error "This header can only be used with CHERI-RISCV"
#endif

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

struct capreloc {
  __SIZE_TYPE__ capability_location;
  __SIZE_TYPE__ object;
  __SIZE_TYPE__ offset;
  __SIZE_TYPE__ size;
  __SIZE_TYPE__ permissions;
};

#define GET_BIT_MASK(OFF_BITS) ~(__SIZE_TYPE__)(OFF_BITS)

static const __SIZE_TYPE__ function_reloc_flag = (__SIZE_TYPE__)1
                                                 << (__SIZE_WIDTH__ - 1);

static const __SIZE_TYPE__ function_pointer_permissions_mask =
    GET_BIT_MASK(__CHERI_BW_CAP_PERMISSION_WRITE__);

static const __SIZE_TYPE__ constant_reloc_flag = (__SIZE_TYPE__)1
                                                 << (__SIZE_WIDTH__ - 2);

static const __SIZE_TYPE__ constant_pointer_permissions_mask = GET_BIT_MASK(
    __CHERI_BW_CAP_PERMISSION_WRITE__ | __CHERI_BW_CAP_PERMISSION_EXECUTE__);

static const __SIZE_TYPE__ global_pointer_permissions_mask =
    GET_BIT_MASK(__CHERI_BW_CAP_PERMISSION_EXECUTE__);

__attribute__((__weak__)) extern struct capreloc __start___cap_relocs;
__attribute__((__weak__)) extern struct capreloc __stop___cap_relocs;


/* By default derive $cgp from $pcc on startup */
#ifndef GET_GCP_BASE_CAPABILITY
/* The initial PCC should have load+load_cap and span the current binary */
#define GET_GCP_BASE_CAPABILITY "cgetpcc $cgp\n\t"
#endif

static __attribute__((always_inline)) void cheri_init_globals_impl(
    const struct capreloc *start_relocs, const struct capreloc *stop_relocs,
    void *__capability data_cap, const void *__capability code_cap,
    const void *__capability rodata_cap, bool tight_code_bounds,
    __SIZE_TYPE__ base_addr) {
  data_cap =
      __builtin_cheri_perms_and(data_cap, global_pointer_permissions_mask);
  code_cap =
      __builtin_cheri_perms_and(code_cap, function_pointer_permissions_mask);
  rodata_cap =
      __builtin_cheri_perms_and(rodata_cap, constant_pointer_permissions_mask);
  for (const struct capreloc *reloc = start_relocs; reloc < stop_relocs;
       reloc++) {
    const void *__capability *__capability dest =
        (const void *__capability *__capability)__builtin_cheri_address_set(
            data_cap, reloc->capability_location + base_addr);
    if (reloc->object == 0) {
      /* XXXAR: clang fills uninitialized capabilities with 0xcacaca..., so we
       * we need to explicitly write NULL here */
      *dest = (void *__capability)0;
      continue;
    }
    const void *__capability base_cap;
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
    const void *__capability src =
        __builtin_cheri_address_set(base_cap, reloc->object + base_addr);
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
cheri_init_globals_3(void *__capability data_cap,
                     const void *__capability code_cap,
                     const void *__capability rodata_cap) {
  const struct capreloc *start_relocs;
  const struct capreloc *stop_relocs;
  __SIZE_TYPE__ start_addr, stop_addr;
#if !defined(__CHERI_PURE_CAPABILITY__)
  __asm__("lla %0, __start___cap_relocs\n\t"
          "lla %1, __stop___cap_relocs\n\t"
          : "=r"(start_addr), "=r"(stop_addr));
#else
  void *__capability tmp;
  __asm__("llc %2, __start___cap_relocs\n\t"
          "cgetaddr %0, %2\n\t"
          "llc %2, __stop___cap_relocs\n\t"
          "cgetaddr %1, %2\n\t"
          : "=r"(start_addr), "=r"(stop_addr), "=&C"(tmp));
#endif

#if !defined(__CHERI_PURE_CAPABILITY__)
  start_relocs = (const struct capreloc *)(__UINTPTR_TYPE__)start_addr;
  stop_relocs = (const struct capreloc *)(__UINTPTR_TYPE__)stop_addr;
#else
  __SIZE_TYPE__ relocs_size = stop_addr - start_addr;
  /*
   * Always get __cap_relocs relative to the initial $pcc. This should span
   * rodata and rw data, too so we can access __cap_relocs, no matter where it
   * was placed.
   */
  start_relocs = (const struct capreloc *)__builtin_cheri_address_set(
      __builtin_cheri_program_counter_get(), start_addr);
  start_relocs = __builtin_cheri_bounds_set(start_relocs, relocs_size);
  /*
   * Note: with imprecise capabilities start_relocs could have a non-zero offset
   * so we must not use setoffset!
   * TODO: use csetboundsexact and teach the linker to align __cap_relocs.
   */
  stop_relocs =
      (const struct capreloc *)(const void *)((const char *)start_relocs +
                                              relocs_size);
#endif

#if !defined(__CHERI_PURE_CAPABILITY__) || __CHERI_CAPABILITY_TABLE__ == 3
  /* pc-relative or hybrid ABI -> need large bounds on $pcc */
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
cheri_init_globals_gdc(void *__capability gdc) {
  cheri_init_globals_3(gdc, __builtin_cheri_program_counter_get(), gdc);
}

#ifndef CHERI_INIT_GLOBALS_GDC_ONLY
static __attribute__((always_inline, unused)) void cheri_init_globals(void) {
  cheri_init_globals_gdc(__builtin_cheri_global_data_get());
}
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CHERI_INIT_GLOBALS_BW_H_INCLUDED
