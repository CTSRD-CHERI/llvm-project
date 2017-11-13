// RUN: %cheri_cc1 -fsyntax-only  %s -verify
int insecure();

__attribute__((sensitive))
int doSomethingCrazy(int * a)
{
  return *a + insecure(); // expected-warning {{Calling non-sensitive function}}
}

