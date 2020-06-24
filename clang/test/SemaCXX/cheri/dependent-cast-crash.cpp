// RUN: %clang_cc1 -fsyntax-only %s -verify
// RUN: %cheri_cc1 -fsyntax-only %s -verify
// RUN: %cheri_purecap_cc1 -fsyntax-only %s -verify
#if !__has_feature(capabilities)
#define __capability
#endif

template <class a>
class b {
public:
  int *__capability constcast(b d) {
    return const_cast<int * __capability>(d); // expected-error-re{{const_cast from 'b<int>' to 'int *{{( __capability)?}}' is not allowed}}
  }
  int *__capability staticcast(b d) {
    return static_cast<int * __capability>(d); // expected-error-re{{cannot cast from type 'b<int>' to pointer type 'int *{{( __capability)?}}'}}
  }
  int *__capability reinterpretcast(b d) {
    return reinterpret_cast<int * __capability>(d); // expected-error-re{{reinterpret_cast from 'b<int>' to 'int *{{( __capability)?}}' is not allowed}}
  }
  int *__capability ccast(b d) {
    return (int *__capability)(d); // expected-error-re{{cannot cast from type 'b<int>' to pointer type 'int *{{( __capability)?}}'}}
  }
};

// Trigger errors by instantiating:
void test() {
  b<int> foo;
  (void)foo.constcast(foo);       // expected-note{{requested here}}
  (void)foo.staticcast(foo);      // expected-note{{requested here}}
  (void)foo.reinterpretcast(foo); // expected-note{{requested here}}
  (void)foo.ccast(foo);           // expected-note{{requested here}}
}
