// RUN: %cheri_cc1 -o - %s -fsyntax-only -verify

int __capability *x; // expected-warning {{use of __capability before the pointer type is deprecated}}

// This used to be mis-parsed due to the combination of a line break and a
// struct or union specifier.
struct S
__capability *y; // expected-warning {{use of __capability before the pointer type is deprecated}}
