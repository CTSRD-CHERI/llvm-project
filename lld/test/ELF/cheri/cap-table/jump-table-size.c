// REQUIRES: clang

// RUN: %cheri_purecap_cc1 -mllvm -mxcaptable -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt %s -o %t.o
// RUN: llvm-objdump -d -r -t %t.o | FileCheck %s --check-prefix OBJECT
// RUN: ld.lld -o %t.exe %t.o
// RUN: llvm-objdump --no-print-imm-hex -d -r --cap-relocs -t %t.exe | %cheri_FileCheck %s --check-prefixes EXE

// OBJECT-LABEL: SYMBOL TABLE:
// OBJECT: 0000000000000000 l       .rodata		 0000000000000024 .LJTI0_0

// OBJECT:      14:	3c 01 00 00 	lui	$1, 0
// OBJECT-NEXT: 0000000000000014:  R_MIPS_CHERI_CAPTAB_HI16/R_MIPS_NONE/R_MIPS_NONE	.LJTI0_0
// OBJECT-NEXT: 18:	64 21 00 00 	daddiu	$1, $1, 0
// OBJECT-NEXT: 0000000000000018:  R_MIPS_CHERI_CAPTAB_LO16/R_MIPS_NONE/R_MIPS_NONE	.LJTI0_0



// EXE: SYMBOL TABLE:
// EXE-DAG: 0000000000010278 l        .rodata		 0000000000000024 .LJTI0_0
// EXE-DAG: 0000000000030340 l     O .captable		 00000000000000{{1|2}}0 .LJTI0_0@CAPTABLE

// __cap_relocs should contain length:
// EXE:      CAPABILITY RELOCATION RECORDS:
// EXE-NEXT: 0x0000000000030340	Base: .LJTI0_0 (0x0000000000010278)	Offset: 0x0000000000000000	Length: 0x0000000000000024	Permissions: 0x4000000000000000 (Constant)

// EXE: 202e4:	3c 01 00 00 	lui	$1, 0
// EXE-NEXT: 202e8:	64 21 00 00 	daddiu	$1, $1, 0
// EXE-NEXT: 202ec:	d8 3a 08 00 	clc	$c1, $1, 0($c26)

int __start(int i) {
  switch(i) {
    case 1: return 9;
    case 4: return 10;
    case 5: return 22;
    case 7: return 77;
    case 9: return 9;
    default: return 0;
  }
}
