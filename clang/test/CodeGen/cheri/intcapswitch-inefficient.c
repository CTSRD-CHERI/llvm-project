// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm | FileCheck %s
// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm -O2 | FileCheck %s -check-prefix CHECK-OPT
// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm -O3 | FileCheck %s -check-prefix CHECK-OPT

#define A (__intcap_t)1
#define B (__intcap_t)(void*)2
#define C (__intcap_t)3
#define D (__intcap_t)(void*)5

int y(void) {
  // FIXME: C + 1 is not constant folded
  __intcap_t foo = C + 1;
  // CHECK: switch i64  %intcap.vaddr, label %sw.epilog [
  switch (foo)
  {
    // CHECK-NEXT: i64 1, label %sw.bb
    case A: return 4;
    // CHECK-NEXT: i64 2, label %sw.bb1
    case B: return 5;
    // CHECK-NEXT: i64 3, label %sw.bb2
    // CHECK-NEXT: i64 4, label %sw.bb2
    // CHECK-NEXT: i64 5, label %sw.bb2
    case C...D: return 6;
  }
  // with optimization this switch gets constant folded:
  // CHECK-OPT:      define dso_local signext i32 @y()
  // CHECK-OPT-NEXT: entry:
  // CHECK-OPT-NEXT:   ret i32 6
  // CHECK-OPT-NEXT: }
  return 0;
}
