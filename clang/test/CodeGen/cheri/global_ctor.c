// Check that we don't emit .ctors and .dtors but use .init_array/.fini_array instead
// llc already defaulted to .init_array, but clang didn't

// RUN: %cheri_purecap_clang -fuse-init-array -o - -O0 -S %s -### 2>&1 | FileCheck %s -check-prefix INIT_ARRAY_CMD
// RUN: %cheri_purecap_clang -fno-use-init-array -o - -O0 -S %s -### 2>&1 | FileCheck %s -check-prefix CTORS_CMD
// INIT_ARRAY_CMD: -fuse-init-array
// CTORS_CMD-NOT: -fuse-init-array
// We should default to using .init_array:
// RUN: %cheri_purecap_clang -o - -O0 -S %s -### 2>&1 | FileCheck %s -check-prefix INIT_ARRAY_CMD


// Check that clang and cc1 default to using .init_array
// RUN: %cheri_purecap_clang -o - -O0 -S %s | FileCheck %s -implicit-check-not .ctors -implicit-check-not .dtors
// RUN: %cheri_purecap_cc1 -o - -O0 -S %s | FileCheck %s -implicit-check-not .ctors -implicit-check-not .dtors
// RUN: %cheri_purecap_cc1 -fno-use-init-array -o /dev/null -O0 -S %s 2>&1 | FileCheck %s -check-prefix WARN
// WARN: warning: The CHERI RTLD does not support .ctors. Passing -fno-use-init-array will probably create broken binaries.
// Check that we don't produce the warning when invoking cheri_purecap_cc1 without the -fuse-init-array option:
// RUN: %cheri_purecap_cc1 -o - -O0 -S %s 2>&1 | FileCheck %s -check-prefix NOWARN -implicit-check-not warning
// RUN: %cheri_purecap_cc1 -fuse-init-array -o - -O0 -S %s 2>&1 | FileCheck %s -check-prefix NOWARN -implicit-check-not warning
// NOWARN: .text

// Check that llc defaults to .init_array:
// RUN: %cheri_purecap_cc1 -o %t.ll -O0 -emit-llvm %s
// RUN: %cheri_purecap_llc -o - -O0 %t.ll | FileCheck %s -implicit-check-not .ctors -implicit-check-not .dtors

// CHECK:      .section	.init_array,"aw",@init_array
// CHECK-NEXT: .p2align	3
// CHECK-NEXT: .8byte	ctor_fn
// CHECK-NEXT: .section	.fini_array,"aw",@fini_array
// CHECK-NEXT: .p2align	3
// CHECK-NEXT: .8byte	dtor_fn

__attribute((constructor))
int ctor_fn(void) {
  return 0;
}

__attribute((destructor))
int dtor_fn(void) {
  return 0;
}
