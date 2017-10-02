// This code generates invalid IR:
// Incorrect number of arguments passed to called function!
//  %call = call i32 @a(i32 signext 3)
// fatal error: error in backend: Broken function found, compilation aborted!

// XFAIL: *
// RUN: %cheri_purecap_cc1 -emit-llvm -mrelax-all -mrelocation-model pic -pic-level 2 -mthread-model posix -mllvm -mips-ssection-threshold=0 -sys-header-deps -D NDEBUG -ftls-model=initial-exec -cheri-linker -x c -o - %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -emit-obj -mrelax-all -mrelocation-model pic -pic-level 2 -mthread-model posix -mllvm -mips-ssection-threshold=0 -sys-header-deps -D NDEBUG -ftls-model=initial-exec -cheri-linker -x c -o /dev/null %s
int a() {
	a(3); // CHECK-NOT: call i32 @a(i32 signext 3)
}

