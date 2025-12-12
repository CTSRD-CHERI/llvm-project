# REQUIRES: riscv

## Ensure that we allow would-be-relro sections to be included in PCC bounds
## even if -z norelro is passed, making them not relro in reality.

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -z separate-code -z norelro %t.o -o %t
# RUN: llvm-readelf -S -l %t | FileCheck %s

# CHECK-LABEL: Section Headers:
# CHECK-NEXT:    [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# CHECK-NEXT:    [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
# CHECK-NEXT:    [ 1] .text             PROGBITS        0000000000011000 001000 000008 00  AX  0   0  8
# CHECK-NEXT:    [ 2] .data.rel.ro      PROGBITS        0000000000012000 002000 000001 00  WA  0   0  1
# CHECK-NEXT:    [ 3] .pad.cheri.pcc    PROGBITS        0000000000012001 002001 000007 00  WA  0   0  1

# CHECK-LABEL: Program Headers:
# CHECK-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# CHECK-NEXT:    PHDR           0x000040 0x0000000000010040 0x0000000000010040 0x000150 0x000150 R   0x8
# CHECK-NEXT:    LOAD           0x000000 0x0000000000010000 0x0000000000010000 0x000190 0x000190 R   0x1000
# CHECK-NEXT:    LOAD           0x001000 0x0000000000011000 0x0000000000011000 0x000008 0x000008 R E 0x1000
# CHECK-NEXT:    LOAD           0x002000 0x0000000000012000 0x0000000000012000 0x000008 0x000008 RW  0x1000
# CHECK-NEXT:    CHERI_PCC      0x001000 0x0000000000011000 0x0000000000011000 0x001008 0x001008 R E 0x8

cllc ct0, foo

.data.rel.ro
foo:
.space 1
