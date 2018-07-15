// RUN: %cheri_cc1 %s -emit-llvm -o - -verify=hybrid | %cheri_FileCheck %s -check-prefixes CHECK,N64
// RUN: %cheri_cc1 -w -DCODEGEN %s -S -o - -O2 | FileCheck %s -check-prefixes ASM,N64-ASM
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt %s -emit-llvm -o - -verify | %cheri_FileCheck %s -check-prefixes CHECK,PURECAP
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -DCODEGEN -w %s -S -o - -O2 | FileCheck %s -check-prefixes ASM,PURECAP-ASM
// The new warning "misaligned or large atomic operation" was causing -Werror failures for CheriBSD
// It is not a false positive since we need libcalls for CHERI cap (cmp)xchg but since this is
// a known issue (https://github.com/CTSRD-CHERI/clang/issues/201) we should not warn.
// This test also checks that we correctly use clc/csc+sync for atomic load/store and
// only need the libcalls for fetchadd sequences.

// expected-no-diagnostics

void func1(void **p) {
  // CHECK-LABEL: @func1(
  void* res;
  void* res2;
  __atomic_load(p, &res, 5);
  __atomic_store(p, &res, 5);
  __atomic_exchange(p, &res, &res2, 5);
  __atomic_compare_exchange(p, &res, &res2, 0, 5, 5);

  // N64: load atomic i64, i64* {{%.+}} seq_cst, align 8
  // N64: store atomic i64 {{%.+}}, i64* {{%.+}} seq_cst, align 8
  // N64: atomicrmw xchg i64* %{{.+}}, i64 %{{.+}} seq_cst
  // N64: cmpxchg i64* %{{.+}}, i64 %{{.+}}, i64 %{{.+}} seq_cst seq_cst

  // PURECAP: load atomic i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: store atomic i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // no more libcalls for the xchg/cmpxchg:
  // PURECAP: atomicrmw xchg i8 addrspace(200)* addrspace(200)* %{{.+}}, i8 addrspace(200)* %{{.+}} seq_cst
  // PURECAP: cmpxchg i8 addrspace(200)* addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}} seq_cst seq_cst



  // ASM-LABEL: func1:
  // N64-ASM: ld	[[LD_ST_REG:\$[0-9]+]], 0($4)
  // N64-ASM-NEXT: sync
  // N64-ASM-NEXT: sync
  // N64-ASM-NEXT: sd	[[LD_ST_REG]], 0($4)
  // N64-ASM-NEXT: sync
  // N64-ASM: .LBB0_1:
  // N64-ASM: lld	$3, 0($4)
  // N64-ASM: scd	$2, 0($4)
  // N64-ASM: beqz	$2, .LBB0_1
  // N64-ASM: .LBB0_3:
  // N64-ASM: lld	$2, 0($4)
  // N64-ASM: scd	$5, 0($4)
  // N64-ASM: beqz	$5, .LBB0_3

  // Check that we are emitting sensible assembly for the atomic load/store:
  // PURECAP-ASM: clc	$c1, $zero, 0($c3)
  // PURECAP-ASM: sync
  // PURECAP-ASM: sync
  // PURECAP-ASM: csc	$c1, $zero, 0($c3)
  // PURECAP-ASM-NEXT: sync
  // xchg:
  // PURECAP-ASM:      cllc	$c{{[0-9]+}}, $c3
  // PURECAP-ASM-NEXT: cscc	$1, $c{{[0-9]+}}, $c3
  // PURECAP-ASM-NEXT: beqz	$1, .LBB0_1
  // cmpxchg:
  // PURECAP-ASM:      cllc [[LOADED_VAL:\$c[0-9]+]], $c3
  // PURECAP-ASM-NEXT: ceq $1, [[LOADED_VAL]], $c{{[0-9]+}}
  // PURECAP-ASM:      cscc $1, $c{{[0-9]+}}, $c3
  // PURECAP-ASM-NEXT: beqz	$1, .LBB0_3
  // ASM: .end	func1
}

void func2(int *__capability *p) {
  // CHECK-LABEL: @func2(
  // PURECAP: load atomic i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: store atomic i32 addrspace(200)* {{%.+}}, i32 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: atomicrmw xchg i32 addrspace(200)* addrspace(200)* %{{.+}}, i32 addrspace(200)* %{{.+}} seq_cst
  // PURECAP: cmpxchg i32 addrspace(200)* addrspace(200)* {{%.+}}, i32 addrspace(200)* {{%.+}}, i32 addrspace(200)* {{%.+}} seq_cst seq_cst

  // hybrid code uses a libcall here and casts the i8 addrspace(200)** to i8* for the libcall
  // N64: [[PTR1:%.+]] = bitcast i32 addrspace(200)** %{{.+}} to i8*
  // N64: call void @__atomic_load(i64 zeroext [[$CAP_SIZE]], i8* [[PTR1]], i8* %{{.+}}, i32 signext 5)
  // N64: call void @__atomic_store(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i32 signext 5)
  // N64: call void @__atomic_exchange(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i8* %{{.+}}, i32 signext 5)
  // N64: call zeroext i1 @__atomic_compare_exchange(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i8* %{{.+}}, i32 signext 5, i32 signext 5)

  int *__capability res;
  int *__capability res2;
  __atomic_load(p, &res, 5);                          // hybrid-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  __atomic_store(p, &res, 5);                         // hybrid-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  __atomic_exchange(p, &res, &res2, 5);               // hybrid-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  __atomic_compare_exchange(p, &res, &res2, 0, 5, 5); // hybrid-warning {{misaligned or large atomic operation may incur significant performance penalty}}

  // ASM-LABEL: func2:
  // Use libcalls for all these:
  // N64-ASM: ld	$25, %call16(__atomic_load)($gp)
  // N64-ASM: ld	$25, %call16(__atomic_store)($gp)
  // N64-ASM: ld	$25, %call16(__atomic_exchange)($gp)
  // N64-ASM: ld	$25, %call16(__atomic_compare_exchange)($gp)

  // purecap doesn't need libcalls for load/store:
  // PURECAP-ASM: clc	$c1, $zero, 0($c3)
  // PURECAP-ASM: sync
  // PURECAP-ASM: sync
  // PURECAP-ASM: csc	$c1, $zero, 0($c3)
  // PURECAP-ASM-NEXT: sync
  // xchg:
  // PURECAP-ASM:      cllc	$c{{[0-9]+}}, $c3
  // PURECAP-ASM-NEXT: cscc	$1, $c{{[0-9]+}}, $c3
  // PURECAP-ASM-NEXT: beqz	$1, .LBB1_1
  // cmpxchg:
  // PURECAP-ASM:      cllc [[LOADED_VAL:\$c[0-9]+]], $c3
  // PURECAP-ASM-NEXT: ceq $1, [[LOADED_VAL]], $c{{[0-9]+}}
  // PURECAP-ASM:      cscc $1, $c{{[0-9]+}}, $c3
  // PURECAP-ASM-NEXT: beqz	$1, .LBB1_3
  // ASM: .end	func2
}

void func3(__uintcap_t *p) {
  // CHECK-LABEL: @func3(
  // PURECAP: load atomic i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: store atomic i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: atomicrmw xchg i8 addrspace(200)* addrspace(200)* %{{.+}}, i8 addrspace(200)* %{{.+}} seq_cst
  // PURECAP: cmpxchg i8 addrspace(200)* addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}} seq_cst seq_cst

  // hybrid code uses a libcall here and casts the i8 addrspace(200)** to i8* for the libcall
  // N64: [[PTR1:%.+]] = bitcast i8 addrspace(200)** %{{.+}} to i8*
  // N64: call void @__atomic_load(i64 zeroext [[$CAP_SIZE]], i8* [[PTR1]], i8* %{{.+}}, i32 signext 5)
  // N64: call void @__atomic_store(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i32 signext 5)
  // N64: call void @__atomic_exchange(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i8* %{{.+}}, i32 signext 5)
  // N64: call zeroext i1 @__atomic_compare_exchange(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i8* %{{.+}}, i32 signext 5, i32 signext 5)


  __uintcap_t res;
  __uintcap_t res2;
  __atomic_load(p, &res, 5);                          // hybrid-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  __atomic_store(p, &res, 5);                         // hybrid-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  __atomic_exchange(p, &res, &res2, 5);               // hybrid-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  __atomic_compare_exchange(p, &res, &res2, 0, 5, 5); // hybrid-warning {{misaligned or large atomic operation may incur significant performance penalty}}

  // ASM-LABEL: func3:
  // Use libcalls for all these:
  // N64-ASM: ld	$25, %call16(__atomic_load)($gp)
  // N64-ASM: ld	$25, %call16(__atomic_store)($gp)
  // N64-ASM: ld	$25, %call16(__atomic_exchange)($gp)
  // N64-ASM: ld	$25, %call16(__atomic_compare_exchange)($gp)
  // purecap doesn't need libcalls for load/store:
  // PURECAP-ASM: clc	$c1, $zero, 0($c3)
  // PURECAP-ASM: sync
  // PURECAP-ASM: sync
  // PURECAP-ASM: csc	$c1, $zero, 0($c3)
  // PURECAP-ASM-NEXT: sync
  // xchg:
  // PURECAP-ASM:      cllc	$c{{[0-9]+}}, $c3
  // PURECAP-ASM-NEXT: cscc	$1, $c{{[0-9]+}}, $c3
  // PURECAP-ASM-NEXT: beqz	$1, .LBB2_1
  // cmpxchg:
  // PURECAP-ASM:      cllc [[LOADED_VAL:\$c[0-9]+]], $c3
  // PURECAP-ASM-NEXT: ceq $1, [[LOADED_VAL]], $c{{[0-9]+}}
  // PURECAP-ASM:      cscc $1, $c{{[0-9]+}}, $c3
  // PURECAP-ASM-NEXT: beqz	$1, .LBB2_3
  // ASM: .end	func3
}

void func4(long *p) {
  // CHECK-LABEL: @func4(
  // PURECAP: load atomic i64, i64 addrspace(200)* {{%.+}} seq_cst, align 8
  // PURECAP: store atomic i64 {{%.+}}, i64 addrspace(200)* {{%.+}} seq_cst, align 8
  // PURECAP: atomicrmw xchg i64 addrspace(200)* %{{.+}}, i64 %{{.+}} seq_cst
  // PURECAP: cmpxchg i64 addrspace(200)* %{{.+}}, i64 %{{.+}}, i64 %{{.+}} seq_cst seq_cst
  // N64: load atomic i64, i64* {{%.+}} seq_cst, align 8
  // N64: store atomic i64 {{%.+}}, i64* {{%.+}} seq_cst, align 8
  // N64: atomicrmw xchg i64* %{{.+}}, i64 %{{.+}} seq_cst
  // N64: cmpxchg i64* %{{.+}}, i64 %{{.+}}, i64 %{{.+}} seq_cst seq_cst

  long res;
  long res2;
  __atomic_load(p, &res, 5);
  __atomic_store(p, &res, 5);
  __atomic_exchange(p, &res, &res2, 5);
  __atomic_compare_exchange(p, &res, &res2, 0, 5, 5);

  // Check that we emit sensible asm
  // ASM-LABEL: func4:
  // N64-ASM: ld	[[LD_ST_REG:\$[0-9]+]], 0($4)
  // N64-ASM-NEXT: sync
  // N64-ASM-NEXT: sync
  // N64-ASM-NEXT: sd	[[LD_ST_REG]], 0($4)
  // N64-ASM-NEXT: sync
  // PURECAP-ASM: cld	[[LD_ST_REG:\$[0-9]+]], $zero, 0($c3)
  // PURECAP-ASM: sync
  // PURECAP-ASM: sync
  // PURECAP-ASM: csd [[LD_ST_REG]], $zero, 0($c3)
  // PURECAP-ASM: sync
  // one ll/sc for the xchg:
  // N64-ASM: lld	$3, 0($4)
  // N64-ASM: scd	$2, 0($4)
  // PURECAP-ASM: clld	$3, $c3
  // PURECAP-ASM: cscd	$2, $1, $c3
  // one ll/sc for the cmpxchg:
  // N64-ASM: lld	$2, 0($4)
  // N64-ASM: scd	$5, 0($4)
  // PURECAP-ASM: clld	$2, $c3
  // PURECAP-ASM: cscd	$4, $3, $c3
  // ASM: .end	func4
}
