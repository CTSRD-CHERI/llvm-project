// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -fsyntax-only -verify
void * __capability b;
void * __capability c;
void a(int x, long long y)
{
	b = (void * __capability)x; // expected-warning {{cast to 'void * __capability' from smaller integer type 'int'}}
	c = (void * __capability)y; // Should be no warning here - long long is 64 bits.
}
