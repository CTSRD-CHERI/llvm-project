// RUN: %cheri_purecap_cc1 -emit-llvm -o - -no-cheri-linker %s | FileCheck %s
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
