// RUN: %cheri_purecap_cc1 -fsyntax-only -verify %s
void *__capability x = (void *__capability)0; // No warning since it's a null pointer
void * __capability y =  (void * __capability)-1;
// expected-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
// expected-note@-2{{insert cast to intptr_t to silence this warning}}
