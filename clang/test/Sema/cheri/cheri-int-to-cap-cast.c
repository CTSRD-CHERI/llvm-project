// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify=expected,hybrid
// RUN: %cheri_purecap_cc1 -o - %s -fsyntax-only -verify=expected,purecap
void *__capability b;
void *c;
void *__capability d;
void a(int x, long long y) {
  b = (void *__capability)x;
  // hybrid-warning@-1 {{cast to 'void * __capability' from smaller integer type 'int'}}
  // purecap-warning@-2 {{cast to 'void *' from smaller integer type 'int'}}
  // purecap-warning@-3 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  c = (void *)x;
  // expected-warning@-1 {{cast to 'void *' from smaller integer type 'int'}}
  // purecap-warning@-2 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}

  // Should be no warning here - long long is 64 bits.
  d = (void *__capability)y;
  // purecap-warning@-1 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
}
