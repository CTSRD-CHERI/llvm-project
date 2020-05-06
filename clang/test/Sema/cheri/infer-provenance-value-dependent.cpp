// RUN: %cheri_purecap_cc1 -o - -fsyntax-only  %s -verify -Weverything -Wno-missing-prototypes

template<int c> void* b() {
  return reinterpret_cast<void *>(c);
  // expected-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  // expected-note@-2{{insert cast to intptr_t to silence this warning}}
}

void* test() {
  // instantiate:
  return b<123>(); // expected-note{{in instantiation of function template specialization 'b<123>' requested here}}
}
