// RUN: %cheri_cc1 -target-abi n64 -fsyntax-only %s -verify
/// Test case for https://github.com/CTSRD-CHERI/clang/issues/157
/// Applying __capability qualifier to a typedef as part of typo correction used to crash.

#define a(b)                 \
  __extension__({            \
    typedef __typeof__(b) c; \
    __capability c d;        \
  })
int e() {
  int *__capability pipv = a(pip); // expected-error{{use of undeclared identifier 'pip'; did you mean 'pipv'?}}
  // expected-error@-1{{initializing 'int * __capability' with an expression of incompatible type 'void'}}
  // expected-note@-2{{'pipv' declared here}}
}
