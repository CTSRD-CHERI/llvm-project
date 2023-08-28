// RUN: %cheri_purecap_cc1 -emit-llvm -std=c++11 -o - %s | FileCheck %s

typedef decltype(nullptr) nullptr_t;

// Check that __null doesn't crash the compiler
int main() {
  void* x = __null;  // CHECK: ptr addrspace(200) null, ptr addrspace(200) %x, align 16
  void* y = nullptr; // CHECK: ptr addrspace(200) null, ptr addrspace(200) %y, align 16
  void* z = 0;       // CHECK: ptr addrspace(200) null, ptr addrspace(200) %z, align 16
  nullptr_t nt1 = nullptr; // CHECK: ptr addrspace(200) null, ptr addrspace(200) %nt1, align 16
  nullptr_t nt2 = __null;  // CHECK: ptr addrspace(200) null, ptr addrspace(200) %nt2, align 16
  nullptr_t nt3 = 0;       // CHECK: ptr addrspace(200) null, ptr addrspace(200) %nt3, align 16
  return !!x;
}
