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
