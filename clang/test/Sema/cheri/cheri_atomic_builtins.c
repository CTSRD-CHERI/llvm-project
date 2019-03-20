// RUN: %cheri_purecap_cc1 -fsyntax-only  %s -verify
// RUN: %cheri_cc1 "-target-abi" "n64" -fsyntax-only  %s -verify

// check that we reject usage of the __sync atomic builtins with capabilites
// as this would result in wrong code generation


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




int capability_ptr() {
  void* __capability foo_cap;
  void* __capability result = 0;
  void* __capability newval = (__cheri_tocap void* __capability * __capability)&foo_cap;
  do_atomic_ops(&foo_cap, result, newval); // expected-error 16 {{the __sync_* atomic builtins only work with integers and not capability type 'void * __capability'.}}
  // check that calling the size-suffixed functions fails too
  do_suffixed_atomic_ops(&foo_cap, result, newval); // expected-error 5 {{the __sync_* atomic builtins only work with integers and not capability type 'void * __capability'.}}
}

int intcap() {
  __intcap_t foo_intcap = 0;
  __intcap_t result = 0;
  __intcap_t newval = (__intcap_t)&foo_intcap;
  do_atomic_ops(&foo_intcap, result, newval); // expected-error 16 {{the __sync_* atomic builtins only work with integers and not capability type '__intcap_t'.}}
  // check that calling the size-suffixed functions fails too
  do_suffixed_atomic_ops(&foo_intcap, result, newval); // expected-error 5 {{the __sync_* atomic builtins only work with integers and not capability type '__intcap_t'.}}
}

int uintptr() {
  __UINTPTR_TYPE__ foo_uintptr = 0;
  __UINTPTR_TYPE__ result = 0;
  __UINTPTR_TYPE__ newval = (__UINTPTR_TYPE__)1;

  // should be acceptable in hybrid ABI but cause errors in pure capability ABI
  do_atomic_ops(&foo_uintptr, result, newval);
#ifdef __CHERI_PURE_CAPABILITY__
  // expected-error@-2 16 {{the __sync_* atomic builtins only work with integers and not capability type '__uintcap_t'.}}
  do_suffixed_atomic_ops(&foo_uintptr, result, newval); // expected-error 5 {{the __sync_* atomic builtins only work with integers and not capability type '__uintcap_t'.}}
#else
  // expected-warning@-5 2 {{the semantics of this intrinsic changed with GCC version 4.4 - the newer semantics are provided here}}
#endif
}


#ifndef __CHERI_PURE_CAPABILITY__
int hybrid_ptr(void) {
  // in hybrid ABI all these should be acceptable
  void* foo_nocap = 0;
  void* result = 0;
  void* newval = 0;
  do_atomic_ops(&foo_nocap, result, newval); // expected-warning 2 {{the semantics of this intrinsic changed with GCC version 4.4 - the newer semantics are provided here}}
  do_suffixed_atomic_ops(&foo_nocap, result, newval);
}
#endif

#ifdef __CHERI_PURE_CAPABILITY__
#define memory_order_seq_cst __ATOMIC_SEQ_CST
#define memory_order_relaxed __ATOMIC_RELAXED

void memory_checks(_Atomic(int*) *Ap, int**p, int* val) {
  (void)__c11_atomic_load(Ap, memory_order_seq_cst);
  (void)__c11_atomic_store(Ap, val, memory_order_seq_cst);
  (void)__c11_atomic_fetch_add(Ap, 1, memory_order_seq_cst);
  (void)__c11_atomic_init(Ap, val);
  (void)__c11_atomic_fetch_sub(Ap, 1, memory_order_seq_cst);
  (void)__c11_atomic_fetch_and(Ap, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to atomic integer ('_Atomic(int * __capability) * __capability' invalid)}}
  (void)__c11_atomic_fetch_or(Ap, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to atomic integer ('_Atomic(int * __capability) * __capability' invalid)}}
  (void)__c11_atomic_fetch_xor(Ap, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to atomic integer ('_Atomic(int * __capability) * __capability' invalid)}}
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
  (void)__atomic_fetch_and(p, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to integer ('int * __capability * __capability' invalid)}}
  (void)__atomic_fetch_or(p, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to integer ('int * __capability * __capability' invalid)}}
  (void)__atomic_fetch_xor(p, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to integer ('int * __capability * __capability' invalid)}}
  (void)__atomic_fetch_nand(p, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to integer ('int * __capability * __capability' invalid)}}
  (void)__atomic_fetch_min(p, val, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to signed or unsigned 32-bit integer}}
  (void)__atomic_fetch_max(p, val, memory_order_seq_cst); // expected-error {{address argument to atomic operation must be a pointer to signed or unsigned 32-bit integer}}
  (void)__atomic_and_fetch(p, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to integer ('int * __capability * __capability' invalid)}}
  (void)__atomic_or_fetch(p, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to integer ('int * __capability * __capability' invalid)}}
  (void)__atomic_xor_fetch(p, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to integer ('int * __capability * __capability' invalid)}}
  (void)__atomic_nand_fetch(p, 1, memory_order_seq_cst); // expected-error {{address argument to bitwise atomic operation must be a pointer to integer ('int * __capability * __capability' invalid)}}
  (void)__atomic_exchange_n(p, val, memory_order_seq_cst);
  (void)__atomic_exchange(p, p, p, memory_order_seq_cst);
  (void)__atomic_compare_exchange(p, p, p, 0, memory_order_seq_cst, memory_order_relaxed);
  (void)__atomic_compare_exchange_n(p, p, val, 0, memory_order_seq_cst, memory_order_relaxed);
}

#endif
