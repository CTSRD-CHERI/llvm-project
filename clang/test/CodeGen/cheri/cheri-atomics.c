// RUN: %cheri_purecap_cc1 -std=c11 %s -emit-llvm -o - -O2 -verify | FileCheck %s -implicit-check-not llvm.memcpy

// CHECK-LABEL: @main(
int main(void) {
  _Atomic(int *) p;

  // TODO: implement this inline
  __c11_atomic_init(&p, (int *)(__intcap_t)7);
  // CHECK: [[P_AS_I8:%.+]] = bitcast i32 addrspace(200)* addrspace(200)* %p to i8 addrspace(200)*
  // CHECK:  store i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 7) to i32 addrspace(200)*), i32 addrspace(200)* addrspace(200)* %p,

  __c11_atomic_fetch_add(&p, 1, __ATOMIC_SEQ_CST); // expected-warning {{unsupported atomic operation may incur significant performance penalty}}
  // CHECK: call i8 addrspace(200)* @__atomic_fetch_add_cap(i8 addrspace(200)* nonnull [[P_AS_I8]], i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 4), i32 signext 5)
  __c11_atomic_fetch_sub(&p, 2, __ATOMIC_SEQ_CST); // expected-warning {{unsupported atomic operation may incur significant performance penalty}}
  // CHECK: call i8 addrspace(200)* @__atomic_fetch_sub_cap(i8 addrspace(200)* nonnull [[P_AS_I8]], i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 8), i32 signext 5)
  int *c = __c11_atomic_load(&p, __ATOMIC_SEQ_CST);
  // CHECK: [[C_VALUE:%.+]] = load atomic i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %p seq_cst, align
  __c11_atomic_store(&p, 0, __ATOMIC_SEQ_CST);
  // CHECK: store atomic i32 addrspace(200)* null, i32 addrspace(200)* addrspace(200)* %p seq_cst, align
  int *expected = (int *)(__UINTPTR_TYPE__)1;
  __c11_atomic_compare_exchange_strong(&p, &expected, c, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  // CHECK: cmpxchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1) to i32 addrspace(200)*), i32 addrspace(200)* [[C_VALUE]] seq_cst seq_cst

  int *old = __c11_atomic_exchange(&p, c, __ATOMIC_SEQ_CST);
  // CHECK: atomicrmw xchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* [[C_VALUE]] seq_cst
  return *old;
}

// These should really take an i64 argument but fixing the atomic codegen is annoying
// CHECK: declare i8 addrspace(200)* @__atomic_fetch_add_cap(i8 addrspace(200)*, i8 addrspace(200)*, i32) local_unnamed_addr
// TODO-CHECK: declare i8 addrspace(200)* @__atomic_fetch_add_cap(i8 addrspace(200)*, i64, i32) local_unnamed_addr
// CHECK: declare i8 addrspace(200)* @__atomic_fetch_sub_cap(i8 addrspace(200)*, i8 addrspace(200)*, i32) local_unnamed_addr
// TODO-CHECK: declare i8 addrspace(200)* @__atomic_fetch_sub_cap(i8 addrspace(200)*, i64, i32) local_unnamed_addr


// Also check the GCC __atomic builtins: https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
int __atomic_stuff(int** p, int* expected, int* newval) {
  int* x1 = __atomic_load_n(p, __ATOMIC_SEQ_CST);
  // CHECK: load atomic i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %p seq_cst, align
  int* x2 = 0;
  __atomic_load(p, &x2, __ATOMIC_SEQ_CST);
  // CHECK: load atomic i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* %p seq_cst, align

  __atomic_store_n(p, x2, __ATOMIC_SEQ_CST);
  // CHECK: store atomic i32 addrspace(200)* %1, i32 addrspace(200)* addrspace(200)* %p seq_cst, align
  __atomic_store(p, &x2, __ATOMIC_SEQ_CST);
  // CHECK: store atomic i32 addrspace(200)* %1, i32 addrspace(200)* addrspace(200)* %p seq_cst, align

  int* x3 = __atomic_exchange_n(p, newval, __ATOMIC_SEQ_CST);
  // CHECK: atomicrmw xchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* %newval seq_cst
  int* x4 = 0;
  __atomic_exchange(p, &newval, &x4, __ATOMIC_SEQ_CST);
  // CHECK: atomicrmw xchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* %newval seq_cst

  __atomic_compare_exchange(p, &expected, &newval, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  // CHECK: cmpxchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* %expected, i32 addrspace(200)* %newval seq_cst seq_cst
  __atomic_compare_exchange(p, &expected, &newval, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  // CHECK: cmpxchg weak i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* %5, i32 addrspace(200)* %newval seq_cst seq_cst
  __atomic_compare_exchange_n(p, &expected, newval, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  // CHECK: cmpxchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* %7, i32 addrspace(200)* %newval seq_cst seq_cst
  __atomic_compare_exchange_n(p, &expected, newval, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  // CHECK: cmpxchg weak i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* %9, i32 addrspace(200)* %newval seq_cst seq_cst

  // Check the __sync_ builtin
  int* prev = __sync_val_compare_and_swap(p, expected, newval);
  // CHECK: cmpxchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* %11, i32 addrspace(200)* %newval seq_cst seq_cst
  int* swapped = __sync_swap(p, newval);
  // CHECK: %13 = atomicrmw xchg i32 addrspace(200)* addrspace(200)* %p, i32 addrspace(200)* %newval seq_cst

  // TODO: fetch_add/add_fetch
  return 0;
}

// CHECK-LABEL: @uint128(
int uint128(_Atomic(__uint128_t)* p) {
  __uint128_t loaded_val = 0;
  // CHECK: [[P_AS_I8CAP:%.+]] = bitcast i128 addrspace(200)* %p to i8 addrspace(200)*
  __c11_atomic_fetch_add(p, 1, __ATOMIC_SEQ_CST); // expected-warning {{large atomic operation may incur significant performance penalty}}
  // CHECK: call i128 @__atomic_fetch_add_16(i8 addrspace(200)* [[P_AS_I8CAP]], i128 zeroext 1, i32 signext 5)
  loaded_val = __c11_atomic_load(p, __ATOMIC_SEQ_CST); // expected-warning {{large atomic operation may incur significant performance penalty}}
  // CHECK: call void @__atomic_load(i64 zeroext 16, i8 addrspace(200)* [[P_AS_I8CAP]], i8 addrspace(200)* nonnull %1, i32 signext 5)
  __c11_atomic_store(p, 0, __ATOMIC_SEQ_CST); // expected-warning {{large atomic operation may incur significant performance penalty}}
  // CHECK: call void @__atomic_store(i64 zeroext 16, i8 addrspace(200)* [[P_AS_I8CAP]], i8 addrspace(200)* nonnull %3, i32 signext 5)
  __uint128_t expected = 0;
  __c11_atomic_compare_exchange_strong(p, &expected, loaded_val, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); // expected-warning {{large atomic operation may incur significant performance penalty}}
  // CHECK: call zeroext i1 @__atomic_compare_exchange(i64 zeroext 16, i8 addrspace(200)* %0, i8 addrspace(200)* nonnull %4, i8 addrspace(200)* nonnull %5, i32 signext 5, i32 signext 5)
  return 0;
}
