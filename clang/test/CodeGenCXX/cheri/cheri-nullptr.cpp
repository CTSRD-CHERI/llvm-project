// RUN: %cheri_purecap_cc1 -emit-llvm -std=c++11 -o - %s | %cheri_FileCheck %s

typedef decltype(nullptr) nullptr_t;

// Check that __null doesn't crash the compiler
int main() {
  void* x = __null;  // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %x, align [[$CAP_SIZE]]
  void* y = nullptr; // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %y, align [[$CAP_SIZE]]
  void* z = 0;       // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %z, align [[$CAP_SIZE]]
  nullptr_t nt1 = nullptr; // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %nt1, align [[$CAP_SIZE]]
  nullptr_t nt2 = __null;  // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %nt2, align [[$CAP_SIZE]]
  nullptr_t nt3 = 0;       // CHECK: store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %nt3, align [[$CAP_SIZE]]
  return !!x;
}
