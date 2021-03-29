// REQUIRES: mips-registered-target
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s -implicit-check-not=__cxx_global_var_init
// RUN: %cheri_purecap_cc1 -S -o - %s
// | FileCheck %s -check-prefix ASM

// CHECK:      @.str = private unnamed_addr addrspace(200) constant [5 x i8] c"gzip\00", align 1
// CHECK-NEXT: @.str.1 = private unnamed_addr addrspace(200) constant [5 x i8] c"-cdq\00", align 1
// CHECK-NEXT: @compr = dso_local addrspace(200) constant [1 x { [8 x i8], i64, [3 x i8 addrspace(200)*], i32 }] [{ [8 x i8], i64, [3 x i8 addrspace(200)*], i32 }
// CHECK-SAME: { [8 x i8] c"\1F\9D\00\00\00\00\00\00", i64 2,
// CHECK-SAME: [3 x i8 addrspace(200)*] [i8 addrspace(200)* getelementptr inbounds ([5 x i8], [5 x i8] addrspace(200)* @.str, i32 0, i32 0),
// CHECK-SAME: i8 addrspace(200)* getelementptr inbounds ([5 x i8], [5 x i8] addrspace(200)* @.str.1, i32 0, i32 0),
// CHECK-SAME: i8 addrspace(200)* null], i32 1 }], align 16

#define NULL (void*)0
const struct {
	const char magic[8];
	__SIZE_TYPE__ maglen;
	const char *argv[3];
	int silent;
} compr[] = {
	{ "\037\235", 2, { "gzip", "-cdq", NULL }, 1 },		/* compressed */
};

// Check that we emit the correct init_array loayout (a single .8byte value)
// ASM-LABEL: compr:
// ASM-NEXT: .asciz	"\037\235\000\000\000\000\000"
// ASM-NEXT: .8byte	2                       # 0x2
// ASM-NEXT: .chericap	.L.str
// ASM-NEXT: .chericap	.L.str.1
// ASM-NEXT: .chericap	0
// ASM-NEXT: .4byte	1                       # 0x1
// ASM-NEXT: .space	12
// ASM-NEXT: .size	compr, 80
