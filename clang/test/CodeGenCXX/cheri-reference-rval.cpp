// RUN: %clang_cc1 -triple cheri-unknown-freebsd -emit-llvm -o - %s | FileCheck %s

__intcap_t g();

void f() {
  __intcap_t&& a2 = g();
  // CHECK: %0 = load i8 addrspace(200)**, i8 addrspace(200)*** %a2
  if (a2 == 0) { }
}
