# RUN: %cheri_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUN: ld.lld -T %S/Inputs/capreloc-aether.script %t.o -o %t
# RUN: llvm-readobj --cap-relocs %t | FileCheck %s

.data
.chericap data_begin
.balign 32
.chericap data_end

# CHECK:      CHERI __cap_relocs [
# CHECK-NEXT:   0x0100a0 Base: 0x10000 (data_begin+0) Length: 0 Perms: Constant
# CHECK-NEXT:   0x0100c0 Base: 0x20000 (data_end+0) Length: 0 Perms: Object
# CHECK-NEXT: ]
