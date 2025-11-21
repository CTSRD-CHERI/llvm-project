# Check that we create an error on an out-of-bounds R_MIPS_CALL_16

# REQUIRES: mips
# RUN: %cheri128_llvm-mc -filetype=obj %s -o %t.o
# RUN: %cheri128_llvm-mc -filetype=obj -defsym=TOO_MANY_SMALL_RELOCS=1 %s -o %t-bad.o
# RUN: ld.lld %t.o -o %t.exe
# RUN: llvm-objdump --cap-relocs -D -t %t.exe | FileCheck %s --check-prefix EXE
# check that symbols with small immediates come first:
# EXE-LABEL: SYMBOL TABLE
# EXE: [[#%.16x,MXCTB000_CTAB:]] l     O .captable 0000000000000010 sym_mxcaptable000@CAPTABLE
# EXE: [[#%.16x,SMALL000_CTAB:]] l     O .captable 0000000000000010 sym_small000@CAPTABLE
# EXE: [[#%.16x,MXCTB000_ADDR:]] g     O .data     0000000000000001 sym_mxcaptable000
# EXE: [[#%.16x,SMALL000_ADDR:]] g     O .data     0000000000000001 sym_small000
# EXE-LABEL: CAPABILITY RELOCATION RECORDS:
# EXE-NEXT: OFFSET           TYPE    VALUE
# EXE-NEXT: [[#SMALL000_CTAB]] DATA    [[#SMALL000_ADDR]] {{\[}}[[#SMALL000_ADDR]]-[[#SMALL000_ADDR+1]]]
# EXE:      [[#MXCTB000_CTAB]] DATA    [[#MXCTB000_ADDR]] {{\[}}[[#MXCTB000_ADDR]]-[[#MXCTB000_ADDR+1]]]
# EXE-LABEL: Disassembly of section .captable:
# EXE-EMPTY:
# EXE-NEXT: <sym_small000@CAPTABLE>:

# But if there are too many small relocs there is nothing we can do
# RUN: not ld.lld %t-bad.o -o %t.exe 2>&1 | FileCheck %s -check-prefix ERR
# ERR: (function load_sym_small_out_of_bounds024: .text+0x2f40): relocation R_MIPS_CHERI_CAPTAB_CLC11 out of range: 1024 is not in [-1024, 1023]; references 'sym_small_out_of_bounds024'

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
          load_sym_\prefix\j\k\l:
          clc $c1, $zero, %captab(sym_\prefix\j\k\l)($c26)
          .size load_sym_\prefix\j\k\l, . - load_sym_\prefix\j\k\l
          .type load_sym_\prefix\j\k\l,@function
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
          load_sym_\prefix\j\k\l:
          .set noat
          lui $1, %captab_hi(sym_\prefix\j\k\l)
          daddiu $1, $1, %captab_lo(sym_\prefix\j\k\l)
          .size load_sym_\prefix\j\k\l, . - load_sym_\prefix\j\k\l
          .type load_sym_\prefix\j\k\l,@function
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
