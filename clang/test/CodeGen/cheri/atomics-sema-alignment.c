// RUN: %cheri_cc1 %s -emit-llvm -o - -verify | %cheri_FileCheck %s -check-prefixes CHECK,N64
// RUN: %cheri_cc1 -w -DCODEGEN %s -S -o - -O2 | FileCheck %s -check-prefixes ASM,N64-ASM
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt %s -emit-llvm -o - -verify | %cheri_FileCheck %s -check-prefixes CHECK,PURECAP
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -DCODEGEN -w %s -S -o - -O2 | FileCheck %s -check-prefixes ASM,PURECAP-ASM
// The new warning "misaligned or large atomic operation" was causing -Werror failures for CheriBSD
// It is not a false positive since we need libcalls for CHERI cap (cmp)xchg but since this is
// a known issue (https://github.com/CTSRD-CHERI/clang/issues/201) we should not warn.
// This test also checks that we correctly use clc/csc+sync for atomic load/store and
// only need the libcalls for RMW sequences.
// TODO: for benchmark comparisions we should either fix this or also use libcalls for MIPS

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
  // libcalls for the xchg/cmpxchg
  // PURECAP: call void @__atomic_exchange(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i32 signext 5)
  // PURECAP: call zeroext i1 @__atomic_compare_exchange(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i32 signext 5, i32 signext 5)




  // ASM-LABEL: func1:
  // N64-ASM: ld	$2, 0($4)
  // N64-ASM-NEXT: sync
  // N64-ASM-NEXT: sync
  // N64-ASM-NEXT: sd	$2, 0($4)
  // N64-ASM-NEXT: sync
  // N64-ASM: .LBB0_1:
  // N64-ASM: lld	$3, 0($4)
  // N64-ASM: scd	$1, 0($4)
  // N64-ASM: beqz	$1, .LBB0_1
  // N64-ASM: .LBB0_3:
  // N64-ASM: lld	$1, 0($4)
  // N64-ASM: scd	$5, 0($4)
  // N64-ASM: beqz	$5, .LBB0_3

  // Check that we are emitting sensible assembly for the atomic load/store:
  // PURECAP-ASM: clc	$c1, $zero, 0($c3)
  // PURECAP-ASM: sync
  // PURECAP-ASM: sync
  // PURECAP-ASM: csc	$c1, $zero, 0($c3)
  // PURECAP-ASM-NEXT: sync
  // PURECAP-ASM: clcbi	$c12, %capcall20(__atomic_exchange)($c26)
  // PURECAP-ASM: clcbi	$c12, %capcall20(__atomic_compare_exchange)($c26)
  // ASM: .end	func1
}

void func2(void* __capability *p) {
  // CHECK-LABEL: @func2(
  // PURECAP: load atomic i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: store atomic i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: call void @__atomic_exchange(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i32 signext 5)
  // PURECAP: call zeroext i1 @__atomic_compare_exchange(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i32 signext 5, i32 signext 5)

  // hybrid code uses a libcall here and casts the i8 addrspace(200)** to i8* for the libcall
  // N64: [[PTR1:%.+]] = bitcast i8 addrspace(200)** %{{.+}} to i8*
  // N64: call void @__atomic_load(i64 zeroext [[$CAP_SIZE]], i8* [[PTR1]], i8* %{{.+}}, i32 signext 5)
  // N64: call void @__atomic_store(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i32 signext 5)
  // N64: call void @__atomic_exchange(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i8* %{{.+}}, i32 signext 5)
  // N64: call zeroext i1 @__atomic_compare_exchange(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i8* %{{.+}}, i32 signext 5, i32 signext 5)

  void* __capability res;
  void* __capability res2;
  __atomic_load(p, &res, 5);
  __atomic_store(p, &res, 5);
  __atomic_exchange(p, &res, &res2, 5);
  __atomic_compare_exchange(p, &res, &res2, 0, 5, 5);

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
  // PURECAP-ASM: clcbi	$c12, %capcall20(__atomic_exchange)($c26)
  // PURECAP-ASM: clcbi	$c12, %capcall20(__atomic_compare_exchange)($c26)
  // ASM: .end	func2
}

void func3(__uintcap_t *p) {
  // CHECK-LABEL: @func3(
  // PURECAP: load atomic i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: store atomic i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: call void @__atomic_exchange(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i32 signext 5)
  // PURECAP: call zeroext i1 @__atomic_compare_exchange(i64 zeroext [[$CAP_SIZE]], i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i32 signext 5, i32 signext 5)

  // hybrid code uses a libcall here and casts the i8 addrspace(200)** to i8* for the libcall
  // N64: [[PTR1:%.+]] = bitcast i8 addrspace(200)** %{{.+}} to i8*
  // N64: call void @__atomic_load(i64 zeroext [[$CAP_SIZE]], i8* [[PTR1]], i8* %{{.+}}, i32 signext 5)
  // N64: call void @__atomic_store(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i32 signext 5)
  // N64: call void @__atomic_exchange(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i8* %{{.+}}, i32 signext 5)
  // N64: call zeroext i1 @__atomic_compare_exchange(i64 zeroext [[$CAP_SIZE]], i8* %{{.+}}, i8* %{{.+}}, i8* %{{.+}}, i32 signext 5, i32 signext 5)


  __uintcap_t res;
  __uintcap_t res2;
  __atomic_load(p, &res, 5);
  __atomic_store(p, &res, 5);
  __atomic_exchange(p, &res, &res2, 5);
  __atomic_compare_exchange(p, &res, &res2, 0, 5, 5);

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
  // PURECAP-ASM: clcbi	$c12, %capcall20(__atomic_exchange)($c26)
  // PURECAP-ASM: clcbi	$c12, %capcall20(__atomic_compare_exchange)($c26)
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
  // N64-ASM: ld	$2, 0($4)
  // N64-ASM-NEXT: sync
  // N64-ASM-NEXT: sync
  // N64-ASM-NEXT: sd	$2, 0($4)
  // N64-ASM-NEXT: sync
  // PURECAP-ASM: cld	$2, $zero, 0($c3)
  // PURECAP-ASM: sync
  // PURECAP-ASM: sync
  // PURECAP-ASM: csd	$2, $zero, 0($c3)
  // PURECAP-ASM: sync
  // one ll/sc for the xchg:
  // N64-ASM: lld	$3, 0($4)
  // N64-ASM: scd	$1, 0($4)
  // PURECAP-ASM: clld	$3, $c3
  // PURECAP-ASM: cscd	$1, $2, $c3
  // one ll/sc for the cmpxchg:
  // N64-ASM: lld	$1, 0($4)
  // N64-ASM: scd	$5, 0($4)
  // PURECAP-ASM: clld	$1, $c3
  // PURECAP-ASM: cscd	$1, $3, $c3
  // ASM: .end	func4
}
