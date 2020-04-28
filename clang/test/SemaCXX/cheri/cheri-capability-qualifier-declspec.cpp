// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify
// expected-no-diagnostics

void * __capability test_decltype(void *p) {
  decltype(p) __capability c = (__cheri_tocap decltype(p) __capability)p;
  return c;
}

// TODO: Clang can't currently represent this and so falls back on
// "__capability only applies to pointers; type here is 'T'". Ideally we would
// support this.
//template<typename T>
//T __capability test_template(T p) {
//  T __capability c = (__cheri_tocap T __capability)p;
//  return c;
//}
//template void * __capability test_template<void *>(void *);
