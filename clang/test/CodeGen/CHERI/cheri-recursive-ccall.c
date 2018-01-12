// For some reason TBAA is broken with recursive cheri_ccalls (cheri_stdio.c)
// RUN: not %cheri_purecap_cc1 -emit-llvm -O2 -o - %s
// RUN: not %cheri_purecap_cc1 -emit-llvm -O2 -o - %s 2>&1 | FileCheck %s
// CHECK: Access type node must be a valid scalar type
struct {
  void *a;
  void *b;
} c;
__attribute__((cheri_ccall)) __attribute__((cheri_method_class(c))) int
d(void) {
  d(); // FIXME: if I comment this it works
  return 0;
}
