// RUN: %cheri_cc1 -fsyntax-only -verify %s -target-abi purecap

// expected-no-diagnostics

// Ensure that intcap_t and long are interchangeable for ICEs

unsigned long a = ((__uintcap_t)32) + 1;
__uintcap_t b = ((__uintcap_t)32) + 1;
__uintcap_t c = 32UL + 1;

long d = ((__intcap_t)32) + 1;
__intcap_t e = ((__intcap_t)32) + 1;
__uintcap_t f = 32L + 1;

int foo = (__uintcap_t)42;
