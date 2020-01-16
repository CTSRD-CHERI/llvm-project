// RUN: %cheri_cc1 -fsyntax-only %s -target-abi purecap -verify
// RUN: %cheri_cc1 -fsyntax-only %s -target-abi n64 -verify
void *__capability a(long b)
{
	return (void *__capability)b; // expected-warning {{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}} expected-note {{insert cast to intptr_t to silence this warning}}
}
void *__capability a_fixed(long b)
{
	return (void *__capability)(__intcap_t)b;
}
void *__capability c(__intcap_t b)
{
	return (void* __capability)b;
}
