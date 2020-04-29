// RUN: %clang_cc1 -fsyntax-only %s -verify=nowarn
// nowarn-no-diagnostics
// Should be enabled by -Wall,-Wcheri and -Wcheri-prototypes
// RUN: %cheri_purecap_cc1 -Wcheri-prototypes -fsyntax-only %s -verify
// RUN: %cheri_purecap_cc1 -Wall -fsyntax-only %s -verify
// RUN: %cheri_purecap_cc1 -Wcheri -fsyntax-only %s -verify
// RUN: %cheri_cc1 -Wcheri-prototypes -fsyntax-only %s -verify
// RUN: %cheri_cc1 -Wall -fsyntax-only %s -verify

int do_append(); // expected-note {{candidate function declaration needs parameter types}}
extern int store_char();

void pappend(char c) {
  do_append(c); // expected-warning {{call to function 'do_append' with no prototype may lead to run-time stack corruption}}
  // expected-note@-1{{Calling functions without prototypes is dangerous because on some architectures (e.g. pure-capability MIPS) integer and pointer arguments are passed in different registers. If the call includes a parameter that does not match the function definition, the function will read uninitialized values from the argument registers.}}
}

int do_append(long c) {
  return store_char();
}

static int kr();
static int kr(arg)
  int arg;
{
  return arg + 1;
}

int call_kr(void) {
  return kr(1);
}
