# REQUIRES: riscv
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -shared %t.o -o %t.so 2>&1 | FileCheck --implicit-check-not=warning: %s

  .type local, @function
local:
  ret
  .size local, . - local

  .type global, @function
  .global global
global:
  ret
  .size global, . - global

  .type undef, @function
  .global undef

  .data
foo:
  .chericap local + 1
  # CHECK: warning: got capability relocation with non-zero addend (0x1) against function global. This may not be supported by the runtime linker.
  .chericap global + 1
  # CHECK: warning: got capability relocation with non-zero addend (0x1) against function undef. This may not be supported by the runtime linker.
  .chericap undef + 1
