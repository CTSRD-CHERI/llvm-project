// RUN: %cheri_cc1 %s "-target-abi" "purecap" -emit-llvm  -o - -cheri-linker | FileCheck --check-prefix=CHECK-PURECAP %s
// RUN: %cheri_cc1 %s -emit-llvm  -o - -cheri-linker | FileCheck --check-prefix=CHECK-HYBRID %s


void* __capability foo(void *__capability x){
  // CHECK-HYBRID: ptrtoint
  // CHECK-PURECAP: [[ADDR:%.+]] = call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* {{%.+}})
  // CHECK-PURECAP-NEXT: trunc i64 [[ADDR]] to i32
  int pi = (int)x; // pi contains the result of CToPtr x, which is probably null
  // CHECK-HYBRID: inttoptr
  // CHECK-PURECAP: [[CONV:%.+]] = sext i32 {{%.+}} to i64
  // CHECK-PURECAP-NEXT: call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 [[CONV]])
  return (void* __capability)pi;
}
