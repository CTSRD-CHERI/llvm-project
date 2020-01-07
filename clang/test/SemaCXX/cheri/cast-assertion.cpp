// RUN: %cheri_purecap_cc1 -fsyntax-only -o - %s -verify
/// This previously caused an assertion in checkCapabilityToIntCast() because
/// c is a "int & __capability" which causes the checks to be performed.

template <typename a> class b {
  // expected-note@-1{{during field initialization in the implicit default constructor}}
  int c = (double)c; // expected-warning{{field 'c' is uninitialized when used here}}
};

b<int> bb; // expected-note{{in implicit default constructor}}
