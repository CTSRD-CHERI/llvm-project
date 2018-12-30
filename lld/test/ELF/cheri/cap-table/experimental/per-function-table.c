
// REQUIRES: clang

// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -O2 %s -o %t.o
// RUN: ld.lld -shared -o %t.so %t.o -captable-scope=function
// RUN: llvm-readobj --cap-relocs --cap-table -dynamic-table -r %t.so | FileCheck %s
// RUN: llvm-objdump --syms %t.so | FileCheck %s -check-prefix SYMBOLS

// CHECK-DYNAMIC:   0x000000007000C002 MIPS_CHERI_FLAGS ABI_PLT CAPTABLE_PER_FUNC

// CHECK:       Relocations [
// CHECK-NEXT:   Section ({{.+}}) .rel.dyn {
// CHECK-NEXT:     0x30090 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// CHECK-NEXT:     0x30098 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// CHECK-NEXT:     0x30040 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_int 0x0 (real addend unknown)
// CHECK-NEXT:   }
// CHECK-NEXT:   Section ({{.+}}) .rel.plt {
// CHECK-NEXT:     0x30000 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// CHECK-NEXT:     0x30010 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// CHECK-NEXT:     0x30020 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// CHECK-NEXT:     0x30030 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int 0x0 (real addend unknown)
// CHECK-NEXT:     0x30050 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_void_ptr 0x0 (real addend unknown)
// CHECK-NEXT:     0x30070 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_char_ptr 0x0 (real addend unknown)
// CHECK-NEXT:     0x30080 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_int 0x0 (real addend unknown)
// CHECK-NEXT:   }
// CHECK-NEXT: ]
// CHECK-LABEL: DynamicSection [ (25 entries)
// CHECK-NEXT:    Tag                Type                 Name/Value
// CHECK:         0x000000007000C002 MIPS_CHERI_FLAGS     ABI_PLTCAPTABLE_PER_FUNC
// CHECK:      ]
// CHECK-NEXT: CHERI __cap_relocs [
// CHECK-NEXT:    0x030060 (function3@CAPTABLE@x.6) Base: 0x100d8 (function3+0) Length: 72 Perms: Function
// CHECK-NEXT: ]
// CHECK-NEXT: CHERI .captable [
// CHECK-NEXT:   0x0      extern_void_ptr@CAPTABLE@function1   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// CHECK-NEXT:   0x10     extern_char_ptr@CAPTABLE@function2   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// CHECK-NEXT:   0x20     extern_void_ptr@CAPTABLE@function4   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// CHECK-NEXT:   0x30     extern_int@CAPTABLE@function4   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_int
// CHECK-NEXT:   0x40     global_int@CAPTABLE@function5   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_int
// CHECK-NEXT:   0x50     extern_void_ptr@CAPTABLE@same_globals_as_function1   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_void_ptr
// CHECK-NEXT:   0x60     function3@CAPTABLE@x.6
// CHECK-NEXT:   0x70     extern_char_ptr@CAPTABLE@function3   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_char_ptr
// CHECK-NEXT:   0x80     extern_int@CAPTABLE@function3   R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against extern_int
// CHECK-NEXT: ]

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

extern void *extern_void_ptr(void);
extern char *extern_char_ptr(void);
extern int extern_int(void);
extern int global_int;

void *function1(void) { return extern_void_ptr(); }

__attribute__((visibility("protected"))) void *function2(void) {
  return extern_char_ptr();
}

__attribute__((noinline)) static void *function3(void) {
  return extern_char_ptr() + extern_int();
}

__attribute__((visibility("protected"))) void *function4(void) {
  return (char *)extern_void_ptr() + extern_int();
}

int function5(void) { return global_int; }

// TODO: this could share a table with function1 but that's not implemented yet
void *same_globals_as_function1(void) { return (char *)extern_void_ptr(); }

void *x(void) { return function3(); }
