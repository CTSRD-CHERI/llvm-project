// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -fsyntax-only -verify
__capability void *b;
__capability void *c;
void a(int x, long long y)
{
	b = (__capability void *)x; // expected-warning {{cast to '__capability void *' from smaller integer type 'int'}}
	c = (__capability void *)y; // Should be no warning here - long long is 64 bits.
}
