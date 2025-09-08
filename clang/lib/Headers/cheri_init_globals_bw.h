#ifndef CHERI_INIT_GLOBALS_BW_H_INCLUDED
#define CHERI_INIT_GLOBALS_BW_H_INCLUDED

#ifndef __riscv_zcheripurecap
#error "This header can only be used with zcheripurecap"
#endif

// TODO - Remove once all references have been updated.
#include <cheri_init_globals.h>

#define ELF64_R_TYPE(INFO) ((INFO) & 0xFFFFFFFF)
#define ELF32_R_TYPE(INFO) ((INFO) & 0xFF)
#if __CHERI_ADDRESS_BITS__ == 32
#define R_TYPE ELF32_R_TYPE
#else
#define R_TYPE ELF64_R_TYPE
#endif

struct ELFRela {
  __SIZE_TYPE__ offset;
  __SIZE_TYPE__ info;
  __SIZE_TYPE__ addend;
};

// Bit of a hack as we should include it from llvm
#ifndef R_RISCV_CHERI_RELATIVE
#define R_RISCV_CHERI_RELATIVE 202
#endif

static __attribute__((always_inline)) void cheri_init_globals_cbuildcap_impl(
    const struct ELFRela *start_rela, const struct ELFRela *stop_rela,
    void *__capability data_cap, const void *__capability code_cap,
    const void *__capability rodata_cap, bool tight_code_bounds,
    __SIZE_TYPE__ base_addr, bool init_with_cbld) {
  for (const struct ELFRela *rela = start_rela; rela < stop_rela; rela++) {
    if (R_TYPE(rela->info) != R_RISCV_CHERI_RELATIVE)
      continue;
    const void *__capability *__capability dest =
        (const void *__capability *__capability)__builtin_cheri_address_set(
            data_cap, rela->offset + base_addr);
    const __SIZE_TYPE__ addr = __builtin_cheri_address_get(*dest);
    const __SIZE_TYPE__ perms = __builtin_cheri_perms_get(*dest);
    const __SIZE_TYPE__ is_sealed = __builtin_cheri_sealed_get(*dest);
    const __SIZE_TYPE__ length = __builtin_cheri_length_get(*dest);
    const bool is_fn = perms & __CHERI_CAP_PERMISSION_EXECUTE__;
    const bool is_rw = !is_fn && (perms & __CHERI_CAP_PERMISSION_WRITE__);
    const bool can_set_bounds = !is_fn || (is_fn && tight_code_bounds);

    if (addr == 0) {
      *dest = (void *__capability)0;
      continue;
    }

    const void *__capability base_cap;
    if (is_fn) {
      base_cap = code_cap;
    } else if (is_rw) {
      base_cap = data_cap;
    } else {
      base_cap = rodata_cap;
    }

    const void *__capability src;
    // if we can't set tight code bounds on the fn then we have
    // to initialize it manually from the base cap.
    // TODO - can we initialize the bounds correctly in the linker
    if (!init_with_cbld) {
      src = __builtin_cheri_address_set(base_cap, addr + base_addr);
      if (can_set_bounds)
        src = __builtin_cheri_bounds_set(src, length);
      src = __builtin_cheri_offset_increment(src, rela->addend);
      src = __builtin_cheri_perms_and(src, perms);
      if (is_sealed) {
        src = __builtin_cheri_seal_entry(src);
      }
    } else {
      src = __builtin_cheri_cap_build(base_cap, (__intcap)*dest);
      // incrementing sealed caps will clear their tag
      // TODO - can exec caps ever have a non-zero addend?
      if (!is_sealed)
        src = __builtin_cheri_offset_increment(src, rela->addend);
    }
    *dest = src;
  }
}

static __attribute__((always_inline)) void
cheri_init_globals_cbuildcap(void *__capability data_cap,
                             const void *__capability code_cap,
                             const void *__capability rodata_cap) {
  const struct ELFRela *start_relocs;
  const struct ELFRela *stop_relocs;
  __SIZE_TYPE__ start_addr, stop_addr;
#if !defined(__CHERI_PURE_CAPABILITY__)
  __asm__(
          ".weak __rela_dyn_start\n\t"
          ".hidden __rela_dyn_start\n\t"
          "lla %0, __rela_dyn_start\n\t"
          ".weak __rela_dyn_end\n\t"
          ".hidden __rela_dyn_end\n\t"
          "lla %1, __rela_dyn_end\n\t"
          : "=r"(start_addr), "=r"(stop_addr));
#else
  void *__capability tmp;
  __asm__(
          ".weak __rela_dyn_start\n\t"
          ".hidden __rela_dyn_start\n\t"
          "llc %2, __rela_dyn_start\n\t"
          "cgetaddr %0, %2\n\t"
          ".weak __rela_dyn_end\n\t"
          ".hidden __rela_dyn_end\n\t"
          "llc %2, __rela_dyn_end\n\t"
          "cgetaddr %1, %2\n\t"
          : "=r"(start_addr), "=r"(stop_addr), "=&C"(tmp));
#endif

  if (start_addr == 0 || stop_addr == 0)
    return;

#if !defined(__CHERI_PURE_CAPABILITY__)
  start_relocs = (const struct ELFRela *)(__UINTPTR_TYPE__)start_addr;
  stop_relocs = (const struct ELFRela *)(__UINTPTR_TYPE__)stop_addr;
#else
  __SIZE_TYPE__ relocs_size = stop_addr - start_addr;
  /*
   * Always get __cap_relocs relative to the initial $pcc. This should span
   * rodata and rw data, too so we can access __cap_relocs, no matter where it
   * was placed.
   */
  start_relocs = (const struct ELFRela *)__builtin_cheri_address_set(
      __builtin_cheri_program_counter_get(), start_addr);
  start_relocs = __builtin_cheri_bounds_set(start_relocs, relocs_size);
  /*
   * Note: with imprecise capabilities start_relocs could have a non-zero offset
   * so we must not use setoffset!
   * TODO: use csetboundsexact and teach the linker to align __cap_relocs.
   */
  stop_relocs =
      (const struct ELFRela *)(const void *)((const char *)start_relocs +
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
  cheri_init_globals_cbuildcap_impl(start_relocs, stop_relocs, data_cap,
                                    code_cap, rodata_cap, can_set_code_bounds,
                                    /*relocbase=*/0, /*init_with_cbld*/ false);
}

#endif // CHERI_INIT_GLOBALS_BW_H_INCLUDED
