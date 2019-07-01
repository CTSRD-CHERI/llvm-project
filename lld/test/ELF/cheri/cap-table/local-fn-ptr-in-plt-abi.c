// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -o %t.o %s
// RUN: llvm-objdump -d -r %t.o | FileCheck %s -check-prefix DISAS
// Should have a R_MIPS_CHERI_CAPTAB20 relocation against the function pointer and a R_MIPS_CHERI_CAPCALL20 against use_callback
// DISAS:          clcbi	$c3, 0($c1)
// DISAS-NEXT: 000000000000001c:  R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE	return1
// DISAS-NEXT:     clcbi	$c12, 0($c1)
// DISAS-NEXT: 0000000000000020:  R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE	use_callback
// DISAS:          clcbi	$c12, 0($c1)
// DISAS-NEXT: 000000000000003c:  R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE	use_callback
// DISAS-NEXT:     clcbi	$c3, 0($c1)
// DISAS-NEXT: 0000000000000040:  R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE	global_return2

// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -o %t-shlib.o -DSHLIB %s
// Build a shared library that uses the function pointer:
// RUN: ld.lld -shared %t-shlib.o -o %t-shlib.so
// RUN: llvm-objdump --syms %t-shlib.so | FileCheck %s -check-prefix SHLIB-DUMP
// SHLIB-DUMP: 0000000000010000 g     F .text		 000000{{[0-9a-f]+}} use_callback


// Should not build with the --building-freebsd-rtld flag
// Check that we emit a R_CHERI_CAPABILITY relocation instead of __cap_relocs for shlib/pie/dynamically linked exe
// RUN: ld.lld -shared %t.o %t-shlib.so -o %t.so  --verbose-cap-relocs 2>&1 | FileCheck %s -check-prefixes VERBOSE-MSG,SHLIB-MSG
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t.so | FileCheck %s -check-prefixes CHECK,CHECK-SHLIB
// RUN: ld.lld -pie %t.o %t-shlib.so -o %t-pie.exe --verbose-cap-relocs 2>&1 | FileCheck %s -check-prefixes VERBOSE-MSG,EXE-MSG
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t-pie.exe | FileCheck %s -check-prefixes CHECK,CHECK-NODYN
// RUN: ld.lld %t.o %t-shlib.so -o %t.exe --verbose-cap-relocs 2>&1 | FileCheck %s -check-prefixes VERBOSE-MSG,EXE-MSG
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t.exe | FileCheck %s -check-prefixes CHECK,CHECK-NODYN
// VERBOSE-MSG:      Using trampoline for function pointer against local function return1
// VERBOSE-MSG-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(return1))
// VERBOSE-MSG-NEXT: Adding new symbol __cheri_fnptr_return1 to allow relocation against local function return1
// VERBOSE-MSG-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(return1))
// VERBOSE-MSG-NEXT: Using trampoline for function pointer against function global_return2
// VERBOSE-MSG-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c
// For executables without --export-dynamic we add a new STV_HIDDEN symbol:
// EXE-MSG-NEXT: Adding new symbol __cheri_fnptr_global_return2 to allow relocation against function global_return2
// EXE-MSG-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(global_return2))
// VERBOSE-MSG-EMPTY:

// CHECK-LABEL: Dynamic Relocations {
// We need a R_MIPS_CHERI_CAPABILITY relocation against return1 to ensure that the dynamic linker creates a unique trampoline
// CHECK-NEXT:    0x{{12002|2}}0000 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __cheri_fnptr_return1 0x0
// R_MIPS_CHERI_CAPABILITY against the global function pointer:
// CHECK-SHLIB:   0x{{12002|2}}0020 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE global_return2 0x0
// In executables with --export-dynamic we add a new hidden symbol:
// CHECK-NODYN:     0x{{12002|2}}0020 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __cheri_fnptr_global_return2 0x0
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
// Check that we added a "fake" symbol for the local return1 symbol:
// CHECK-NEXT:     Name: __cheri_fnptr_return1 ({{.+}})
// CHECK-NEXT:     Value: 0x{{12001|1}}0080
// CHECK-NEXT:     Size: 12
// CHECK-NEXT:     Binding: Local (0x0)
// CHECK-NEXT:     Type: Function (0x2)
// CHECK-NEXT:     Other [ (0x2)
// CHECK-NEXT:       STV_HIDDEN (0x2)
// CHECK-NEXT:     ]
// CHECK-NEXT:     Section: .text (0x{{.+}})
// CHECK-NEXT:   }
// For executables without --export-dynamic we also create a "fake" symbol alias for global_return2
// CHECK-NODYN-NEXT:   Symbol {
// CHECK-NODYN-NEXT:     Name: __cheri_fnptr_global_return2 ({{.+}})
// CHECK-NODYN-NEXT:     Value: 0x{{12001|1}}0000
// CHECK-NODYN-NEXT:     Size: 12
// CHECK-NODYN-NEXT:     Binding: Local (0x0)
// CHECK-NODYN-NEXT:     Type: Function (0x2)
// CHECK-NODYN-NEXT:     Other [ (0x2)
// CHECK-NODYN-NEXT:       STV_HIDDEN (0x2)
// CHECK-NODYN-NEXT:     ]
// CHECK-NODYN-NEXT:     Section: .text (0x{{.+}})
// CHECK-NODYN-NEXT:   }
// CHECK-SHLIB-NEXT:   Symbol {
// CHECK-SHLIB-NEXT:     Name: global_return2 ({{.+}})
// CHECK-SHLIB-NEXT:     Value: 0x{{12001|1}}0000
// CHECK-SHLIB-NEXT:     Size: 12
// CHECK-SHLIB-NEXT:     Binding: Global (0x1)
// CHECK-SHLIB-NEXT:     Type: Function (0x2)
// CHECK-SHLIB-NEXT:     Other: 0
// CHECK-SHLIB-NEXT:     Section: .text (0x{{.+}})
// CHECK-SHLIB-NEXT:   }
// CHECK-NEXT:   Symbol {
// CHECK-NEXT:     Name: use_callback ({{.+}})
// CHECK-NEXT:     Value: 0x0
// CHECK-NEXT:     Size: 0
// CHECK-NEXT:     Binding: Global (0x1)
// CHECK-NEXT:     Type: Function (0x2)
// CHECK-NEXT:     Other: 0
// CHECK-NEXT:     Section: Undefined (0x0)
// CHECK-NEXT:   }
// CHECK-NEXT: ]
// __cap_relocs should be empty! we need a dynamic relocaiton for return1 since we are using it as a function pointer
// CHECK-NEXT: There is no __cap_relocs section in the file.
// CHECK-NEXT: CHERI .captable [
// CHECK-NEXT:   0x0      return1@CAPTABLE.0              R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against __cheri_fnptr_return1
// CHECK-NEXT:   0x10     use_callback@CAPTABLE           R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE against use_callback
// CHECK-NEXT:   0x20     global_return2@CAPTABLE
// CHECK-SHLIB-SAME: R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against global_return2
// CHECK-NODYN-SAME: R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against __cheri_fnptr_global_return2
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
// STATIC-NEXT:     0x120020000 (return1@CAPTABLE.0) Base: 0x120010080 (return1+0) Length: 12 Perms: Function
// STATIC-NEXT:     0x120020010 (use_callback@CAPTABLE) Base: 0x120010090 (use_callback+0) Length: {{[0-9]+}} Perms: Function
// STATIC-NEXT:     0x120020020 (global_return2@CAPTABLE) Base: 0x120010000 (global_return2+0) Length: 12 Perms: Function
// STATIC-NEXT:  ]
// STATIC-NEXT:  CHERI .captable [
// STATIC-NEXT:    0x0      return1@CAPTABLE.0
// STATIC-NEXT:    0x10     use_callback@CAPTABLE
// STATIC-NEXT:    0x20     global_return2@CAPTABLE
// STATIC-NEXT:  ]

// Should not build as RTLD (due to the R_CHERI_CAPABILITY relocation)
// RUN: not ld.lld -Bsymbolic -shared --building-freebsd-rtld %t.o %t-shlib.o --verbose-cap-relocs -o /dev/null 2>&1 | FileCheck %s -check-prefix RTLD-ERROR
// RTLD-ERROR:      Using trampoline for function pointer against local function return1
// RTLD-ERROR-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(return1))
// RTLD-ERROR-NEXT: ld.lld: error: relocation R_MIPS_CHERI_CAPABILITY against local function return1
// RTLD-ERROR-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(return1)) cannot be using when building FreeBSD RTLD
// RTLD-ERROR-NEXT: >>> referenced by return1@CAPTABLE.0
// RTLD-ERROR-NEXT: >>> first used in function __start
// RTLD-ERROR-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(__start))

// RTLD-ERROR-NEXT: Using trampoline for function pointer against function global_return2
// RTLD-ERROR-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(global_return2))
// RTLD-ERROR-EMPTY:
// RTLD-ERROR-NEXT: ld.lld: error: relocation R_MIPS_CHERI_CAPABILITY against function global_return2
// RTLD-ERROR-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(global_return2)) cannot be using when building FreeBSD RTLD
// RTLD-ERROR-NEXT: >>> referenced by global_return2@CAPTABLE
// RTLD-ERROR-NEXT: >>> first used in function __start
// RTLD-ERROR-NEXT: >>> defined in local-fn-ptr-in-plt-abi.c ({{.+}}local-fn-ptr-in-plt-abi.c.tmp.o:(__start))
// RTLD-ERROR-EMPTY:

// Check that we don't crash when a version script marks a symbol as non-preemptible:
// RUN: echo "VERSION_1.0 { local: *; };" > %t.script
// RUN: ld.lld -shared %t.o %t-shlib.so --verbose-cap-relocs -o %t-version-script.so --version-script %t.script
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t-version-script.so | FileCheck %s -check-prefixes CHECK,CHECK-NODYN

typedef int (*func_ptr)(void);
int use_callback(func_ptr ptr);

#ifdef SHLIB
int use_callback(func_ptr ptr) { return ptr(); }
#else

static __attribute__((noinline)) int return1(void) { return 1; }

__attribute__((noinline, visibility("default"))) int global_return2(void) { return 2; }


__attribute__((visibility("hidden"))) int __start(void) {
  // Using a call can emit a __cap_relocs_entry
  // int x = return1();
  return use_callback(&return1) + use_callback(&global_return2);
}
#endif
