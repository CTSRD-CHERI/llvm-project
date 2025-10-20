// REQUIRES: clang

// RUN: %cheri128_purecap_cc1 -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt %s -o %t.o
// RUNNOT: llvm-objdump -d -r %t.o | FileCheck %s --check-prefix OBJECT
// RUN: ld.lld -o %t.exe %t.o
// RUN: ld.lld -z now -pie -o %t-pie.exe %t.o
// RUN: llvm-objdump -r --cap-relocs -t %t.exe | FileCheck %s --check-prefixes EXE,STATIC-EXE
// RUN: llvm-objdump -r --cap-relocs -t %t-pie.exe | FileCheck %s --check-prefixes EXE,DYNAMIC-EXE


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
// EXE-LABEL: SYMBOL TABLE
// DYNAMIC-EXE: 0000000000020810 l       .preinit_array   0000000000000008 .hidden __preinit_array_start
// DYNAMIC-EXE: 0000000000020818 l       .preinit_array   0000000000000000 .hidden __preinit_array_end
// DYNAMIC-EXE: 0000000000010700 l       .text            0000000000000000 .hidden __init_array_start
//                      ^------- Start of .text segment (actual value doesn't matter as long as iteration terminates immediately)
// DYNAMIC-EXE: 0000000000010700 l       .text            0000000000000000 .hidden __init_array_end
// DYNAMIC-EXE: 0000000000000310 l       .dynamic         0000000000000{{.+}} .hidden _DYNAMIC
// DYNAMIC-EXE: 0000000000030900 g       .bss             0000000000000000 end
// DYNAMIC-EXE: 0000000000030900 g       .bss             0000000000000000 _end
// DYNAMIC-EXE: 0000000000010810 g       .text            0000000000000000 etext
// DYNAMIC-EXE: 0000000000010810 g       .text            0000000000000000 _etext
// DYNAMIC-EXE: 0000000000030900 g       .pad.cheri.pcc   0000000000000000 edata
// DYNAMIC-EXE: 0000000000030900 g       .pad.cheri.pcc   0000000000000000 _edata
// DYNAMIC-EXE: 0000000000010808 g     F .text            0000000000000008 __start

// STATIC-EXE: 0000000000030610 l       .preinit_array    0000000000000008 .hidden __preinit_array_start
// STATIC-EXE: 0000000000030618 l       .preinit_array    0000000000000000 .hidden __preinit_array_end
// STATIC-EXE: 0000000000020500 l       .text             0000000000000000 .hidden __init_array_start
//                      ^------- Start of .text segment (actual value doesn't matter as long as iteration terminates immediately)
// STATIC-EXE: 0000000000020500 l       .text             0000000000000000 .hidden __init_array_end
// STATIC-EXE: 0000000000040700 g       .bss              0000000000000000 end
// STATIC-EXE: 0000000000040700 g       .bss              0000000000000000 _end
// STATIC-EXE: 0000000000020610 g       .text             0000000000000000 etext
// STATIC-EXE: 0000000000020610 g       .text             0000000000000000 _etext
// STATIC-EXE: 0000000000040700 g       .pad.cheri.pcc    0000000000000000 edata
// STATIC-EXE: 0000000000040700 g       .pad.cheri.pcc    0000000000000000 _edata
// STATIC-EXE: 0000000000000000  w      *UND*             0000000000000000 _DYNAMIC
// STATIC-EXE: 0000000000020608 g     F .text             0000000000000008 __start
