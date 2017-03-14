// RUN: %clang_cc1 -triple cheri-unknown-freebsd -emit-llvm -o - %s | FileCheck %s

int main(void) {
  __intcap_t *t;
  // CHECK: %0 = load i8 addrspace(200)**, i8 addrspace(200)*** %t
  if (t) {
  }
  return 0;
}
