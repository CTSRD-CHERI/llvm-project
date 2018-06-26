// REQUIRES: clang

// RUN: %cheri128_purecap_cc1 -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt %s -o %t.o
// RUNNOT: llvm-objdump -d -r %t.o | FileCheck %s -check-prefix OBJECT
// RUN: ld.lld -o %t.exe %t.o
// RUN: ld.lld -pie -o %t-pie.exe %t.o
// RUN: llvm-objdump -d -r -C -t %t.exe | FileCheck %s -check-prefixes EXE,STATIC-EXE
// RUN: llvm-objdump -d -r -C -t %t-pie.exe | FileCheck %s -check-prefixes EXE,DYNAMIC-EXE


__attribute__((section(".preinit_array"))) long x = 1;

#define TEST(name) __attribute__((weak)) extern void name; \
  int test_##name(void) { return &name != 0; }

TEST(end)
TEST(_end)
TEST(etext)
TEST(_etext)
TEST(edata)
TEST(_edata)

TEST(__preinit_array_start)
TEST(__preinit_array_end)
TEST(__init_array_start)
TEST(__init_array_end)

TEST(_DYNAMIC)


int __start(void) {
  return 0;
}
// DYNAMIC-EXE: 0000000000000238 .dynamic		 00000160 .hidden _DYNAMIC
// DYNAMIC-EXE: 0000000000010000 .text 00000000 .hidden __init_array_end
// DYNAMIC-EXE: 0000000000010000 .text 00000000 .hidden __init_array_start
// STATIC-EXE: 0000000120010000 .text 00000000 .hidden __init_array_end
// STATIC-EXE: 0000000120010000 .text 00000000 .hidden __init_array_start
//                      ^------- Start of .text segment (actual value doesn't matter as long as iteration terminates immediately)
// EXE: 0030008         .preinit_array		 00000000 .hidden __preinit_array_end
// EXE: 0030000         .preinit_array		 00000008 .hidden __preinit_array_start
// EXE: 0030010         .cap_table		 000000b0 _CHERI_CAPABILITY_TABLE_
// STATIC-EXE: 0000000000000000  w      *UND*		 00000000 _DYNAMIC
// EXE: 00100b0 g     F .text		 00000008 __start
// EXE: 0030278         __cap_relocs		 00000000 _edata
// EXE: 0040000         .bss		 00000000 _end
// EXE: 00100b8         .text		 00000000 _etext
// EXE: 0030278         __cap_relocs		 00000000 edata
// EXE: 0040000         .bss		 00000000 end
// EXE: 00100b8         .text		 00000000 etext
