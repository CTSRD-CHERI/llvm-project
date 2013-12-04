// RUN: %clang %s -target cheri-unknown-freebsd  -S -o - -O0 -emit-llvm | FileCheck %s
__capability void *results[12];

long long test(__capability void* foo)
{
	long long x;
	// CHECK: call i64 @llvm.cheri.get.cap.length
	// CHECK: call i64 @llvm.cheri.get.cap.perms
	// CHECK: call i64 @llvm.cheri.get.cap.type
	// CHECK: call i64 @llvm.cheri.get.cap.tag
	// CHECK: call i64 @llvm.cheri.get.cap.unsealed
	// CHECK: call i8 addrspace(200)* @llvm.cheri.set.cap.length
	// CHECK: call i8 addrspace(200)* @llvm.cheri.and.cap.perms
	// CHECK: call i8 addrspace(200)* @llvm.cheri.set.cap.type
	// CHECK: call i8 addrspace(200)* @llvm.cheri.seal.cap.code
	// CHECK: call i8 addrspace(200)* @llvm.cheri.seal.cap.data
	// CHECK: call i8 addrspace(200)* @llvm.cheri.unseal.cap
	// CHECK: call void @llvm.cheri.set.cause(i64 42)
	// CHECK: call i64 @llvm.cheri.get.cause()
	x &= __builtin_cheri_get_cap_length(foo);
	x &= __builtin_cheri_get_cap_perms(foo);
	x &= __builtin_cheri_get_cap_type(foo);
	x &= __builtin_cheri_get_cap_tag(foo);
	x &= __builtin_cheri_get_cap_unsealed(foo);
	results[0] = __builtin_cheri_set_cap_length(foo, 42);
	results[1] = __builtin_cheri_and_cap_perms(foo, 12);
	results[2] = __builtin_cheri_set_cap_type(foo, 0x23);
	results[3] = __builtin_cheri_seal_cap_code(foo);
	results[4] = __builtin_cheri_seal_cap_data(foo, foo);
	results[5] = __builtin_cheri_unseal_cap(foo, foo);
	__builtin_cheri_set_cause(42);
	return x & __builtin_cheri_get_cause();
}
