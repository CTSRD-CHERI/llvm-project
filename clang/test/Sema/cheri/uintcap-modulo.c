// RUN: %cheri_purecap_cc1 -verify %s


typedef __UINTPTR_TYPE__ uintptr_t;
typedef __UINT32_TYPE__ uint32_t;

void Q_ASSERT(_Bool cond);

// the original test case from qrandom.cpp:
void _fillRange(void *buffer, void *bufferEnd) {
  // Verify that the pointers are properly aligned for 32-bit
  Q_ASSERT((uintptr_t)buffer % sizeof(uint32_t) == 0); // expected-warning {{using remainder on a capability type only operates on the offset; consider using __builtin_is_aligned() instead or explicitly get the offset with __builtin_cheri_offset_get().}}
  Q_ASSERT((uintptr_t)bufferEnd % sizeof(uint32_t) == 0); // expected-warning {{using remainder on a capability type only operates on the offset; consider using __builtin_is_aligned() instead or explicitly get the offset with __builtin_cheri_offset_get().}}
}

// TODO: should this actually be an error?

void check_modulo(void *ptr, uintptr_t cap, int i) {
  uintptr_t int_and_int = i % i; // fine
  // i is promoted to __intcap_t here so the warning triggers:
  uintptr_t int_and_cap = i % cap;   // expected-warning {{using remainder on a capability type only operates on the offset}}
  uintptr_t cap_and_int = cap % i;   // expected-warning {{using remainder on a capability type only operates on the offset}}
  uintptr_t cap_and_cap = cap % cap; // expected-warning {{using remainder on a capability type only operates on the offset}}
  i %= i;
  i %= cap;   // expected-warning {{using remainder on a capability type only operates on the offset}}
  cap %= i;   // expected-warning {{using remainder on a capability type only operates on the offset}}
  cap %= cap; // expected-warning {{using remainder on a capability type only operates on the offset}}
}
