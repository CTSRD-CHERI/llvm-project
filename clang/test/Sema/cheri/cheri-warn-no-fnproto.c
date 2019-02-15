// RUN: %clang_cc1 -fsyntax-only %s -verify=nowarn
// nowarn-no-diagnostics
// RUN: %cheri_purecap_cc1 -Wmips-cheri-prototypes -fsyntax-only %s -verify
// RUN: %cheri_cc1 -Wmips-cheri-prototypes -fsyntax-only %s -verify

int do_append(); // expected-note {{candidate function declaration needs parameter types}}
extern int store_char();

void pappend(char c) {
	do_append(c); // expected-warning {{call to function 'do_append' with no prototype may lead to run-time stack corruption on CHERI}}
  // expected-note@-1{{will read garbage values from the argument registers}}
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
