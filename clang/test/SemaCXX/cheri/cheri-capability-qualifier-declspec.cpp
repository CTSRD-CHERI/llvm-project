// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify

void * __capability test_decltype(void *p) {
  decltype(p) __capability c = (__cheri_tocap decltype(p) __capability)p;
  return c;
}

// XXX: Clang can't currently represent this, but ideally we would support it.
// Document the current behaviour and ensure we don't crash.
template<typename T>
T __capability test_template(T p) {
  // expected-error@-1 {{capability only applies to pointers; type here is 'T'}}
  // expected-note@-2 {{candidate template ignored: could not match 'void *(void *)' against 'void * __capability (void *)'}}}
  T __capability c = (__cheri_tocap T __capability)p;
  // expected-error@-1 {{capability only applies to pointers; type here is 'T'}}
  // expected-error@-2 {{capability only applies to pointers; type here is 'T'}}
  return c;
}
template void * __capability test_template<void *>(void *); // expected-error {{explicit instantiation of 'test_template' does not refer to a function template, variable template, member function, member class, or static data member}}
