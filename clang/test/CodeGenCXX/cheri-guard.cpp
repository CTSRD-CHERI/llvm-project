// RUN:  %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -emit-llvm -std=c++11 -o - %s | FileCheck %s
struct foo {
  foo();
};

// CHECK: @_ZGVZ1fvE1x = internal addrspace(200) global i64 0, align 8
foo *f() {
	// CHECK: call i32 @__cxa_guard_acquire(i64 addrspace(200)* @_ZGVZ1fvE1x)
  static foo x;
	// CHECK: call void @__cxa_guard_release(i64 addrspace(200)* @_ZGVZ1fvE1x)
  return &x;
}

// CHECK: declare i32 @__cxa_guard_acquire(i64 addrspace(200)*) #1
