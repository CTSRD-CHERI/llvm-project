// RUN: %clang_cc1 -fsyntax-only %s -verify=nowarn -Wno-deprecated-non-prototype
// nowarn-no-diagnostics
// Should be enabled by -Wall,-Wcheri and -Wcheri-prototypes
// RUN: %cheri_purecap_cc1 -Wcheri-prototypes -fsyntax-only %s -verify
// RUN: %cheri_purecap_cc1 -Wall -fsyntax-only %s -verify
// RUN: %cheri_purecap_cc1 -Wcheri -fsyntax-only %s -verify
// RUN: %cheri_cc1 -Wcheri-prototypes -fsyntax-only %s -verify
// RUN: %cheri_cc1 -Wall -fsyntax-only %s -verify

int do_append(); // expected-note {{candidate function declaration needs parameter types}}
// expected-warning@-1{{a function declaration without a prototype is deprecated}}
extern int store_char();

void pappend(char c) {
  do_append(c); // expected-warning {{call to function 'do_append' with no prototype may lead to run-time stack corruption}}
  // expected-note@-1{{Calling functions without prototypes is dangerous because on some architectures (e.g. pure-capability MIPS) integer and pointer arguments are passed in different registers. If the call includes a parameter that does not match the function definition, the function will read uninitialized values from the argument registers.}}
  //expected-warning@-2{{passing arguments to 'do_append' without a prototype is deprecated in all versions of C and is not supported in C2x}}
}

int do_append(long c) { // expected-note{{conflicting prototype is here}}
  return store_char();
}

static int kr(); // expected-warning {{a function declaration without a prototype is deprecated in all versions of C and is treated as a zero-parameter prototype in C2x, conflicting with a subsequent definition}}
static int kr(arg) // expected-warning{{a function definition without a prototype is deprecated}}
  int arg;
{
  return arg + 1;
}

int call_kr(void) {
  return kr(1);
}
