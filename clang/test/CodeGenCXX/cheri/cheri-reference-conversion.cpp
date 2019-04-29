// RUN: %cheri_cc1 %s -fsyntax-only -Wall -Wno-unused-variable -std=c++11 -verify

struct aa {
  int a;
  int b;
};

struct bb {
  int &__capability a;
};

int &__capability fun1(struct aa &__capability in) {
  return in.a;
}

int &__capability fun2(struct aa *__capability in) {
  return in->b;
}

int &__capability fun3(struct bb *in) {
  return in->a;
}

int &__capability fun4(int *__capability in) {
  return in[10];
}

int &__capability fun5(int &__capability in) {
  return ++in;
}

int &__capability fun6(int &__capability in) {
  return --in;
}

int &__capability fun7(char *__capability in) {
  return (int &__capability)(*(in + 7));
}

int &__capability fun8(int &__capability x, int y) {
  return x = y;
}

int &__capability fun9(bool cond, int &__capability x, int &__capability y) {
  return cond ? x : y;
}

int &__capability fun10(struct aa &in) {
  return in.a; // expected-error {{converting non-capability type 'int &' to capability type 'int & __capability' without an explicit cast; if this is intended use __cheri_tocap}}
}

int &__capability fun11(struct aa *in) {
  return in->b; // expected-error {{converting non-capability type 'int &' to capability type 'int & __capability' without an explicit cast; if this is intended use __cheri_tocap}}
}

int &__capability fun12(struct bb *__capability in) {
  return in->a;
}

int &__capability fun13(int *in) {
  return in[10]; // expected-error {{onverting non-capability type 'int &' to capability type 'int & __capability' without an explicit cast; if this is intended use __cheri_tocap}}
}

int &__capability fun14(int &in) {
  return ++in; // expected-error {{converting non-capability type 'int &' to capability type 'int & __capability' without an explicit cast; if this is intended use __cheri_tocap}}
}

int &__capability fun15(int &in) {
  return --in; // expected-error {{converting non-capability type 'int &' to capability type 'int & __capability' without an explicit cast; if this is intended use __cheri_tocap}}
}

int &__capability fun16(char *in) {
  return (int &__capability)(*(in + 7)); // expected-error {{C-style cast to reference type 'int & __capability' changes __capability qualifier}}
}

int &__capability fun17(int &x, int y) {
  return x = y; // expected-error {{converting non-capability type 'int &' to capability type 'int & __capability' without an explicit cast; if this is intended use __cheri_tocap}}
}

int &__capability fun18(bool cond, int &x, int &__capability y) {
  return cond ? x : y; // expected-error {{converting non-capability type 'int &' to capability type 'int & __capability' without an explicit cast; if this is intended use __cheri_tocap}}
}

template <class T>
T get(T a) {
 return a;
}

int & __capability fun19(int *__capability x) {
  return get<int &__capability>(*x);
}

int & fun20(int *__capability x) {
  return get<int &__capability>(*x); // expected-error {{converting capability type 'int & __capability' to non-capability type 'int &' without an explicit cast; if this is intended use __cheri_fromcap}}
}
