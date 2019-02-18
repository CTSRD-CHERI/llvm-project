// RUN: %cheri_clang -S -o - -O2 %s -mllvm -debug-only="mips-delay-slot-filler" -mllvm -print-before="mips-delay-slot-filler" -mllvm -print-after="mips-delay-slot-filler" | FileCheck %s
// REQUIRES: asserts
// XFAIL: *
// https://github.com/CTSRD-CHERI/llvm-project/issues/290

int *__capability test(int *__capability inCap, int test, int inInt) {
  int *__capability
  ret = inCap;
  if (test) {
    ret += inInt + 4;
  }
  return ret;
}

// CHECK-LABEL: # %bb.0:                                # %entry
// CHECK-NEXT: daddiu	$sp, $sp, -16
// CHECK-NEXT: sd	$fp, 8($sp)             # 8-byte Folded Spill
// CHECK-NEXT: beqz	$4, .LBB0_2
// CHECK-NEXT: move	$fp, $sp
// CHECK-NEXT: # %bb.1:                                # %if.then
// CHECK-NEXT: addiu	$1, $5, 4
// CHECK-NEXT: dsll	$1, $1, 2
// CHECK-NEXT: cincoffset	$c3, $c3, $1
// CHECK-NEXT: .LBB0_2:                                # %if.end
// CHECK-NEXT: move	$sp, $fp
// CHECK-NEXT: ld	$fp, 8($sp)             # 8-byte Folded Reload
// CHECK-NEXT: jr	$ra
// CHECK-NEXT: daddiu	$sp, $sp, 16
