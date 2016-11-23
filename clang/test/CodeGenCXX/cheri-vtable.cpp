// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fno-rtti -target-abi sandbox -emit-llvm -o - %s | FileCheck %s

// Check that vtable func pointers have the correct addrspace casts

// CHECK: @_ZTV1A = linkonce_odr unnamed_addr addrspace(200) constant { [3 x i8 addrspace(200)*] } { [i8 addrspace(200)* null, i8 addrspace(200)* null, i8 addrspace(200)* addrspacecast (i8* bitcast (void (%class.A addrspace(200)*)* @_ZN1A1fEv to i8*) to i8 addrspace(200)*)] }, comdat, align 8
class A {
  public:
    virtual void f() {
    }
};

int main(void) {
  A *a = new A;
  a->f();
  return 0;
}
