// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm -o - %s | FileCheck %s

class A { };

void f() {
  A a1;
  // CHECK: %a1 = alloca %class.A, align 1
  A& a2 = a1;
  // CHECK: %a2 = alloca %class.A addrspace(200)*, align 8
  // CHECK: store %class.A addrspace(200)* %a1, %class.A addrspace(200)* addrspace(200)* %a2, align 8
}
