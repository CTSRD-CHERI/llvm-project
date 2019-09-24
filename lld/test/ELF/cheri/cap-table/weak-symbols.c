// REQUIRES: clang

// RUN: %cheri128_purecap_cc1 -emit-obj -O2 -mllvm -cheri-cap-table-abi=plt %s -o %t.o
// RUNNOT: llvm-objdump -d -r %t.o | FileCheck %s -check-prefix OBJECT
// RUN: ld.lld -o %t.exe %t.o
// RUN: ld.lld -z now -pie -o %t-pie.exe %t.o
// RUN: llvm-objdump -r -cap-relocs -t %t.exe | FileCheck %s -check-prefixes EXE,STATIC-EXE
// RUN: llvm-objdump -r -cap-relocs -t %t-pie.exe | FileCheck %s -check-prefixes EXE,DYNAMIC-EXE


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
// DYNAMIC-EXE: 00000000000002d8         .dynamic		 00000{{.+}} .hidden _DYNAMIC
// DYNAMIC-EXE: 0000000000010620         .text		 00000000 .hidden __init_array_end
// DYNAMIC-EXE: 0000000000010620         .text		 00000000 .hidden __init_array_start
//                      ^------- Start of .text segment (actual value doesn't matter as long as iteration terminates immediately)
// DYNAMIC-EXE: 0000000000020738         .preinit_array		 00000000 .hidden __preinit_array_end
// DYNAMIC-EXE: 0000000000020730         .preinit_array		 00000008 .hidden __preinit_array_start
// DYNAMIC-EXE: 0000000000020740         .captable		 000000b0 _CHERI_CAPABILITY_TABLE_
// DYNAMIC-EXE: 0000000000010728 g     F .text		 00000008 __start
// DYNAMIC-EXE: 0000000000030810         .got	 00000000 _edata
// DYNAMIC-EXE: 0000000000030810         .bss		 00000000 _end
// DYNAMIC-EXE: 0000000000010730         .text		 00000000 _etext
// DYNAMIC-EXE: 0000000000030810         .got	 00000000 edata
// DYNAMIC-EXE: 0000000000030810         .bss		 00000000 end
// DYNAMIC-EXE: 0000000000010730         .text		 00000000 etext

// STATIC-EXE: 0000000120010430         .text		 00000000 .hidden __init_array_end
// STATIC-EXE: 0000000120010430         .text		 00000000 .hidden __init_array_start
//                      ^------- Start of .text segment (actual value doesn't matter as long as iteration terminates immediately)
// STATIC-EXE: 0000000120020548         .preinit_array		 00000000 .hidden __preinit_array_end
// STATIC-EXE: 0000000120020540         .preinit_array		 00000008 .hidden __preinit_array_start
// STATIC-EXE: 0000000120020550         .captable		 000000b0 _CHERI_CAPABILITY_TABLE_
// STATIC-EXE: 0000000000000000  w      *UND*		 00000000 _DYNAMIC
// STATIC-EXE: 0000000120010538 g     F .text		 00000008 __start
// STATIC-EXE: 0000000120030610         .got	 00000000 _edata
// STATIC-EXE: 0000000120030610         .bss		 00000000 _end
// STATIC-EXE: 0000000120010540         .text		 00000000 _etext
// STATIC-EXE: 0000000120030610         .got	 00000000 edata
// STATIC-EXE: 0000000120030610         .bss		 00000000 end
// STATIC-EXE: 0000000120010540         .text		 00000000 etext
