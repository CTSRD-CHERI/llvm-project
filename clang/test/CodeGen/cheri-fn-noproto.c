// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -emit-llvm -o - %s | FileCheck %s

void asctime_r(buf)
char * buf;
{
}

void asctime()
{
  // CHECK: [[TMP:%[0-9]]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  // CHECK-NEXT: call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[TMP]], i64 ptrtoint (void (i8 addrspace(200)*)* @asctime_r to i64))
  asctime_r(0);
}
