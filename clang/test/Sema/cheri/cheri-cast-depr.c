// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only -verify %s

// Test that we output a deprecated warning if __cheri_cast is used

void f(char * __capability c) {
  char *d = (__cheri_cast char *)c; // expected-error {{Using __cheri_cast is no longer valid}}
}
