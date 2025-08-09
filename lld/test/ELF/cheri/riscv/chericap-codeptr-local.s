# REQUIRES: riscv
# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -shared -cheri-codeptr-relocs %t.o -o %t.so
# RUN: llvm-readobj --relocs --cap-relocs %t.so | FileCheck %s

 .text
 .type func, %function
 .size func, 4
func:
 ret

 .data
 .chericap func
 .chericap func@code

# CHECK:      Relocations [
# CHECK-NEXT: ]
# CHECK-NEXT: CHERI __cap_relocs [
# CHECK-NEXT:   0x003330 Base: 0x1298 (func+0) Length: 4 Perms: Function
# CHECK-NEXT:   0x003340 Base: 0x1298 (func+0) Length: 4 Perms: Code
# CHECK-NEXT: ]
