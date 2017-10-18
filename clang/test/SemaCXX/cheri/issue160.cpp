// XXXAR: we need to do -emit-llvm to get the errors and need three invocations since it will abort after the first error

// RUN: %cheri_cc1 -DPTR -target-abi n64 -fsyntax-only -std=c++11 -o - -emit-llvm -verify %s
// RUN: %cheri_cc1 -DREF -target-abi n64 -fsyntax-only -std=c++11 -o - -emit-llvm -verify %s
// RUN: %cheri_cc1 -DRREF -target-abi n64 -fsyntax-only -std=c++11 -o - -emit-llvm -verify %s

// See Sema/cheri/issue160.c
// Slightly different in C++, but we also have to disallow functions that differ
// only by __capability because the mangled name would be the same

struct a;

#ifdef PTR
void cap(struct a *) {} // expected-note{{previous definition is here}}
void cap(struct a * __capability c) {} // expected-error {{definition with same mangled name as another definition}}
#endif

#ifdef REF
void ref(struct a &) {} // expected-note{{previous definition is here}}
void ref(struct a & __capability) {}  // expected-error {{definition with same mangled name as another definition}}
#endif

#ifdef RREF
void rref(struct a &&) {} // expected-note{{previous definition is here}}
void rref(struct a && __capability) {} // expected-error {{definition with same mangled name as another definition}}
#endif
