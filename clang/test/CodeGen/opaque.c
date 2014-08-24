// RUN: %clang %s -target cheri-unknown-freebsd -o - -emit-llvm -S | FileCheck %s

// XFAIL:

typedef __capability struct foo* foo_c;
typedef struct foo* foo_t;

void *oldKey;
__capability void *key;
struct foo
{
	int a;
	int b;
};

#pragma opaque foo_c key
#pragma opaque foo_t oldKey 

void *calloc(unsigned long, unsigned long);
// CHECK: newFoo
foo_c newFoo(void)
{
	foo_c foo = (foo_c)calloc(1,sizeof(struct foo));
	// Check that the result is sealed
	// CHECK: llvm.mips.set.cap.length
	// CHECK: llvm.mips.seal.cap.data
	return __builtin_cheri_set_cap_length(foo, sizeof(struct foo));
}
// CHECK: newFoo_c
foo_t newFoo_c(void)
{
	// CHECK: xor
	return (foo_t)calloc(1,sizeof(struct foo));
}

// CHECK: getA
int getA(foo_t foo)
{
	// CHECK: xor
	return foo->a;
}
// CHECK: getA_c
int getA_c(foo_c foo)
{
	// CHECK: llvm.mips.unseal.cap
	return foo->a;
}

// CHECK: internal_call
int internal_call(void)
{
	// CHECK: llvm.mips.unseal.cap
	return newFoo()->a;
}
// CHECK: internal_call2
int internal_call2(void)
{
	// CHECK: xor
	return newFoo_c()->a;
}
