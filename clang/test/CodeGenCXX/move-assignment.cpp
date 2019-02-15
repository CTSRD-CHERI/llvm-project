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
// CHERI: define linkonce_odr dereferenceable(32) %struct.B addrspace(200)* @_ZN1BaSEU3capOS_(%struct.B addrspace(200)* %this, %struct.B addrspace(200)* dereferenceable(32))
// X86:   call dereferenceable(1) %struct.A* @_ZN1AaSEOS_(%struct.A* %a, %struct.A* dereferenceable(1) %a2)
// CHERI: call dereferenceable(1) %struct.A addrspace(200)* @_ZN1AaSEU3capOS_(%struct.A addrspace(200)* %a, %struct.A addrspace(200)* dereferenceable(1) %a2)
// BOTH-NOT: store
// X86:   call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 {{.+}}, i8* align 4 {{.+}}, i64 24, i1 false)
// CHERI: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 4 {{.+}}, i8 addrspace(200)* align 4 {{.+}}, i64 24, i1 false)
// BOTH-NOT: store
// X86:   ret %struct.B* %this1
// CHERI: ret %struct.B addrspace(200)* %this1
