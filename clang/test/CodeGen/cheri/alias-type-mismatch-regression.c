// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 -std=gnu99 -o - -O0 -emit-llvm %s | FileCheck %s -check-prefixes CHECK
// RUN: %cheri_purecap_cc1 -std=gnu99 -o - -O0 -S %s | FileCheck %s -check-prefix ASM
// RUN: %cheri_purecap_cc1 -std=gnu99 -o - -O0 -emit-obj %s | llvm-objdump -t -d - | FileCheck %s -check-prefix DUMP
void __ledf2() {}
void a(void) __attribute__((__alias__("__ledf2")));

// CHECK: @a = alias void (), void () addrspace(200)* @__ledf2
// CHECK: define void @__ledf2()

// ASM:      .globl	a
// ASM-NEXT: .type	a,@function
// ASM-NEXT: .set a, __ledf2

// DUMP: 0000000000000000 g     F .text		 0000000000000008 __ledf2
// DUMP: 0000000000000000 g     F .text		 0000000000000008 a
