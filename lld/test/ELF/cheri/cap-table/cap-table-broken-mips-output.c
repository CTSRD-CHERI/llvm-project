// Previously, lld would create a broken _CHERI_CAPABILITY_TABLE_ symbol even if
// the output doesn't contain a CHERI captable
// This previously caused the following error in llvm-objdump: index past the end of the symbol table

// create a n64 output that doesn't have any cap-table entries
// RUN: %cheri_cc1 %s -emit-obj -x c -O3 -o %t.o
// RUN: llvm-objdump -t %t.o | FileCheck -check-prefix OBJ %s
// RUN: ld.lld %t.o -o %t.exe
// RUN: llvm-objdump -t %t.exe | FileCheck %s

int global = 1;
long global2 = 3;

static int __start(void) {
  return global + global2;
}

// OBJ-LABEL: SYMBOL TABLE:
// OBJ-NEXT: 0000000000000000 l    df *ABS*		 00000000 cap-table-broken-mips-output.c
// OBJ-NEXT: 0000000000000000 g     O .data		 00000004 global
// OBJ-NEXT: 0000000000000008 g     O .data		 00000008 global2

// CHECK-LABEL: SYMBOL TABLE:
// CHECK-NEXT: 0000000000000000 l    df *ABS*    00000000 cap-table-broken-mips-output.c
// CHECK-NEXT: 0000000000038210         .got		 00000000 .hidden _gp
// CHECK-NEXT: 0000000000030210 g     O .data		 00000004 global
// CHECK-NEXT: 0000000000030218 g     O .data		 00000008 global2
