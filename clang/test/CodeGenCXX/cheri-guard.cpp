// RUN:  %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -emit-llvm -std=c++11 -o - %s | FileCheck %s
struct foo {
  foo();
};

// CHECK: @_ZGVZ1fvE1x = internal addrspace(200) global i64 0, align 8
foo *f() {
  // CHECK: %0 = load atomic i8, i8 addrspace(200)* bitcast (i64 addrspace(200)* @_ZGVZ1fvE1x to i8 addrspace(200)*) acquire, align 8
  // CHECK: call i32 @__cxa_guard_acquire(i64 addrspace(200)* @_ZGVZ1fvE1x)
  // CHECK: call void @_ZN3fooC1Ev(%struct.foo addrspace(200)* @_ZZ1fvE1x)
  // CHECK: call void @__cxa_guard_release(i64 addrspace(200)* @_ZGVZ1fvE1x)
  // CHECK: ret %struct.foo addrspace(200)* @_ZZ1fvE1x
  static foo x;
  return &x;
}

// CHECK: declare i32 @__cxa_guard_acquire(i64 addrspace(200)*) #1
// CHECK: declare void @__cxa_guard_release(i64 addrspace(200)*) #1

