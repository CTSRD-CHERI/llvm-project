// RUN: %clang %s -target cheri-unknown-freebsd  -S -o - -O0 -emit-llvm | FileCheck %s
__capability void *results[12];

long long test(__capability void* foo)
{
	long long x;
	// CHECK: call i64 @llvm.mips.get.cap.length
	// CHECK: call i64 @llvm.mips.get.cap.perms
	// CHECK: call i64 @llvm.mips.get.cap.type
	// CHECK: call i64 @llvm.mips.get.cap.tag
	// CHECK: call i64 @llvm.mips.get.cap.unsealed
	// CHECK: call i8 addrspace(200)* @llvm.mips.set.cap.length
	// CHECK: call i8 addrspace(200)* @llvm.mips.and.cap.perms
	// CHECK: call i8 addrspace(200)* @llvm.mips.set.cap.type
	// CHECK: call i8 addrspace(200)* @llvm.mips.seal.cap
	// CHECK: call i8 addrspace(200)* @llvm.mips.unseal.cap
	// CHECK: call void @llvm.mips.set.cause(i64 42)
	// CHECK: call void @llvm.mips.check.perms
	// CHECK: call void @llvm.mips.check.type
	// CHECK: call i64 @llvm.mips.get.cause()
	x &= __builtin_cheri_get_cap_length(foo);
	x &= __builtin_cheri_get_cap_perms(foo);
	x &= __builtin_cheri_get_cap_type(foo);
	x &= __builtin_cheri_get_cap_tag(foo);
	x &= __builtin_cheri_get_cap_unsealed(foo);
	results[0] = __builtin_cheri_set_cap_length(foo, 42);
	results[1] = __builtin_cheri_and_cap_perms(foo, 12);
	results[2] = __builtin_cheri_set_cap_type(foo, 0x23);
	results[4] = __builtin_cheri_seal_cap(foo, foo);
	results[5] = __builtin_cheri_unseal_cap(foo, foo);
	__builtin_cheri_set_cause(42);
	__builtin_cheri_check_perms(foo, 12);
	__builtin_cheri_check_type(foo, results[0]);
	return x & __builtin_cheri_get_cause();
}
