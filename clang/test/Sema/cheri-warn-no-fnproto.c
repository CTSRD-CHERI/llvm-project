// RUN: %clang_cc1 -DNO_WARNING=1 -triple cheri-unknown-freebsd -fsyntax-only %s -verify
// RUN: %clang_cc1 -triple cheri-unknown-freebsd -Wmips-cheri-prototypes -fsyntax-only %s -verify
// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -Wmips-cheri-prototypes -fsyntax-only %s -verify

#ifdef NO_WARNING
// expected-no-diagnostics
#else
// expected-warning@16 {{call to function 'do_append' with no prototype may lead to runtime stack corruption on CHERI}}
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
