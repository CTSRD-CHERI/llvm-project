// RUN: %cheri_purecap_cc1 -pedantic -Weverything %s -emit-obj -mllvm -mxgot -mllvm -cheri-cap-table -x c -O3 -o %t.o
// RUN: ld.lld %t.o -o %t.exe
// RUN: llvm-objdump -t -d -s %t.exe | FileCheck %s

#include <cheri_init_globals.h>


int global = 1;
long global2 = 3;

int main();

static int _start(void) __attribute__((used));
DEFINE_CHERI_START_FUNCTION(_start)

static int _start(void) {
  return main();
  main();
}

int main() {
  return global + global2;
}
// CHECK-LABEL: SYMBOL TABLE:
// CHECK-DAG: 0000000120010050    l     F .text		 0000002c _start
// CHECK-DAG: 0000000120010000    g     F .text		 00000050 __start
// CHECK-DAG: 00000001200300{{2|4}}0         .cap_table		 00000000 .hidden __cap_table_end
// CHECK-DAG: 0000000120030000         .cap_table		 00000000 .hidden __cap_table_start
// CHECK-DAG: 0000000120030000 l    d  .cap_table		 00000000 .hidden .cap_table
// CHECK-DAG: 0000000120030000 l       .cap_table		 00000020 .hidden global@CAPTABLE
// CHECK-DAG: 00000001200300{{1|2}}0 l       .cap_table		 00000020 .hidden global2@CAPTABLE
