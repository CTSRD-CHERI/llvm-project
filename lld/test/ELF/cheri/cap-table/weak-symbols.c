// REQUIRES: clang

// RUN: %cheri128_cc1 -emit-obj -O2 -target-abi purecap -mllvm -cheri-cap-table %s -o %t.o
// RUNNOT: llvm-objdump -d -r -t %t.o
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
// DYNAMIC-EXE: 0000000000000230         .dynamic		 00000140 .hidden _DYNAMIC
// DYNAMIC-EXE: 0000000000000000 .MIPS.abiflags 00000000 .hidden __init_array_end
// DYNAMIC-EXE: 0000000000000000 .MIPS.abiflags 00000000 .hidden __init_array_start
// STATIC-EXE: 0000000         *ABS*		 00000000 .hidden __init_array_end
// STATIC-EXE: 0000000         *ABS*		 00000000 .hidden __init_array_start
// EXE: 0030008         .preinit_array		 00000000 .hidden __preinit_array_end
// EXE: 0030000         .preinit_array		 00000008 .hidden __preinit_array_start
// STATIC-EXE: 0000000000000000  w      *UND*		 00000000 _DYNAMIC
// EXE: 0010160 g     F .text		 00000008 __start
// EXE: 0030278         __cap_relocs		 00000000 _edata
// EXE: 0040000         .bss		 00000000 _end
// EXE: 0010168         .text		 00000000 _etext
// EXE: 0030278         __cap_relocs		 00000000 edata
// EXE: 0040000         .bss		 00000000 end
// EXE: 0010168         .text		 00000000 etext
