// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -S -emit-llvm | FileCheck %s
static int foo;
__capability int * bar1 = (__capability int*)&foo;
// CHECK: store i32 addrspace(200)* addrspacecast (i32* @foo to i32 addrspace(200)*), i32 addrspace(200)** @bar1, align 32

