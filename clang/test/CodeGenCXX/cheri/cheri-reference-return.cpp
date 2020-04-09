// RUN: %cheri_cc1 -emit-llvm -o - %s | FileCheck %s

// CHECK-LABEL:  @_Z2t1U12__capabilityPi(
int &t1(int *__capability tab) {
  // CHECK:  [[IDX0:%.*]] = getelementptr inbounds i32, i32 addrspace(200)* {{%.*}}, i64 2
  // CHECK-NEXT: [[PTR1:%.*]] = addrspacecast i32 addrspace(200)* [[IDX0]] to i32*
  // CHECK-NEXT:  ret i32* [[PTR1]]
  return *(__cheri_fromcap int *)&tab[2];
}

// CHECK-LABEL:  @_Z2t2U12__capabilityPi(
int &t2(int *__capability t) {
  // CHECK: [[PTR1:%.*]] = addrspacecast i32 addrspace(200)* {{.*}} to i32*
  // CHECK-NEXT: ret i32* [[PTR1]]
  return *(__cheri_fromcap int *)t;
}

// CHECK-LABEL:  @_Z2t3U12__capabilityPi(
const int &t3(int *__capability t) {
  // CHECK: [[PTR1:%.*]] = addrspacecast i32 addrspace(200)* {{.*}} to i32*
  // CHECK-NEXT:  ret i32* [[PTR1]]
  return *(__cheri_fromcap int *)t;
}

// CHECK-LABEL:  @_Z2t4U12__capabilityPi(
const int &t4(int *__capability t) {
  // CHECK: [[PTR1:%.*]] = addrspacecast i32 addrspace(200)* {{.*}} to i32*
  // CHECK-NEXT:  ret i32* [[PTR1]]
  return *(__cheri_fromcap int *)(t);
}
