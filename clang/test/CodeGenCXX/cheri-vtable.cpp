// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fno-rtti -std=c++11 -target-abi sandbox -emit-llvm -o - %s | FileCheck %s

// Check that vtable func pointers have the correct addrspace casts

// CHECK: @_ZTV1B = linkonce_odr unnamed_addr addrspace(200) constant { [5 x i8 addrspace(200)*] } { [5 x i8 addrspace(200)*] [i8 addrspace(200)* null, i8 addrspace(200)* null, i8 addrspace(200)* addrspacecast (i8* bitcast (void (%class.A addrspace(200)*)* @_ZN1A1fEv to i8*) to i8 addrspace(200)*), i8 addrspace(200)* addrspacecast (i8* bitcast (void (%class.B addrspace(200)*)* @_ZN1B1gEv to i8*) to i8 addrspace(200)*), i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @__cxa_deleted_virtual to i8*) to i8 addrspace(200)*)] }, comdat, align 8
// CHECK: @_ZTV1A = linkonce_odr unnamed_addr addrspace(200) constant { [5 x i8 addrspace(200)*] } { [5 x i8 addrspace(200)*] [i8 addrspace(200)* null, i8 addrspace(200)* null, i8 addrspace(200)* addrspacecast (i8* bitcast (void (%class.A addrspace(200)*)* @_ZN1A1fEv to i8*) to i8 addrspace(200)*), i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @__cxa_pure_virtual to i8*) to i8 addrspace(200)*), i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @__cxa_deleted_virtual to i8*) to i8 addrspace(200)*)] }
class A {
  public:
    virtual void f() {}
    virtual void g() = 0;
    virtual void h() = delete;
};

class B : public A {
public:
  virtual void g() {}
};


int main(void) {
  A *a = new B;
  a->f();
  a->g();
  return 0;
}
