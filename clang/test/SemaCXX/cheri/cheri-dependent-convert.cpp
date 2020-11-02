// RUN: %cheri_cc1 -std=c++17 -fsyntax-only %s -verify=expected,hybrid
// RUN: %cheri_purecap_cc1 -std=c++17 -fsyntax-only %s -verify=expected,purecap

// Check that declaring the templated IsCapability does not trigger any errors
// due to having dependent types rather than known pointers. Also check that
// instantiating it then gives the expected warnings and errors for the
// specific instantiations.

template <typename T, T>
struct SameType;

template <typename T>
struct IsCapability {
  static SameType<void * __capability, T::x> dummy;
  // hybrid-error@-1 {{value of type 'const int' is not implicitly convertible to 'void * __capability'}}
  // purecap-error@-2 {{value of type 'const int' is not implicitly convertible to 'void *'}}
  // hybrid-warning@-3 {{converting non-capability type 'void *' to capability type 'void * __capability' without an explicit cast; if this is intended use __cheri_tocap}}
};

struct HasInt {
  static constexpr int x = 0;
};

struct HasPtr {
  static constexpr void *x = 0;
};

struct HasCap {
  static constexpr void * __capability x = 0;
};

template struct IsCapability<HasInt>; // expected-note {{in instantiation of template class 'IsCapability<HasInt>' requested here}}
template struct IsCapability<HasPtr>; // hybrid-note {{in instantiation of template class 'IsCapability<HasPtr>' requested here}}
template struct IsCapability<HasCap>;
