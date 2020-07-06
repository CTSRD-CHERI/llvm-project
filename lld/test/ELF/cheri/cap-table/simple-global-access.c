// REQUIRES: clang

// RUN: %cheri128_purecap_cc1 -mllvm -mxcaptable -emit-obj -O0 -mllvm -cheri-cap-table-abi=plt %s -o %t.o
// RUN: llvm-objdump -d -r %t.o | FileCheck %s --check-prefix OBJECT
// RUN: ld.lld -o %t.exe %t.o
// RUN: llvm-objdump -d -r --cap-relocs -t %t.exe | FileCheck %s --check-prefixes EXE,EXE128

// OBJECT:       0000000000000004: R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE	global
// OBJECT:       0000000000000008: R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE	global
// OBJECT:       000000000000018:  R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE  global2
// OBJECT:       00000000000001c:  R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE  global2



// EXE-LABEL: SYMBOL TABLE:
// EXE-DAG: [[GLOBAL_CAPTAB:[0-9a-f]+]]  l     O .captable		 00000000000000{{1|2}}0  global@CAPTABLE
// EXE-DAG: [[GLOBAL2_CAPTAB:[0-9a-f]+]]  l     O .captable		 00000000000000{{1|2}}0  global2@CAPTABLE
// EXE-LABEL: CAPABILITY RELOCATION RECORDS:
// EXE-NEXT: 0x[[GLOBAL_CAPTAB]]      Base: global (0x00000001200303{{3|6}}0)       Offset: 0x0000000000000000      Length: 0x0000000000000004    Permissions: 0x00000000
// EXE-NEXT: 0x[[GLOBAL2_CAPTAB]]      Base: global2 (0x00000001200303{{3|6}}8)      Offset: 0x0000000000000000      Length: 0x0000000000000008    Permissions: 0x00000000


// EXE:          3c 01 00 00 	lui	$1, 0
// EXE-NEXT:     64 21 00 00 	daddiu	$1, $1, 0
// EXE:          3c 01 00 00   lui $1, 0 <simple-global-access.c+0x120000000>
// This is the second entry into the cap table so it should be 16 bytes
// EXE-NEXT:  64 21 00 10   daddiu  $1, $1, 16 <simple-global-access.c+0x120000010>

int global = 1;
long global2 = 3;

int __start(void) {
  return global + global2;
}
