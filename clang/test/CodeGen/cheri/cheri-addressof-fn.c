// REQUIRES: mips-registered-target
// RUN: %cheri_purecap_cc1 %s -emit-llvm -o - | FileCheck %s
// RUN: %cheri_purecap_cc1 %s -S -o - | FileCheck %s -check-prefix ASM
typedef void (*foo)(void);
void fn(void);

foo f = fn;
foo g = &fn;
// CHECK: @f = addrspace(200) global void () addrspace(200)* @fn, align 16
// CHECK: @g = addrspace(200) global void () addrspace(200)* @fn, align 16

// ASM-LABEL: f:
// ASM-NEXT: .chericap	fn
// ASM-NEXT: .size	f, 16
// ASM-LABEL: g:
// ASM-NEXT: .chericap	fn
// ASM-NEXT: .size	g, 16
