// RUN: %cheri_cc1 -o - -O0 -emit-llvm %s | FileCheck %s
// FIXME: we shouldn't really be testing ASM output in clang
// RUN: %cheri_cc1 -o - -O0 -S %s
// RUN: %cheri_cc1 -o - -O0 -S %s | FileCheck %s -check-prefix=ASM
void * __capability results[12];

long long testDeprecated(void * __capability foo)
{
	long long x;
	// CHECK: call i64 @llvm.cheri.cap.length.get
	// CHECK: call i64 @llvm.cheri.cap.perms.get
	// CHECK: call i64 @llvm.cheri.cap.type.get
	// CHECK: call i64 @llvm.cheri.cap.tag.get
	// CHECK: call i64 @llvm.cheri.cap.sealed.get
	// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.perms.and
	// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.seal
	// CHECK: call i8 addrspace(200)* @llvm.cheri.cap.unseal
	// CHECK: call void @llvm.mips.cap.cause.set(i64 42)
	// CHECK: call void @llvm.cheri.cap.perms.check
	// CHECK: call void @llvm.cheri.cap.type.check
	// CHECK: call i64 @llvm.mips.cap.cause.get()
	x &= __builtin_mips_cheri_get_cap_length(foo);
	x &= __builtin_mips_cheri_get_cap_perms(foo);
	x &= __builtin_mips_cheri_get_cap_type(foo);
	x &= __builtin_mips_cheri_get_cap_tag(foo);
	x &= __builtin_mips_cheri_get_cap_sealed(foo);
	results[1] = __builtin_mips_cheri_and_cap_perms(foo, 12);
	results[4] = __builtin_mips_cheri_seal_cap(foo, foo);
	results[5] = __builtin_mips_cheri_unseal_cap(foo, foo);
	__builtin_mips_cheri_set_cause(42);
	__builtin_mips_cheri_check_perms(foo, 12);
	__builtin_mips_cheri_check_type(foo, results[0]);
	return x & __builtin_mips_cheri_get_cause();
}
long long test(void* __capability foo)
{
  long long x;
  x &= __builtin_cheri_length_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.length.get
  // ASM: cgetlen ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_perms_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.perms.get
  // ASM: cgetperm ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_type_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.type.get
  // ASM: cgettype ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_tag_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.tag.get
  // ASM: cgettag ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_offset_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.offset.get
  // ASM: cgetoffset ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_base_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.base.get
  // ASM: cgetbase ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_sealed_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.sealed.get
  // ASM: cgetsealed ${{[0-9]+}}, $c{{[0-9]+}}
  void * bar = __builtin_cheri_cap_to_pointer(foo, foo);
  // CHECK: call i64 @llvm.cheri.cap.to.pointer
  // ASM: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  results[0] = __builtin_cheri_cap_from_pointer(foo, bar);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.from.pointer
  // ASM: cfromptr $c{{[0-9]+}}, $c{{[0-9]+}}, ${{[0-9]+}}
  results[1] = __builtin_cheri_perms_and(foo, 12);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.perms.and
  // ASM: candperm $c{{[0-9]+}}, $c{{[0-9]+}}, ${{[0-9]+}}
  results[4] = __builtin_cheri_seal(foo, foo);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.seal
  // ASM: cseal $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  results[5] = __builtin_cheri_unseal(foo, foo);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.unseal
  // ASM: cunseal $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  results[6] = __builtin_cheri_bounds_set(foo, 42);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set
  // ASM: csetbounds $c{{[0-9]+}}, $c{{[0-9]+}}, 42
  __builtin_mips_cheri_cause_set(42);
  // CHECK: call void @llvm.mips.cap.cause.set(i64 42)
  // ASM: csetcause ${{[0-9]+}}
  __builtin_cheri_perms_check(foo, 12);
  // CHECK: call void @llvm.cheri.cap.perms.check
  // ASM: ccheckperm $c{{[0-9]+}}, ${{[0-9]+}}
  __builtin_cheri_type_check(foo, results[0]);
  // CHECK: call void @llvm.cheri.cap.type.check
  // ASM: cchecktype $c{{[0-9]+}}, $c{{[0-9]+}}
  return x & __builtin_mips_cheri_get_cause();
  // CHECK: call i64 @llvm.mips.cap.cause.get()
  // ASM: cgetcause ${{[0-9]+}}
}

void buildcap(void * __capability auth, __intcap_t bits) {
  // ASM-LABEL: buildcap:
  void * __capability tagged = __builtin_cheri_cap_build(auth, (void * __capability)bits);
  // ASM: cbuildcap $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  void * __capability sealed = __builtin_cheri_cap_type_copy(auth, tagged);
  // ASM: ccopytype $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  void * __capability condseal = __builtin_cheri_conditional_seal(auth, tagged);
  // ASM: ccseal $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
}
