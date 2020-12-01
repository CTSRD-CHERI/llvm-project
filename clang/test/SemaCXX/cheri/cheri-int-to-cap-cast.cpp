// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify=expected,hybrid
// RUN: %cheri_purecap_cc1 -o - %s -fsyntax-only -verify=expected,purecap

void test_static_cast(int x, long y) {
  void *ptr;
  void * __capability cap;

  // static_cast isn't valid for integer to pointer casts.

  cap = static_cast<void * __capability>(x);
  // hybrid-error@-1 {{cannot cast from type 'int' to pointer type 'void * __capability'}}
  // purecap-error@-2 {{cannot cast from type 'int' to pointer type 'void *'}}
  ptr = static_cast<void *>(x);
  // expected-error@-1 {{cannot cast from type 'int' to pointer type 'void *'}}

  cap = static_cast<void * __capability>(y);
  // hybrid-error@-1 {{cannot cast from type 'long' to pointer type 'void * __capability'}}
  // purecap-error@-2 {{cannot cast from type 'long' to pointer type 'void *'}}
}

void test_reinterpret_cast(int x, long y) {
  void *ptr;
  void * __capability cap;

  // These are all currently fine for hybrid.

  cap = reinterpret_cast<void * __capability>(x);
  // purecap-warning@-1 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  ptr = reinterpret_cast<void *>(x);
  // purecap-warning@-1 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  cap = reinterpret_cast<void * __capability>(y);
  // purecap-warning@-1 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
}

void test_dynamic_cast(int x, long y) {
  void *ptr;
  void * __capability cap;

  // dynamic_cast isn't valid for integer to pointer casts.

  cap = dynamic_cast<void * __capability>(x);
  // hybrid-error@-1 {{cannot use dynamic_cast to convert from 'int' to 'void * __capability'}}
  // purecap-error@-2 {{cannot use dynamic_cast to convert from 'int' to 'void *'}}
  ptr = dynamic_cast<void *>(x);
  // expected-error@-1 {{cannot use dynamic_cast to convert from 'int' to 'void *'}}

  cap = dynamic_cast<void * __capability>(y);
  // hybrid-error@-1 {{cannot use dynamic_cast to convert from 'long' to 'void * __capability'}}
  // purecap-error@-2 {{cannot use dynamic_cast to convert from 'long' to 'void *'}}
}

template <typename S, typename T>
class TestBasic {
  T test_static_cast(S s) {
    // static_cast isn't valid for integer to pointer casts.
    return static_cast<T>(s);
    // hybrid-error@-1 {{cannot cast from type 'int' to pointer type 'void * __capability'}}
    // purecap-error@-2 {{cannot cast from type 'int' to pointer type 'void *'}}
    // hybrid-error@-3 {{cannot cast from type 'long' to pointer type 'void * __capability'}}
    // purecap-error@-4 {{cannot cast from type 'long' to pointer type 'void *'}}
  }

  T test_reinterpret_cast(S s) {
    // These are all currently fine for hybrid.
    return reinterpret_cast<T>(s);
    // purecap-warning@-1 2 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  }

  T test_dynamic_cast(S s) {
    return dynamic_cast<T>(s);
    // hybrid-error@-1 {{cannot use dynamic_cast to convert from 'int' to 'void * __capability'}}
    // purecap-error@-2 {{cannot use dynamic_cast to convert from 'int' to 'void *'}}
    // hybrid-error@-3 {{cannot use dynamic_cast to convert from 'long' to 'void * __capability'}}
    // purecap-error@-4 {{cannot use dynamic_cast to convert from 'long' to 'void *'}}
  }
};

template class TestBasic<int, void * __capability>;
// hybrid-note@-1 {{in instantiation of member function 'TestBasic<int, void * __capability>::test_static_cast' requested here}}
// hybrid-note@-2 {{in instantiation of member function 'TestBasic<int, void * __capability>::test_dynamic_cast' requested here}}
// purecap-note@-3 {{in instantiation of member function 'TestBasic<int, void *>::test_static_cast' requested here}}
// purecap-note@-4 {{in instantiation of member function 'TestBasic<int, void *>::test_reinterpret_cast' requested here}}
// purecap-note@-5 {{in instantiation of member function 'TestBasic<int, void *>::test_dynamic_cast' requested here}}
template class TestBasic<long, void * __capability>;
// hybrid-note@-1 {{in instantiation of member function 'TestBasic<long, void * __capability>::test_static_cast' requested here}}
// hybrid-note@-2 {{in instantiation of member function 'TestBasic<long, void * __capability>::test_dynamic_cast' requested here}}
// purecap-note@-3 {{in instantiation of member function 'TestBasic<long, void *>::test_static_cast' requested here}}
// purecap-note@-4 {{in instantiation of member function 'TestBasic<long, void *>::test_reinterpret_cast' requested here}}
// purecap-note@-5 {{in instantiation of member function 'TestBasic<long, void *>::test_dynamic_cast' requested here}}

template <typename S, typename T>
class TestQualified {
  T __capability test_static_cast(S s) {
    // static_cast isn't valid for integer to pointer casts.
    return static_cast<T __capability>(s);
    // hybrid-error@-1 {{cannot cast from type 'int' to pointer type 'void * __capability'}}
    // purecap-error@-2 {{cannot cast from type 'int' to pointer type 'void *'}}
    // hybrid-error@-3 {{cannot cast from type 'long' to pointer type 'void * __capability'}}
    // purecap-error@-4 {{cannot cast from type 'long' to pointer type 'void *'}}
  }

  T __capability test_reinterpret_cast(S s) {
    // These are all currently fine for hybrid.
    return reinterpret_cast<T __capability>(s);
    // purecap-warning@-1 2 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
  }

  T __capability test_dynamic_cast(S s) {
    // dynamic_cast isn't valid for integer to pointer casts.
    return dynamic_cast<T __capability>(s);
    // hybrid-error@-1 {{cannot use dynamic_cast to convert from 'int' to 'void * __capability'}}
    // purecap-error@-2 {{cannot use dynamic_cast to convert from 'int' to 'void *'}}
    // hybrid-error@-3 {{cannot use dynamic_cast to convert from 'long' to 'void * __capability'}}
    // purecap-error@-4 {{cannot use dynamic_cast to convert from 'long' to 'void *'}}
  }
};

template class TestQualified<int, void *>;
// hybrid-note@-1 {{in instantiation of member function 'TestQualified<int, void *>::test_static_cast' requested here}}
// hybrid-note@-2 {{in instantiation of member function 'TestQualified<int, void *>::test_dynamic_cast' requested here}}
// purecap-note@-3 {{in instantiation of member function 'TestQualified<int, void *>::test_static_cast' requested here}}
// purecap-note@-4 {{in instantiation of member function 'TestQualified<int, void *>::test_reinterpret_cast' requested here}}
// purecap-note@-5 {{in instantiation of member function 'TestQualified<int, void *>::test_dynamic_cast' requested here}}
template class TestQualified<long, void *>;
// hybrid-note@-1 {{in instantiation of member function 'TestQualified<long, void *>::test_static_cast' requested here}}
// hybrid-note@-2 {{in instantiation of member function 'TestQualified<long, void *>::test_dynamic_cast' requested here}}
// purecap-note@-3 {{in instantiation of member function 'TestQualified<long, void *>::test_static_cast' requested here}}
// purecap-note@-4 {{in instantiation of member function 'TestQualified<long, void *>::test_reinterpret_cast' requested here}}
// purecap-note@-5 {{in instantiation of member function 'TestQualified<long, void *>::test_dynamic_cast' requested here}}
