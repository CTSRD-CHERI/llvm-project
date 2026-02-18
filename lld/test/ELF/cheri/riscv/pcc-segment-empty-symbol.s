# REQUIRES: riscv

## We don't emit a PT_CHERI_PCC when .text is empty, but if there are symbols
## defined within it then we could have relocations referencing them, and
## should give sensible (i.e. not base+length 0) bounds, so treat that case as
## if .text is non-empty.
##
## This is a degenerate case that should not show up in the real world (there's
## no code to jump to for the given symbol), but does in some Morello LLD
## tests.

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld %t.o -o %t
# RUN: llvm-readelf -S -l --cap-relocs %t | FileCheck %s

  .type emptyfunc, @function
emptyfunc:
  .size emptyfunc, . - emptyfunc

  .data
  .type fptr, @object
fptr:
  .chericap emptyfunc
  .size fptr, . - fptr

# CHECK-LABEL: Section Headers:
# CHECK-NEXT:    [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# CHECK-NEXT:    [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
# CHECK-NEXT:    [ 1] __cap_relocs      PROGBITS        0000000000010190 000190 000028 28   A  0   0  8
# CHECK-NEXT:    [ 2] .text             PROGBITS        00000000000111b8 0001b8 000000 00  AX  0   0  4
# CHECK-NEXT:    [ 3] .pad.cheri.pcc    PROGBITS        00000000000111b8 0001b8 000000 00  AX  0   0  1


# CHECK-LABEL: Program Headers:
# CHECK-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# CHECK-NEXT:    PHDR           0x000040 0x0000000000010040 0x0000000000010040 0x000150 0x000150 R   0x8
# CHECK-NEXT:    LOAD           0x000000 0x0000000000010000 0x0000000000010000 0x0001b8 0x0001b8 R   0x1000
# CHECK-NEXT:    LOAD           0x0001c0 0x00000000000121c0 0x00000000000121c0 0x000010 0x000010 RW  0x1000
# CHECK-NEXT:    CHERI_PCC      0x0001b8 0x00000000000111b8 0x00000000000111b8 0x000000 0x000000 R E 0x4

# CHECK-LABEL: CHERI capability relocation section '__cap_relocs' at offset {{.*}} contains 1 entries:
# CHECK-NEXT:      Offset             Info         Type        Value
# CHECK-NEXT:  00000000000121c0  8000000000000000 FUNC    00000000000111b8 [00000000000111b8-00000000000111b8]
