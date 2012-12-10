// RUN: %clang_cc1 -fsyntax-only -triple cheri-unknown-freebsd %s -verify
int insecure();

__attribute__((sensitive))
int doSomethingCrazy(int * a)
{
  return *a + insecure(); // expected-warning {{Calling non-sensitive function}}
}

