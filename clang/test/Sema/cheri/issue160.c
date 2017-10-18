// RUN: %cheri_cc1 -target-abi n64 -fsyntax-only -verify %s

// We would previously not reject functions where the defintion and the declaration
// differ by a __capability qualifier. This resulted in assertions in CodeGen later
struct a;

// int vs long gives an error as expected
void foo(int a);   // expected-note{{previous declaration is here}}
void foo(long a) {};  // expected-error{{conflicting types for 'foo'}}

// same if it's a redeclaration
void foo1(int a);   // expected-note{{previous declaration is here}}
void foo1(long a);  // expected-error{{conflicting types for 'foo1'}}


// the same should be true for __capability vs plain pointer
void b(struct a *); // expected-note{{previous declaration is here}}
void b(struct a * __capability c) {}  // expected-error{{conflicting types for 'b'}}

void c(struct a * __capability d); // expected-note{{previous declaration is here}}
void c(struct a * d) {}  // expected-error{{conflicting types for 'c'}}
