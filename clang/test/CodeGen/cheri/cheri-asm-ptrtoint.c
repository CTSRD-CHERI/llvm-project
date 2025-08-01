// RUN: %riscv32_cheri_purecap_cc1 -S -emit-llvm -o /dev/null %s -verify
// RUN: %riscv64_cheri_purecap_cc1 -S -emit-llvm -o /dev/null %s -verify
// RUN: not %riscv32_cheri_purecap_cc1 -S -o - %s 2>&1 | FileCheck %s
// RUN: not %riscv64_cheri_purecap_cc1 -S -o - %s 2>&1 | FileCheck %s
//
// This asm syntax will fail when it gets to the backend, but at least prevent
// a crash in clang and display the correct error message.

// expected-no-diagnostics
int ptr_to_int(const void *p) {
  int i;
// CHECK: error: couldn't allocate output register for constraint 'r'
  asm volatile("" : /*outputs*/ "=r"(i) : /*inputs*/ "0"(p));
  return i;
}
