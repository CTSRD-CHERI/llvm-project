// RUN: %clang_cc1 "-target-abi" "sandbox" -fsyntax-only -triple cheri-unknown-freebsd %s -verify
// expected-no-diagnostics

// TODO: add an AST dump test to check that we are creating longs instead of pointers

_Pragma("pointer_interpretation push")
_Pragma("pointer_interpretation integer")
struct foo {
	void *a;
	long d;
	long e;
};
struct foo si;
void *a;
_Pragma("pointer_interpretation pop")
struct bar {
	void *a;
	long d;
	long e;
};
void *b;
_Static_assert(sizeof(a) == 8, "Pointer size incorrect");
_Static_assert(sizeof(b) == 32, "Pointer size incorrect");
_Static_assert(sizeof(si) == 24, "Pointer size incorrect");
_Static_assert(sizeof(struct bar) == 64, "Pointer size incorrect");
