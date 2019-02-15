// RUN: %cheri_purecap_cc1 -o - -O0 -emit-llvm %s | FileCheck %s -implicit-check-not foo
// RUN: %cheri_purecap_cc1 -o - -O0 -mllvm -cheri-cap-table-abi=pcrel -emit-llvm %s | FileCheck %s -implicit-check-not foo
// RUN: %cheri_purecap_cc1 -o - -O2 -emit-llvm %s | FileCheck %s -implicit-check-not foo
// Check that we haven't broken the inliner and that, even at -O0, we end up
// with foo being eliminated and baz calling bar directly

void bar();

__attribute__ ((__always_inline__))
static void foo()
{
	bar();
}

// CHECK-LABEL: define void @baz()
// CHECK: @bar
void baz()
{
	foo();
}
