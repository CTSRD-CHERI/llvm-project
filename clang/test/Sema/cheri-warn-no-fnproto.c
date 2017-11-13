// RUN: %cheri_cc1 -DNO_WARNING=1  -fsyntax-only %s -verify
// RUN: %cheri_cc1 -Wmips-cheri-prototypes -fsyntax-only %s -verify
// RUN: %cheri_purecap_cc1 -Wmips-cheri-prototypes -fsyntax-only %s -verify

#ifdef NO_WARNING
// expected-no-diagnostics
#else
// expected-warning@16 {{call to function 'do_append' with no prototype may lead to run-time stack corruption on CHERI}}
// expected-note@12 {{candidate function declaration needs parameter types}}
#endif

int do_append();
extern int store_char();

void pappend(char c) {
	do_append(c);
}

int do_append(long c) {
  return store_char();
}
