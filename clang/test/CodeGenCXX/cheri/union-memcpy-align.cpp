// RUN: %cheri_purecap_cc1 %s -fno-rtti -std=c++11 -o - -emit-llvm | FileCheck %s

union tt {
  void * tt[1];
  struct {
    long xx[2];
  } tq;
};

// CHECK-LABEL: @_Z5test1
void test1(union tt &x1, union tt &x2) {
  // Make sure we preserve the alignment on the copy from the base struct.
  // CHECK: call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 {{%.*}}, ptr addrspace(200) align 16 {{%.*}}, i64 16, i1 false)
  x2.tq = x1.tq;
}

// CHECK-LABEL: @_Z5test2
void test2(union tt &x1, union tt &x2) {
  // Same when copying the whole struct instead of just one member.
  // CHECK: call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 {{%.*}}, ptr addrspace(200) align 16 {{%.*}}, i64 16, i1 false)
  x2 = x1;
}
