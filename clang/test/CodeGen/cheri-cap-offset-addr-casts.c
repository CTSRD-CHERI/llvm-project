// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s 2>&1 | FileCheck %s
//
// Test that IR generation is correct for __cheri_offset and __cheri_addr
//

void check_offset(char *c) {
  long x1 = (__cheri_offset long)c;
  // CHECK: call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  short x2 = (__cheri_offset short)c;
  // CHECK: [[TMP:%[0-9]]] = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // CHECK-NEXT: trunc i64 [[TMP]] to i16
  long x3 = (__cheri_offset short)c;
  // CHECK: [[TMP1:%[0-9]]] = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)*
  // CHECK-NEXT: [[TMP2:%[a-z0-9]+]] = trunc i64 [[TMP1]] to i16
  // CHECK-NEXT: sext i16 [[TMP2]] to i64
}

void check_addr(char *c) {
  long x1 = (__cheri_addr long)c;
  // CHECK: call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)*
  short x2 = (__cheri_addr short)c;
  // CHECK: [[TMP:%[0-9]]] = call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)*
  // CHECK-NEXT: trunc i64 [[TMP]] to i16
  long x3 = (__cheri_addr short)c;
  // CHECK: [[TMP1:%[0-9]]] = call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)*
  // CHECK-NEXT: [[TMP2:%[a-z0-9]+]] = trunc i64 [[TMP1]] to i16
  // CHECK-NEXT: sext i16 [[TMP2]] to i64
}
