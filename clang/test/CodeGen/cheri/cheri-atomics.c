// RUNNOT: %cheri_purecap_cc1 -std=c11 %s -S -o - -O2 -verify
// RUN: %cheri_purecap_cc1 -std=c11 %s -emit-llvm -o - -O2 -verify
// RUN: %cheri_purecap_cc1 -std=c11 %s -S -o - -O2 -verify
// RUN: %cheri_purecap_cc1 -std=c11 %s -emit-llvm -o - -O2 -verify | FileCheck %s -implicit-check-not llvm.memcpy
// Check that we can generate assembly without crashing
// RUN: %cheri_purecap_cc1 -std=c11 %s -S -o /dev/null -verify
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c11 %s -S -o /dev/null -verify

// CHECK-LABEL: @main(
int main(void) {
  _Atomic(int*) p;

  // TODO: implement this inline
  __c11_atomic_fetch_add(&p, 1, __ATOMIC_SEQ_CST);  // expected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  // CHECK: [[P_AS_I8:%.+]] = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
  // CHECK: [[INC:%.+]] = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* null, i64 4)
  // CHECK: call i8 addrspace(200)* @__atomic_fetch_add_cap(i8 addrspace(200)* nonnull [[P_AS_I8]], i8 addrspace(200)* [[INC]], i32 signext 5)
  __c11_atomic_fetch_sub(&p, 2, __ATOMIC_SEQ_CST); // expected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  // CHECK: [[INC2:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* null, i64 8)
  // CHECK: call i8 addrspace(200)* @__atomic_fetch_sub_cap(i8 addrspace(200)* nonnull [[P_AS_I8]], i8 addrspace(200)* [[INC2]], i32 signext 5)
  int *c = __c11_atomic_load(&p, __ATOMIC_SEQ_CST);
  // CHECK: [[C_VALUE:%.+]] = load atomic i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %p seq_cst, align
  __c11_atomic_store(&p, 0, __ATOMIC_SEQ_CST);
  // CHECK: store atomic i32 addrspace(200)* null, i32 addrspace(200)* addrspace(200)* %p seq_cst, align
  int* expected = (int*)(__UINTPTR_TYPE__)1;
  __c11_atomic_compare_exchange_strong(&p, &expected, c, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  // CHECK: [[EXPECTED_I8:%.+]] = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* null, i64 1)
  // CHECK: [[EXPECTED:%.+]] = bitcast i8 addrspace(200)* [[EXPECTED_I8]] to i32 addrspace(200)*
  // CHECK: cmpxchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* [[EXPECTED]], i32 addrspace(200)* [[C_VALUE]] seq_cst seq_cst

  int* old = __c11_atomic_exchange(&p, c, __ATOMIC_SEQ_CST);
  // CHECK: atomicrmw xchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* [[C_VALUE]] seq_cst
  return *old;
}

// These should really take an i64 argument but fixing the atomic codegen is annoying
// TODO-CHECK: declare i8 addrspace(200)* @__atomic_fetch_add_cap(i8 addrspace(200)*, i8 addrspace(200)*, i32) local_unnamed_addr
// CHECK: declare i8 addrspace(200)* @__atomic_fetch_add_cap(i8 addrspace(200)*, i64, i32) local_unnamed_addr
// TODO-CHECK: declare i8 addrspace(200)* @__atomic_fetch_sub_cap(i8 addrspace(200)*, i8 addrspace(200)*, i32) local_unnamed_addr
// CHECK: declare i8 addrspace(200)* @__atomic_fetch_sub_cap(i8 addrspace(200)*, i64, i32) local_unnamed_addr


// CHECK-LABEL: @uint128(
int uint128(void) {
  _Atomic(__uint128_t) p;

  __c11_atomic_fetch_add(&p, 1, __ATOMIC_SEQ_CST);  // not-expected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  // CHECK: atomicrmw add i128 addrspace(200)* %p, i128 1 seq_cst
  __uint128_t c = __c11_atomic_load(&p, __ATOMIC_SEQ_CST);  // not-xpected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  // CHECK: load atomic i128, i128 addrspace(200)* %p seq_cst
  __c11_atomic_store(&p, 0, __ATOMIC_SEQ_CST);  // not-expected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  // CHECK: store atomic i128 0, i128 addrspace(200)* %p seq_cst
  __uint128_t expected = 0;
  __c11_atomic_compare_exchange_strong(&p, &expected, c, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);  // not-expected-warning {{misaligned or large atomic operation may incur significant performance penalty}}
  // CHECK: cmpxchg i128 addrspace(200)* %p, i128 0, i128 %{{.+}} seq_cst seq_cst
  return 0;
}
