// REQUIRES: clang

// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable -emit-obj -O0 -mllvm -cheri-cap-table-abi=plt %s -o %t-128.o
// RUN: %cheri256_purecap_cc1 -mllvm -mxcaptable -emit-obj -O0 -mllvm -cheri-cap-table-abi=plt %s -o %t-256.o
// RUN: llvm-objdump -d -r %t-128.o | FileCheck %s -check-prefix OBJECT
// RUN: llvm-objdump -d -r %t-256.o | FileCheck %s -check-prefix OBJECT
// RUN: ld.lld -o %t-128.exe %t-128.o
// RUN: ld.lld -o %t-256.exe %t-256.o
// RUN: llvm-objdump -d -r -cap-relocs -t %t-128.exe | FileCheck %s -check-prefixes EXE,EXE128
// RUN: llvm-objdump -d -r -cap-relocs -t %t-256.exe | FileCheck %s -check-prefixes EXE,EXE256

// OBJECT:       4: 3c 01 00 00 lui	$1, 0
// OBJECT-NEXT:       0000000000000004: R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE	global
// OBJECT-NEXT:  8: 64 21 00 00 daddiu	$1, $1, 0
// OBJECT-NEXT:       0000000000000008: R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE	global
// OBJECT-NEXT:  c: d8 41 08 00 clc $c2, $1, 0($c1)
// OBJECT:      18: 3c 03 00 00 lui     $3, 0
// OBJECT-NEXT:       000000000000018:  R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE  global2
// OBJECT-NEXT: 1c: 64 63 00 00 daddiu  $3, $3, 0
// OBJECT-NEXT:       00000000000001c:  R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE  global2
// OBJECT-NEXT: 20: d8 21 18 00 clc     $c1, $3, 0($c1)



// EXE-LABEL: SYMBOL TABLE:
// EXE-DAG: [[GLOBAL_CAPTAB:[0-9a-f]+]]  l     O .captable		 000000{{1|2}}0  global@CAPTABLE
// EXE-DAG: [[GLOBAL2_CAPTAB:[0-9a-f]+]]  l     O .captable		 000000{{1|2}}0  global2@CAPTABLE
// EXE-LABEL: CAPABILITY RELOCATION RECORDS:
// EXE-NEXT: 0x[[GLOBAL_CAPTAB]]      Base: global (0x00000001200303{{3|6}}0)       Offset: 0x0000000000000000      Length: 0x0000000000000004    Permissions: 0x00000000
// EXE-NEXT: 0x[[GLOBAL2_CAPTAB]]      Base: global2 (0x00000001200303{{3|6}}8)      Offset: 0x0000000000000000      Length: 0x0000000000000008    Permissions: 0x00000000


// EXE:          3c 01 00 00 	lui	$1, 0
// EXE-NEXT:     64 21 00 00 	daddiu	$1, $1, 0
// EXE:          3c 03 00 00   lui     $3, 0
// This is the second entry into the cap table so it should be 32/16 bytes
// EXE128-NEXT:  64 63 00 10   daddiu  $3, $3, 16
// EXE256-NEXT:  64 63 00 20   daddiu  $3, $3, 32

int global = 1;
long global2 = 3;

int __start(void) {
  return global + global2;
}
