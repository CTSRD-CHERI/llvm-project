// RUN: %cheri_purecap_cc1 -verify %s
// also check that the same warnings trigger in C++ mode
// RUNTODO: %cheri_purecap_cc1 -xc++ -verify %s

// See https://github.com/CTSRD-CHERI/clang/issues/189
typedef __uintcap_t uintptr_t;

void check_xor(void *ptr, uintptr_t cap, int i) {

  // TODO: should storing intcap_t in a smaller type such as int without a cast cause a warning?

  uintptr_t int_and_int = i ^ i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i ^ cap;   // expected-warning{{binary expression on capability and non-capability types: 'int' and 'uintptr_t' (aka '__uintcap_t')}}
  uintptr_t cap_and_int = cap ^ i;   // expected-warning{{using xor on a capability type only operates on the offset; consider using vaddr_t if this is used for pointer hashing or explicitly get the offset with __builtin_cheri_offset_get().}}
  uintptr_t cap_and_cap = cap ^ cap; // expected-warning{{using xor on a capability type only operates on the offset}}

  int int_and_int_2 = i ^ i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  int int_and_cap_2 = i ^ cap;         // expected-warning{{binary expression on capability and non-capability types: 'int' and 'uintptr_t' (aka '__uintcap_t')}}
  uintptr_t cap_and_int_2 = cap ^ i;   // expected-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t cap_and_cap_2 = cap ^ cap; // expected-warning{{using xor on a capability type only operates on the offset}}

  i ^= i;
  // FIXME: shouldn't this really be an invalid operand error?
  i ^= cap;   // expected-warning{{binary expression on capability and non-capability types}}
  cap ^= i;   // expected-warning{{using xor on a capability type only operates on the offset}}
  cap ^= cap; // expected-warning{{using xor on a capability type only operates on the offset}}

  uintptr_t hash = ptr ^ 0x1234567;                                  // expected-error{{invalid operands to binary expression ('void * __capability' and 'int')}}
  uintptr_t hash2 = (__intcap_t)ptr ^ 0x1234567;                     // expected-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t hash3 = cap ^ 0x1234567;                                 // expected-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t nonsense = (__intcap_t)ptr ^ cap;                        // expected-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t is_this_really_zero = (__intcap_t)ptr ^ (__intcap_t)ptr; // expected-warning{{using xor on a capability type only operates on the offset}}
  uintptr_t is_this_really_zero2 = (__intcap_t)ptr ^ cap;            // expected-warning{{using xor on a capability type only operates on the offset}}
}

void check_shift_left(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i << i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i << cap;   // expected-warning{{using shifts on a capability type only operates on the offset}}
  uintptr_t cap_and_int = cap << i;   // expected-warning{{using shifts on a capability type only operates on the offset}}
  uintptr_t cap_and_cap = cap << cap; // expected-warning{{using shifts on a capability type only operates on the offset}}
  i <<= i;
  // FIXME: shouldn't this really be an invalid operand error?
  i <<= cap;   // expected-warning{{using shifts on a capability type only operates on the offset}}
  cap <<= i;   // expected-warning{{using shifts on a capability type only operates on the offset}}
  cap <<= cap; // expected-warning{{using shifts on a capability type only operates on the offset}}
}

void check_shift_right(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i >> i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i >> cap;   // expected-warning{{using shifts on a capability type only operates on the offset}}
  uintptr_t cap_and_int = cap >> i;   // expected-warning{{using shifts on a capability type only operates on the offset}}
  uintptr_t cap_and_cap = cap >> cap; // expected-warning{{using shifts on a capability type only operates on the offset}}
  i >>= i;
  // FIXME: shouldn't this really be an invalid operand error?
  i >>= cap;   // expected-warning{{using shifts on a capability type only operates on the offset}}
  cap >>= i;   // expected-warning{{using shifts on a capability type only operates on the offset}}
  cap >>= cap; // expected-warning{{using shifts on a capability type only operates on the offset}}
}

void check_and(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i & i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i & cap;   // expected-warning{{binary expression on capability and non-capability types: 'int' and 'uintptr_t' (aka '__uintcap_t')}}
  uintptr_t cap_and_int = cap & i;   // expected-warning{{using bitwise and on capability types may give surprising results;}}
  uintptr_t cap_and_cap = cap & cap; // expected-warning{{using bitwise and on capability types may give surprising results;}}
  i &= i;
  // FIXME: shouldn't this really be an invalid operand error?
  i &= cap;   // expected-warning{{binary expression on capability and non-capability types}}
  cap &= i;   // expected-warning{{using bitwise and on capability types may give surprising results;}}
  cap &= cap; // expected-warning{{using bitwise and on capability types may give surprising results;}}
}

// Bitwise or operations just operate on the offset field and this behaviour should always be fine -> no warnings
void check_or(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i | i; // fine
  // TODO: bitwise or of integer and capability should probably be an error since it isn't clear if vaddr or offset is wanted
  // however, by the time we do the check i has been promoted so we don't get a warning
  uintptr_t int_and_cap = i | cap; // expected-warning{{binary expression on capability and non-capability types}}
  uintptr_t cap_and_int = cap | i;
  uintptr_t cap_and_cap = cap | cap;
  i |= i;
  // TODO: bitwise or of integer and capability should probably be an error since it isn't clear if vaddr or offset is wanted
  i |= cap; // expected-warning{{binary expression on capability and non-capability types}}
  cap |= i;
  // TODO: this is also not really sensible since it only uses the offset
  cap |= cap;
}

void set_low_pointer_bits(void *ptr, uintptr_t cap) {
  _Bool aligned = ptr & 7;     // expected-error{{invalid operands to binary expression ('void * __capability' and 'int')}}
  _Bool aligned_bad = cap & 7; // expected-warning{{using bitwise and on capability types may give surprising results;}}

  // store flag in low pointer bits
  uintptr_t with_flags = cap | 3; // bitwise or works as expected
  if ((with_flags & 3) == 3) {    // expected-warning{{using bitwise and on capability types may give surprising results;}}
    // clear the flags again:
    with_flags &= ~3; // expected-warning{{using bitwise and on capability types may give surprising results;}}
  }

  ptr &= 3; // expected-error{{invalid operands to binary expression ('void * __capability' and 'int')}}
  cap &= 3; // expected-warning{{using bitwise and on capability types may give surprising results;}}

  ptr &= ~3; // expected-error{{invalid operands to binary expression ('void * __capability' and 'int')}}
  cap &= ~3; // expected-warning{{using bitwise and on capability types may give surprising results;}}

  ptr &= cap; // expected-error{{invalid operands to binary expression ('void * __capability' and 'uintptr_t' (aka '__uintcap_t'))}}
  cap &= cap; // expected-warning{{using bitwise and on capability types may give surprising results;}}

  ptr &= ~cap; // expected-error{{invalid operands to binary expression ('void * __capability' and 'uintptr_t' (aka '__uintcap_t'))}}
  cap &= ~cap; // expected-warning{{using bitwise and on capability types may give surprising results;}}
}

// verify that we warn for the QMutexLocker issue:
void do_unlock(void);
void this_broke_qmutex(uintptr_t mtx) {
  if ((mtx & (uintptr_t)1) == (uintptr_t)1) { // expected-warning{{using bitwise and on capability types may give surprising results;}}
    do_unlock();
  }
}
