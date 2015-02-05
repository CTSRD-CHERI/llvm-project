// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -S -emit-llvm | FileCheck %s
#define CHERI_CCALL(n, suffix, cls) \
	__attribute__((cheri_ccall))\
	__attribute__((cheri_method_number(n)))\
	__attribute__((cheri_method_suffix(suffix)))\
	__attribute__((cheri_method_class(cls)))

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
	// CHECK: call chericcallcc void @cheriinvoke(i8 addrspace(200)* inreg %2, i8 addrspace(200)* inreg %3, i64 zeroext 1, i32 signext %0, i32 signext %1)
	foo_cap(other, a, b);
	// CHECK:  chericcallcc void @cheriinvoke(i8 addrspace(200)* inreg %9, i8 addrspace(200)* inreg %11, i64 zeroext 1, i32 signext %4, i32 signext %5)
	foo(a,b);
}
