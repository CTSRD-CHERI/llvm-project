# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
# RUN: %cheri_llvm-mc %s -filetype=obj -o - | llvm-objdump -d -r - | FileCheck %s -check-prefix DUMP
#
# Check that the assembler is able to handle the new capability table relocations.
#
  .text
	.set	noat
	lui	$1, %captab_hi(foo)
# CHECK:      encoding: [0x3c,0x01,A,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %captab_hi(foo), kind: fixup_CHERI_CAPTABLE_HI16
	ori	$1, $1, %captab_lo(foo)
# CHECK:      encoding: [0x34,0x21,A,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %captab_lo(foo), kind: fixup_CHERI_CAPTABLE_LO16

	lui	$1, %capcall_hi(foo)
# CHECK:      encoding: [0x3c,0x01,A,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %capcall_hi(foo), kind: fixup_CHERI_CAPCALL_HI16
	ori	$1, $1, %capcall_lo(foo)
# CHECK:      encoding: [0x34,0x21,A,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %capcall_lo(foo), kind: fixup_CHERI_CAPCALL_LO16

	clc	$c1, $zero, %captab(foo)($c3)
# CHECK:      encoding: [0xd8,0x23,0b00000AAA,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %captab(foo), kind: fixup_CHERI_CAPTABLE11
	clc	$c1, $zero, %capcall(foo)($c3)
# CHECK:      encoding: [0xd8,0x23,0b00000AAA,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %capcall(foo), kind: fixup_CHERI_CAPCALL11
	csc	$c1, $zero, %captab(foo)($c3)
# CHECK:      encoding: [0xf8,0x23,0b00000AAA,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %captab(foo), kind: fixup_CHERI_CAPTABLE11
	csc	$c1, $zero, %capcall(foo)($c3)
# CHECK:      encoding: [0xf8,0x23,0b00000AAA,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %capcall(foo), kind: fixup_CHERI_CAPCALL11


	clc	$c1, $zero, %captab(foo)($c3)
# CHECK:     encoding: [0xd8,0x23,0b00000AAA,A]
# CHECK-NEXT:  fixup A - offset: 0, value: %captab(foo), kind: fixup_CHERI_CAPTABLE11
	clc	$c1, $zero, %capcall(foo)($c3)
# CHECK:      encoding: [0xd8,0x23,0b00000AAA,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %capcall(foo), kind: fixup_CHERI_CAPCALL11
	csc	$c1, $zero, %captab(foo)($c3)
# CHECK:      encoding: [0xf8,0x23,0b00000AAA,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %captab(foo), kind: fixup_CHERI_CAPTABLE11
	csc	$c1, $zero, %capcall(foo)($c3)
# CHECK:      encoding: [0xf8,0x23,0b00000AAA,A]
# CHECK-NEXT: fixup A - offset: 0, value: %capcall(foo), kind: fixup_CHERI_CAPCALL11


# also check the new big immediate instructions:
	clcbi	$c1, %captab20(foo)($c3)
# CHECK:     encoding: [0x74,0x23,A,A]
# CHECK-NEXT:  fixup A - offset: 0, value: %captab20(foo), kind: fixup_CHERI_CAPTABLE20
	clcbi	$c1, %capcall20(foo)($c3)
# CHECK:      encoding: [0x74,0x23,A,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %capcall20(foo), kind: fixup_CHERI_CAPCALL20
	cscbi	$c1, %captab20(foo)($c3)
# CHECK:      encoding: [0x78,0x23,A,A]
# CHECK-NEXT:   fixup A - offset: 0, value: %captab20(foo), kind: fixup_CHERI_CAPTABLE20
	cscbi	$c1, %capcall20(foo)($c3)
# CHECK:      encoding: [0x78,0x23,A,A]
# CHECK-NEXT: fixup A - offset: 0, value: %capcall20(foo), kind: fixup_CHERI_CAPCALL20

lui	$1, %hi(%neg(%captab_rel(foo)))
# CHECK:      encoding: [0x3c,0x01,A,A]
# CHECK-NEXT: fixup A - offset: 0, value: %hi(%neg(%captab_rel(foo))), kind: fixup_Mips_CAPTABLEREL_HI
daddiu	$1, $1, %lo(%neg(%captab_rel(foo)))
# CHECK:      encoding: [0x64,0x21,A,A]
# CHECK-NEXT: fixup A - offset: 0, value: %lo(%neg(%captab_rel(foo))), kind: fixup_Mips_CAPTABLEREL_LO

        .data
        .word 0
foo:
        .word 1

# DUMP-LABEL: Disassembly of section .text:
# DUMP-NEXT:  .text:
# DUMP-NEXT:  0:       3c 01 00 00     lui     $1, 0
# DUMP-NEXT:           0000000000000000:  R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE     foo
# DUMP-NEXT:  4:       34 21 00 00     ori     $1, $1, 0
# DUMP-NEXT:           0000000000000004:  R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE     foo
# DUMP-NEXT:  8:       3c 01 00 00     lui     $1, 0
# DUMP-NEXT:           0000000000000008:  R_MIPS_CHERI_CAPCALL_HI16/R_MIPS_NONE/R_MIPS_NONE    foo
# DUMP-NEXT:  c:       34 21 00 00     ori     $1, $1, 0
# DUMP-NEXT:           000000000000000c:  R_MIPS_CHERI_CAPCALL_LO16/R_MIPS_NONE/R_MIPS_NONE    foo
# DUMP-NEXT: 10:       d8 23 00 00     clc     $c1, $zero, 0($c3)
# DUMP-NEXT:           0000000000000010:  R_MIPS_CHERI_CAPTAB_CLC11/R_MIPS_NONE/R_MIPS_NONE    foo
# DUMP-NEXT: 14:       d8 23 00 00     clc     $c1, $zero, 0($c3)
# DUMP-NEXT:           0000000000000014:  R_MIPS_CHERI_CAPCALL_CLC11/R_MIPS_NONE/R_MIPS_NONE   foo
# DUMP-NEXT: 18:       f8 23 00 00     csc     $c1, $zero, 0($c3)
# DUMP-NEXT:           0000000000000018:  R_MIPS_CHERI_CAPTAB_CLC11/R_MIPS_NONE/R_MIPS_NONE    foo
# DUMP-NEXT: 1c:       f8 23 00 00     csc     $c1, $zero, 0($c3)
# DUMP-NEXT:           000000000000001c:  R_MIPS_CHERI_CAPCALL_CLC11/R_MIPS_NONE/R_MIPS_NONE   foo
# DUMP-NEXT: 20:       d8 23 00 00     clc     $c1, $zero, 0($c3)
# DUMP-NEXT:           0000000000000020:  R_MIPS_CHERI_CAPTAB_CLC11/R_MIPS_NONE/R_MIPS_NONE    foo
# DUMP-NEXT: 24:       d8 23 00 00     clc     $c1, $zero, 0($c3)
# DUMP-NEXT:           0000000000000024:  R_MIPS_CHERI_CAPCALL_CLC11/R_MIPS_NONE/R_MIPS_NONE   foo
# DUMP-NEXT: 28:       f8 23 00 00     csc     $c1, $zero, 0($c3)
# DUMP-NEXT:           0000000000000028:  R_MIPS_CHERI_CAPTAB_CLC11/R_MIPS_NONE/R_MIPS_NONE    foo
# DUMP-NEXT: 2c:       f8 23 00 00     csc     $c1, $zero, 0($c3)
# DUMP-NEXT:           000000000000002c:  R_MIPS_CHERI_CAPCALL_CLC11/R_MIPS_NONE/R_MIPS_NONE   foo
# DUMP-NEXT: 30:       74 23 00 00     clcbi   $c1, 0($c3)
# DUMP-NEXT:           0000000000000030:  R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE        foo
# DUMP-NEXT: 34:       74 23 00 00     clcbi   $c1, 0($c3)
# DUMP-NEXT:           0000000000000034:  R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE       foo
# DUMP-NEXT: 38:       78 23 00 00     cscbi   $c1, 0($c3)
# DUMP-NEXT:           0000000000000038:  R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE        foo
# DUMP-NEXT: 3c:       78 23 00 00     cscbi   $c1, 0($c3)
# DUMP-NEXT:           000000000000003c:  R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE       foo
# DUMP-NEXT: 40:       3c 01 00 00     lui $1, 0
# DUMP-NEXT:           0000000000000040:  R_MIPS_CHERI_CAPTABLEREL16/R_MIPS_SUB/R_MIPS_HI16    foo
# DUMP-NEXT: 44:       64 21 00 00     daddiu  $1, $1, 0
# DUMP-NEXT:           0000000000000044:  R_MIPS_CHERI_CAPTABLEREL16/R_MIPS_SUB/R_MIPS_LO16    foo
