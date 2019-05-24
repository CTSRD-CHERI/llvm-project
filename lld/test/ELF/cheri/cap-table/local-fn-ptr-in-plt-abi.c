// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -o %t.o %s
// RUN: llvm-objdump -d -r %t.o | FileCheck %s -check-prefix DISAS
// Should have a R_MIPS_CHERI_CAPTAB20 relocation against the function pointer and a R_MIPS_CHERI_CAPCALL20 against use_callback
// DISAS:      c:	74 61 00 00 	clcbi	$c3, 0($c1)
// DISAS: 000000000000000c:  R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE	return1
// DISAS:     10:	75 81 00 00 	clcbi	$c12, 0($c1)
// DISAS: 0000000000000010:  R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE	use_callback
// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -o %t-shlib.o -DSHLIB %s
// Build a shared library that uses the function pointer:
// RUN: ld.lld -shared %t-shlib.o -o %t-shlib.so
// RUN: llvm-objdump --syms %t-shlib.so | FileCheck %s -check-prefix SHLIB-DUMP
// SHLIB-DUMP: 0000000000010000 g     F .text		 00000050 use_callback


// Should not build with the --building-freebsd-rtld flag
// Check that we emit a R_CHERI_CAPABILITY relocation instead of __cap_relocs for shlib/pie/dynamically linked exe
// RUN: ld.lld -shared %t.o %t-shlib.so -o %t.so  --verbose-cap-relocs 2>&1 | FileCheck %s -check-prefix VERBOSE-MSG
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t.so | FileCheck %s -check-prefix CHECK
// RUN: ld.lld -pie %t.o %t-shlib.so -o %t-pie.exe --verbose-cap-relocs 2>&1 | FileCheck %s -check-prefix VERBOSE-MSG
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t-pie.exe | FileCheck %s
// RUN: ld.lld %t.o %t-shlib.so -o %t.exe --verbose-cap-relocs 2>&1 | FileCheck %s -check-prefix VERBOSE-MSG
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t.exe | FileCheck %s
// VERBOSE-MSG: Using preemptible relocation for function pointer against local function return1
// VERBOSE-MSG-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(return1))
// VERBOSE-MSG-NEXT: Adding new symbol __cheri_fnptr_return1 to allow relocation against local symbol local function return1
// VERBOSE-MSG-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(return1))
// VERBOSE-MSG-EMPTY:

// CHECK-LABEL: Dynamic Relocations {
// We need a R_MIPS_CHERI_CAPABILITY relocation against return1 to ensure that the dynamic linker creates a unique trampoline
// CHECK-NEXT:    0x{{12002|2}}0000 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __cheri_fnptr_return1 0x0
// We need a R_MIPS_CHERI_CAPABILITY_CALL relocation against use_callback to call a function in another DSO
// CHECK-NEXT:    0x{{12002|2}}0010 R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE use_callback 0x0
// CHECK-NEXT:  }
// CHECK-LABEL: DynamicSymbols [
// CHECK-NEXT:   Symbol {
// CHECK-NEXT:     Name:  (0)
// CHECK-NEXT:     Value: 0x0
// CHECK-NEXT:     Size: 0
// CHECK-NEXT:     Binding: Local (0x0)
// CHECK-NEXT:     Type: None (0x0)
// CHECK-NEXT:     Other: 0
// CHECK-NEXT:     Section: Undefined (0x0)
// CHECK-NEXT:   }
// CHECK-NEXT:   Symbol {
// CHECK-NEXT:     Name: use_callback ({{.+}})
// CHECK-NEXT:     Value: 0x0
// CHECK-NEXT:     Size: 0
// CHECK-NEXT:     Binding: Global (0x1)
// CHECK-NEXT:     Type: Function (0x2)
// CHECK-NEXT:     Other: 0
// CHECK-NEXT:     Section: Undefined (0x0)
// CHECK-NEXT:   }
// CHECK-NEXT:   Symbol {
// Check that we added a "fake" symbol for the local return1 symbol:
// CHECK-NEXT:     Name: __cheri_fnptr_return1 ({{.+}})
// CHECK-NEXT:     Value: 0x{{12001|1}}0040
// CHECK-NEXT:     Size: 12
// CHECK-NEXT:     Binding: Local (0x0)
// CHECK-NEXT:     Type: Function (0x2)
// CHECK-NEXT:     Other [ (0x2)
// CHECK-NEXT:       STV_HIDDEN (0x2)
// CHECK-NEXT:     ]
// CHECK-NEXT:     Section: .text (0xA)
// CHECK-NEXT:   }
// CHECK-NEXT: ]
// __cap_relocs should be empty! we need a dynamic relocaiton for return1 since we are using it as a function pointer
// CHECK-NEXT: There is no __cap_relocs section in the file.
// CHECK-NEXT: CHERI .captable [
// CHECK-NEXT:   0x0      return1@CAPTABLE.0              R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against __cheri_fnptr_return1
// CHECK-NEXT:   0x10     use_callback@CAPTABLE           R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against use_callback
// CHECK-NEXT: ]

// no need for trampolines in static executable
// RUN: ld.lld %t.o %t-shlib.o -o %t-static.exe --verbose-cap-relocs 2>&1 | FileCheck %s -check-prefix STATIC-MESSAGE
// STATIC-MESSAGE: Do not need function pointer trampoline for return1 in static binary

// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t-static.exe | FileCheck %s -check-prefix STATIC
// STATIC-LABEL: Dynamic Relocations {
// STATIC-NEXT:  }
// STATIC-NEXT:  DynamicSymbols [
// STATIC-NEXT:  ]
// STATIC-NEXT:  CHERI __cap_relocs [
// STATIC-NEXT:     0x120020000 (return1@CAPTABLE.0) Base: 0x120010040 (return1+0) Length: 12 Perms: Function
// STATIC-NEXT:     0x120020010 (use_callback@CAPTABLE) Base: 0x120010050 (use_callback+0) Length: 80 Perms: Function
// STATIC-NEXT:  ]
// STATIC-NEXT:  CHERI .captable [
// STATIC-NEXT:    0x0      return1@CAPTABLE.0
// STATIC-NEXT:    0x10     use_callback@CAPTABLE
// STATIC-NEXT:  ]

// Should not build as RTLD (due to the R_CHERI_CAPABILITY relocation)
// RUN: not ld.lld -Bsymbolic -shared --building-freebsd-rtld %t.o %t-shlib.o --verbose-cap-relocs -o /dev/null 2>&1 | FileCheck %s -check-prefix RTLD-ERROR
// RTLD-ERROR:      Using preemptible relocation for function pointer against local function return1
// RTLD-ERROR-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(return1))
// RTLD-ERROR-NEXT: ld.lld: error: relocation R_MIPS_CHERI_CAPABILITY against local function return1
// RTLD-ERROR-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(return1)) cannot be using when building FreeBSD RTLD
// RTLD-ERROR-NEXT: >>> referenced by return1@CAPTABLE.0
// RTLD-ERROR-NEXT: >>> first used in function __start
// RTLD-ERROR-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(__start))

typedef int (*func_ptr)(void);
int use_callback(func_ptr ptr);

#ifdef SHLIB
int use_callback(func_ptr ptr) { return ptr(); }
#else

static __attribute__((noinline)) int return1(void) { return 1; }

__attribute__((visibility("hidden"))) int __start(void) {
  // Using a call can emit a __cap_relocs_entry
  // int x = return1();
  return use_callback(&return1);
}
#endif
