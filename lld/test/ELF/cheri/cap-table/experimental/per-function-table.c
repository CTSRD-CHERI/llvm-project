
// REQUIRES: clang, mips

// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -O2 %s -o %t.o
// RUN: ld.lld -z now -shared -o %t.so %t.o -captable-scope=function -z captabledebug
// RUN: llvm-readobj --cap-relocs --cap-table --dynamic-table -r %t.so | FileCheck %s --check-prefixes CHECK,PER-FUNCTION
// RUN: llvm-objdump --syms -d %t.so | FileCheck %s --check-prefixes SYMBOLS,DISAS
// RUN: ld.lld -z now -shared -o %t-file.so %t.o -captable-scope=file
// RUN: llvm-readobj --cap-relocs --cap-table --dynamic-table -r %t-file.so | FileCheck %s --check-prefixes CHECK,PER-FILE
// RUN: ld.lld -z now -shared -o %t-all.so %t.o -captable-scope=all
// RUN: llvm-readobj --cap-relocs --cap-table --dynamic-table -r %t-all.so | FileCheck %s --check-prefixes CHECK,GLOBAL

// CHECK-LABEL: DynamicSection [
// CHECK-NEXT:    Tag                Type                 Name/Value
// PER-FUNCTION:  0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PLT CAPTABLE_PER_FUNC RELATIVE_CAPRELOCS {{$}}
// PER-FUNCTION:  0x000000007000C005 MIPS_CHERI_CAPTABLE_MAPPING 0x6E8
// PER-FUNCTION:  0x000000007000C006 MIPS_CHERI_CAPTABLE_MAPPINGSZ 0xA8
// PER-FILE:      0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PLT CAPTABLE_PER_FILE RELATIVE_CAPRELOCS {{$}}
// PER-FILE:      0x000000007000C005 MIPS_CHERI_CAPTABLE_MAPPING 0x6A8
// PER-FILE:      0x000000007000C006 MIPS_CHERI_CAPTABLE_MAPPINGSZ 0xA8
// GLOBAL:        0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PLT RELATIVE_CAPRELOCS {{$}}
// CHECK:      ]

// CHECK:       Relocations [
// CHECK-NEXT:   Section ({{.+}}) .rel.dyn {
// PER-FUNCTION-NEXT: 0x20960 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_int{{$}}
// PER-FILE-NEXT:     0x20910 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_int{{$}}
// GLOBAL-NEXT:       0x20840 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_int{{$}}
// CHECK-NEXT:   }
// CHECK-NEXT:   Section ({{.+}}) .rel.plt {
// PER-FUNCTION-NEXT:     0x20920 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr{{$}}
// PER-FUNCTION-NEXT:     0x20930 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr{{$}}
// PER-FUNCTION-NEXT:     0x20940 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr{{$}}
// PER-FUNCTION-NEXT:     0x20950 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int{{$}}
// PER-FUNCTION-NEXT:     0x20970 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr{{$}}
// PER-FUNCTION-NEXT:     0x20990 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr{{$}}
// PER-FUNCTION-NEXT:     0x209A0 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int{{$}}

// PER-FILE-NEXT:    0x208E0 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr{{$}}
// PER-FILE-NEXT:    0x208F0 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr{{$}}
// PER-FILE-NEXT:    0x20900 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int{{$}}

// GLOBAL-NEXT:    0x20810 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr{{$}}
// GLOBAL-NEXT:    0x20820 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr{{$}}
// GLOBAL-NEXT:    0x20830 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int{{$}}

// CHECK-NEXT:   }
// CHECK-NEXT: ]
// CHECK-NEXT: CHERI __cap_relocs [
// PER-FUNCTION-NEXT:    0x020980 (function3@CAPTABLE@x.6) Base: 0x10{{.+}} (function3+0) Length: {{.+}} Perms: Function
// PER-FILE-NEXT:        0x020920 (function3@CAPTABLE@per-function-table.c.tmp.o.4) Base: 0x10{{.+}} (function3+0) Length: {{[0-9]+}} Perms: Function
// GLOBAL-NEXT:          0x020850 (function3@CAPTABLE.4) Base: 0x10{{.+}} (function3+0) Length: {{.+}} Perms: Function
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

// SYMBOLS-LABEL: SYMBOL TABLE:
// SYMBOLS: 0000000000020920 l     O .captable		 0000000000000010 extern_void_ptr@CAPTABLE@function1
// SYMBOLS: 0000000000020930 l     O .captable		 0000000000000010 extern_char_ptr@CAPTABLE@function2
// SYMBOLS: 0000000000020940 l     O .captable		 0000000000000010 extern_void_ptr@CAPTABLE@function4
// SYMBOLS: 0000000000020950 l     O .captable		 0000000000000010 extern_int@CAPTABLE@function4
// SYMBOLS: 0000000000020960 l     O .captable		 0000000000000010 global_int@CAPTABLE@function5
// SYMBOLS: 0000000000020970 l     O .captable		 0000000000000010 extern_void_ptr@CAPTABLE@same_globals_as_function1
// SYMBOLS: 0000000000020980 l     O .captable		 0000000000000010 function3@CAPTABLE@x.6
// SYMBOLS: 0000000000020990 l     O .captable		 0000000000000010 extern_char_ptr@CAPTABLE@function3
// SYMBOLS: 00000000000209a0 l     O .captable		 0000000000000010 extern_int@CAPTABLE@function3



// DISAS-LABEL: Disassembly of section .text:
void *function1(void) { return extern_void_ptr(); }
// DISAS: <function1>:
// DISAS: <$captable_load_extern_void_ptr>:
// DISAS-NEXT: clcbi	$c12, 0($c18)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

__attribute__((visibility("protected"))) void *function2(void) {
  return extern_char_ptr();
}
// DISAS: <function2>:
// DISAS: <$captable_load_extern_char_ptr>:
// DISAS-NEXT: clcbi	$c12, 0($c18)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

__attribute__((visibility("protected"))) void *function4(void) {
  return (char *)extern_void_ptr() + extern_int();
}
// DISAS: <function4>:
// DISAS:      cmove	$c19, $c26
// DISAS: <$captable_load_extern_void_ptr>:
// DISAS-NEXT: clcbi	$c12, 0($c19)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS: <$captable_load_extern_int>:
// DISAS-NEXT: clcbi	$c12, 16($c19)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

int function5(void) { return global_int; }
// DISAS: <function5>:
// DISAS-NEXT: clcbi	$c1, 0($c26)
// DISAS-NEXT: cjr	$c17
// DISAS-NEXT: clw	$2, $zero, 0($c1)

// TODO: this could share a table with function1 but that's not implemented yet
void *same_globals_as_function1(void) { return (char *)extern_void_ptr(); }
// DISAS: <same_globals_as_function1>:
// DISAS: <$captable_load_extern_void_ptr>:
// DISAS-NEXT: clcbi	$c12, 0($c18)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

static void *function3(void);
void *x(void) { return function3(); }
// DISAS: <x>:
// DISAS: <$captable_load_function3>:
// DISAS-NEXT: clcbi $c12, 0($c26)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

__attribute__((noinline)) static void *function3(void) {
  return extern_char_ptr() + extern_int();
}
// DISAS: <function3>:
// DISAS:      cmove	$c19, $c26
// DISAS: <$captable_load_extern_char_ptr>:
// DISAS-NEXT: clcbi	$c12, 0($c19)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS: <$captable_load_extern_int>:
// DISAS-NEXT: clcbi	$c12, 16($c19)
// DISAS-NEXT: cjalr	$c12, $c17
// DISAS:      cjr	$c17

// RUN: llvm-readobj --cap-table-mapping %t.so | FileCheck %s --check-prefix READOBJ-MAPPING
// Check that the mapping between functions + captable subsets is sensible:

// READOBJ-MAPPING: CHERI .captable per-file/per-function mapping information [
// READOBJ-MAPPING:   Function start: 0x107C0 (function1) Function end: 0x107F0 .captable offset: 0x0 Length:0x10
// READOBJ-MAPPING:   Function start: 0x107F0 (function2) Function end: 0x10820 .captable offset: 0x10 Length:0x10
// READOBJ-MAPPING:   Function start: 0x10820 (function4) Function end: 0x10870 .captable offset: 0x20 Length:0x20
// READOBJ-MAPPING:   Function start: 0x10870 (function5) Function end: 0x1087C .captable offset: 0x40 Length:0x10
// READOBJ-MAPPING:   Function start: 0x10880 (same_globals_as_function1) Function end: 0x108B0 .captable offset: 0x50 Length:0x10
// READOBJ-MAPPING:   Function start: 0x108B0 (x) Function end: 0x108D0 .captable offset: 0x60 Length:0x10
// READOBJ-MAPPING:   Function start: 0x108D0 (function3) Function end: 0x10920 .captable offset: 0x70 Length:0x20
// READOBJ-MAPPING: ]


// Also check that the raw bytes are correct in addition to the llvm-readobj output
// RUN: llvm-objdump --full-contents --section-headers --syms --section=.captable_mapping %t.so | FileCheck %s --check-prefix MAPPING
// MAPPING: Sections:
// MAPPING-NEXT: Idx Name          Size      VMA          Type
// MAPPING-NEXT:   9 .captable_mapping 000000a8 00000000000006e8 DATA
// MAPPING-EMPTY:
// MAPPING-NEXT: SYMBOL TABLE:
// MAPPING: 00000000[[FUNCTION3_ADDR:000108d0]] l     F .text		 0000000000000050 function3
// MAPPING: 00000000[[FUNCTION1_ADDR:000107c0]] g     F .text		 0000000000000030 function1
// MAPPING: 00000000[[FUNCTION2_ADDR:000107f0]] g     F .text		 0000000000000030 .protected function2
// MAPPING: 00000000[[FUNCTION4_ADDR:00010820]] g     F .text		 0000000000000050 .protected function4
// MAPPING: 00000000[[FUNCTION5_ADDR:00010870]] g     F .text		 000000000000000c function5
// MAPPING: 0000000000000000         *UND*		 0000000000000000 global_int
// MAPPING: 00000000[[SAME_GLOBALS_ADDR:00010880]] g     F .text		 0000000000000030 same_globals_as_function1
// MAPPING: 00000000[[X_ADDR:000108b0]] g     F .text		 0000000000000020 x
// MAPPING: Contents of section .captable_mapping:
// MAPPING-NEXT:  06e8 00000000 [[FUNCTION1_ADDR]]
// MAPPING-SAME:       00000000 000107f0
// MAPPING-NEXT:  06f8 00000000 00000010
// Start addr  000107c0, size 0x20, captable index 0, size 1
// MAPPING-SAME:       00000000 [[FUNCTION2_ADDR]]
// MAPPING-NEXT:  0708 00000000 00010820
// MAPPING-SAME:       00000010 00000010
// Start addr  000107f0, size 0x20, captable index 1, size 1
// MAPPING-NEXT:  0718 00000000 [[FUNCTION4_ADDR]]
// MAPPING-SAME:       00000000 00010870
// MAPPING-NEXT:  0728 00000020 00000020
// Start addr  00010820, size 0x48, captable index 2, size 2
// MAPPING-SAME:       00000000 [[FUNCTION5_ADDR]]
// MAPPING-NEXT:  0738 00000000 0001087c
// MAPPING-SAME:       00000040 00000010
// Start addr  00010870, size 0xc, captable index 4, size 1
// MAPPING-NEXT:  0748 00000000 [[SAME_GLOBALS_ADDR]]
// MAPPING-SAME:       00000000 000108b0
// MAPPING-NEXT:  0758 00000050 00000010
// Start addr  00010880, size 0x20, captable index 5, size 1
// MAPPING-SAME:       00000000 [[X_ADDR]]
// MAPPING-NEXT:  0768 00000000 000108d0
// MAPPING-SAME:       00000060 00000010
// Start addr  000108b0, size 0x20, captable index 6, size 1
// MAPPING-NEXT:  0778 00000000 [[FUNCTION3_ADDR]]
// MAPPING-SAME:       00000000 00010920
// MAPPING-NEXT:  0788 00000070 00000020
// Start addr  000108d0, size 0x48, captable index 7, size 2
