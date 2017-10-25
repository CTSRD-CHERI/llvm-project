// RUN: %cheri_purecap_cc1 -emit-obj -fno-jump-tables -O2 -std=gnu99 -mllvm -cheri-cap-table -x c -o - %s -emit-llvm | FileCheck %s
// RUN: %cheri_purecap_cc1 -S -fno-jump-tables -O2 -std=gnu99 -mllvm -cheri-cap-table -x c -o - %s | FileCheck %s -check-prefix ASM

extern int a(void (*fn)(void));

void snprintf_func(void) {
  a(snprintf_func);
    // CHECK-LABEL: define void @snprintf_func()
    // CHECK:       %call = tail call i32 @a(void () addrspace(200)* addrspacecast (void ()* @snprintf_func to void () addrspace(200)*)) #2
}

// ASM:      lui	$1, %capcall_hi(snprintf_func)
// ASM-NEXT: daddiu	$1, $1, %capcall_lo(snprintf_func)
// ASM-NEXT: clc	$c3, $1, 0($c26)
// ASM-NEXT: lui	$1, %capcall_hi(a)
// ASM-NEXT: daddiu	$1, $1, %capcall_lo(a)
// ASM-NEXT: clc	$c12, $1, 0($c26)
// ASM-NEXT: cjalr	$c12, $c17
// ASM-NEXT: cfromptr	$c13, $c0, $zero

// Checking the asm output should be done in LLVM but the IR will probably change

