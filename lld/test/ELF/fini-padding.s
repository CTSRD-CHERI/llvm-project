# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t.o
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %S/Inputs/freebsd-mips-crtn.s -o %t-crtn.o
# RUN: ld.lld -Bstatic %t.o %t-crtn.o -o %t.exe
# RUN: llvm-readobj -s %t.exe | FileCheck %s -check-prefix SECTIONS
# RUN: llvm-objdump -d -p -h %t.exe | FileCheck %s

# LLD was inserting trap instructions into FreeBSD binaries .init/.fini sections
# The crtn.s file wrongly aligned .init to 2**4 instead of 4 so lld was filling
# in padding with trap instructions

# Even though the FreeBSD code was wrong we should still not insert trap
# instructions into .init/.fini. It contains concatenated snippets of code that
# runs in order. We should insert nops here the same way BFD does it

.text
.global __start
__start:
  jr $ra
  nop

.global dtor
dtor:
  jr $ra
  nop

.section .fini,"ax",%progbits
.set noreorder
dla $t9, dtor
li $a0, 42
jalr $t9
nop
li $a0, 0x1234

.section .init,"ax",%progbits
.p2align 4
.set noreorder
li $a0, 42

.section .init,"ax",%progbits
.p2align 4
.set noreorder
li $a0, 43


# SECTIONS:      Section {
# SECTIONS:           Name: .fini (36)
# SECTIONS-NEXT:      Type: SHT_PROGBITS (0x1)
# SECTIONS-NEXT:      Flags [ (0x6)
# SECTIONS-NEXT:        SHF_ALLOC (0x2)
# SECTIONS-NEXT:        SHF_EXECINSTR (0x4)
# SECTIONS-NEXT:      ]
# SECTIONS-NEXT:      Address: 0x20020
# SECTIONS-NEXT:      Offset: 0x10020
# SECTIONS-NEXT:      Size: 64
# SECTIONS-NEXT:      Link: 0
# SECTIONS-NEXT:      Info: 0
# SECTIONS-NEXT:      AddressAlignment: 16
# SECTIONS-NEXT:      EntrySize: 0
# SECTIONS-NEXT:    }


# We don't want any trap instructions inserted between the .fini and .init sections

# CHECK-LABEL: Disassembly of section .fini:
# CHECK-NEXT: .fini:
# CHECK-NEXT: 20020: 3c 19 00 00     lui     $25, 0
# CHECK:      2003c: 03 20 f8 09     jalr    $25
# CHECK-NEXT: 20040: 00 00 00 00     nop
# CHECK-NEXT: 20044: 24 04 12 34     addiu   $4, $zero, 4660
# The following two instructions were previously filled with traps:
# CHECK-NEXT: 20048: 00 00 00 00     nop
# CHECK-NEXT: 2004c: 00 00 00 00     nop
# now the code from crtn.o:
# CHECK-NEXT: 20050: ff bc 00 00     sd      $gp, 0($sp)
# CHECK-NEXT: 20054: ff bf 00 08     sd      $ra, 8($sp)
# CHECK-NEXT: 20058: 03 e0 00 08     jr      $ra
# CHECK-NEXT: 2005c: 67 bd 00 20     daddiu  $sp, $sp, 32


# CHECK-LABEL: Disassembly of section .init:
# CHECK-NEXT: .init:
# CHECK-NEXT: 24 04 00 2a     addiu   $4, $zero, 42
# CHECK-NEXT: 00 00 00 00     nop
# CHECK-NEXT: 00 00 00 00     nop
# CHECK-NEXT: 00 00 00 00     nop
# CHECK-NEXT: 24 04 00 2b     addiu   $4, $zero, 43
