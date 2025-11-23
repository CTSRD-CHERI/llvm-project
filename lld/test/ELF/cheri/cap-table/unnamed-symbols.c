// RUN: %cheri128_purecap_cc1 -DGET_NUMBER=1 -emit-obj -O0 -mllvm -cheri-cap-table-abi=plt %s -o %t-get_number.o
// RUN: %cheri128_purecap_cc1 -DGET_ORDINAL=1 -emit-obj -O0 -mllvm -cheri-cap-table-abi=plt %s -o %t-get_ordinal.o
// RUN: %cheri128_purecap_cc1 -DMAIN=1 -emit-obj -O0 -mllvm -cheri-cap-table-abi=plt %s -o %t-main.o
// RUN: ld.lld %t-get_number.o %t-get_ordinal.o %t-main.o -o %t.exe
// RUN: llvm-objdump --cap-relocs %t.exe | FileCheck %s

struct table {
  int value;
  const char *str;
};

#ifdef GET_ORDINAL
#define ONE_STR "first"
#define TWO_STR "second"
#endif
#ifdef GET_NUMBER
#define ONE_STR "one"
#define TWO_STR "two"
#endif

#ifndef MAIN
static struct table t[] = {
    {1, ONE_STR},
    {2, TWO_STR},
};
#endif

const char *get_number(int i);
const char *get_ordinal(int i);

#ifdef GET_NUMBER
const char *get_number(int i) { return t[i].str; }
#endif
#ifdef GET_ORDINAL
const char *get_ordinal(int i) { return t[i].str; }
#endif
#ifdef MAIN
int __start(void) { return get_number(1) == get_ordinal(2); }
#endif

// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// CHECK-NEXT: TYPE    VALUE
// CHECK-NEXT: DATA    00000000000404e0 [00000000000404e0-0000000000040520]
// CHECK-NEXT: DATA    0000000000040520 [0000000000040520-0000000000040560]
// CHECK-NEXT: FUNC    00000000000203d0 [{{[0-9a-f]+}}-{{[0-9a-f]+}}]
// CHECK-NEXT: FUNC    0000000000020400 [{{[0-9a-f]+}}-{{[0-9a-f]+}}]
// CHECK-NEXT: RODATA  00000000000103c5 [00000000000103c5-00000000000103c9]
// CHECK-NEXT: RODATA  00000000000103c9 [00000000000103c9-00000000000103cd]
// CHECK-NEXT: RODATA  00000000000103bf [00000000000103bf-00000000000103c5]
// CHECK-NEXT: RODATA  00000000000103b8 [00000000000103b8-00000000000103bf]
