// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -S -emit-llvm | FileCheck %s
static int foo;
int * __capability bar1 = (int* __capability)&foo;
// CHECK: store i32 addrspace(200)* addrspacecast (i32* @foo to i32 addrspace(200)*), i32 addrspace(200)** @bar1, align [[$CAP_SIZE:16|32]]

