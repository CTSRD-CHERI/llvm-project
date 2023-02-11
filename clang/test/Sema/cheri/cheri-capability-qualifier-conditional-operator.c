// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify

struct A;
struct B;

void *__capability test_implicit_cast_mixed(_Bool b, struct A *__capability p, struct B *q) {
  return b ? p : q; // expected-error {{converting non-capability type 'struct B *' to capability type 'void * __capability' without an explicit cast}}
}

void *__capability test_implicit_cast_both(_Bool b, struct A *__capability p, struct B *__capability q) {
  return b ? p : q; // expected-warning {{pointer type mismatch ('struct A * __capability' and 'struct B * __capability')}}
}
