// RUN: %riscv32_cheri_purecap_cc1 -fsyntax-only -verify %s
// RUN: %riscv64_cheri_purecap_cc1 -fsyntax-only -verify %s

/// Verify that __builtin_bit_cast does not require both types to be consistent
/// with respect to whether each is a capability, since its whole purpose is to
/// arbitrarily reinterpret the object representation.

// expected-no-diagnostics

typedef struct {
  void *p;
} wrap;

typedef struct {
  __PTRADDR_TYPE__ a[sizeof(void *) / sizeof(__PTRADDR_TYPE__)];
} words;

wrap ptr2wrap(void *src) {
  return __builtin_bit_cast(wrap, src);
}

void *wrap2ptr(wrap src) {
  return __builtin_bit_cast(void *, src);
}

words ptr2words(void *src) {
  return __builtin_bit_cast(words, src);
}

void *words2ptr(words src) {
  return __builtin_bit_cast(void *, src);
}
