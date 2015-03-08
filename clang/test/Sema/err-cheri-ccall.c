// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only -verify %s 

struct cheri_class_wrong
{
	__capability void *a, *b, *c;
};
struct cheri_class_wrong2
{
	__capability void *a, *b;
	int c;
};

extern struct cheri_class def;
struct cheri_class_wrong def2;
struct cheri_class_wrong2 def3;


__attribute__((cheri_ccall))
__attribute__((cheri_method_class(def))) // expected-error{{must be a CHERI class}}
void a(int a, int b) {}

__attribute__((cheri_ccall))
__attribute__((cheri_method_class(def2))) // expected-error{{must be a CHERI class}}
void b(int a, int b) {}
__attribute__((cheri_ccall))
__attribute__((cheri_method_class(def3))) // expected-error{{must be a CHERI class}}
void c(int a, int b) {}
__attribute__((cheri_ccall))
__attribute__((cheri_method_class(def4))) // expected-error{{requires a valid declaration}}
void d(int a, int b) {}
