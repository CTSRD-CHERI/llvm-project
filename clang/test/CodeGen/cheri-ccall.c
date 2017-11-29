// RUN: %cheri_cc1 %s -O1 -o - -emit-llvm | FileCheck %s
void
__attribute__((cheri_ccall))
b(__capability void*a1, __capability void*a2, __capability void *a3, __capability void *a4);

void a(__capability void *a1, __capability void *a2)
{
	// CHECK: a(
	// CHECK: call chericcallcc void @b
	b(a1, a2, a1, a2);
}
