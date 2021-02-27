// RUN: %cheri_cc1 -fsyntax-only -verify %s
// RUN: %cheri_purecap_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

_Bool cast(void * __capability ptr) {
  return (_Bool)ptr;
}
