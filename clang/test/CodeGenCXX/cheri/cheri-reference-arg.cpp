// RUN: %cheri_cc1 -emit-llvm -o - %s | FileCheck %s
extern void bar(int &a);

// CHECK-LABEL: define dso_local void @_Z3fooU12__capabilityPi(
void foo(int *__capability p) {
  // CHECK:  [[PTR:%.*]] = addrspacecast i32 addrspace(200)* %0 to i32*
  // CHECK:  call void @_Z3barRi(i32* nonnull align 4 dereferenceable(4) [[PTR]])
  bar(*(__cheri_fromcap int *)p);
}
