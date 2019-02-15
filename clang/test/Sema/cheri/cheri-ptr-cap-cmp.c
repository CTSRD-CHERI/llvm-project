// RUN: %cheri_cc1 -fsyntax-only  %s -verify

unsigned long a;
int  fn1() {
  unsigned long *__attribute__((cheri_capability)) b;
  return b != &a; // expected-error {{comparison between integer pointer and capability ('unsigned long * __capability' and 'unsigned long *')}}
}
