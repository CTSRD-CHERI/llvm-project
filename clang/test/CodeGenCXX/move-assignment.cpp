// RUN: %clang_cc1 -emit-llvm -std=c++11 -o - %s -triple x86_64-pc-linux-gnu | FileCheck %s -check-prefixes=X86,BOTH
// RUN: %cheri_cc1 -emit-llvm -std=c++11 -o - %s  -target-abi purecap | FileCheck %s -check-prefixes=CHERI,BOTH

struct A {
  A &operator=(A&&);
};

struct B {
  A a;
  int i;
  bool b;
  char c;
  long l;
  float f;
};

void test1() {
  B b1, b2;
  b1 = static_cast<B&&>(b2);
}

// X86-LABEL: define {{.*}} @_ZN1BaSEOS_
// CHERI: define linkonce_odr noundef nonnull align 8 dereferenceable(32) ptr addrspace(200) @_ZN1BaSEOS_(ptr addrspace(200) noundef nonnull align 8 dereferenceable(32) %this, ptr addrspace(200) noundef nonnull align 8 dereferenceable(32) %0) addrspace(200)
// X86:   call noundef nonnull align 1 dereferenceable(1) ptr @_ZN1AaSEOS_(ptr noundef nonnull align 1 dereferenceable(1) %a, ptr noundef nonnull align 1 dereferenceable(1) %a2)
// CHERI: call noundef nonnull align 1 dereferenceable(1) ptr addrspace(200) @_ZN1AaSEOS_(ptr addrspace(200) noundef nonnull align 1 dereferenceable(1) %a, ptr addrspace(200) noundef nonnull align 1 dereferenceable(1) %a2)
// BOTH-NOT: store
// X86:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 {{.+}}, ptr align 4 {{.+}}, i64 24, i1 false)
// CHERI: call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 4 %i, ptr addrspace(200) align 4 %i3, i64 24, i1 false)
// BOTH-NOT: store
// X86:   ret ptr %this1
// CHERI: ret ptr addrspace(200) %this1
