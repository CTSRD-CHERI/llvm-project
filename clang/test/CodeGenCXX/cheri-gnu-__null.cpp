// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm -o - %s | FileCheck %s

// Check that __null doesn't crash the compiler
int main() {
  void* x = __null; // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %x, align 32
  return !!x;
}
