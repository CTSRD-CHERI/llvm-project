// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -ast-dump | FileCheck %s
#define CHERI_CCALL(n, suffix, class) \
	__attribute__((cheri_ccall))\
	__attribute__((cheri_method_number(n)))\
	__attribute__((cheri_method_suffix(suffix)))\
	__attribute__((cheri_method_class(cls)))

// CHECK: used foo_cap 'void (struct cheri_class, int, int) __attribute__((cheri_ccall))' extern
// CHECK: CheriMethodNumberAttr
// CHECK: used foo 'void (int, int) __attribute__((cheri_ccall))
// CHECK: CheriMethodNumberAttr
struct cheri_class
{
	__capability void *a, *b;
};

struct cheri_class def;
struct cheri_class other;


CHERI_CCALL(1, "_cap", def)
void foo(int, int);


void bar(int a, int b)
{
	foo_cap(other, a, b);
	foo(a,b);
}
