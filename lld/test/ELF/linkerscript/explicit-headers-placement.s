# REQUIRES: x86
# RUN: split-file %s %t
# RUN: llvm-mc -filetype=obj -triple=x86_64 %t/main.s -o %t/main.o
# RUN: ld.lld -T %t/start.lds %t/main.o -o %t/start
# RUN: llvm-readelf -S -l -s %t/start | FileCheck %s --check-prefix=START
# RUN: ld.lld -T %t/start-merge.lds %t/main.o -o %t/start-merge
# RUN: llvm-readelf -S -l -s %t/start-merge | FileCheck %s --check-prefix=START-MERGE
# RUN: ld.lld -T %t/middle.lds %t/main.o -o %t/middle
# RUN: llvm-readelf -S -l -s %t/middle | FileCheck %s --check-prefix=MIDDLE
# RUN: ld.lld -T %t/middle-noalign.lds %t/main.o -o %t/middle-noalign
# RUN: llvm-readelf -S -l -s %t/middle-noalign | FileCheck %s --check-prefix=MIDDLE-NOALIGN
# RUN: ld.lld -T %t/end.lds %t/main.o -o %t/end
# RUN: llvm-readelf -S -l -s %t/end | FileCheck %s --check-prefix=END
# RUN: ld.lld -T %t/lma.lds %t/main.o -o %t/lma
# RUN: llvm-readelf -S -l -s %t/lma | FileCheck %s --check-prefix=LMA

# START-LABEL: Section Headers:
# START-NEXT:    [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# START-NEXT:    [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
# START-NEXT:    [ 1] .text             PROGBITS        0000000000002158 000158 000001 00  AX  0   0  4
# START-NEXT:    [ 2] .rodata           PROGBITS        0000000000003159 000159 000008 00   A  0   0  1
# START-LABEL: Program Headers:
# START-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# START-NEXT:    PHDR           0x000040 0x0000000000001040 0x0000000000001040 0x000118 0x000118 R   0x8
# START-NEXT:    LOAD           0x000000 0x0000000000001000 0x0000000000001000 0x000158 0x000158 R   0x1000
# START-NEXT:    LOAD           0x000158 0x0000000000002158 0x0000000000002158 0x000001 0x000001 R E 0x1000
# START-NEXT:    LOAD           0x000159 0x0000000000003159 0x0000000000003159 0x000008 0x000008 R   0x1000
## Headers are first so symbols referencing them get a dummy 1 index
# START:       0000000000001000     0 NOTYPE  LOCAL  HIDDEN      1 __ehdr_start
# START:       0000000000001000     0 NOTYPE  GLOBAL DEFAULT     1 my_ehdr
# START:       0000000000003161     0 NOTYPE  GLOBAL DEFAULT     2 my_end

# START-MERGE-LABEL: Section Headers:
# START-MERGE-NEXT:    [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# START-MERGE-NEXT:    [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
# START-MERGE-NEXT:    [ 1] .rodata           PROGBITS        0000000000002120 001120 000008 00   A  0   0  1
# START-MERGE-NEXT:    [ 2] .text             PROGBITS        0000000000003128 001128 000001 00  AX  0   0  4
## We can use a single PT_LOAD if the headers are first
# START-MERGE-LABEL: Program Headers:
# START-MERGE-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# START-MERGE-NEXT:    PHDR           0x000040 0x0000000000001040 0x0000000000001040 0x0000e0 0x0000e0 R   0x8
# START-MERGE-NEXT:    LOAD           0x000000 0x0000000000001000 0x0000000000001000 0x001128 0x001128 R   0x1000
# START-MERGE-NEXT:    LOAD           0x001128 0x0000000000003128 0x0000000000003128 0x000001 0x000001 R E 0x1000
## Headers are first so symbols referencing them get a dummy 1 index
# START-MERGE:       0000000000001000     0 NOTYPE  LOCAL  HIDDEN      1 __ehdr_start
# START-MERGE:       0000000000001000     0 NOTYPE  GLOBAL DEFAULT     1 my_ehdr
# START-MERGE:       0000000000003129     0 NOTYPE  GLOBAL DEFAULT     2 my_end

# MIDDLE-LABEL: Section Headers:
# MIDDLE-NEXT:    [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# MIDDLE-NEXT:    [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
# MIDDLE-NEXT:    [ 1] .text             PROGBITS        0000000000001000 001000 000001 00  AX  0   0  4
# MIDDLE-NEXT:    [ 2] .rodata           PROGBITS        0000000000003158 001158 000008 00   A  0   0  1
# MIDDLE-LABEL: Program Headers:
# MIDDLE-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# MIDDLE-NEXT:    PHDR           0x000040 0x0000000000002040 0x0000000000002040 0x000118 0x000118 R   0x8
# MIDDLE-NEXT:    LOAD           0x001000 0x0000000000001000 0x0000000000001000 0x000001 0x000001 R E 0x1000
# MIDDLE-NEXT:    LOAD           0x000000 0x0000000000002000 0x0000000000002000 0x000158 0x000158 R   0x1000
# MIDDLE-NEXT:    LOAD           0x001158 0x0000000000003158 0x0000000000003158 0x000008 0x000008 R   0x1000
## Headers are after .text so symbols referencing them get .text's index
# MIDDLE:       0000000000002000     0 NOTYPE  LOCAL  HIDDEN      1 __ehdr_start
# MIDDLE:       0000000000002000     0 NOTYPE  GLOBAL DEFAULT     1 my_ehdr
# MIDDLE:       0000000000003160     0 NOTYPE  GLOBAL DEFAULT     2 my_end

# MIDDLE-NOALIGN-LABEL: Section Headers:
# MIDDLE-NOALIGN-NEXT:    [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# MIDDLE-NOALIGN-NEXT:    [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
# MIDDLE-NOALIGN-NEXT:    [ 1] .text             PROGBITS        0000000000001000 001000 000001 00  AX  0   0  4
# MIDDLE-NOALIGN-NEXT:    [ 2] .rodata           PROGBITS        0000000000002158 001158 000008 00   A  0   0  1
# MIDDLE-NOALIGN-LABEL: Program Headers:
# MIDDLE-NOALIGN-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# MIDDLE-NOALIGN-NEXT:    PHDR           0x000040 0x0000000000002040 0x0000000000002040 0x000118 0x000118 R   0x8
# MIDDLE-NOALIGN-NEXT:    LOAD           0x001000 0x0000000000001000 0x0000000000001000 0x000001 0x000001 R E 0x1000
# MIDDLE-NOALIGN-NEXT:    LOAD           0x000000 0x0000000000002000 0x0000000000002000 0x000158 0x000158 R   0x1000
# MIDDLE-NOALIGN-NEXT:    LOAD           0x001158 0x0000000000002158 0x0000000000002158 0x000008 0x000008 R   0x1000
## Headers are after .text so symbols referencing them get .text's index
# MIDDLE-NOALIGN:       0000000000002000     0 NOTYPE  LOCAL  HIDDEN      1 __ehdr_start
# MIDDLE-NOALIGN:       0000000000002000     0 NOTYPE  GLOBAL DEFAULT     1 my_ehdr
# MIDDLE-NOALIGN:       0000000000002160     0 NOTYPE  GLOBAL DEFAULT     2 my_end

# END-LABEL: Section Headers:
# END-NEXT:    [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# END-NEXT:    [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
# END-NEXT:    [ 1] .text             PROGBITS        0000000000001000 001000 000001 00  AX  0   0  4
# END-NEXT:    [ 2] .rodata           PROGBITS        0000000000002001 001001 000008 00   A  0   0  1
# END-LABEL: Program Headers:
# END-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# END-NEXT:    PHDR           0x000040 0x0000000000003040 0x0000000000003040 0x000118 0x000118 R   0x8
# END-NEXT:    LOAD           0x001000 0x0000000000001000 0x0000000000001000 0x000001 0x000001 R E 0x1000
# END-NEXT:    LOAD           0x001001 0x0000000000002001 0x0000000000002001 0x000008 0x000008 R   0x1000
# END-NEXT:    LOAD           0x000000 0x0000000000003000 0x0000000000003000 0x000158 0x000158 R   0x1000
## Headers are after .rodata so symbols referencing them get .rodata's index
# END:       0000000000003000     0 NOTYPE  LOCAL  HIDDEN      2 __ehdr_start
# END:       0000000000003000     0 NOTYPE  GLOBAL DEFAULT     2 my_ehdr
# END:       0000000000003158     0 NOTYPE  GLOBAL DEFAULT     2 my_end

# LMA-LABEL: Section Headers:
# LMA-NEXT:    [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
# LMA-NEXT:    [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
# LMA-NEXT:    [ 1] .text             PROGBITS        ffffffff80000000 001000 000001 00  AX  0   0  4
# LMA-NEXT:    [ 2] .rodata           PROGBITS        ffffffff80001001 001001 000008 00   A  0   0  1
# LMA-LABEL: Program Headers:
# LMA-NEXT:    Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
# LMA-NEXT:    PHDR           0x000040 0xffffffff80002040 0x0000000000003040 0x000118 0x000118 R   0x8
# LMA-NEXT:    LOAD           0x001000 0xffffffff80000000 0x0000000000001000 0x000001 0x000001 R E 0x1000
# LMA-NEXT:    LOAD           0x001001 0xffffffff80001001 0x0000000000002001 0x000008 0x000008 R   0x1000
# LMA-NEXT:    LOAD           0x000000 0xffffffff80002000 0x0000000000003000 0x000158 0x000158 R   0x1000
## Headers are after .rodata so symbols referencing them get .rodata's index
# LMA:       ffffffff80002000     0 NOTYPE  LOCAL  HIDDEN      2 __ehdr_start
# LMA:       ffffffff80002000     0 NOTYPE  GLOBAL DEFAULT     2 my_ehdr
# LMA:       ffffffff80002158     0 NOTYPE  GLOBAL DEFAULT     2 my_end

#--- main.s
.type _start, @function
.globl _start
_start:
  ret

.rodata
.quad __ehdr_start

## Alignments in the linker script sources are done to match the default
## behaviour so no on-disk padding is needed to align the offsets, except for
## middle-noalign.
##
## middle-noalign fails if the headers and .rodata are put into the same
## segment since it would try to overlap file offsets, so we verify that
## doesn't happen.

#--- start.lds
SECTIONS {
  . = 0x1000;
  HEADERS : { my_ehdr = .; }
  . = ALIGN(0x1000) + . % 0x1000;
  .text : { *(.text) }
  . = ALIGN(0x1000) + . % 0x1000;
  .rodata : { *(.rodata) }
  . = .;
  my_end = .;
}

#--- start-merge.lds
SECTIONS {
  . = 0x1000;
  HEADERS : { my_ehdr = .; }
  . = ALIGN(0x1000) + . % 0x1000;
  .rodata : { *(.rodata) }
  . = ALIGN(0x1000) + . % 0x1000;
  .text : { *(.text) }
  . = .;
  my_end = .;
}

#--- middle.lds
SECTIONS {
  . = 0x1000;
  .text : { *(.text) }
  HEADERS : { my_ehdr = .; }
  . = ALIGN(0x1000) + . % 0x1000;
  .rodata : { *(.rodata) }
  . = .;
  my_end = .;
}

#--- middle-noalign.lds
SECTIONS {
  . = 0x1000;
  .text : { *(.text) }
  HEADERS : { my_ehdr = .; }
  .rodata : { *(.rodata) }
  . = .;
  my_end = .;
}

#--- end.lds
SECTIONS {
  . = 0x1000;
  .text : { *(.text) }
  . = ALIGN(0x1000) + . % 0x1000;
  .rodata : { *(.rodata) }
  HEADERS : { my_ehdr = .; }
  . = .;
  my_end = .;
}

#--- lma.lds
SECTIONS {
  . = 0xffffffff80000000;
  .text : AT(0x1000) { *(.text) }
  . = ALIGN(0x1000) + . % 0x1000;
  .rodata : { *(.rodata) }
  HEADERS : { my_ehdr = .; }
  . = .;
  my_end = .;
}
