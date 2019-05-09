// RUN: %cheri_purecap_cc1 -fsyntax-only -o - %s  -verify
// expected-no-diagnostics
enum { b = 3 };
template <__uintcap_t c> class d {
public:
  enum { value = b * c };
};
d<1> e;
static_assert(d<1>::value == 3, "");
static_assert(d<2>::value == 6, "");
