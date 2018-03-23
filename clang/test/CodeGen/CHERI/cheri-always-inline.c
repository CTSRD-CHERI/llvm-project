// RUN: %cheri_purecap_cc1 -o - -O0 -emit-llvm %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -o - -O2 -emit-llvm %s | FileCheck %s
// Check that we haven't broken the inliner and that, even at -O0, we end up
// with foo being eliminated and baz calling bar directly

void bar();

__attribute__ ((__always_inline__))
static void foo()
{
	bar();
}

// CHECK-NOT: foo
// CHECK-LABEL: baz
// CHECK: bar
// CHECK-NOT: foo
void baz()
{
	foo();
}
