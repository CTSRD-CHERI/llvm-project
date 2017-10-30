// RUN: %cheri128_cc1 -DGET_NUMBER=1 -emit-obj -O2 -target-abi purecap -mllvm -cheri-cap-table %s -o %t-get_number.o
// RUN: %cheri128_cc1 -DGET_ORDINAL=1 -emit-obj -O2 -target-abi purecap -mllvm -cheri-cap-table %s -o %t-get_ordinal.o
// RUN: %cheri128_cc1 -DMAIN=1 -emit-obj -O2 -target-abi purecap -mllvm -cheri-cap-table %s -o %t-main.o
// RUN: ld.lld %t-get_number.o %t-get_ordinal.o %t-main.o -o %t.exe
// RUN: llvm-objdump -C -d -t -s %t.exe
// RUN: false

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
