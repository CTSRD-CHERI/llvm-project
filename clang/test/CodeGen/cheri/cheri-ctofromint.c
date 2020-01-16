// RUN: %cheri_purecap_cc1 %s -emit-llvm -Wall -o - -verify=expected,purecap | FileCheck --check-prefix=CHECK-PURECAP %s
// RUN: %cheri_cc1 %s -emit-llvm -o - -Wall -verify=expected,hybrid
// RUN: %cheri_cc1 %s -emit-llvm -o - -Wall -verify=expected,hybrid | FileCheck --check-prefix=CHECK-HYBRID %s

void* __capability foo(void *__capability x){
  // CHECK-HYBRID:  [[ADDR:%.+]] = call i64 @llvm.cheri.cap.to.pointer.i64(i8 addrspace(200)* %1, i8 addrspace(200)* %0)
  // CHECK-PURECAP: [[ADDR:%.+]] = call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* {{%.+}})
  // CHECK-PURECAP-NEXT: trunc i64 [[ADDR]] to i32
  int pi = (int)x; // pi contains the result of CToPtr x, which is probably null
  // hybrid-warning@-1{{the following conversion will result in a CToPtr operation}}
  // hybrid-note@-2{{if you really intended to use CToPtr use}}
  // CHECK-HYBRID: inttoptr
  // CHECK-PURECAP: [[CONV:%.+]] = sext i32 {{%.+}} to i64
  // CHECK-PURECAP-NEXT: call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* null, i64 [[CONV]])
  return (void* __capability)pi;
  // expected-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  // expected-warning@-2{{cast to 'void * __capability' from smaller integer type 'int'}}
  // expected-note@-3{{insert cast to intptr_t to silence this warning}}
}
