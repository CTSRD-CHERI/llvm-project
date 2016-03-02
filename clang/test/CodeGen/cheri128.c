// RUN: %clang -target cheri-unknown-freebsd -S -o - -O1 -emit-llvm %s | FileCheck -check-prefix=CHECK256  %s
// RUN: %clang -target cheri-unknown-freebsd -S -o - -O1 -emit-llvm -mllvm -cheri128-test-mode -mllvm -cheri128 %s | FileCheck -check-prefix=CHECK128 %s 

// CHECK128: target datalayout = "E-m:m-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128"
// CHECK256: target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
__capability void *array[2];

__capability void *get(int x)
{
	// CHECK128: align 16
	// CHECK256: align 32
	return array[x];
}

int size(void)
{
	// CHECK128: ret i32 16
	// CHECK256: ret i32 32
	return sizeof(__capability void*);
}
