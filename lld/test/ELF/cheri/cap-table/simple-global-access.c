// REQUIRES: clang
// FIXME: why is this not an assembly test...

// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable -emit-obj -O0 -mllvm -cheri-cap-table-abi=plt %s -o %t.o
// RUN: llvm-objdump -d -r %t.o | FileCheck %s --check-prefix OBJECT
// RUN: ld.lld -o %t.exe %t.o
// RUN: llvm-objdump --no-print-imm-hex -d -r --cap-relocs -t %t.exe | FileCheck %s --check-prefixes EXE

// OBJECT:       0000000000000004: R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE	global
// OBJECT:       0000000000000008: R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE	global
// OBJECT:       000000000000018:  R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE  global2
// OBJECT:       00000000000001c:  R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE  global2

// EXE-LABEL: SYMBOL TABLE:
// EXE-DAG: [[#%.16x,GLOBAL_CAPTAB:]]  l     O .captable         0000000000000010  global@CAPTABLE
// EXE-DAG: [[#%.16x,GLOBAL2_CAPTAB:]] l     O .captable         0000000000000010  global2@CAPTABLE
// EXE-DAG: [[#%.16x,GLOBAL_ADDR:]]    g     O .data             0000000000000004  global
// EXE-DAG: [[#%.16x,GLOBAL2_ADDR:]]   g     O .data             0000000000000008  global2
// EXE-LABEL: CAPABILITY RELOCATION RECORDS:
// EXE-NEXT: OFFSET           TYPE    VALUE
// EXE-NEXT: [[#GLOBAL_CAPTAB]]  DATA    [[#GLOBAL_ADDR]] {{\[}}[[#GLOBAL_ADDR]]-[[#GLOBAL_ADDR+4]]]
// EXE-NEXT: [[#GLOBAL2_CAPTAB]] DATA    [[#GLOBAL2_ADDR]] {{\[}}[[#GLOBAL2_ADDR]]-[[#GLOBAL2_ADDR+8]]]

// EXE:          3c 01 00 00 	lui	$1, 0
// EXE-NEXT:     64 21 00 00 	daddiu	$1, $1, 0
// EXE:          3c 02 00 00  	lui	$2, 0
// This is the second entry into the cap table so it should be 16 bytes
// EXE-NEXT:  64 42 00 10  	daddiu	$2, $2, 16

int global = 1;
long global2 = 3;

int __start(void) {
  return global + global2;
}
