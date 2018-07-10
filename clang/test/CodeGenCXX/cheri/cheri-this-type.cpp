// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s

class A {
  public:
    // CHECK: void @_ZN1A1fEv(%class.A addrspace(200)* %this)
    void f() { }
};

void g() {
  A a;
  a.f();
}
