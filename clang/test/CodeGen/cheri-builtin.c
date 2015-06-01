// RUN: %clang %s -target cheri-unknown-freebsd  -S -o - -O0 -emit-llvm | FileCheck %s
__capability void *results[12];

long long testDeprecated(__capability void* foo)
{
	long long x;
	// CHECK: call i64 @llvm.mips.cap.length.get
	// CHECK: call i64 @llvm.mips.cap.perms.get
	// CHECK: call i64 @llvm.mips.cap.type.get
	// CHECK: call i64 @llvm.mips.cap.tag.get
	// CHECK: call i64 @llvm.mips.cap.sealed.get
	// CHECK: call i8 addrspace(200)* @llvm.mips.cap.length.set
	// CHECK: call i8 addrspace(200)* @llvm.mips.cap.perms.and
	// CHECK: call i8 addrspace(200)* @llvm.mips.cap.seal
	// CHECK: call i8 addrspace(200)* @llvm.mips.cap.unseal
	// CHECK: call void @llvm.mips.cap.cause.set(i64 42)
	// CHECK: call void @llvm.mips.cap.perms.check
	// CHECK: call void @llvm.mips.cap.type.check
	// CHECK: call i64 @llvm.mips.cap.cause.get()
	x &= __builtin_cheri_get_cap_length(foo);
	x &= __builtin_cheri_get_cap_perms(foo);
	x &= __builtin_cheri_get_cap_type(foo);
	x &= __builtin_cheri_get_cap_tag(foo);
	x &= __builtin_cheri_get_cap_sealed(foo);
	results[0] = __builtin_cheri_set_cap_length(foo, 42);
	results[1] = __builtin_cheri_and_cap_perms(foo, 12);
	results[4] = __builtin_cheri_seal_cap(foo, foo);
	results[5] = __builtin_cheri_unseal_cap(foo, foo);
	__builtin_cheri_set_cause(42);
	__builtin_cheri_check_perms(foo, 12);
	__builtin_cheri_check_type(foo, results[0]);
	return x & __builtin_cheri_get_cause();
}
long long test(__capability void* foo)
{
	long long x;
	// CHECK: call i64 @llvm.mips.cap.length.get
	// CHECK: call i64 @llvm.mips.cap.perms.get
	// CHECK: call i64 @llvm.mips.cap.type.get
	// CHECK: call i64 @llvm.mips.cap.tag.get
	// CHECK: call i64 @llvm.mips.cap.sealed.get
	// CHECK: call i8 addrspace(200)* @llvm.mips.cap.length.set
	// CHECK: call i8 addrspace(200)* @llvm.mips.cap.perms.and
	// CHECK: call i8 addrspace(200)* @llvm.mips.cap.seal
	// CHECK: call i8 addrspace(200)* @llvm.mips.cap.unseal
	// CHECK: call void @llvm.mips.cap.cause.set(i64 42)
	// CHECK: call void @llvm.mips.cap.perms.check
	// CHECK: call void @llvm.mips.cap.type.check
	// CHECK: call i64 @llvm.mips.cap.cause.get()
	x &= __builtin_memcap_length_get(foo);
	x &= __builtin_memcap_perms_get(foo);
	x &= __builtin_memcap_type_get(foo);
	x &= __builtin_memcap_tag_get(foo);
	x &= __builtin_memcap_sealed_get(foo);
	results[0] = __builtin_memcap_length_set(foo, 42);
	results[1] = __builtin_memcap_perms_and(foo, 12);
	results[4] = __builtin_memcap_seal(foo, foo);
	results[5] = __builtin_memcap_unseal(foo, foo);
	__builtin_cheri_cause_set(42);
	__builtin_memcap_perms_check(foo, 12);
	__builtin_memcap_type_check(foo, results[0]);
	return x & __builtin_cheri_get_cause();
}
