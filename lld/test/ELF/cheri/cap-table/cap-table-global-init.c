// RUN: %cheri128_purecap_cc1 -pedantic -Wextra %s -emit-obj -mllvm -mxgot -mllvm -cheri-cap-table-abi=plt -x c -O0 -o %t.o
// RUN: ld.lld %t.o -o %t.exe
// RUN: llvm-objdump -t -d -s %t.exe | FileCheck %s

#include <cheri_init_globals.h>


int global = 1;
long global2 = 3;

int main(void);

static int _start(void) __attribute__((used));
DEFINE_CHERI_START_FUNCTION(_start)

static int _start(void) {
  return main();
}

int main(void) {
  return global + global2;
}

// CHECK-LABEL: SYMBOL TABLE:
// CHECK-DAG: 0000000120010068    l        F .text		 00000034 _start
// CHECK-DAG: 0000000120010000    g        F .text		 00000068 __start
// CHECK-DAG: 0000000120020030               .captable		 00000000 .hidden __cap_table_end
// CHECK-DAG: 0000000120020000               .captable		 00000030 .hidden __cap_table_start
// CHECK-DAG: 0000000120020000 l           O .captable		 00000010 main@CAPTABLE
// CHECK-DAG: 0000000120020010 l           O .captable		 00000010 global@CAPTABLE
// CHECK-DAG: 0000000120020020 l           O .captable		 00000010 global2@CAPTABLE
