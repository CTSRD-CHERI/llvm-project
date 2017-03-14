// RUN: %clang_cc1 -triple cheri-unknown-freebsd -emit-llvm -o - %s | FileCheck %s

int main(void) {
  char * __capability p;
  // CHECK: %0 = load i8 addrspace(200)*, i8 addrspace(200)** %p
  // CHECK-NEXT: %cmp = icmp eq i8 addrspace(200)* %0, null
  if (p == (void*)0) {
  }
  // CHECK: %1 = load i8 addrspace(200)*, i8 addrspace(200)** %p
  // CHECK-NEXT: %cmp1 = icmp eq i8 addrspace(200)* null, %1
  else if ((void*)0 == p) {
  }
  return 0;
}
