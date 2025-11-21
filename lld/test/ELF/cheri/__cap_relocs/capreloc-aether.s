# RUN: %cheri_purecap_llvm-mc %s -filetype=obj -o %t.o
# RUN: ld.lld -T %S/Inputs/capreloc-aether.script %t.o -o %t
# RUN: llvm-readobj --cap-relocs %t | FileCheck %s

.data
.chericap data_begin
.balign 32
.chericap data_end

# CHECK:      __cap_relocs {
# CHECK-NEXT:   0x100A0 RODATA - 0x10000 [0x10000-0x10000]
# CHECK-NEXT:   0x100C0 DATA - 0x20000 [0x20000-0x20000]
# CHECK-NEXT: }
