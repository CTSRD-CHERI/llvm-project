// RUN: %cheri_purecap_cc1 -emit-obj -fno-jump-tables -O2 -std=gnu99 -mllvm -cheri-cap-table-abi=plt -x c -o - %s -emit-llvm | FileCheck %s
// RUN: %cheri_purecap_cc1 -S -fno-jump-tables -O2 -std=gnu99 -mllvm -cheri-cap-table-abi=plt -x c -o - %s | FileCheck %s -check-prefix ASM

extern int a(void (*fn)(void));

void snprintf_func(void) {
  a(snprintf_func);
  // CHECK-LABEL: define void @snprintf_func()
  // CHECK:       %call = tail call i32 @a(void () addrspace(200)* nonnull @snprintf_func) #2
}

// ASM:      clcbi	$c3, %capcall20(snprintf_func)($c26)
// ASM-NEXT: clcbi $c12, %capcall20(a)($c26)
// ASM-NEXT: cjalr	$c12, $c17
// ASM-NEXT: nop

// Checking the asm output should be done in LLVM but the IR will probably change

