// RUN: %cheri_purecap_cc1 -std=gnu++1z -o - -emit-llvm -O0 %s | FileCheck %s

template <__uintcap_t c> struct d {
  static int x() { return (int)c; }
};

extern "C" int f() { return d<0>::x(); }
extern "C" int f2() { return d<1>::x(); }
extern "C" int f3() { return d<__UINT64_MAX__ + 1>::x(); }
extern "C" int f4() { return d<__UINT64_MAX__ + 4>::x(); }


// CHECK: define signext i32 @f() addrspace(200) #0 {
// CHECK-NEXT: entry:
// CHECK-NEXT:   %call = call signext i32 @_ZN1dILu11__uintcap_t0EE1xEv()
// CHECK-NEXT:   ret i32 %call
// CHECK-NEXT: }
// CHECK-EMPTY:
// CHECK-NEXT: ; Function Attrs: noinline nounwind optnone
// CHECK-NEXT: define linkonce_odr signext i32 @_ZN1dILu11__uintcap_t0EE1xEv() addrspace(200) #0 comdat align 2 {
// CHECK-NEXT: entry:
// CHECK-NEXT:   ret i32 0
// CHECK-NEXT: }
// CHECK-EMPTY:
// CHECK-NEXT:  Function Attrs: noinline nounwind optnone
// CHECK-NEXT: define signext i32 @f2() addrspace(200) #0 {
// CHECK-NEXT: entry:
// CHECK-NEXT:   %call = call signext i32 @_ZN1dILu11__uintcap_t1EE1xEv()
// CHECK-NEXT:   ret i32 %call
// CHECK-NEXT: }
// CHECK-EMPTY:
// CHECK-NEXT: ; Function Attrs: noinline nounwind optnone
// CHECK-NEXT: define linkonce_odr signext i32 @_ZN1dILu11__uintcap_t1EE1xEv() addrspace(200) #0 comdat align 2 {
// CHECK-NEXT: entry:
// CHECK-NEXT:   ret i32 1
// CHECK-NEXT: }
// CHECK-EMPTY:
// CHECK-NEXT:  Function Attrs: noinline nounwind optnone
// CHECK-NEXT: define signext i32 @f3() addrspace(200) #0 {
// CHECK-NEXT: entry:
// CHECK-NEXT:   %call = call signext i32 @_ZN1dILu11__uintcap_t0EE1xEv()
// CHECK-NEXT:   ret i32 %call
// CHECK-NEXT: }
// CHECK-EMPTY:
// CHECK-NEXT:  Function Attrs: noinline nounwind optnone
// CHECK-NEXT: define signext i32 @f4() addrspace(200) #0 {
// CHECK-NEXT: entry:
// CHECK-NEXT:   %call = call signext i32 @_ZN1dILu11__uintcap_t3EE1xEv()
// CHECK-NEXT:   ret i32 %call
// CHECK-NEXT: }
// CHECK-EMPTY:
// CHECK-NEXT:  Function Attrs: noinline nounwind optnone
// CHECK-NEXT: define linkonce_odr signext i32 @_ZN1dILu11__uintcap_t3EE1xEv() addrspace(200) #0 comdat align 2 {
// CHECK-NEXT: entry:
// CHECK-NEXT:   ret i32 3
// CHECK-NEXT: }
