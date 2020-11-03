// RUN: %cheri_cc1 %s -emit-llvm -o - -verify | %cheri_FileCheck %s -check-prefixes CHECK,HYBRID
// RUN: %riscv64_cheri_cc1 %s -target-feature +a -emit-llvm -o - -verify | %cheri_FileCheck %s -check-prefixes CHECK,HYBRID
// RUN: %cheri_purecap_cc1 %s -emit-llvm -o - -verify | %cheri_FileCheck %s -check-prefixes CHECK,PURECAP
// RUN: %riscv64_cheri_purecap_cc1 -target-feature +a %s -emit-llvm -o - -verify | %cheri_FileCheck %s -check-prefixes CHECK,PURECAP
// We used to require libcalls for CHERI cap (cmp)xchg. This has since been fixed for
// load/store/xchg/cmpxchg, but we should still warn for fetch_add.

void func1(void **p) {
  // CHECK-LABEL: @func1(
  void *res;
  void *res2;
  __atomic_load(p, &res, 5);
  __atomic_store(p, &res, 5);
  __atomic_exchange(p, &res, &res2, 5);
  __atomic_compare_exchange(p, &res, &res2, 0, 5, 5);

  // HYBRID: load atomic i64, i64* {{%.+}} seq_cst, align 8
  // HYBRID: store atomic i64 {{%.+}}, i64* {{%.+}} seq_cst, align 8
  // HYBRID: atomicrmw xchg i64* %{{.+}}, i64 %{{.+}} seq_cst
  // HYBRID: cmpxchg i64* %{{.+}}, i64 %{{.+}}, i64 %{{.+}} seq_cst seq_cst

  // PURECAP: load atomic i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // PURECAP: store atomic i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // no more libcalls for the xchg/cmpxchg:
  // PURECAP: atomicrmw xchg i8 addrspace(200)* addrspace(200)* %{{.+}}, i8 addrspace(200)* %{{.+}} seq_cst
  // PURECAP: cmpxchg i8 addrspace(200)* addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}} seq_cst seq_cst
}

void func2(int *__capability *p) {
  // CHECK-LABEL: @func2(
  // PURECAP: load atomic i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // HYBRID: load atomic i32 addrspace(200)*, i32 addrspace(200)** {{%.+}} seq_cst
  // PURECAP: store atomic i32 addrspace(200)* {{%.+}}, i32 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // HYBRID: store atomic i32 addrspace(200)* {{%.+}}, i32 addrspace(200)** {{%.+}} seq_cst
  // PURECAP: atomicrmw xchg i32 addrspace(200)* addrspace(200)* %{{.+}}, i32 addrspace(200)* %{{.+}} seq_cst
  // HYBRID: atomicrmw xchg i32 addrspace(200)** %{{.+}}, i32 addrspace(200)* %{{.+}} seq_cst
  // PURECAP: cmpxchg i32 addrspace(200)* addrspace(200)* {{%.+}}, i32 addrspace(200)* {{%.+}}, i32 addrspace(200)* {{%.+}} seq_cst seq_cst
  // HYBRID: cmpxchg i32 addrspace(200)** {{%.+}}, i32 addrspace(200)* {{%.+}}, i32 addrspace(200)* {{%.+}} seq_cst seq_cst

  int *__capability res;
  int *__capability res2;
  __atomic_load(p, &res, __ATOMIC_SEQ_CST);
  __atomic_store(p, &res, __ATOMIC_SEQ_CST);
  __atomic_exchange(p, &res, &res2, __ATOMIC_SEQ_CST);
  __atomic_compare_exchange(p, &res, &res2, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  __atomic_fetch_add(p, 1, __ATOMIC_SEQ_CST); // expected-warning{{unsupported atomic operation may incur significant performance penalty}}
}

void func3(__uintcap_t *p) {
  // CHECK-LABEL: @func3(
  // PURECAP: load atomic i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // HYBRID: load atomic i8 addrspace(200)*, i8 addrspace(200)** {{%.+}} seq_cst
  // PURECAP: store atomic i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* addrspace(200)* {{%.+}} seq_cst
  // HYBRID: store atomic i8 addrspace(200)* {{%.+}}, i8 addrspace(200)** {{%.+}} seq_cst
  // PURECAP: atomicrmw xchg i8 addrspace(200)* addrspace(200)* %{{.+}}, i8 addrspace(200)* %{{.+}} seq_cst
  // HYBRID: atomicrmw xchg i8 addrspace(200)** %{{.+}}, i8 addrspace(200)* %{{.+}} seq_cst
  // PURECAP: cmpxchg i8 addrspace(200)* addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}} seq_cst seq_cst
  // HYBRID: cmpxchg i8 addrspace(200)** {{%.+}}, i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* {{%.+}} seq_cst seq_cst

  __uintcap_t res;
  __uintcap_t res2;
  __atomic_load(p, &res, __ATOMIC_SEQ_CST);
  __atomic_store(p, &res, __ATOMIC_SEQ_CST);
  __atomic_exchange(p, &res, &res2, __ATOMIC_SEQ_CST);
  __atomic_compare_exchange(p, &res, &res2, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  __atomic_fetch_add(p, 1, __ATOMIC_SEQ_CST); // expected-warning{{unsupported atomic operation may incur significant performance penalty}}
}

void func4(long *p) {
  // CHECK-LABEL: @func4(
  // PURECAP: load atomic i64, i64 addrspace(200)* {{%.+}} seq_cst, align 8
  // PURECAP: store atomic i64 {{%.+}}, i64 addrspace(200)* {{%.+}} seq_cst, align 8
  // PURECAP: atomicrmw xchg i64 addrspace(200)* %{{.+}}, i64 %{{.+}} seq_cst
  // PURECAP: cmpxchg i64 addrspace(200)* %{{.+}}, i64 %{{.+}}, i64 %{{.+}} seq_cst seq_cst
  // HYBRID: load atomic i64, i64* {{%.+}} seq_cst, align 8
  // HYBRID: store atomic i64 {{%.+}}, i64* {{%.+}} seq_cst, align 8
  // HYBRID: atomicrmw xchg i64* %{{.+}}, i64 %{{.+}} seq_cst
  // HYBRID: cmpxchg i64* %{{.+}}, i64 %{{.+}}, i64 %{{.+}} seq_cst seq_cst

  long res;
  long res2;
  __atomic_load(p, &res, __ATOMIC_SEQ_CST);
  __atomic_store(p, &res, __ATOMIC_SEQ_CST);
  __atomic_exchange(p, &res, &res2, __ATOMIC_SEQ_CST);
  __atomic_compare_exchange(p, &res, &res2, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}
