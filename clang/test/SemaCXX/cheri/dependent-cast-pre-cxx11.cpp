// RUN: %cheri_cc1 -std=c++98 -fsyntax-only %s -verify
// RUN: %cheri_purecap_cc1 -std=c++98 -fsyntax-only %s -verify

// This previously crashed an assertions build of Clang in pre-C++11 (or MSVC
// compatibility) mode due to checkProvenanceImpl for provenanceCheckedTy not
// handling value-dependent expressions.

// expected-no-diagnostics

template <typename T>
T * __capability intcap_to_capability(__intcap c) {
  return (T * __capability)c;
}

template <typename T>
__intcap capability_to_intcap(T * __capability c) {
  return (__intcap)c;
}

int * __capability test_intcap_to_capability(__intcap c) {
  return intcap_to_capability<int>(c);
}

__intcap test_capability_to_intcap(int * __capability c) {
  return capability_to_intcap<int>(c);
}
