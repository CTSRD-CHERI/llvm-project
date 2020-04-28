// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify

// Previously this would erroneously give "use of __capability before the
// pointer type is deprecated" warnings for both lines.

int __capability x; // expected-error{{__capability only applies to pointers; type here is 'int'}}
int * __capability y;
