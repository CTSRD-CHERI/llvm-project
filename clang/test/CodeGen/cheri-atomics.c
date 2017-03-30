// RUN: %clang_cc1 %s -target-abi purecap -emit-llvm -triple cheri-unknown-freebsd -o - | FileCheck %s
int main(void) {
  _Atomic(int*) p;
	// CHECK:  %p = alloca i32 addrspace(200)*, align 32
	// CHECK:  %.atomictmp = alloca i64, align 8
	// CHECK:  %atomic-temp = alloca i32 addrspace(200)*, align 32
	// CHECK:  %atomic-temp1 = alloca i32 addrspace(200)*, align 32
	// CHECK:  %c = alloca i32 addrspace(200)*, align 32
	// CHECK:  %atomic-temp2 = alloca i32 addrspace(200)*, align 32
	// CHECK:  store i64 4, i64 addrspace(200)* %.atomictmp, align 8
	// CHECK:  %0 = bitcast i32 addrspace(200)* addrspace(200)* %atomic-temp to i8 addrspace(200)*
	// CHECK:  %1 = bitcast i64 addrspace(200)* %.atomictmp to i8 addrspace(200)*
	// CHECK:  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* %0, i8 addrspace(200)* %1, i64 8, i32 8, i1 false)
	// CHECK:  %2 = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
	// CHECK:  %3 = bitcast i32 addrspace(200)* addrspace(200)* %atomic-temp to i8 addrspace(200)*
	// CHECK:  %4 = bitcast i32 addrspace(200)* addrspace(200)* %atomic-temp1 to i8 addrspace(200)*
	// CHECK:  call void @__atomic_fetch_add(i64 zeroext 32, i8 addrspace(200)* %2, i8 addrspace(200)* %3, i8 addrspace(200)* %4, i32 signext 5)
  __c11_atomic_fetch_add(&p, 1, __ATOMIC_SEQ_CST);

	// CHECK:  %5 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %atomic-temp1, align 32
	// CHECK:  %6 = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
	// CHECK:  %7 = bitcast i32 addrspace(200)* addrspace(200)* %atomic-temp2 to i8 addrspace(200)*
	// CHECK:  call void @__atomic_load(i64 zeroext 32, i8 addrspace(200)* %6, i8 addrspace(200)* %7, i32 signext 5)
	// CHECK:  %8 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %atomic-temp2, align 32
	// CHECK:  store i32 addrspace(200)* %8, i32 addrspace(200)* addrspace(200)* %c, align 32
  int *c = __c11_atomic_load(&p, __ATOMIC_SEQ_CST);

  // CHECK: store i32 addrspace(200)* null, i32 addrspace(200)* addrspace(200)* %.atomictmp3, align 32
  // CHECK: %9 = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
  // CHECK: %10 = bitcast i32 addrspace(200)* addrspace(200)* %.atomictmp3 to i8 addrspace(200)*
  // CHECK: call void @__atomic_store(i64 zeroext 32, i8 addrspace(200)* %9, i8 addrspace(200)* %10, i32 signext 5)
	__c11_atomic_store(&p, 0, __ATOMIC_SEQ_CST);

	// CHECK:  %11 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %c, align 32
	// CHECK:  store i32 addrspace(200)* %11, i32 addrspace(200)* addrspace(200)* %.atomictmp4, align 32
	// CHECK:  %12 = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
	// CHECK:  %13 = bitcast i32 addrspace(200)* addrspace(200)* %.atomictmp4 to i8 addrspace(200)*
	// CHECK:  %call = call zeroext i1 @__atomic_compare_exchange(i64 zeroext 32, i8 addrspace(200)* %12, i8 addrspace(200)* null, i8 addrspace(200)* %13, i32 signext 5, i32 signext 5)
	__c11_atomic_compare_exchange_strong(&p, 0, c, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return 0;
}
