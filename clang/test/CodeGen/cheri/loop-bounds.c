// REQUIRES: mips-registered-target

// When CheriBoundedStackPseudo was marked as isAsCheapAsAMove the cincoffset +
// csetbounds would be placed inside the loop instead of before
// RUN: %cheri128_purecap_cc1 -o - -mllvm -cheri-cap-table-abi=plt -S -O2 %s | FileCheck %s
// XFAIL: *

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
// MachineLICM should hoist this outside of the loop and use a callee-save register:
// CHECK-NOT:  csetbounds $c3
// TODO-CHECK-NEXT:  cincoffset $c18, $c11, 12
// TODO-CHECK-NEXT:  csetbounds $c18, $c18, 4

