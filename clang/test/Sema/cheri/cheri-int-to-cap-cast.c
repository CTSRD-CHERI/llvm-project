// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify
void * __capability b;
void * __capability c;
void a(int x, long long y)
{
	b = (void * __capability)x; // expected-warning {{cast to 'void * __capability' from smaller integer type 'int'}}
	// expected-warning@-1 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
	// expected-note@-2 {{insert cast to intptr_t to silence this warning}}
	c = (void * __capability)y; // Should be no warning here - long long is 64 bits.
	// expected-warning@-1 {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
	// expected-note@-2 {{insert cast to intptr_t to silence this warning}}
}
