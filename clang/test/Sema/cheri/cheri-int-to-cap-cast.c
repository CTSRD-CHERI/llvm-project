// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify=expected,hybrid
// RUN: %cheri_purecap_cc1 -o - %s -fsyntax-only -verify=expected,purecap
// RUN: %cheri_cc1 -o - -x c++ %s -fsyntax-only -verify=expected,hybrid
// RUN: %cheri_purecap_cc1 -o - -x c++ %s -fsyntax-only -verify=expected,purecap

void test_explicit_cast(int x, long y) {
  void *ptr;
  void * __capability cap;

  // Explicit C-style casts should behave identically between C and C++.

  cap = (void * __capability)x;
  // hybrid-warning@-1 {{cast to 'void * __capability' from smaller integer type 'int'}}
  // purecap-warning@-2 {{cast to 'void *' from smaller integer type 'int'}}
  // purecap-warning@-3 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  ptr = (void *)x;
  // expected-warning@-1 {{cast to 'void *' from smaller integer type 'int'}}
  // purecap-warning@-2 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}

  // Should be no warning here for hybrid - long is address-sized.
  cap = (void * __capability)y;
  // purecap-warning@-1 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
}
