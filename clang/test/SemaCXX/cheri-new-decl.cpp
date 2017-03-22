// RUN: %clang_cc1 -triple cheri-unknown-freebsd -target-abi purecap -fsyntax-only -verify %s
// expected-no-diagnostics

void* operator new(unsigned long);
void operator delete(void*);
