// REQUIRES: mips
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
// CHECK-NEXT: Base: t (0x00000000000404e0)	Offset: 0x0000000000000000	Length: 0x0000000000000040	Permissions: 0x00000000
// CHECK-NEXT: Base: t (0x0000000000040520)	Offset: 0x0000000000000000	Length: 0x0000000000000040	Permissions: 0x00000000
// CHECK-NEXT: Base: get_number (0x00000000000203d0)	Offset: 0x0000000000000000	Length: 0x00000000000000{{.+}}	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT: Base: get_ordinal (0x0000000000020{{.+}})	Offset: 0x0000000000000000	Length: 0x00000000000000{{.+}}	Permissions: 0x8000000000000000 (Function)
// CHECK-NEXT: Base: <Unnamed symbol> (0x0000000000010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x4000000000000000 (Constant)
// CHECK-NEXT: Base: <Unnamed symbol> (0x0000000000010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000004	Permissions: 0x4000000000000000 (Constant)
// CHECK-NEXT: Base: <Unnamed symbol> (0x0000000000010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000006	Permissions: 0x4000000000000000 (Constant)
// CHECK-NEXT: Base: <Unnamed symbol> (0x0000000000010{{.+}})	Offset: 0x0000000000000000	Length: 0x0000000000000007	Permissions: 0x4000000000000000 (Constant)
