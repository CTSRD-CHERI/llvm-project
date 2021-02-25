// REQUIRES: mips-registered-target

// RUN: %cheri128_cc1 -o - -O0 -emit-llvm %s | FileCheck %s --check-prefixes=CHECK
// RUN: %cheri128_cc1 -o - -O0 -S %s | FileCheck %s -check-prefixes=ASM
void * __capability results[12];

long long testDeprecated(void * __capability foo)
{
	// CHECK-LABEL: @testDeprecated(
	long long x;
	// CHECK: call void @llvm.mips.cap.cause.set(i64 42)
	// CHECK: call i64 @llvm.mips.cap.cause.get()
	__builtin_mips_cheri_set_cause(42);
	return x & __builtin_mips_cheri_get_cause();
}
long long test(void* __capability foo)
{
  // CHECK-LABEL: @test(
  // ASM-LABEL: test:
  long long x;
  x &= __builtin_cheri_length_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.length.get.i64
  // ASM: cgetlen ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_perms_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.perms.get.i64
  // ASM: cgetperm ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_type_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.type.get.i64
  // ASM: cgettype ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_tag_get(foo);
  // CHECK: call i1 @llvm.cheri.cap.tag.get
  // ASM: cgettag ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_offset_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.offset.get.i64
  // ASM: cgetoffset ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_base_get(foo);
  // CHECK: call i64 @llvm.cheri.cap.base.get.i64
  // ASM: cgetbase ${{[0-9]+}}, $c{{[0-9]+}}
  x &= __builtin_cheri_sealed_get(foo);
  // CHECK: call i1 @llvm.cheri.cap.sealed.get
  // ASM: cgetsealed ${{[0-9]+}}, $c{{[0-9]+}}
  void * bar = __builtin_cheri_cap_to_pointer(foo, foo);
  // CHECK: call i64 @llvm.cheri.cap.to.pointer.i64
  // ASM: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  results[0] = __builtin_cheri_cap_from_pointer(foo, bar);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.from.pointer.i64
  // ASM: cfromptr $c{{[0-9]+}}, $c{{[0-9]+}}, ${{[0-9]+}}
  results[1] = __builtin_cheri_perms_and(foo, 12);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.perms.and.i64
  // ASM: candperm $c{{[0-9]+}}, $c{{[0-9]+}}, ${{[0-9]+}}
  results[4] = __builtin_cheri_seal(foo, foo);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.seal
  // ASM: cseal $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  results[5] = __builtin_cheri_unseal(foo, foo);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.unseal
  // ASM: cunseal $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  results[6] = __builtin_cheri_bounds_set(foo, 42);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* {{.+}}, i64 42)
  // ASM: csetbounds $c{{[0-9]+}}, $c{{[0-9]+}}, 42
  results[6] = __builtin_cheri_bounds_set(foo, 16384); // too big for immediate csetbounds
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* {{.+}}, i64 16384)
  // ASM: daddiu [[INEXACT_SIZE:\$[0-9]+]], $zero, 16384
  // ASM: csetbounds $c{{[0-9]+}}, $c{{[0-9]+}}, [[INEXACT_SIZE]]
  results[7] = __builtin_cheri_bounds_set_exact(foo, 43);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.exact.i64(i8 addrspace(200)* {{.+}}, i64 43)
  // ASM: daddiu [[EXACT_SIZE:\$[0-9]+]], $zero, 43
  // ASM: csetboundsexact $c{{[0-9]+}}, $c{{[0-9]+}}, [[EXACT_SIZE]]
  results[8] = __builtin_cheri_seal_entry(foo);
  // CHECK: call i8 addrspace(200)* @llvm.cheri.cap.seal.entry(i8 addrspace(200)* {{.+}})
  // ASM: csealentry $c{{[0-9]+}}, $c{{[0-9]+}}

  __builtin_mips_cheri_cause_set(42);
  // CHECK: call void @llvm.mips.cap.cause.set(i64 42)
  // ASM: csetcause ${{[0-9]+}}
  __builtin_cheri_perms_check(foo, 12);
  // CHECK: call void @llvm.cheri.cap.perms.check.i64
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
  void * __capability tagged = __builtin_cheri_cap_build(auth, bits);
  // ASM: cbuildcap $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  void * __capability unseal_auth = __builtin_cheri_cap_type_copy(auth, bits);
  // ASM: ccopytype $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
  void * __capability condseal = __builtin_cheri_conditional_seal(tagged, unseal_auth);
  // ASM: ccseal $c{{[0-9]+}}, $c{{[0-9]+}}, $c{{[0-9]+}}
}


int crap_cram(int len) {
  // CHECK-LABEL: @crap_cram(
  // ASM-LABEL: crap_cram:
  return __builtin_cheri_round_representable_length(len) & __builtin_cheri_representable_alignment_mask(len);
  // CHECK: call i64 @llvm.cheri.round.representable.length.i64(
  // CHECK: call i64 @llvm.cheri.representable.alignment.mask.i64(
  // ASM: croundrepresentablelength	${{[0-9]+}}, ${{[0-9]+}}
  // ASM: crepresentablealignmentmask	${{[0-9]+}}, ${{[0-9]+}}
}
