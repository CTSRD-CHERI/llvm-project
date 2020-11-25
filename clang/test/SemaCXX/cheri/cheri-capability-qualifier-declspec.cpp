// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify
// expected-no-diagnostics

void * __capability test_decltype(void *p) {
  decltype(p) __capability c = (__cheri_tocap decltype(p) __capability)p;
  return c;
}
