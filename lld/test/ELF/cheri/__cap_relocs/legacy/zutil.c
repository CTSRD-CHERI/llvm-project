// REQUIRES: clang

// RUN: %cheri_purecap_clang %legacy_caprelocs_flag %s -c -o %t.o
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
// OBJ-RELOCS-NEXT:   Section (9) .rela__cap_relocs {
// OBJ-RELOCS-NEXT:     0x0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data.rel.ro 0x0
// OBJ-RELOCS-NEXT:     0x8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x0
// OBJ-RELOCS-NEXT:     0x28 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data.rel.ro 0x{{1|2}}0
// OBJ-RELOCS-NEXT:     0x30 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x10
// OBJ-RELOCS-NEXT:     0x50 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data.rel.ro 0x{{2|4}}0
// OBJ-RELOCS-NEXT:     0x58 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .rodata.str1.1 0x1B
// OBJ-RELOCS-NEXT:   }
// OBJ-RELOCS-NEXT: ]

// length is wrong with
// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// CHECK-NEXT:  0x0000000000020000	      Base: <Unnamed symbol> (0x0000000000000203)	Offset: 0x0000000000000000	Length: 0x000000000000001b	Permissions: 0x00000000
// CHECK-NEXT:  0x00000000000200{{1|2}}0	Base: <Unnamed symbol> (0x0000000000000213)	Offset: 0x0000000000000000	Length: 0x000000000000000b	Permissions: 0x00000000
// CHECK-NEXT:  0x00000000000200{{2|4}}0	Base: <Unnamed symbol> (0x0000000000000202)	Offset: 0x0000000000000000	Length: 0x000000000000001c	Permissions: 0x00000000

// CHECK-LABEL:  Contents of section .rodata:
// CHECK-NEXT:   006e6565 64206469 6374696f 6e617279  .need dictionary
// CHECK-NEXT:   00737472 65616d20 656e6400           .stream end.
