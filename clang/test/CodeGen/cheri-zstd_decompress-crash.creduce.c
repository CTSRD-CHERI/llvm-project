// RUN: %cheri_cc1 -target-abi n64 -O0 -emit-llvm -o - %s | FileCheck %s -check-prefixes CHECK,N64
// RUN: %cheri_purecap_cc1 -O0 -emit-llvm -o - %s | FileCheck %s -check-prefixes CHECK,PURECAP
#define ZSTD_PREFETCH() __builtin_prefetch(0)
int fn1(void) {
  ZSTD_PREFETCH();
  return 0;
}

// Another function to check non-null parameter
void prefetch(void* arg) {
  __builtin_prefetch(arg);
}

// CHECK-LABEL: define i32 @fn1()
// CHECK: call void @llvm.prefetch(
// N64-SAME: i8* null,
// PURECAP-SAME: i8* addrspacecast (i8 addrspace(200)* null to i8*),
// CHECK-SAME: i32 0, i32 3, i32 1)

// CHECK-LABEL: declare void @llvm.prefetch(i8* nocapture readonly, i32, i32, i32)


// CHECK-LABEL: define void @prefetch
// N64-SAME: (i8*{{.*}}) #0 {
// PURECAP-SAME: (i8 addrspace(200)*{{.*}}) #0 {
// N64: [[PREFETCH_ARG:%.+]] = load i8*, i8** {{%.+}}, align 8
// PURECAP: [[AS200_ARG:%.+]] = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* {{%.+}}, align {{16|32}}
// PURECAP: [[PREFETCH_ARG:%.+]] = addrspacecast i8 addrspace(200)* [[AS200_ARG]] to i8*
// CHECK: call void @llvm.prefetch(i8* [[PREFETCH_ARG]], i32 0, i32 3, i32 1)
