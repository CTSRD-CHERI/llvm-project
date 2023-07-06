// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify
// expected-no-diagnostics

void * __capability test_decltype(void *p) {
  decltype(p) __capability c = (__cheri_tocap decltype(p) __capability)p;
  static_assert(__is_same(typeof(c) *, void *__capability *), "");
  static_assert(__is_same(decltype(c) &__capability, void *__capability &__capability), "");
  void *__capability &cref = c;
  static_assert(__is_same(decltype(cref) __capability, void *__capability &__capability), "");
  static_assert(__is_same(__capability decltype(cref), void *__capability &__capability), "");

  using intref = int &;
  static_assert(__is_same(intref __capability, int &__capability), "");
  static_assert(__is_same(__capability intref, int &__capability), "");
  return c;
}
