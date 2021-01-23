# Check that we create an error on an out-of-bounds R_MIPS_CALL_16

# REQUIRES: mips
# RUN: %cheri_llvm-mc -filetype=obj %s -o %t.o
# RUN: llvm-objdump -d -r -t %t.o > /dev/null
# RUN: not ld.lld %t.o -o %t.exe 2>&1 | FileCheck %s -check-prefixes CHECK

# TODO: 2000 symbols should be okay once we point to the middle of the captable
# CHECK:      out-of-bounds-captable-reloc.s.tmp.o:(.text+0x{{.+}}): relocation R_MIPS_CHERI_CAPTAB_CLC11 out of range: 1024 is not in [-1024, 1023]
# CHECK-NEXT: >>> defined in
# CHECK-EMPTY:
# For CHERI256 we have to skip every second entry so it can is 1026 instead of 1025
# CHECK-NEXT: out-of-bounds-captable-reloc.s.tmp.o:(.text+0x{{.+}}): relocation R_MIPS_CHERI_CAPTAB_CLC11 out of range: 1025 is not in [-1024, 1023]
# CHECK-NEXT: >>> defined in
# CHECK-EMPTY:
# CHECK:      error: too many errors emitted, stopping now (use -error-limit=0 to see all errors)

.macro generate_values
  .irp i, 0, 1
    .irp j, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      .irp k, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
        .irp l, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
          .data
          .globl sym_\i\j\k\l
          sym_\i\j\k\l:
          .byte 1
          .size sym_\i\j\k\l, 1
          .type sym_\i\j\k\l,@object

          .text
          clc $c1, $zero, %captab(sym_\i\j\k\l)($c26)
        .endr
      .endr
    .endr
  .endr
.endm

generate_values

.globl __start
__start:
  nop
