// RUN: %cheri_purecap_cc1 -std=c++11 -fcxx-exceptions -fexceptions -o - -emit-llvm %s | FileCheck %s

class a {
public:
  static a b;
};
class c {
public:
  c(int *, a);
};
int d;
// CHECK: define dso_local void @_Z3fn1v() {{.+}} personality i8 addrspace(200)* bitcast (i32 (...) addrspace(200)* @__gxx_personality_v0 to i8 addrspace(200)*)
void fn1() {
  // Invoke c::c(int* cap, a)
  // CHECK:       invoke void @_ZN1cC1EPi1a(%class.c addrspace(200)* noundef nonnull align 1 dereferenceable(1) %0, i32 addrspace(200)* noundef @d, i8 inreg %3)
  // CHECK-NEXT:  to label %[[INVOKE_CONT:.+]] unwind label %lpad
  // CHECK: [[INVOKE_CONT]]
  // CHECK-NEXT: ret void
  new c(&d, a::b);
}

// CHECK: declare i32 @__gxx_personality_v0(...)
