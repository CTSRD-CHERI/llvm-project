// REQUIRES: clang

// Check that when we use R_CHERI_CAPABILITY instead of legacy cap_relocs the zutil.c test works as expected (i.e. bounds on the strings)

// RUN: %cheri_purecap_clang %s -c -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck -check-prefix OBJ-RELOCS %s
// RUN: ld.lld -shared --enable-new-dtags -o %t.so --fatal-warnings %t.o
// RUN: llvm-objdump -C -s -t %t.so | FileCheck %s

#define z_const const

z_const char * const z_errmsg[10] = {
  (z_const char *)"need dictionary",     /* Z_NEED_DICT       2  */
  (z_const char *)"stream end",          /* Z_STREAM_END      1  */
  (z_const char *)"",                    /* Z_OK              0  */
#if 0 // make the test case a bit shorter
  (z_const char *)"file error",          /* Z_ERRNO         (-1) */
  (z_const char *)"stream error",        /* Z_STREAM_ERROR  (-2) */
  (z_const char *)"data error",          /* Z_DATA_ERROR    (-3) */
  (z_const char *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
  (z_const char *)"buffer error",        /* Z_BUF_ERROR     (-5) */
  (z_const char *)"incompatible version",/* Z_VERSION_ERROR (-6) */
  (z_const char *)""
#endif
};

// OBJ-RELOCS-LABEL: Relocations [
// OBJ-RELOCS-LABEL:  Section (6) .rela.data.rel.ro {
// OBJ-RELOCS-LABEL:   0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str 0x0
// OBJ-RELOCS-LABEL:   0x{{1|2}}0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str.1 0x0
// OBJ-RELOCS-LABEL:   0x{{2|4}}0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE .L.str.2 0x0
// OBJ-RELOCS-LABEL:  }
// OBJ-RELOCS-NEXT:  ]

// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// CHECK-NEXT:  0x0000000000020000	Base: <Unnamed symbol> (0x0000000000000203)	Offset: 0x0000000000000000	Length: 0x0000000000000010	Permissions: 0x00000000
// CHECK-NEXT:  0x00000000000200{{1|2}}0	Base: <Unnamed symbol> (0x0000000000000213)	Offset: 0x0000000000000000	Length: 0x000000000000000b	Permissions: 0x00000000
// CHECK-NEXT:  0x00000000000200{{2|4}}0	Base: <Unnamed symbol> (0x0000000000000202)	Offset: 0x0000000000000000	Length: 0x0000000000000001	Permissions: 0x00000000

// CHECK-LABEL:  Contents of section .rodata:
// CHECK-NEXT:   006e6565 64206469 6374696f 6e617279  .need dictionary
// CHECK-NEXT:   00737472 65616d20 656e6400           .stream end.
