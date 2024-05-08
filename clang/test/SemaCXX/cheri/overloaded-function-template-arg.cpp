/// Regression test based on libcxx/test/libcxx/utilities/function.objects/func.require/bullet_7.pass.cpp
/// which started to fail after recent changes to CHERI cap <-> non-cap conversion checks.
/// See https://github.com/CTSRD-CHERI/llvm-project/issues/500
// RUN: %clang_cc1 -std=c++14 -fsyntax-only  -verify %s
// RUN: %cheri_purecap_cc1 -std=c++14 -fsyntax-only -verify %s
// RUN: %cheri_cc1 -std=c++14 -fsyntax-only -verify=expected,hybrid,hybrid-explicit %s -cheri-int-to-cap=explicit
// RUN: %cheri_cc1 -std=c++14 -fsyntax-only -verify=expected,hybrid,hybrid-implicit %s

void a(int);  // expected-note 1-2 {{candidate function has type mismatch at 1st parameter (expected 'char' but has 'int')}}
void a(long); // expected-note 1-2 {{candidate function has type mismatch at 1st parameter (expected 'char' but has 'long')}}
template <class T, T *Func>
struct c {
  static constexpr T *function = Func;
};
/// Note: One level of indirection through templates is required for this
/// regression (direct `c<void(int), a>::function(1);` uses were still okay).
template <class T, class Arg>
c<T, a> test_overloaded_fn_ptr(Arg arg) {
  // expected-note@-1{{candidate template ignored: substitution failure [with T = void (char)]: address of overloaded function 'a' does not match required type 'void (char)'}}
  c<T, a>::function(arg);
  return {};
}

void instantiate() {
  // The following (valid) expressions were previously rejected by CHERI Clang:
  test_overloaded_fn_ptr<void(int)>(1);
  c<void(int), a>::function(1);
  test_overloaded_fn_ptr<void(long)>(1);
  c<void(long), a>::function(1);
  // This one should always be an error:
  test_overloaded_fn_ptr<void(char)>(1); // expected-error{{no matching function for call to 'test_overloaded_fn_ptr'}}
  c<void(char), a>::function(1);         // expected-error{{address of overloaded function 'a' does not match required type 'void (char)'}}
}

#if __has_feature(capabilities)
template <class T, T *__capability Func>
// hybrid-note@-1 3 {{template parameter is declared here}}
struct c_cap {
  static constexpr T *__capability function = Func;
};
template <class T, class Arg>
c_cap<T, a> test_overloaded_fn_cap(Arg arg) {
  // expected-note@-1{{candidate template ignored: substitution failure [with T = void (char)]: address of overloaded function 'a' does not match required type 'void (char)'}}
  // hybrid-note@-2{{candidate template ignored: substitution failure [with T = void (int)]: non-type template argument of type 'void (int)' cannot be converted to a value of type 'void (* __capability)(int)'}}
  // hybrid-note@-3{{candidate template ignored: substitution failure [with T = void (long)]: non-type template argument of type 'void (long)' cannot be converted to a value of type 'void (* __capability)(long)'}}
  c_cap<T, a>::function(arg);
  return {};
}
void instantiate_cap() {
  // TODO: These were previously allowed, but had the wrong semantics, since
  // T * __capability dropped the __capability when instantiated. Should these
  // only warn for implicit?
  test_overloaded_fn_cap<void(int)>(1);
  // hybrid-error@-1{{no matching function for call to 'test_overloaded_fn_cap'}}
  c_cap<void(int), a>::function(1);
  // hybrid-error@-1{{non-type template argument of type 'void (int)' cannot be converted to a value of type 'void (* __capability)(int)'}}
  test_overloaded_fn_cap<void(long)>(1);
  // hybrid-error@-1{{no matching function for call to 'test_overloaded_fn_cap'}}
  c_cap<void(long), a>::function(1);
  // hybrid-error@-1{{non-type template argument of type 'void (long)' cannot be converted to a value of type 'void (* __capability)(long)'}}

  // These should always be an error:
  test_overloaded_fn_cap<void(char)>(1); // expected-error{{no matching function for call to 'test_overloaded_fn_cap'}}
  c_cap<void(char), a>::function(1);     // expected-error{{address of overloaded function 'a' does not match required type 'void (char)'}}

  // TODO: this should probably also warn by default when we allow implicit conversions:
  void (*__capability tmp1)(long) = a;
  // hybrid-explicit-error@-1{{converting non-capability type '<overloaded function type>' to capability type 'void (* __capability)(long)' without an explicit cast}}
  void (*__capability tmp2)(long) = static_cast<void (*)(long)>(&a);
  // hybrid-explicit-error@-1{{converting non-capability type 'void (*)(long)' to capability type 'void (* __capability)(long)' without an explicit cast}}
  // hybrid-implicit-warning@-2{{converting non-capability type 'void (*)(long)' to capability type 'void (* __capability)(long)' without an explicit cast}}
  void (*__capability tmp3)(long) = c<void(long), a>::function;
  // hybrid-explicit-error@-1{{converting non-capability type 'void (*const)(long)' to capability type 'void (* __capability)(long)' without an explicit cast}}
  // hybrid-implicit-warning@-2{{converting non-capability type 'void (*)(long)' to capability type 'void (* __capability)(long)' without an explicit cast}}
  void (*__capability tmp4)(long) = c_cap<void(long), a>::function;
  // hybrid-error@-1{{non-type template argument of type 'void (long)' cannot be converted to a value of type 'void (* __capability)(long)'}}
}
#endif
