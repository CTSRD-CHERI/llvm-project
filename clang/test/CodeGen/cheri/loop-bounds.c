// REQUIRES: mips-registered-target

// When CheriBoundedStackPseudo was marked as isAsCheapAsAMove the cincoffset +
// csetbounds would be placed inside the loop instead of before
// RUN: %cheri128_purecap_cc1 -o - -mllvm -cheri-cap-table-abi=plt -S -O2 %s | FileCheck %s

extern int foo(int* x);

// TODO: the csetbounds should be hoisted out of the loop!

int test(int count) {
    int x = 0;
    for (int i = 0; i < count; i++) {
        x += foo(&x);
    }
    return x;
}

// CHECK-LABEL: test:
// CHECK: cincoffset $c11, $c11, -80
// CHECK-NEXT:  csd $16, $zero, 72($c11)
// CHECK-NEXT:  csc $c19, $zero, 48($c11)
// CHECK-NEXT:  csc $c18, $zero, 32($c11)
// CHECK-NEXT:  csc $c17, $zero, 16($c11)
// CHECK-NEXT:  move $16, $4
// CHECK-NEXT:  blez $16, .LBB0_4
// CHECK-NEXT:  csw $zero, $zero, 12($c11)
// CHECK-NEXT:  cmove $c19, $c26
// MachineLICM should hoist this outside of the loop:
// CHECK-NEXT:  cincoffset $c18, $c11, 12
// CHECK-NEXT:  csetbounds $c18, $c18, 4
// CHECK-NEXT: .LBB0_2:
// CHECK-NEXT:  clcbi $c12, %capcall20(foo)($c19)
// CHECK-NEXT:  cmove $c3, $c18
// CHECK-NEXT:  cjalr $c12, $c17
// CHECK-NEXT:  cmove $c26, $c19
// CHECK-NEXT:  clw $1, $zero, 12($c11)
// CHECK-NEXT:  addu $2, $1, $2
// CHECK-NEXT:  addiu $16, $16, -1
// CHECK-NEXT:  bnez $16, .LBB0_2
// CHECK-NEXT:  csw $2, $zero, 12($c11)
// CHECK-NEXT:  b .LBB0_5
// CHECK-NEXT:  nop
// CHECK-NEXT: .LBB0_4:
// CHECK-NEXT:  addiu $2, $zero, 0
// CHECK-NEXT: .LBB0_5:
// CHECK-NEXT:  sll $2, $2, 0
// CHECK-NEXT:  clc $c17, $zero, 16($c11)
// CHECK-NEXT:  clc $c18, $zero, 32($c11)
// CHECK-NEXT:  clc $c19, $zero, 48($c11)
// CHECK-NEXT:  cld $16, $zero, 72($c11)
// CHECK-NEXT:  cjr $c17
// CHECK-NEXT:  cincoffset $c11, $c11, 80
