// REQUIRES: mips-registered-target

// RUN: %cheri_purecap_cc1 -emit-obj -fno-jump-tables -O2 -std=gnu99 -mllvm -cheri-cap-table-abi=plt -x c -o - %s -emit-llvm | FileCheck %s
// RUN: %cheri_purecap_cc1 -S -fno-jump-tables -O2 -std=gnu99 -mllvm -cheri-cap-table-abi=plt -x c -o - %s | FileCheck %s -check-prefix ASM

extern int a(void (*fn)(void));

void snprintf_func(void) {
  a(snprintf_func);
  // CHECK-LABEL: define dso_local void @snprintf_func()
  // CHECK:       %call = tail call signext i32 @a(void () addrspace(200)* nonnull @snprintf_func) #2
}

// This should use captab20 for loading the address of snprintf_func
// ASM:      clcbi	$c3, %captab20(snprintf_func)($c18)
// ASM-NEXT: clcbi $c12, %capcall20(a)($c18)
// ASM-NEXT: cjalr	$c12, $c17
// ASM-NEXT: nop


