// RUN: %clang_cc1 "-cc1" "-triple" "cheri-unknown-freebsd" "-emit-llvm" "-target-abi" "sandbox" "-o" "-" %s | FileCheck %s
// CHECK: __cxx_global_var_init
// Check that this generates an initialiser
#define NULL (void*)0
const struct {
	const char magic[8];
	__SIZE_TYPE__ maglen;
	const char *argv[3];
	int silent;
} compr[] = {
	{ "\037\235", 2, { "gzip", "-cdq", NULL }, 1 },		/* compressed */
};
