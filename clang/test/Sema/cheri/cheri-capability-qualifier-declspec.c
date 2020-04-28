// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify
// expected-no-diagnostics

void * __capability test_typeof(void *p) {
  __typeof__(p) __capability c = (__cheri_tocap __typeof__(p) __capability)p;
  return c;
}
