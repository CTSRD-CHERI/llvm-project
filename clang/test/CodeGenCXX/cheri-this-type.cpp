// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi sandbox -emit-llvm -o - %s | FileCheck %s

class A {
  public:
    // CHECK: void @_ZN1A1fEv(%class.A addrspace(200)* %this)
    void f() { }
};

void g() {
  A a;
  a.f();
}
