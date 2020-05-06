// RUN: %cheri_purecap_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics
void *__capability x = (void *__capability)0; // No warning since it's a null pointer
void * __capability y =  (void * __capability)-1;
