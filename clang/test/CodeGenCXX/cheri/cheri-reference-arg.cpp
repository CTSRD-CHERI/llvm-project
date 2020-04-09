// RUN: %cheri_cc1 -emit-llvm -o - %s | FileCheck %s
extern void bar(int &a);

// CHECK-LABEL: define void @_Z3fooU12__capabilityPi(
void foo(int *__capability p) {
  // CHECK:  [[PTR:%.*]] = addrspacecast i32 addrspace(200)* %0 to i32*
  // CHECK:  call void @_Z3barRi(i32* dereferenceable({{[0-9]+}}) [[PTR]])
  bar(*(__cheri_fromcap int *)p);
}
