// REQUIRES: mips-registered-target

// Check that all variadic arguments are promoted to i64
// RUN: %cheri_purecap_cc1 -S -o - %s -O2 | FileCheck %s

extern int foo(int, ...);

int test(void) {
  char c = 1;
  short s = 2;
  int i = 3;
  long l = 4;
  return foo(100, c, s, i, l);
}

// CHECK:      daddiu	$1, $zero, 1
// CHECK-NEXT: csd	$1, $zero, 0($c11)
// CHECK-NEXT: daddiu	$1, $zero, 4
// CHECK-NEXT: csd	$1, $zero, 24($c11)
// CHECK-NEXT: daddiu	$1, $zero, 3
// CHECK-NEXT: csd	$1, $zero, 16($c11)
// CHECK-NEXT: daddiu	$1, $zero, 2
// CHECK-NEXT: csd	$1, $zero, 8($c11)
