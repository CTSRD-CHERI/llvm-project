/// Check that the cheri.h header does not produce any warnings (including -Wshorten-cap-to-int)
// RUN: %cheri_cc1 -fsyntax-only -verify -Weverything -Wsystem-headers %s
// RUN: %cheri_purecap_cc1 -fsyntax-only -verify -Weverything -Wsystem-headers %s

#include <cheri.h>
extern void assert(_Bool cond);
#define FlagsMask 31
unsigned __intcap test_low_bits_clear(unsigned __intcap u);
unsigned __intcap test_low_bits_clear(unsigned __intcap u) {
  return cheri_low_bits_clear(u, FlagsMask); // no warning
}
unsigned long test_low_bits_clear_trunc(unsigned __intcap u);
unsigned long test_low_bits_clear_trunc(unsigned __intcap u) {
  return cheri_low_bits_clear(u, FlagsMask); // expected-warning{{implicit conversion loses capability metadata: 'uintcap_t' (aka 'unsigned __intcap') to 'unsigned long'}}
}
unsigned long test_low_bits_get(unsigned __intcap u);
unsigned long test_low_bits_get(unsigned __intcap u) {
  return cheri_low_bits_get(u, FlagsMask); // no warning
}
unsigned __intcap test_low_bits_set(unsigned __intcap u);
unsigned __intcap test_low_bits_set(unsigned __intcap u) {
  return cheri_low_bits_set(u, FlagsMask, 16); // no warning
}
unsigned long test_low_bits_set_trunc(unsigned __intcap u);
unsigned long test_low_bits_set_trunc(unsigned __intcap u) {
  return cheri_low_bits_set(u, FlagsMask, 16); // expected-warning{{implicit conversion loses capability metadata: 'uintcap_t' (aka 'unsigned __intcap') to 'unsigned long'}}
}
