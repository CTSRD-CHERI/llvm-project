// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm -std=c++11 -o - %s | FileCheck %s

// Check that __null doesn't crash the compiler
int main() {
  void* x = __null;  // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %x, align 32
  void* y = nullptr; // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %y, align 32
  void* z = 0;       // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %z, align 32
  return !!x;
}
