# REQUIRES: riscv

## If PCC's representable alignment will exceed page alignment, ensure that the
## section, PT_LOAD and PT_CHERI_PCC all follow this alignment. In particular,
## the PT_LOAD's p_offset must be congruent to its p_vaddr, which will be the
## first section's sh_offset and sh_addr respectively, and since the latter
## must be suitably aligned, so must all the others.

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -Ttext=0x1000 %t.o -o %t
# RUN: llvm-readelf -S -l %t | FileCheck %s

# CHECK-LABEL: Section Headers:
# CHECK-NEXT:    [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# CHECK-NEXT:    [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
# CHECK-NEXT:    [ 1] .text             PROGBITS        0000000000002000 002000 000008 00  AX  0   0 8192
# CHECK-NEXT:    [ 2] .rodata           PROGBITS        0000000000003008 002008 400000 00   A  0   0  1
# CHECK-NEXT:    [ 3] .pad.cheri.pcc    PROGBITS        0000000000403008 402008 000ff8 00   A  0   0  1

# CHECK-LABEL: Program Headers:
# CHECK-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# CHECK-NEXT:    PHDR           0x000040 0x0000000000010040 0x0000000000010040 0x000150 0x000150 R   0x8
# CHECK-NEXT:    LOAD           0x000000 0x0000000000010000 0x0000000000010000 0x000190 0x000190 R   0x1000
# CHECK-NEXT:    LOAD           0x002000 0x0000000000002000 0x0000000000002000 0x000008 0x000008 R E 0x2000
# CHECK-NEXT:    LOAD           0x002008 0x0000000000003008 0x0000000000003008 0x400ff8 0x400ff8 R   0x1000
# CHECK-NEXT:    CHERI_PCC      0x002000 0x0000000000002000 0x0000000000002000 0x401000 0x402000 R E 0x2000

cllc ct0, foo

.rodata
foo:
.space 4194304
