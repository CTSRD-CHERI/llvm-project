// RUN: %cheri_cc1 %s -emit-llvm -o - -verify=atomics | %cheri_FileCheck %s -check-prefixes CHECK,HYBRID
// RUN: %riscv64_cheri_cc1 %s -target-feature +a -emit-llvm -o - -verify=atomics | FileCheck %s -check-prefixes CHECK,HYBRID
// RUN: %cheri_purecap_cc1 %s -emit-llvm -o - -verify=atomics | %cheri_FileCheck %s -check-prefixes CHECK,PURECAP
// RUN: %riscv64_cheri_purecap_cc1 -target-feature +a %s -emit-llvm -o - -verify=atomics | FileCheck %s -check-prefixes CHECK,PURECAP

// RUN: %riscv64_cheri_cc1 %s -emit-llvm -o - -verify=libcalls | FileCheck %s -check-prefixes CHECK,HYBRID-LIBCALLS
// RUN: %riscv64_cheri_purecap_cc1 %s -emit-llvm -o - -verify=libcalls | FileCheck %s -check-prefixes CHECK,PURECAP-LIBCALLS

// We used to require libcalls for CHERI cap atomics. Check that we don't any
// more for architectures with atomics support.

// atomics-no-diagnostics

void func1(void **p) {
  // CHECK-LABEL: @func1(
  void *res;
  void *res2;
  __atomic_load(p, &res, 5); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_store(p, &res, 5); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_exchange(p, &res, &res2, 5); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_compare_exchange(p, &res, &res2, 0, 5, 5); // libcalls-warning{{large atomic operation may incur significant performance penalty}}

  // HYBRID: load atomic i64, ptr {{%.+}} seq_cst, align 8
  // HYBRID: store atomic i64 {{%.+}}, ptr {{%.+}} seq_cst, align 8
  // HYBRID: atomicrmw xchg ptr %{{.+}}, i64 %{{.+}} seq_cst
  // HYBRID: cmpxchg ptr %{{.+}}, i64 %{{.+}}, i64 %{{.+}} seq_cst seq_cst

  // PURECAP: load atomic ptr addrspace(200), ptr addrspace(200) {{%.+}} seq_cst
  // PURECAP: store atomic ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}} seq_cst
  // PURECAP: atomicrmw xchg ptr addrspace(200) %{{.+}}, ptr addrspace(200) %{{.+}} seq_cst
  // PURECAP: cmpxchg ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}} seq_cst seq_cst

  // HYBRID-LIBCALLS: call i64 @__atomic_load_8(ptr noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call void @__atomic_store_8(ptr noundef %{{.+}}, i64 noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call i64 @__atomic_exchange_8(ptr noundef %{{.+}}, i64 noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call{{.+}} i1 @__atomic_compare_exchange_8(ptr noundef %{{.+}}, ptr noundef %{{.+}}, i64 noundef %{{.+}}, i32{{.+}} 5, i32{{.+}} 5)

  // PURECAP-LIBCALLS: call ptr addrspace(200) @__atomic_load_cap(ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call void @__atomic_store_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call ptr addrspace(200) @__atomic_exchange_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call{{.+}} i1 @__atomic_compare_exchange_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5, i32{{.+}} 5)
}

void func2(int *__capability *p) {
  // CHECK-LABEL: @func2(
  // PURECAP: load atomic ptr addrspace(200), ptr addrspace(200) {{%.+}} seq_cst
  // HYBRID: load atomic ptr addrspace(200), ptr {{%.+}} seq_cst
  // PURECAP: store atomic ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}} seq_cst
  // HYBRID: store atomic ptr addrspace(200) {{%.+}}, ptr {{%.+}} seq_cst
  // PURECAP: atomicrmw xchg ptr addrspace(200) %{{.+}}, ptr addrspace(200) %{{.+}} seq_cst
  // HYBRID: atomicrmw xchg ptr %{{.+}}, ptr addrspace(200) %{{.+}} seq_cst
  // PURECAP: cmpxchg ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}} seq_cst seq_cst
  // HYBRID: cmpxchg ptr {{%.+}}, ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}} seq_cst seq_cst
  // PURECAP: atomicrmw add ptr addrspace(200) %{{.+}}, ptr addrspace(200) %{{.+}} seq_cst
  // HYBRID: atomicrmw add ptr %{{.+}}, ptr addrspace(200) %{{.+}} seq_cst

  // HYBRID-LIBCALLS: call ptr addrspace(200) @__atomic_load_cap(ptr noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call ptr addrspace(200) @__atomic_load_cap(ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call void @__atomic_store_cap(ptr noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call void @__atomic_store_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call ptr addrspace(200) @__atomic_exchange_cap(ptr noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call ptr addrspace(200) @__atomic_exchange_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call{{.+}} i1 @__atomic_compare_exchange_cap(ptr noundef %{{.+}}, ptr noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call{{.+}} i1 @__atomic_compare_exchange_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call ptr addrspace(200) @__atomic_fetch_add_cap(ptr noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call ptr addrspace(200) @__atomic_fetch_add_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)

  int *__capability res;
  int *__capability res2;
  __atomic_load(p, &res, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_store(p, &res, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_exchange(p, &res, &res2, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_compare_exchange(p, &res, &res2, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_fetch_add(p, 1, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
}

void func3(__uintcap_t *p) {
  // CHECK-LABEL: @func3(
  // PURECAP: load atomic ptr addrspace(200), ptr addrspace(200) {{%.+}} seq_cst
  // HYBRID: load atomic ptr addrspace(200), ptr {{%.+}} seq_cst
  // PURECAP: store atomic ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}} seq_cst
  // HYBRID: store atomic ptr addrspace(200) {{%.+}}, ptr {{%.+}} seq_cst
  // PURECAP: atomicrmw xchg ptr addrspace(200) %{{.+}}, ptr addrspace(200) %{{.+}} seq_cst
  // HYBRID: atomicrmw xchg ptr %{{.+}}, ptr addrspace(200) %{{.+}} seq_cst
  // PURECAP: cmpxchg ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}} seq_cst seq_cst
  // HYBRID: cmpxchg ptr {{%.+}}, ptr addrspace(200) {{%.+}}, ptr addrspace(200) {{%.+}} seq_cst seq_cst
  // PURECAP: atomicrmw add ptr addrspace(200) %{{.+}}, ptr addrspace(200) %{{.+}} seq_cst
  // HYBRID: atomicrmw add ptr %{{.+}}, ptr addrspace(200) %{{.+}} seq_cst

  // HYBRID-LIBCALLS: call ptr addrspace(200) @__atomic_load_cap(ptr noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call ptr addrspace(200) @__atomic_load_cap(ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call void @__atomic_store_cap(ptr noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call void @__atomic_store_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call ptr addrspace(200) @__atomic_exchange_cap(ptr noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call ptr addrspace(200) @__atomic_exchange_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call{{.+}} i1 @__atomic_compare_exchange_cap(ptr noundef %{{.+}}, ptr noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call{{.+}} i1 @__atomic_compare_exchange_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call ptr addrspace(200) @__atomic_fetch_add_cap(ptr noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call ptr addrspace(200) @__atomic_fetch_add_cap(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)

  __uintcap_t res;
  __uintcap_t res2;
  __atomic_load(p, &res, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_store(p, &res, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_exchange(p, &res, &res2, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_compare_exchange(p, &res, &res2, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_fetch_add(p, 1, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
}

void func4(long *p) {
  // CHECK-LABEL: @func4(
  // PURECAP: load atomic i64, ptr addrspace(200) {{%.+}} seq_cst, align 8
  // PURECAP: store atomic i64 {{%.+}}, ptr addrspace(200) {{%.+}} seq_cst, align 8
  // PURECAP: atomicrmw xchg ptr addrspace(200) %{{.+}}, i64 %{{.+}} seq_cst
  // PURECAP: cmpxchg ptr addrspace(200) %{{.+}}, i64 %{{.+}}, i64 %{{.+}} seq_cst seq_cst
  // HYBRID: load atomic i64, ptr {{%.+}} seq_cst, align 8
  // HYBRID: store atomic i64 {{%.+}}, ptr {{%.+}} seq_cst, align 8
  // HYBRID: atomicrmw xchg ptr %{{.+}}, i64 %{{.+}} seq_cst
  // HYBRID: cmpxchg ptr %{{.+}}, i64 %{{.+}}, i64 %{{.+}} seq_cst seq_cst

  // HYBRID-LIBCALLS: call i64 @__atomic_load_8(ptr noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call i64 @__atomic_load_8(ptr addrspace(200) noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call void @__atomic_store_8(ptr noundef %{{.+}}, i64 noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call void @__atomic_store_8(ptr addrspace(200) noundef %{{.+}}, i64 noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call i64 @__atomic_exchange_8(ptr noundef %{{.+}}, i64 noundef %{{.+}}, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call i64 @__atomic_exchange_8(ptr addrspace(200) noundef %{{.+}}, i64 noundef %{{.+}}, i32{{.+}} 5)
  // HYBRID-LIBCALLS: call{{.+}} i1 @__atomic_compare_exchange_8(ptr noundef %{{.+}}, ptr noundef %{{.+}}, i64 noundef %{{.+}}, i32{{.+}} 5, i32{{.+}} 5)
  // PURECAP-LIBCALLS: call{{.+}} i1 @__atomic_compare_exchange_8(ptr addrspace(200) noundef %{{.+}}, ptr addrspace(200) noundef %{{.+}}, i64 noundef %{{.+}}, i32{{.+}} 5, i32{{.+}} 5)

  long res;
  long res2;
  __atomic_load(p, &res, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_store(p, &res, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_exchange(p, &res, &res2, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
  __atomic_compare_exchange(p, &res, &res2, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); // libcalls-warning{{large atomic operation may incur significant performance penalty}}
}
