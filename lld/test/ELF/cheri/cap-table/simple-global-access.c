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
// EXE-DAG: [[GLOBAL_CAPTAB:[0-9a-f]+]]  l     O .captable		 00000000000000{{1|2}}0  global@CAPTABLE
// EXE-DAG: [[GLOBAL2_CAPTAB:[0-9a-f]+]]  l     O .captable		 00000000000000{{1|2}}0  global2@CAPTABLE
// EXE-LABEL: CAPABILITY RELOCATION RECORDS:
// EXE-NEXT: 0x[[GLOBAL_CAPTAB]]      Base: global (0x00000000000403{{3|6}}0)       Offset: 0x0000000000000000      Length: 0x0000000000000004    Permissions: 0x00000000
// EXE-NEXT: 0x[[GLOBAL2_CAPTAB]]      Base: global2 (0x00000000000403{{3|6}}8)      Offset: 0x0000000000000000      Length: 0x0000000000000008    Permissions: 0x00000000

// EXE:          3c 01 00 00 	lui	$1, 0
// EXE-NEXT:     64 21 00 00 	daddiu	$1, $1, 0
// EXE:          3c 02 00 00  	lui	$2, 0 <simple-global-access.c>
// This is the second entry into the cap table so it should be 16 bytes
// EXE-NEXT:  64 42 00 10  	daddiu	$2, $2, 16 <simple-global-access.c+0x10>

int global = 1;
long global2 = 3;

int __start(void) {
  return global + global2;
}
