# Check that we create an error on an out-of-bounds R_MIPS_CALL_16

# REQUIRES: mips
# RUN: %cheri128_llvm-mc -filetype=obj %s -o %t.o
# RUN: %cheri128_llvm-mc -filetype=obj -defsym=TOO_MANY_SMALL_RELOCS=1 %s -o %t-bad.o
# RUN: ld.lld %t.o -o %t.exe
# RUN: llvm-objdump -C -t -d -D %t.exe | FileCheck %s -check-prefix EXE
# check that symbols with small immediates come first:
# EXE-LABEL: Disassembly of section .cap_table:
# EXE-NEXT: sym_small000@CAPTABLE:
# EXE-LABEL: CAPABILITY RELOCATION RECORDS:
# EXE-NEXT: 0x0000000000050000	Base: sym_small000 (0x00000000000403e8)	Offset: 0x0000000000000000	Length: 0x0000000000000001	Permissions: 0x00000000
# EXE:      0x0000000000053e80	Base: sym_mxcaptable000 (0x0000000000040000)	Offset: 0x0000000000000000	Length: 0x0000000000000001	Permissions: 0x00000000
# EXE-LABEL: SYMBOL TABLE
# EXE: 0000000000053e80 l       .cap_table		 00000010 sym_mxcaptable000@CAPTABLE
# EXE: 0000000000050000 l       .cap_table		 00000010 sym_small000@CAPTABLE


# But if there are too many small relocs there is nothing we can do
# RUN: not ld.lld %t-bad.o -o %t.exe 2>&1 | FileCheck %s -check-prefix ERR
# ERR: (.text+0x2F40): relocation R_MIPS_CHERI_CAPTAB_CLC11 out of range: 1024 is not in [-1024, 1023]

.macro generate_1000_captable_values, prefix=0
    .irp j, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      .irp k, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
        .irp l, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
          .data
          .globl sym_\prefix\j\k\l
          sym_\prefix\j\k\l:
          .byte 1
          .size sym_\prefix\j\k\l, 1
          .type sym_\prefix\j\k\l,@object

          .text
          clc $c1, $zero, %captab(sym_\prefix\j\k\l)($c26)
        .endr
      .endr
    .endr
.endm

.macro generate_1000_mxcaptable_values, prefix=mxcaptable
    .irp j, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      .irp k, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
        .irp l, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
          .data
          .globl sym_\prefix\j\k\l
          sym_\prefix\j\k\l:
          .byte 1
          .size sym_\prefix\j\k\l, 1
          .type sym_\prefix\j\k\l,@object

          .text
          .set noat
          lui $1, %captab_hi(sym_\prefix\j\k\l)
          daddiu $1, $1, %captab_lo(sym_\prefix\j\k\l)
        .endr
      .endr
    .endr
.endm

generate_1000_mxcaptable_values

generate_1000_captable_values small
.ifdef TOO_MANY_SMALL_RELOCS
generate_1000_captable_values small_out_of_bounds
.endif

# This would be nicer but won't work due to recursion limit

# .macro  generate to=5, index=0
# 
# .data
# .globl sym_\index
# sym_\index:
# .long \index
# .size sym_\index, 8
# .type sym_\index,@object
# 
# .text
# clc $c1, $zero, %captab(sym_\index)($c26)
# 
# 
# # recurse to get a loop:
# .if     \to-\index
# generate     \to, \index+1
# .endif
# .endm
# 
# generate 2048

.globl __start
__start:
  nop
