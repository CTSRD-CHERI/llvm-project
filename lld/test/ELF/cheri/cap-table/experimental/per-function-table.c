
// REQUIRES: clang

// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -O2 %s -o %t.o
// RUN: ld.lld -shared -o %t.so %t.o -captable-scope=function
// RUN: llvm-readobj --cap-relocs --cap-table -dynamic-table -r %t.so | FileCheck %s -check-prefixes CHECK,PER-FUNCTION
// RUN: llvm-objdump --syms -d %t.so | FileCheck %s -check-prefixes SYMBOLS,DISAS
// RUN: ld.lld -shared -o %t-file.so %t.o -captable-scope=file
// RUN: llvm-readobj --cap-relocs --cap-table -dynamic-table -r %t-file.so | FileCheck %s -check-prefixes CHECK,PER-FILE
// RUN: ld.lld -shared -o %t-all.so %t.o -captable-scope=all
// RUN: llvm-readobj --cap-relocs --cap-table -dynamic-table -r %t-all.so | FileCheck %s -check-prefixes CHECK,GLOBAL

// CHECK-DYNAMIC:   0x000000007000C002 MIPS_CHERI_FLAGS ABI_PLT CAPTABLE_PER_FUNC

// CHECK:       Relocations [
// CHECK-NEXT:   Section ({{.+}}) .rel.dyn {
// CHECK-NEXT:     0x300{{9|5}}0 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// CHECK-NEXT:     0x300{{9|5}}8 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// CHECK-NEXT:     0x300{{4|3}}0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_int 0x0 (real addend unknown)
// CHECK-NEXT:   }
// CHECK-NEXT:   Section ({{.+}}) .rel.plt {
// PER-FUNCTION-NEXT:     0x30000 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// PER-FUNCTION-NEXT:     0x30010 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// PER-FUNCTION-NEXT:     0x30020 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// PER-FUNCTION-NEXT:     0x30030 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int 0x0 (real addend unknown)
// PER-FUNCTION-NEXT:     0x30050 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// PER-FUNCTION-NEXT:     0x30070 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// PER-FUNCTION-NEXT:     0x30080 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int 0x0 (real addend unknown)

// PER-FILE-NEXT:    0x30000 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// PER-FILE-NEXT:    0x30010 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// PER-FILE-NEXT:    0x30020 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int 0x0 (real addend unknown)

// GLOBAL-NEXT:    0x30000 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// GLOBAL-NEXT:    0x30010 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// GLOBAL-NEXT:    0x30020 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int 0x0 (real addend unknown)

// CHECK-NEXT:   }
// CHECK-NEXT: ]
// CHECK-LABEL: DynamicSection [ (25 entries)
// CHECK-NEXT:    Tag                Type                 Name/Value
// PER-FUNCTION:  0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PLT CAPTABLE_PER_FUNC
// PER-FILE:      0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PLT CAPTABLE_PER_FILE
// GLOBAL:        0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PLT{{$}}
// CHECK:      ]
// CHECK-NEXT: CHERI __cap_relocs [
// PER-FUNCTION-NEXT:    0x030060 (function3@CAPTABLE@x.6) Base: 0x100d8 (function3+0) Length: 72 Perms: Function
// PER-FILE-NEXT:        0x030040 (function3@CAPTABLE@per-function-table.c.tmp.o.4) Base: 0x100d8 (function3+0) Length: 72 Perms: Function
// GLOBAL-NEXT:          0x030040 (function3@CAPTABLE.4) Base: 0x100d8 (function3+0) Length: 72 Perms: Function
// CHECK-NEXT: ]
// CHECK-NEXT:        CHERI .captable [
// PER-FUNCTION-NEXT:   0x0      extern_void_ptr@CAPTABLE@function1   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// PER-FUNCTION-NEXT:   0x10     extern_char_ptr@CAPTABLE@function2   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// PER-FUNCTION-NEXT:   0x20     extern_void_ptr@CAPTABLE@function4   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// PER-FUNCTION-NEXT:   0x30     extern_int@CAPTABLE@function4   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_int
// PER-FUNCTION-NEXT:   0x40     global_int@CAPTABLE@function5   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// PER-FUNCTION-NEXT:   0x50     extern_void_ptr@CAPTABLE@same_globals_as_function1   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// PER-FUNCTION-NEXT:   0x60     function3@CAPTABLE@x.6
// PER-FUNCTION-NEXT:   0x70     extern_char_ptr@CAPTABLE@function3   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// PER-FUNCTION-NEXT:   0x80     extern_int@CAPTABLE@function3   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_int

// PER-FILE-NEXT:       0x0      extern_void_ptr@CAPTABLE@per-function-table.c.tmp.o   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// PER-FILE-NEXT:       0x10     extern_char_ptr@CAPTABLE@per-function-table.c.tmp.o   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// PER-FILE-NEXT:       0x20     extern_int@CAPTABLE@per-function-table.c.tmp.o   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_int
// PER-FILE-NEXT:       0x30     global_int@CAPTABLE@per-function-table.c.tmp.o   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// PER-FILE-NEXT:       0x40     function3@CAPTABLE@per-function-table.c.tmp.o.4

// GLOBAL-NEXT:         0x0      extern_void_ptr@CAPTABLE   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// GLOBAL-NEXT:         0x10     extern_char_ptr@CAPTABLE   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// GLOBAL-NEXT:         0x20     extern_int@CAPTABLE   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_int
// GLOBAL-NEXT:         0x30     global_int@CAPTABLE   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// GLOBAL-NEXT:         0x40     function3@CAPTABLE.4

// CHECK-NEXT:        ]

extern void *extern_void_ptr(void);
extern char *extern_char_ptr(void);
extern int extern_int(void);
extern int global_int;

// Check that the indices for the per-function table are correct (should be 0 or 16)

// DISAS-LABEL: Disassembly of section .text:
void *function1(void) { return extern_void_ptr(); }
// DISAS-LABEL: function1:
// DISAS:      clcbi	$c12, 0($c26)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

__attribute__((visibility("protected"))) void *function2(void) {
  return extern_char_ptr();
}
// DISAS-LABEL: function2:
// DISAS:      clcbi	$c12, 0($c26)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

__attribute__((visibility("protected"))) void *function4(void) {
  return (char *)extern_void_ptr() + extern_int();
}
// DISAS-LABEL: function4:
// DISAS:      cmove	$c19, $c26
// DISAS:      clcbi	$c12, 0($c19)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      clcbi	$c12, 16($c19)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

int function5(void) { return global_int; }
// DISAS-LABEL: function5:
// DISAS-NEXT: clcbi	$c1, 0($c26)
// DISAS-NEXT: cjr	$c17
// DISAS-NEXT: clw	$2, $zero, 0($c1)

// TODO: this could share a table with function1 but that's not implemented yet
void *same_globals_as_function1(void) { return (char *)extern_void_ptr(); }
// DISAS-LABEL: same_globals_as_function1:
// DISAS:      clcbi	$c12, 0($c26)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

static void *function3(void);
void *x(void) { return function3(); }
// DISAS-LABEL: x:
// DISAS:      clcbi	$c12, 0($c26)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

__attribute__((noinline)) static void *function3(void) {
  return extern_char_ptr() + extern_int();
}
// DISAS-LABEL: function3:
// DISAS:      cmove	$c19, $c26
// DISAS:      clcbi	$c12, 0($c19)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      clcbi	$c12, 16($c19)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17


// SYMBOLS-LABEL: SYMBOL TABLE:
// SYMBOLS: 0000000000030000         .captable		 00000090 _CHERI_CAPABILITY_TABLE_
// SYMBOLS: 0000000000030000 l     O .captable		 00000010 extern_void_ptr@CAPTABLE@function1
// SYMBOLS: 0000000000030010 l     O .captable		 00000010 extern_char_ptr@CAPTABLE@function2
// SYMBOLS: 0000000000030020 l     O .captable		 00000010 extern_void_ptr@CAPTABLE@function4
// SYMBOLS: 0000000000030030 l     O .captable		 00000010 extern_int@CAPTABLE@function4
// SYMBOLS: 0000000000030040 l     O .captable		 00000010 global_int@CAPTABLE@function5
// SYMBOLS: 0000000000030050 l     O .captable		 00000010 extern_void_ptr@CAPTABLE@same_globals_as_function1
// SYMBOLS: 0000000000030060 l     O .captable		 00000010 function3@CAPTABLE@x.6
// SYMBOLS: 0000000000030070 l     O .captable		 00000010 extern_char_ptr@CAPTABLE@function3
// SYMBOLS: 0000000000030080 l     O .captable		 00000010 extern_int@CAPTABLE@function3
