// RUN: %cheri_cc1 %s -emit-llvm -o - -verify
// RUN: %cheri_purecap_cc1 %s -emit-llvm -o - -verify
// This new warning wasn't handling CHERI capabilities:
typedef struct {
  int a, b;
} IntPair;

typedef struct {
  long long a;
} LongStruct;

typedef int __attribute__((aligned(1))) unaligned_int;

void func(IntPair *p) {
  IntPair res;
  __atomic_load(p, &res, 0); // expected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  __atomic_store(p, &res, 0); // expected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  __atomic_fetch_add((unaligned_int *)p, 1, 2); // expected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  __atomic_fetch_sub((unaligned_int *)p, 1, 3); // expected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
}

void func1(void **p) {
  void* res;
  __atomic_load(p, &res, 0);
  __atomic_store(p, &res, 0);
  // N64: load atomic i64, i64* {{%.+}} monotonic, align 8
  // N64: store atomic i64 {{%.+}}, i64* {{%.+}} monotonic, align 8
  // PURECAP: load atomic i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* addrspace(200)* {{%.+}} monotonic
  // PURECAP: store atomic i8 addrspace(200)* {{%.+}}, i8 addrspace(200)* addrspace(200)* {{%.+}} monotonic
}

#if 0 // FIXME: this crashes

void func2(void* __capability *p) {
  void* __capability res;
  __atomic_load(p, &res, 0);
  __atomic_store(p, &res, 0);
  __atomic_fetch_add((int *)p, 1, 2);
  __atomic_fetch_sub((int *)p, 1, 3);
}

void func3(__uintcap_t *p) {
  __uintcap_t res;
  __atomic_load(p, &res, 0);
  __atomic_store(p, &res, 0);
  __atomic_fetch_add((int *)p, 1, 2);
  __atomic_fetch_sub((int *)p, 1, 3);
}

#endif
