// REQUIRES: clang

// RUN: mkdir -p %t
// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -O2 -DFILE1 %s -o %t/file1.o
// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -O2 -DFILE2 %s -o %t/foo.o
// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -O2 -DFILE3 %s -o %t/bar.o

// RUN: ld.lld -shared -o %t/per-file.so %t/file1.o %t/foo.o %t/bar.o -captable-scope=file
// RUN: llvm-readobj --cap-relocs --cap-table -dynamic-table -r %t/per-file.so | FileCheck %s -check-prefixes PER-FILE,PER-FILE-OBJECTS
// No new globals used -> when using all it should be indentical!
// RUN: ld.lld -shared -o %t/global.so %t/file1.o %t/foo.o %t/bar.o -captable-scope=all
// RUN: llvm-readobj --cap-relocs --cap-table -dynamic-table -r %t/global.so | FileCheck %s -check-prefix GLOBAL

// Also try the per-file with an archive (should result in different @CAPTABLE symbol names)
// RUN: rm -f %t/somelib.a && llvm-ar cqs %t/somelib.a %t/foo.o %t/bar.o
// RUN: ld.lld -shared -o %t/per-file-with-archive.so %t/file1.o --whole-archive %t/somelib.a -captable-scope=file
// RUN: llvm-readobj --cap-relocs --cap-table -dynamic-table -r %t/per-file-with-archive.so | FileCheck %s -check-prefixes PER-FILE,PER-FILE-ARCHIVE


// PER-FILE: Relocations [
// PER-FILE-NEXT:   Section ({{.+}}) .rel.dyn {
// PER-FILE-NEXT:     0x20090 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// PER-FILE-NEXT:     0x20098 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// PER-FILE-NEXT:     0x20030 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_int 0x0 (real addend unknown)
// PER-FILE-NEXT:     0x20060 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_int 0x0 (real addend unknown)
// PER-FILE-NEXT:     0x20080 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_int 0x0 (real addend unknown)
// PER-FILE-NEXT:   }
// PER-FILE-NEXT:   Section ({{.+}}) .rel.plt {
// PER-FILE-NEXT:     0x20000 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// PER-FILE-NEXT:     0x20010 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// PER-FILE-NEXT:     0x20020 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int 0x0 (real addend unknown)
// PER-FILE-NEXT:     0x20050 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// PER-FILE-NEXT:     0x20070 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// PER-FILE-NEXT:   }
// PER-FILE-NEXT: ]
// PER-FILE-NEXT: DynamicSection [
// PER-FILE:       0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PLT CAPTABLE_PER_FILE
// PER-FILE:      ]
// PER-FILE-NEXT: CHERI __cap_relocs [
// PER-FILE-NEXT:    0x020040 (function3@CAPTABLE@file1.o.4) Base: 0x100{{.+}} (function3+0) Length: {{.+}} Perms: Function
// PER-FILE-NEXT: ]
// PER-FILE-NEXT: CHERI .captable [
// PER-FILE-NEXT:   0x0      extern_void_ptr@CAPTABLE@file1.o   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// PER-FILE-NEXT:   0x10     extern_char_ptr@CAPTABLE@file1.o   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// PER-FILE-NEXT:   0x20     extern_int@CAPTABLE@file1.o     R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_int
// PER-FILE-NEXT:   0x30     global_int@CAPTABLE@file1.o     R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// PER-FILE-NEXT:   0x40     function3@CAPTABLE@file1.o.4
// PER-FILE-OBJECTS-NEXT:   0x50     extern_char_ptr@CAPTABLE@foo.o   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// PER-FILE-OBJECTS-NEXT:   0x60     global_int@CAPTABLE@foo.o   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// PER-FILE-OBJECTS-NEXT:   0x70     extern_char_ptr@CAPTABLE@bar.o   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// PER-FILE-OBJECTS-NEXT:   0x80     global_int@CAPTABLE@bar.o   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// PER-FILE-ARCHIVE-NEXT:   0x50     extern_char_ptr@CAPTABLE@somelib.a(foo.o)   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// PER-FILE-ARCHIVE-NEXT:   0x60     global_int@CAPTABLE@somelib.a(foo.o)   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// PER-FILE-ARCHIVE-NEXT:   0x70     extern_char_ptr@CAPTABLE@somelib.a(bar.o)   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// PER-FILE-ARCHIVE:   0x80     global_int@CAPTABLE@somelib.a(bar.o)   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// PER-FILE-NEXT: ]

// Less entries and relocations with a global captable:
// GLOBAL: Relocations [
// GLOBAL-NEXT:   Section (7) .rel.dyn {
// GLOBAL-NEXT:     0x20050 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// GLOBAL-NEXT:     0x20058 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// GLOBAL-NEXT:     0x20030 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_int 0x0 (real addend unknown)
// GLOBAL-NEXT:   }
// GLOBAL-NEXT:   Section (8) .rel.plt {
// GLOBAL-NEXT:     0x20000 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// GLOBAL-NEXT:     0x20010 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// GLOBAL-NEXT:     0x20020 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int 0x0 (real addend unknown)
// GLOBAL-NEXT:   }
// GLOBAL-NEXT: ]
// GLOBAL-LABEL: CHERI .captable [
// GLOBAL-NEXT:   0x0      extern_void_ptr@CAPTABLE        R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// GLOBAL-NEXT:   0x10     extern_char_ptr@CAPTABLE        R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// GLOBAL-NEXT:   0x20     extern_int@CAPTABLE             R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_int
// GLOBAL-NEXT:   0x30     global_int@CAPTABLE             R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// GLOBAL-NEXT:   0x40     function3@CAPTABLE.4
// GLOBAL-NEXT: ]


extern void *extern_void_ptr(void);
extern char *extern_char_ptr(void);
extern int extern_int(void);
extern int global_int;

#ifdef FILE1
#include "per-function-table.c"
#endif

// TODO: File2 and File3 use the same globals -> could share a table but that's not implemented yet
#ifdef FILE2
void *file2_charptr(void) { return extern_char_ptr(); }
int file2_global_int(void) { return global_int; }
#endif
#ifdef FILE3
void *file3_charptr(void) { return extern_char_ptr() + global_int; }
#endif
