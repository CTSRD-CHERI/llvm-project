// RUN: %cheri_purecap_cc1 -x c++ -std=c++11 -emit-llvm -o - %s | %cheri_FileCheck %s
// Check that enum values whereby the underlying enum type is [u]intcap_t are
// converted to i8 addrspace(200)*
typedef __intcap_t a;
enum : a { b, d };
a c() {
  // CHECK: ret i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1)
  return d;
}

void e() {
  // CHECK: [[OFFSET:%.+]] = call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1))
  // CHECK-NEXT: %conv = trunc i64 [[OFFSET]] to i32
  (int(d));
}
