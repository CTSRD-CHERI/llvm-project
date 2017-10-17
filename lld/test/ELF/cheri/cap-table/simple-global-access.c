// REQUIRES: clang

// RUN: %cheri128_cc1 -emit-obj -O2 -target-abi purecap -mllvm -cheri-cap-table %s -o %t-128.o
// RUN: %cheri256_cc1 -emit-obj -O2 -target-abi purecap -mllvm -cheri-cap-table %s -o %t-256.o
// RUN: llvm-objdump -d -r %t-128.o | FileCheck %s -check-prefix OBJECT
// RUN: llvm-objdump -d -r %t-256.o | FileCheck %s -check-prefix OBJECT
// RUN: ld.lld -o %t-128.exe %t-128.o
// RUN: ld.lld -o %t-256.exe %t-256.o
// RUN: llvm-objdump -d -r -C -t %t-128.exe | FileCheck %s -check-prefixes EXE,EXE128
// RUN: llvm-objdump -d -r -C -t %t-256.exe | FileCheck %s -check-prefixes EXE,EXE256

// OBJECT:        0:	3c 01 00 00 	lui	$1, 0
// OBJECT-NEXT:       0000000000000000: R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE	global
// OBJECT-NEXT:   4:	64 21 00 00 	daddiu	$1, $1, 0
// OBJECT-NEXT:       0000000000000004: R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE	global
// OBJECT-NEXT:   8:  d8 3a 08 00     clc     $c1, $1, 0($c26)
// OBJECT:        c:  3c 01 00 00     lui     $1, 0
// OBJECT-NEXT:       00000000000000c:  R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE  global2
// OBJECT-NEXT:  10:  64 21 00 00     daddiu  $1, $1, 0
// OBJECT-NEXT:       0000000000000010:  R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE  global2
// OBJECT-NEXT:  14:  d8 5a 08 00     clc     $c2, $1, 0($c26)



// EXE:          0:	3c 01 00 00 	lui	$1, 0
// EXE-NEXT:     4:	64 21 00 00 	daddiu	$1, $1, 0
// EXE:          c: 3c 01 00 00   lui     $1, 0
// This is the second entry into the cap table so it should be 32/16 bytes
// EXE128-NEXT: 10: 64 21 00 10   daddiu  $1, $1, 16
// EXE256-NEXT: 10: 64 21 00 20   daddiu  $1, $1, 32

// EXE:      CAPABILITY RELOCATION RECORDS:
// EXE-NEXT: 0x0000000120030000      Base: global (0x0000000120020000)       Offset: 0x0000000000000000      Length: 0x0000000000000004    Permissions: 0x00000000
// EXE128-NEXT: 0x0000000120030010      Base: global2 (0x0000000120020008)      Offset: 0x0000000000000000      Length: 0x0000000000000008    Permissions: 0x00000000
// EXE256-NEXT: 0x0000000120030020      Base: global2 (0x0000000120020008)      Offset: 0x0000000000000000      Length: 0x0000000000000008    Permissions: 0x00000000

// EXE: SYMBOL TABLE:


int global = 1;
long global2 = 3;

int __start(void) {
  return global + global2;
}
