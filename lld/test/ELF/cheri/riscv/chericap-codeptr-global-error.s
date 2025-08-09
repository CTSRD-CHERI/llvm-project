# REQUIRES: riscv
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: not ld.lld -shared -cheri-codeptr-relocs %t.o -o %t.so 2>&1 | FileCheck %s

 .text
 .globl func
 .type func, %function
 .size func, 4
func:
 ret

 .data
ptr:
 .chericap func@code

# CHECK: error: Cannot relocate code capability to preemptible symbol: func
