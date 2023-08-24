// RUN: %cheri_cc1 -o - %s -emit-llvm | %cheri_FileCheck %s
static int foo;
int * __capability bar1 = (int* __capability)&foo;
// CHECK: @bar1 = global ptr addrspace(200) addrspacecast (ptr @foo to ptr addrspace(200)), align [[#CAP_SIZE]]
