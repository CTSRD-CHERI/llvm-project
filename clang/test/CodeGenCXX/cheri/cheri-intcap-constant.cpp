// RUN: %cheri_cc1 -std=c++11 -x c++ -emit-llvm -o - %s | %cheri_FileCheck %s

// Check that constant initialization works even for binary operations
class A {
  public:
    static const __uintcap_t thinFlag = 1;
    static const __uintcap_t reservedFlag = 2;
    static const __uintcap_t flags = thinFlag | reservedFlag;
};

void f() {
  A a;
  __uintcap_t x;
  x = a.thinFlag;
  x = a.reservedFlag;
  x = a.flags;
}

// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 1)
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 2)
// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 3)
