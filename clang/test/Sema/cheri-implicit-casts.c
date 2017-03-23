// RUN: %clang_cc1 %s -triple cheri-unknown-freebsd -fsyntax-only -verify
void f() {
  char * __capability x;
  char *y = x; // expected-error {{initializing 'char *' with an expression of incompatible type 'char * __capability'}}
}
void g() {
  char *x;
  char * __capability y = x; // expected-error {{initializing 'char * __capability' with an expression of incompatible type 'char *'}}
}
void h() {
  char * __capability x;
  int y = x; // expected-error {{initializing 'int' with an expression of incompatible type 'char * __capability'}}
}
void m() {
  int x;
  char * __capability y = x; // expected-error {{initializing 'char * __capability' with an expression of incompatible type 'int'}}
}
void i() {
  __uintcap_t x;
  char * __capability y = x; // expected-warning {{incompatible integer to pointer conversion initializing 'char * __capability' with an expression of type '__uintcap_t'}}
}
void j() {
  char * __capability x;
  __uintcap_t y = x; // expected-warning {{incompatible pointer to integer conversion initializing '__uintcap_t' with an expression of type 'char * __capability'}}
}
