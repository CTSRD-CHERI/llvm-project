// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fno-rtti -std=c++11 -target-abi sandbox -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fno-rtti -std=c++11 -target-abi sandbox -emit-llvm -o /dev/null \
// RUN:   -fdump-vtable-layouts %s 2>&1 | FileCheck -check-prefix=CHECK-VTABLE-LAYOUT %s


// Check that vtable func pointers have the correct addrspace casts

// CHECK: @_ZTV1B = linkonce_odr unnamed_addr addrspace(200) constant { [5 x i8 addrspace(200)*] } { [5 x i8 addrspace(200)*] [i8 addrspace(200)* null, i8 addrspace(200)* null, i8 addrspace(200)* addrspacecast (i8* bitcast (void (%class.A addrspace(200)*)* @_ZN1A1fEv to i8*) to i8 addrspace(200)*), i8 addrspace(200)* addrspacecast (i8* bitcast (void (%class.B addrspace(200)*)* @_ZN1B1gEv to i8*) to i8 addrspace(200)*), i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @__cxa_deleted_virtual to i8*) to i8 addrspace(200)*)] }, comdat, align 32
// CHECK: @_ZTV1A = linkonce_odr unnamed_addr addrspace(200) constant { [5 x i8 addrspace(200)*] } { [5 x i8 addrspace(200)*] [i8 addrspace(200)* null, i8 addrspace(200)* null, i8 addrspace(200)* addrspacecast (i8* bitcast (void (%class.A addrspace(200)*)* @_ZN1A1fEv to i8*) to i8 addrspace(200)*), i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @__cxa_pure_virtual to i8*) to i8 addrspace(200)*), i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @__cxa_deleted_virtual to i8*) to i8 addrspace(200)*)] }, comdat, align 32


// CHECK-VTABLE-LAYOUT:      Vtable for 'A' (5 entries).
// CHECK-VTABLE-LAYOUT-NEXT:    0 | offset_to_top (0)
// CHECK-VTABLE-LAYOUT-NEXT:    1 | A RTTI
// CHECK-VTABLE-LAYOUT-NEXT:        -- (A, 0) vtable address --
// CHECK-VTABLE-LAYOUT-NEXT:    2 | void A::f()
// CHECK-VTABLE-LAYOUT-NEXT:    3 | void A::g() [pure]
// CHECK-VTABLE-LAYOUT-NEXT:    4 | void A::h() [deleted]

// CHECK-VTABLE-LAYOUT:      VTable indices for 'A' (3 entries).
// CHECK-VTABLE-LAYOUT-NEXT:    0 | void A::f()
// CHECK-VTABLE-LAYOUT-NEXT:    1 | void A::g()
// CHECK-VTABLE-LAYOUT-NEXT:    2 | void A::h()

// CHECK-VTABLE-LAYOUT:      Vtable for 'B' (5 entries).
// CHECK-VTABLE-LAYOUT-NEXT:    0 | offset_to_top (0)
// CHECK-VTABLE-LAYOUT-NEXT:    1 | B RTTI
// CHECK-VTABLE-LAYOUT-NEXT:        -- (A, 0) vtable address --
// CHECK-VTABLE-LAYOUT-NEXT:        -- (B, 0) vtable address --
// CHECK-VTABLE-LAYOUT-NEXT:    2 | void A::f()
// CHECK-VTABLE-LAYOUT-NEXT:    3 | void B::g()
// CHECK-VTABLE-LAYOUT-NEXT:    4 | void A::h() [deleted]

// CHECK-VTABLE-LAYOUT:      VTable indices for 'B' (1 entries).
// CHECK-VTABLE-LAYOUT-NEXT:    1 | void B::g()

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
