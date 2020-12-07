// RUN: %cheri128_purecap_cc1 -pedantic -Wextra %s -emit-obj -mllvm -cheri-cap-table-abi=plt -x c -O0 -o %t.o
// RUN: ld.lld %t.o -o %t.exe
// RUN: llvm-objdump -t %t.exe | FileCheck %s

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
// CHECK-DAG: 0000000000020358    l        F .text		 0000000000000038 _start
// CHECK-DAG: 00000000000202f0    g        F .text		 0000000000000068 __start
// CHECK-DAG: 0000000000030400 l             .captable		 0000000000000000 .hidden __cap_table_end
// CHECK-DAG: 00000000000303d0 l             .captable		 0000000000000030 .hidden __cap_table_start
// CHECK-DAG: 00000000000303d0 l           O .captable		 0000000000000010 main@CAPTABLE
// CHECK-DAG: 00000000000303e0 l           O .captable		 0000000000000010 global@CAPTABLE
// CHECK-DAG: 00000000000303f0 l           O .captable		 0000000000000010 global2@CAPTABLE
