// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -emit-llvm -std=c++11 -o - %s | FileCheck %s

typedef decltype(nullptr) nullptr_t;

// Check that __null doesn't crash the compiler
int main() {
  void* x = __null;  // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %x, align 32
  void* y = nullptr; // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %y, align 32
  void* z = 0;       // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %z, align 32
  nullptr_t nt1 = nullptr; // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %nt1, align 32
  nullptr_t nt2 = __null;  // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %nt2, align 32
  nullptr_t nt3 = 0;       // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %nt3, align 32
  return !!x;
}
