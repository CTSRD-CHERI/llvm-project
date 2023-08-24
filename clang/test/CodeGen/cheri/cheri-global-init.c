// RUN: %riscv64_cheri_purecap_cc1 %s -emit-llvm  -o - | FileCheck %s

int a[5];
int *b[] = {&a[2], &a[1], a};
// CHECK: @b = addrspace(200) global [3 x ptr addrspace(200)] [ptr addrspace(200) getelementptr (i8, ptr addrspace(200) @a, i64 8), ptr addrspace(200) getelementptr (i8, ptr addrspace(200) @a, i64 4), ptr addrspace(200) @a], align 16
// CHECK-NOT: __cxx_global_var_init
