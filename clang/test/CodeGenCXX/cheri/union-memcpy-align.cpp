// RUN: %cheri_purecap_cc1 %s -fno-rtti -std=c++11 -o - -emit-llvm | %cheri_FileCheck %s

union tt {
  void * tt[1];
  struct {
    long xx[2];
  } tq;
};

// CHECK-LABEL: @_Z5test1
void test1(union tt &x1, union tt &x2) {
  // Make sure we preserve the alignment on the copy from the base struct.

  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align [[#CAP_SIZE]] {{.*}}, i8 addrspace(200)* align [[#CAP_SIZE]] {{.*}}, i64 16, i1 false)
  x2.tq = x1.tq;
}

// CHECK-LABEL: @_Z5test2
void test2(union tt &x1, union tt &x2) {
  // Same when copying the whole struct instead of just one member.
  // CHECK: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align [[#CAP_SIZE]] {{.*}}, i8 addrspace(200)* align [[#CAP_SIZE]] {{.*}}, i64 [[#CAP_SIZE]], i1 false)
  x2 = x1;
}
