// RUN: %cheri_cc1 -o - %s -ast-dump | FileCheck %s
#define CHERI_CCALL(n, suffix, cls) \
	__attribute__((cheri_ccall))\
	__attribute__((cheri_method_suffix(suffix)))\
	__attribute__((cheri_method_class(cls)))

// CHECK: used foo_cap 'void (struct cheri_object, int, int) __attribute__((cheri_ccall))' extern
// CHECK: used foo 'void (int, int) __attribute__((cheri_ccall))
struct cheri_object
{
	__capability void *a, *b;
};

struct cheri_object def;
struct cheri_object other;


CHERI_CCALL(1, "_cap", def)
void foo(int, int);


void bar(int a, int b)
{
	foo_cap(other, a, b);
	foo(a,b);
}
