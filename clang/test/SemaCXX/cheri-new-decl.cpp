// RUN: %cheri_purecap_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

void* operator new(unsigned long);
void operator delete(void*);
