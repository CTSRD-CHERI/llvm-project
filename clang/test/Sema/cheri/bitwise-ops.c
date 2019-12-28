// RUN: %cheri_purecap_cc1 -std=c11 -cheri-uintcap=offset -verify=expected,offset,bitand -Wpedantic -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-self-assign -Wno-missing-prototypes -Wno-sign-conversion %s
// also check that the same warnings trigger in C++ modes
// RUN: %cheri_purecap_cc1 -xc++ -cheri-uintcap=offset -verify=expected,offset,bitand -Wpedantic -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-self-assign -Wno-missing-prototypes -Wno-sign-conversion %s

// Check that we don't warn about bitwise operations when in cheri-uintcap=addr mode
// RUN: %cheri_purecap_cc1 -cheri-uintcap=addr -xc++ -verify=expected -Wpedantic -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-self-assign -Wno-missing-prototypes -Wno-sign-conversion %s
// However, with exact equals we still need to warn about it
// RUN: %cheri_purecap_cc1 -cheri-uintcap=addr -cheri-comparison=exact -xc++ -verify=expected,bitand -Wpedantic -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-self-assign -Wno-missing-prototypes -Wno-sign-conversion %s

// Check that -cheri-data-dependent-provenance affects the displayed diagnostics
// If we are in cheri-uintcap=offset mode we should still display the diagnostic since it won't do what we expect
// RUN: %cheri_purecap_cc1 -cheri-uintcap=offset -cheri-data-dependent-provenance -xc++ -verify=expected,offset,bitand -Wpedantic -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-self-assign -Wno-missing-prototypes -Wno-sign-conversion %s
// However in the -cheri-uintcap=addr mode we are operating on vaddrs and using data-dependent provenance
// so the bitwise-and operations should mostly behave the way a C programmer expects them to so we shouldn't warn:
// RUN: %cheri_purecap_cc1 -cheri-uintcap=addr -cheri-data-dependent-provenance -xc++ -verify=expected -Wpedantic -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-self-assign -Wno-missing-prototypes -Wno-sign-conversion %s

// See https://github.com/CTSRD-CHERI/clang/issues/189
typedef __uintcap_t uintptr_t;

void check_xor(void *ptr, uintptr_t cap, int i) {

  // TODO: should storing intcap_t in a smaller type such as int without a cast cause a warning?

  uintptr_t int_and_int = i ^ i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i ^ cap;   // offset-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t cap_and_int = cap ^ i;   // offset-warning{{using xor on a capability type only operates on the offset; consider using vaddr_t if this is used for pointer hashing or explicitly get the offset with __builtin_cheri_offset_get().}}
  uintptr_t cap_and_cap = cap ^ cap; // offset-warning{{using xor on a capability type only operates on the offset}}
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}

  int int_and_int_2 = i ^ i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  int int_and_cap_2 = i ^ cap;         // offset-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t cap_and_int_2 = cap ^ i;   // offset-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t cap_and_cap_2 = cap ^ cap; // offset-warning{{using xor on a capability type only operates on the offset}}
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}

  i ^= i;
  i ^= cap;
  cap ^= i;   // offset-warning{{using xor on a capability type only operates on the offset}}
  cap ^= cap; // offset-warning{{using xor on a capability type only operates on the offset}}

  uintptr_t hash = ptr ^ 0x1234567;                                  // expected-error{{invalid operands to binary expression ('void * __capability' and 'int')}}
  uintptr_t hash2 = (__intcap_t)ptr ^ 0x1234567;                     // offset-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t hash3 = cap ^ 0x1234567;                                 // offset-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t nonsense = (__intcap_t)ptr ^ cap;                        // offset-warning{{using xor on a capability type only operates on the offset}}
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}
  uintptr_t is_this_really_zero = (__intcap_t)ptr ^ (__intcap_t)ptr; // offset-warning{{using xor on a capability type only operates on the offset}}
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}
  uintptr_t is_this_really_zero2 = (__intcap_t)ptr ^ cap;            // offset-warning{{using xor on a capability type only operates on the offset}}
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}
}

void check_shift_left(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i << i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i << cap;   // offset-warning{{using shifts on a capability type only operates on the offset}}
  uintptr_t cap_and_int = cap << i;   // offset-warning{{using shifts on a capability type only operates on the offset}}
  uintptr_t cap_and_cap = cap << cap; // offset-warning{{using shifts on a capability type only operates on the offset}}
  i <<= i;
  // FIXME: shouldn't this really be an invalid operand error?
  i <<= cap;   // offset-warning{{using shifts on a capability type only operates on the offset}}
  cap <<= i;   // offset-warning{{using shifts on a capability type only operates on the offset}}
  cap <<= cap; // offset-warning{{using shifts on a capability type only operates on the offset}}
}

void check_shift_right(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i >> i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i >> cap;   // offset-warning{{using shifts on a capability type only operates on the offset}}
  uintptr_t cap_and_int = cap >> i;   // offset-warning{{using shifts on a capability type only operates on the offset}}
  uintptr_t cap_and_cap = cap >> cap; // offset-warning{{using shifts on a capability type only operates on the offset}}
  i >>= i;
  // FIXME: shouldn't this really be an invalid operand error?
  i >>= cap;   // offset-warning{{using shifts on a capability type only operates on the offset}}
  cap >>= i;   // offset-warning{{using shifts on a capability type only operates on the offset}}
  cap >>= cap; // offset-warning{{using shifts on a capability type only operates on the offset}}
}

void check_modulo(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i % i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i % cap;   // offset-warning{{using remainder on a capability type only operates on the offset}}
  uintptr_t cap_and_int = cap % i;   // offset-warning{{using remainder on a capability type only operates on the offset}}
  uintptr_t cap_and_cap = cap % cap; // offset-warning{{using remainder on a capability type only operates on the offset}}
  i %= i;
  i %= cap;   // offset-warning{{using remainder on a capability type only operates on the offset}}
  cap %= i;   // offset-warning{{using remainder on a capability type only operates on the offset}}
  cap %= cap; // offset-warning{{using remainder on a capability type only operates on the offset}}
}

// TODO: what about div/mul?

void check_and(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i & i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i & cap;   // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
  // Verify the full message once:
  uintptr_t cap_and_int = cap & i;   // bitand-warning{{using bitwise and on a capability type may give surprising results; if this is an alignment check use __builtin_{is_aligned,align_up,align_down}(); if you are operating on integer values only consider using size_t/vaddr_t; if you are attempting to store data in the low pointer bits use the cheri_{get,set,clear}_low_ptr_bits() macros.}}
  uintptr_t cap_and_cap = cap & cap; // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}
  i &= i;
  i &= cap;
  cap &= i;   // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
  cap &= cap; // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
}

// Bitwise or operations just operate on the offset field and this behaviour should always be fine -> no warnings
void check_or(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i | i; // fine
  // TODO: bitwise or of integer and capability should probably be an error since it isn't clear if vaddr or offset is wanted
  // however, by the time we do the check i has been promoted so we don't get a warning
  uintptr_t int_and_cap = i | cap;
  uintptr_t cap_and_int = cap | i;
  uintptr_t cap_and_cap = cap | cap;
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}
  i |= i;
  // TODO: bitwise or of integer and capability should probably be an error since it isn't clear if vaddr or offset is wanted
  i |= cap;
  cap |= i;
  // TODO: this is also not really sensible since it only uses the offset
  cap |= cap;
}

#ifndef __cplusplus
typedef _Bool bool;
#endif

void set_low_pointer_bits(void *ptr, uintptr_t cap) {
  bool aligned = ptr & 7;     // expected-error{{invalid operands to binary expression ('void * __capability' and 'int')}}
  bool aligned_bad = cap & 7; // bitand-warning{{using bitwise and on a capability type may give surprising results;}}

  // store flag in low pointer bits
  uintptr_t with_flags = cap | 3; // bitwise or works as expected
  if ((with_flags & 3) == 3) {    // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
    // clear the flags again:
    with_flags &= ~3; // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
  }

  ptr &= 3; // expected-error{{invalid operands to binary expression ('void * __capability' and 'int')}}
  cap &= 3; // bitand-warning{{using bitwise and on a capability type may give surprising results;}}

  ptr &= ~3; // expected-error{{invalid operands to binary expression ('void * __capability' and 'int')}}
  cap &= ~3; // bitand-warning{{using bitwise and on a capability type may give surprising results;}}

  ptr &= cap; // expected-error{{invalid operands to binary expression ('void * __capability' and 'uintptr_t' (aka '__uintcap_t'))}}
  cap &= cap; // bitand-warning{{using bitwise and on a capability type may give surprising results;}}

  ptr &= ~cap; // expected-error{{invalid operands to binary expression ('void * __capability' and 'uintptr_t' (aka '__uintcap_t'))}}
  cap &= ~cap; // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
}

// verify that we warn for the QMutexLocker issue:
void do_unlock(void);
void this_broke_qmutex(uintptr_t mtx) {
  if ((mtx & (uintptr_t)1) == (uintptr_t)1) { // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
    do_unlock();
  }
}

extern void __assert(int);
#define assert(expr) __assert(expr);

// Test that the the macros from cheri.h work as expected
#include <cheri.h>

void check_without_macros(void *mtx) {
  __uintcap_t u = (__uintcap_t)mtx;
  u |= 1;
  // Bug: will always be false
  // TODO: we should also warn about always false here
  if ((u & 1) == 1) { // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
    do_unlock();
  }
  // clear the bit again, warning should trigger
  u &= ~1; // bitand-warning{{using bitwise and on a capability type may give surprising results;}}
}

void check_with_macros(void *mtx) {
  // This should not trigger any warnings
  __uintcap_t u = (__uintcap_t)mtx;
  u = cheri_set_low_ptr_bits(u, 1);
  if (cheri_get_low_ptr_bits(u, 3) == 1) {
    do_unlock();
  }
  u = cheri_clear_low_ptr_bits(u, 3);
}

void check_bad_macro_values(void *mtx) {
  __uintcap_t u = (__uintcap_t)mtx;
  // Bad replacement of u &= ~3: should trigger unused result warning
  cheri_clear_low_ptr_bits(u, 3); // expected-warning{{ignoring return value of function declared with 'warn_unused_result' attribute}}
  // Bad replacement of u | 3: should trigger unused result warning
  cheri_set_low_ptr_bits(u, 3); // expected-warning{{ignoring return value of function declared with 'warn_unused_result' attribute}}
  // get should also have warn unused result
  cheri_get_low_ptr_bits(u, 3); // expected-warning{{ignoring return value of function declared with 'warn_unused_result' attribute}}

  // call the implementation directly
  __cheri_clear_low_ptr_bits(u, 3); // expected-warning{{ignoring return value of function declared with 'warn_unused_result' attribute}}
  __cheri_set_low_ptr_bits(u, 3);   // expected-warning{{ignoring return value of function declared with 'warn_unused_result' attribute}}
  __cheri_get_low_ptr_bits(u, 3);   // expected-warning{{ignoring return value of function declared with 'warn_unused_result' attribute}}

  // getting or clearing too many bits should warn:
  unsigned long value;
  value = cheri_get_low_ptr_bits(u, 32); // expected-error{{static_assert failed due to requirement '32 < 32' "Should only use the low 5 pointer bits"}}
  value = cheri_get_low_ptr_bits(u, 31); // This is fine

  // Check that the other macros also work in if statements
  if (cheri_set_low_ptr_bits(u, 3) == 1) {
    do_unlock();
  }
  if (cheri_clear_low_ptr_bits(u, 3) == 1) {
    do_unlock();
  }
}


int check_without_runtime_behaviour(__uintcap_t cap1, __uintcap_t cap2) {
  int x1 = (cap1 ^ cap2);  // offset-warning{{using xor on a capability type}}
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}
  x1 = (cap1 & cap2); // bitand-warning{{using bitwise and on a capability type}}
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}
  x1 = (cap1 >> cap2); // offset-warning{{using shifts on a capability type}}
  x1 = (cap1 << cap2); // offset-warning{{using shifts on a capability type}}
  x1 = (cap1 | cap2);
  // expected-warning@-1{{it is not clear which should be used as the source of provenance}}

  // No warnings here since the code is not emitted for sizeof()
  return x1 +
    sizeof(cap1 ^ cap2) +
    sizeof(cap1 & cap2) +
    sizeof(cap1 >> cap2) +
    sizeof(cap1 << cap2) +
    sizeof(cap1 | cap2);
}
