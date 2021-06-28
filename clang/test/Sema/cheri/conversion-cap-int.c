// RUN: %cheri_cc1 -fsyntax-only -verify -Wshorten-cap-to-int %s
// RUN: %cheri_purecap_cc1 -fsyntax-only -verify -Wshorten-cap-to-int %s

// Similar to -Wshorten-64-to-32, this warning should be enabled by -Wconversion:
// RUN: %cheri_cc1 -fsyntax-only -verify=expected -Wconversion %s
// Since this warning is currently in -Wconversion, it is not enabled by any of these flags:
// RUN: %cheri_cc1 -fsyntax-only -verify=not-enabled -Wpedantic %s
// RUN: %cheri_cc1 -fsyntax-only -verify=not-enabled -Wall -Wextra %s

// not-enabled-no-diagnostics

long test_ret(__intcap v) {
  return v; // expected-warning {{implicit conversion loses capability metadata: '__intcap' to 'long'}}
}

long test_ret_cast(void) {
  // We should not warn if we know that the intcap_t does not carry provenace
  return (__intcap)1;
}

long test_ret_cast_2(char *a, char *b) {
  // We should not warn if we know that the intcap_t does not carry provenace
  return (__intcap)(a - b);
}

long test_ret_cast_3(long l) {
  // We should not warn if we know that the intcap_t does not carry provenace
  return (__intcap)l;
}

int test_arith_1(__intcap v) {
  return v + 2; // expected-warning {{implicit conversion loses capability metadata: '__intcap' to 'int'}}
}
int test_arith_2(__intcap v) {
  return v & 2; // expected-warning {{implicit conversion loses capability metadata: '__intcap' to 'int'}}
}
int test_arith_3(__intcap v) {
  return v | 2; // expected-warning {{implicit conversion loses capability metadata: '__intcap' to 'int'}}
}
int test_arith_4(__intcap v) {
  return v - 2; // expected-warning {{implicit conversion loses capability metadata: '__intcap' to 'int'}}
}
int test_arith_5(__intcap v) {
  return v / 2; // expected-warning {{implicit conversion loses capability metadata: '__intcap' to 'int'}}
}
int test_arith_6(__intcap v) {
  return v % 2; // expected-warning {{implicit conversion loses capability metadata: '__intcap' to 'int'}}
}
