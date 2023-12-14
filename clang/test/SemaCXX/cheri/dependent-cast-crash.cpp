// RUN: %clang_cc1 -fsyntax-only %s -verify=expected,nocheri
// RUN: %cheri_cc1 -fsyntax-only %s -verify=expected,hybrid
// RUN: %cheri_purecap_cc1 -fsyntax-only %s -verify=expected,purecap
#if !__has_feature(capabilities)
#define __capability
#endif

template <class a>
class b {
public:
  int *__capability constcast(b d) {
    return const_cast<int * __capability>(d);
    // nocheri-error@-1{{const_cast from 'b<int>' to 'int *' is not allowed}}
    // hybrid-error@-2{{const_cast from 'b<int>' to 'int * __capability' is not allowed}}
    // purecap-error@-3{{const_cast from 'b<int>' to 'int * __capability' (aka 'int *') is not allowed}}
  }
  int *__capability staticcast(b d) {
    return static_cast<int * __capability>(d);
    // nocheri-error@-1{{cannot cast from type 'b<int>' to pointer type 'int *'}}
    // hybrid-error@-2{{cannot cast from type 'b<int>' to pointer type 'int * __capability'}}
    // purecap-error@-3{{cannot cast from type 'b<int>' to pointer type 'int * __capability' (aka 'int *')}}
  }
  int *__capability reinterpretcast(b d) {
    return reinterpret_cast<int * __capability>(d);
    // nocheri-error@-1{{reinterpret_cast from 'b<int>' to 'int *' is not allowed}}
    // hybrid-error@-2{{reinterpret_cast from 'b<int>' to 'int * __capability' is not allowed}}
    // purecap-error@-3{{reinterpret_cast from 'b<int>' to 'int * __capability' (aka 'int *') is not allowed}}
  }
  int *__capability ccast(b d) {
    return (int *__capability)(d);
    // nocheri-error@-1{{cannot cast from type 'b<int>' to pointer type 'int *'}}
    // hybrid-error@-2{{cannot cast from type 'b<int>' to pointer type 'int * __capability'}}
    // purecap-error@-3{{cannot cast from type 'b<int>' to pointer type 'int * __capability' (aka 'int *')}}
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
