// RUN: %clang_cc1 -triple avr-unknown-unknown -emit-llvm %s -o - | FileCheck %s

// CHECK: define void @foo() addrspace(1) #0
__attribute__((interrupt)) void foo(void) { }

// CHECK: attributes #0 = {{{.*interrupt.*}}}
