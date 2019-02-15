// RUN:  %cheri_purecap_cc1 -emit-llvm -std=c++11 -o - %s | FileCheck %s
struct foo {
  foo();
};

// CHECK: @_ZGVZ1fvE1x = internal addrspace(200) global i64 0, align 8
foo *f() {
  // CHECK-LABEL: define %struct.foo addrspace(200)* @_Z1fv()
  // CHECK: [[LOADED_GUARD:%.+]] = load atomic i8, i8 addrspace(200)* bitcast (i64 addrspace(200)* @_ZGVZ1fvE1x to i8 addrspace(200)*) acquire, align 8
  // CHECK-NEXT: [[UNINITIALIZED:%.+]] = icmp eq i8 [[LOADED_GUARD]], 0
  // CHECK-NEXT: br i1 [[UNINITIALIZED]], label %[[ACQUIRE_LABEL:.+]], label %[[EXIT_LABEL:[^, !]+]]

  // CHECK: [[ACQUIRE_LABEL]]:
  // CHECK-NEXT: [[ACQUIRED:%.+]] = call i32 @__cxa_guard_acquire(i64 addrspace(200)* @_ZGVZ1fvE1x)
  // CHECK-NEXT: [[TOBOOL:%.+]] = icmp ne i32 [[ACQUIRED]], 0
  // CHECK-NEXT: br i1 [[TOBOOL]], label %[[INITIALIZE_LABEL:.+]], label %[[EXIT_LABEL]]

  // CHECK: [[INITIALIZE_LABEL]]:
  // CHECK-NEXT: call void @_ZN3fooC1Ev(%struct.foo addrspace(200)* @_ZZ1fvE1x)
  // CHECK-NEXT: call void @__cxa_guard_release(i64 addrspace(200)* @_ZGVZ1fvE1x)
  // CHECK-NEXT: br label %[[EXIT_LABEL]]

  // CHECK [[EXIT_LABEL]]:
  // CHECK: ret %struct.foo addrspace(200)* @_ZZ1fvE1x
  static foo x;
  return &x;
}

// CHECK: declare i32 @__cxa_guard_acquire(i64 addrspace(200)*)
// CHECK: declare void @__cxa_guard_release(i64 addrspace(200)*)

