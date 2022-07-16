// RUN: %cheri_purecap_cc1 -fsyntax-only  %s -verify=expected
// RUN: %cheri_cc1 "-target-abi" "n64" -fsyntax-only  %s -verify=expected

// Check that we can emit assembly without crashing
// RUN: %cheri_cc1 "-target-abi" "n64" -S -o /dev/null %s -verify=expected
// RUN: %cheri_purecap_cc1 -S -o /dev/null %s -verify=expected

#define do_atomic_ops(ptr, result, newval) do { \
  result = __sync_fetch_and_add(ptr, 0); \
  result = __sync_fetch_and_sub(ptr, 0); \
  result = __sync_fetch_and_or(ptr, 0); \
  result = __sync_fetch_and_and(ptr, 0); \
  result = __sync_fetch_and_xor(ptr, 0); \
  result = __sync_fetch_and_nand(ptr, 0); \
  result = __sync_add_and_fetch(ptr, 0); \
  result = __sync_sub_and_fetch(ptr, 0); \
  result = __sync_or_and_fetch(ptr, 0); \
  result = __sync_xor_and_fetch(ptr, 0); \
  result = __sync_nand_and_fetch(ptr, 0); \
  _Bool b = __sync_bool_compare_and_swap(ptr, result, newval); \
  result = __sync_val_compare_and_swap(ptr, result, newval); \
  result = __sync_swap(ptr, newval); \
  result = __sync_lock_test_and_set(ptr, newval); \
  __sync_lock_release(ptr); \
} while(0)

#define do_suffixed_atomic_ops(ptr, result, newval) do { \
  result = __sync_fetch_and_add_16(ptr, 0); \
  __sync_lock_release_8(ptr); \
  result = __sync_lock_test_and_set_4(ptr, newval); \
  _Bool b = __sync_bool_compare_and_swap_2(ptr, result, newval); \
  result = __sync_sub_and_fetch_1(ptr, 0); \
} while(0)




void capability_ptr() {
  void* __capability foo_cap = 0;
  void* __capability result = 0;
  void* __capability newval = (__cheri_tocap void* __capability * __capability)&foo_cap;
  do_atomic_ops(&foo_cap, result, newval);
  // expected-warning@-1 2 {{the semantics of this intrinsic changed with GCC version 4.4 - the newer semantics are provided here}}
  do_suffixed_atomic_ops(&foo_cap, result, newval);
}

void intcap() {
  __intcap foo_intcap = 0;
  __intcap result = 0;
  __intcap newval = (__intcap)&foo_intcap;
  do_atomic_ops(&foo_intcap, result, newval);
  // expected-warning@-1 2 {{the semantics of this intrinsic changed with GCC version 4.4 - the newer semantics are provided here}}
  do_suffixed_atomic_ops(&foo_intcap, result, newval);
}

void uintptr() {
  __UINTPTR_TYPE__ foo_uintptr = 0;
  __UINTPTR_TYPE__ result = 0;
  __UINTPTR_TYPE__ newval = (__UINTPTR_TYPE__)1;

  do_atomic_ops(&foo_uintptr, result, newval);
  // expected-warning@-1 2 {{the semantics of this intrinsic changed with GCC version 4.4 - the newer semantics are provided here}}
  do_suffixed_atomic_ops(&foo_uintptr, result, newval);
}


void native_ptr(void) {
  void* foo = 0;
  void* result = 0;
  void* newval = 0;
  do_atomic_ops(&foo, result, newval);
  // expected-warning@-1 2 {{the semantics of this intrinsic changed with GCC version 4.4 - the newer semantics are provided here}}
  do_suffixed_atomic_ops(&foo, result, newval);
}

#define memory_order_seq_cst __ATOMIC_SEQ_CST
#define memory_order_relaxed __ATOMIC_RELAXED

void memory_checks(_Atomic(int*) *Ap, int**p, int* val) {
  (void)__c11_atomic_load(Ap, memory_order_seq_cst);
  (void)__c11_atomic_store(Ap, val, memory_order_seq_cst);
  (void)__c11_atomic_fetch_add(Ap, 1, memory_order_seq_cst);
  (void)__c11_atomic_init(Ap, val);
  (void)__c11_atomic_fetch_sub(Ap, 1, memory_order_seq_cst);
  (void)__c11_atomic_fetch_and(Ap, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to atomic integer ('_Atomic(int *) *' invalid)}}
  (void)__c11_atomic_fetch_or(Ap, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to atomic integer ('_Atomic(int *) *' invalid)}}
  (void)__c11_atomic_fetch_xor(Ap, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to atomic integer ('_Atomic(int *) *' invalid)}}
  (void)__c11_atomic_exchange(Ap, val, memory_order_seq_cst);
  (void)__c11_atomic_compare_exchange_strong(Ap, p, val, memory_order_seq_cst, memory_order_relaxed);
  (void)__c11_atomic_compare_exchange_weak(Ap, p, val, memory_order_seq_cst, memory_order_relaxed);
  (void)__atomic_load_n(p, memory_order_seq_cst);
  (void)__atomic_load(p, p, memory_order_seq_cst);
  (void)__atomic_store(p, p, memory_order_seq_cst);
  (void)__atomic_store_n(p, val, memory_order_seq_cst);
  (void)__atomic_fetch_add(p, 1, memory_order_seq_cst);
  (void)__atomic_fetch_sub(p, 1, memory_order_seq_cst);
  (void)__atomic_add_fetch(p, 1, memory_order_seq_cst);
  (void)__atomic_sub_fetch(p, 1, memory_order_seq_cst);
  (void)__atomic_fetch_and(p, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_fetch_or(p, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_fetch_xor(p, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_fetch_nand(p, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_fetch_min(p, val, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_fetch_max(p, val, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_and_fetch(p, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_or_fetch(p, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_xor_fetch(p, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_nand_fetch(p, 1, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to integer ('int **' invalid)}}
  (void)__atomic_exchange_n(p, val, memory_order_seq_cst);
  (void)__atomic_exchange(p, p, p, memory_order_seq_cst);
  (void)__atomic_compare_exchange(p, p, p, 0, memory_order_seq_cst, memory_order_relaxed);
  (void)__atomic_compare_exchange_n(p, p, val, 0, memory_order_seq_cst, memory_order_relaxed);
}
