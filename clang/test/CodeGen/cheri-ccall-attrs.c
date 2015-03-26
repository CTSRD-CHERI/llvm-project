// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -S -emit-llvm | FileCheck %s
#define CHERI_CCALL(suffix, cls) \
	__attribute__((cheri_ccall))\
	__attribute__((cheri_method_suffix(suffix)))\
	__attribute__((cheri_method_class(cls)))

struct cheri_class
{
	__capability void *a, *b;
};

struct cheri_class cls;
struct cheri_class other;


CHERI_CCALL("_cap", cls)
void foo(int, int);

// CHECK: @__cheri_method.cls.foo = linkonce_odr global i64 0, section ".CHERI_CALLER"
// CHECK: @__cheri_callee_method.cls.fish = global void ()* @fish, section ".CHERI_CALLEE"

void bar(int a, int b)
{
	// CHECK: load i64, i64* @__cheri_method.cls.foo, !invariant.load !1
	// CHECK: call chericcallcc void @cheri_invoke(i8 addrspace(200)* inreg %3, i8 addrspace(200)* inreg %4, i64 zeroext %2, i32 signext %0, i32 signext %1)
	foo_cap(other, a, b);
	// CHECK: call chericcallcc void @cheri_invoke(i8 addrspace(200)* inreg %11, i8 addrspace(200)* inreg %13, i64 zeroext %7, i32 signext %5, i32 signext %6)
	foo(a,b);
}

__attribute__((cheri_ccallee))
__attribute__((cheri_method_class(cls)))
__attribute__((cheri_method_suffix("_cap")))
void fish(void)
{
	// CHECK: define chericcallcce void @fish
}

__attribute__((cheri_method_suffix("_cap")))
__attribute__((cheri_method_class(cls)))
void flibble(void);

// CHECK: define void @call()
void call(void)
{
	// CHECK: call chericcallcce void @fish()
	fish();
	// Check that we get a ccall to cheri_invoke with the correct method number
	// CHECK: load i64, i64* @__cheri_method.cls.fish, !invariant.load !1
	// CHECK: call chericcallcc void
	// CHECK: @cheri_invoke
	fish_cap(other);
	// CHECK: call void @flibble()
	flibble();
	// CHECK: load i64, i64* @__cheri_method.cls.flibble, !invariant.load !1
	// CHECK: call chericcallcc void
	// CHECK: @cheri_invoke
	flibble_cap(other);
}
