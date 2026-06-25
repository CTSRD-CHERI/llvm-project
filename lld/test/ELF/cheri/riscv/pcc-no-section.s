# REQUIRES: riscv

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld %t.o --defsym abs=0x12000 -o %t
# RUN: llvm-readelf -S %t | FileCheck %s --check-prefix=SEC
# RUN: llvm-readelf -l %t | FileCheck %s --check-prefix=SEG
# RUN: llvm-objdump -d --no-show-raw-insn %t | FileCheck %s --check-prefix=DIS

## PCC-relative relocations against symbols not in a section
## (such as absolute symbols) should be permitted like those
## against symbols in non-PCC sections, but similarly won't
## affect the bounds.

# SEC: [ 1] .text             PROGBITS        0000000000011158 000158 000008 00  AX  0   0  4
# SEG: CHERI_PCC      0x000158 0x0000000000011158 0x0000000000011158 0x000008 0x000008 R E 0x4

## 0x12000 - 0x11158 = 1*4096 - 344
# DIS:      11158: auipcc ca0, 1
# DIS-NEXT:        cincoffset ca0, ca0, -344

cllc ca0, abs
