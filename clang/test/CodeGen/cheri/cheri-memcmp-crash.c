// RUN: %cheri_purecap_cc1 -O2 -o - -emit-llvm %s | FileCheck %s
// This code was hitting an assertion due to memcpy optimizations assuming address space zero
int memcmp(const void * __capability, const void * __capability, unsigned long);

struct foo { char a[8]; };

int test(void) {
  struct foo d;
  if (memcmp(d.a, "", sizeof(d)))
    return 0;
  return 1;
}


int a[];
int b;
int test2(void) { return !memcmp(a, &b, sizeof b); }
// CHECK-LABEL: test2
// CHECK: [[LHS:%.+]] = load i32, i32 addrspace(200)* getelementptr inbounds ([1 x i32], [1 x i32] addrspace(200)* @a, i64 0, i64 0), align 4
// CHECK: [[RHS:%.+]] = load i32, i32 addrspace(200)* @b, align 4
// CHECK: icmp eq i32 [[LHS]], [[RHS]]

