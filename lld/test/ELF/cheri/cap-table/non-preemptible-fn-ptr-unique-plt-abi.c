// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -o %t.o %s
// RUN: llvm-objdump -d -r %t.o | FileCheck %s -check-prefix DISAS -implicit-check-not R_MIPS_CHERI
// Should have a R_MIPS_CHERI_CAPTAB20 relocation against the function pointer and a R_MIPS_CHERI_CAPCALL20 against use_callback
// DISAS: 000000000000001c: R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE default_callback
// DISAS: 0000000000000020: R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE check_if_default
// DISAS: 0000000000000064: R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE static_callback
// DISAS: 0000000000000068: R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE check_if_default
// DISAS: 00000000000000b4: R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE static_callback
// DISAS: 00000000000000b8: R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE check_if_default
// DISAS: 00000000000000f4: R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE static_callback
// DISAS: 00000000000000f8: R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE check_if_default
// DISAS: 0000000000000130: R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE static_callback
// DISAS: 0000000000000134: R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE check_if_default
// DISAS: 00000000000001d8: R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE callbacks
// DISAS: 00000000000001e0: R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE check_if_default

// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -o %t2.o -DOTHER_FILE %s
// RUN: llvm-objdump --syms %t2.o | FileCheck %s -check-prefix OTHER_FILE-SYMS
// OTHER_FILE-SYMS: 0000000000000000 g F .text 00000028 .protected check_if_default

// Check that we emit a R_CHERI_CAPABILITY relocation instead of __cap_relocs for shlib/pie/dynamically linked exe
// Also check that we only emit exactly one fake symbol for each function pointer!

// RUN: ld.lld -shared %t.o %t2.o -o %t.so --verbose-cap-relocs
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t.so | FileCheck %s -check-prefixes CHECK,CHECK-SHLIB,CHECK-NO-VERSION-SCRIPT
// Should be the same for -Bsymbolic
// RUN: ld.lld -Bsymbolic -shared %t.o %t2.o -o %t.so --verbose-cap-relocs
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t.so | FileCheck %s -check-prefixes CHECK,CHECK-SHLIB,CHECK-NO-VERSION-SCRIPT

// Create an empty .so file just to force non-static linking:
// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -o %t-empty.o -xc /dev/null
// RUN: ld.lld -shared %t-empty.o -o %t-empty-shlib.so
// RUN: ld.lld -pie %t.o %t2.o %t-empty-shlib.so -o %t-pie.exe  --verbose-cap-relocs
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t-pie.exe | FileCheck %s -check-prefixes CHECK,CHECK-NONPREEMPTIBLE
// RUN: ld.lld %t.o %t2.o %t-empty-shlib.so -o %t.exe  --verbose-cap-relocs
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t.exe | FileCheck %s -check-prefixes CHECK,CHECK-NONPREEMPTIBLE

// CHECK-LABEL: Dynamic Relocations {
// With the version script default_callback comes first
// CHECK-NONPREEMPTIBLE-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __cheri_fnptr_default_callback 0x0
// CHECK-NONPREEMPTIBLE-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __cheri_fnptr_default_callback 0x0
// one reference in .captable
// CHECK-NONPREEMPTIBLE-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __cheri_fnptr_default_callback 0x0
// We need exactly one R_MIPS_CHERI_CAPABILITY relocation against static_callback to ensure that
// the dynamic linker creates a unique trampoline
// two global references to static_callback:
// CHECK-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __cheri_fnptr_static_callback 0x0
// CHECK-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __cheri_fnptr_static_callback 0x0
// one reference in .captable
// CHECK-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __cheri_fnptr_static_callback 0x0

// The function pointer to the global function should be in .captable once and twice in .rodata
// CHECK-NO-VERSION-SCRIPT-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE default_callback 0x0
// CHECK-NO-VERSION-SCRIPT-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE default_callback 0x0
// one reference in .captable
// CHECK-NO-VERSION-SCRIPT-NEXT:   R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE default_callback 0x0

// We need a R_MIPS_CHERI_CAPABILITY_CALL relocation against use_callback to call a function (potentially) in another DSO
// CHECK-SHLINEXT:     R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE check_if_default 0x0
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
// With the version script/when building an executable we also need to add a fake symbol for default_callback
// CHECK-NONPREEMPTIBLE-NEXT:   Symbol {
// CHECK-NONPREEMPTIBLE-NEXT:     Name: __cheri_fnptr_default_callback ({{.+}})
// CHECK-NONPREEMPTIBLE-NEXT:     Value: 0x{{12001|1}}0000
// CHECK-NONPREEMPTIBLE-NEXT:     Size: 12
// CHECK-NONPREEMPTIBLE-NEXT:     Binding: Local (0x0)
// CHECK-NONPREEMPTIBLE-NEXT:     Type: Function (0x2)
// CHECK-NONPREEMPTIBLE-NEXT:     Other [ (0x2)
// CHECK-NONPREEMPTIBLE-NEXT:       STV_HIDDEN (0x2)
// CHECK-NONPREEMPTIBLE-NEXT:     ]
// CHECK-NONPREEMPTIBLE-NEXT:     Section: .text (0x
// CHECK-NONPREEMPTIBLE-NEXT:   }
// CHECK-NEXT:   Symbol {
// Check that we added a "fake" symbol for the local return1 symbol:
// CHECK-NEXT:     Name: __cheri_fnptr_static_callback ({{.+}})
// CHECK-NEXT:     Value: 0x{{12001|1}}0098
// CHECK-NEXT:     Size: 12
// CHECK-NEXT:     Binding: Local (0x0)
// CHECK-NEXT:     Type: Function (0x2)
// CHECK-NEXT:     Other [ (0x2)
// CHECK-NEXT:       STV_HIDDEN (0x2)
// CHECK-NEXT:     ]
// CHECK-NEXT:     Section: .text (0x
// CHECK-NEXT:   }
// CHECK-SHLIB-NEXT:  Symbol {
// CHECK-SHLIB-NEXT:    Name: check_if_default (
// CHECK-SHLIB-NEXT:    Value: 0x{{12001|1}}0270
// CHECK-SHLIB-NEXT:    Size: 52
// CHECK-SHLIB-NEXT:    Binding: Global (0x1)
// CHECK-SHLIB-NEXT:    Type: Function (0x2)
// CHECK-SHLIB-NEXT:    Other [ (0x3)
// CHECK-SHLIB-NEXT:      STV_PROTECTED (0x3)
// CHECK-SHLIB-NEXT:    ]
// CHECK-SHLIB-NEXT:    Section: .text (0x
// CHECK-SHLIB-NEXT:  }
// CHECK-NO-VERSION-SCRIPT-NEXT:  Symbol {
// CHECK-NO-VERSION-SCRIPT-NEXT:    Name: default_callback (
// CHECK-NO-VERSION-SCRIPT-NEXT:    Value: {{0x12001|0x1}}0000
// CHECK-NO-VERSION-SCRIPT-NEXT:    Size: 12
// CHECK-NO-VERSION-SCRIPT-NEXT:    Binding: Global (0x1)
// CHECK-NO-VERSION-SCRIPT-NEXT:    Type: Function (0x2)
// CHECK-NO-VERSION-SCRIPT-NEXT:    Other: 0
// CHECK-NO-VERSION-SCRIPT-NEXT:    Section: .text (0x
// CHECK-NO-VERSION-SCRIPT-NEXT:  }
// CHECK-NEXT: ]
// __cap_relocs should be empty other than the one global reference to callbacks
// CHECK-NEXT: CHERI __cap_relocs [
// CHECK-NEXT:   0x{{.+}} (check_if_default@CAPTABLE) Base: 0x{{.+}} (check_if_default+0) Length: 52 Perms: Function
// CHECK-NEXT:   0x{{.+}} (callbacks@CAPTABLE.3) Base: 0x{{.+}} (callbacks+0) Length: 64 Perms: Constant
// CHECK-NEXT: ]
// CHECK-NEXT: CHERI .captable [
// CHECK-NEXT:   0x0 default_callback@CAPTABLE R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against {{default_callback|__cheri_fnptr_default_callback}}
// CHECK-NEXT:   0x10 check_if_default@CAPTABLE
// CHECK-NEXT:   0x20 static_callback@CAPTABLE.2 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE against __cheri_fnptr_static_callback
// CHECK-NEXT:   0x30 callbacks@CAPTABLE.3
// CHECK-NEXT: ]

// no need for trampolines in static executable
// RUN: ld.lld %t.o %t2.o -o %t-static.exe --verbose-cap-relocs 2>&1 | FileCheck %s -check-prefix STATIC-MESSAGE
// STATIC-MESSAGE: Do not need function pointer trampoline for default_callback in static binary
// STATIC-MESSAGE-NEXT: Do not need function pointer trampoline for static_callback in static binary
// STATIC-MESSAGE-NEXT: Do not need function pointer trampoline for default_callback in static binary
// STATIC-MESSAGE-NEXT: Do not need function pointer trampoline for static_callback in static binary
// STATIC-MESSAGE-NEXT: Do not need function pointer trampoline for default_callback in static binary
// STATIC-MESSAGE-NEXT: Do not need function pointer trampoline for static_callback in static binary
// STATIC-MESSAGE-EMPTY:

// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t-static.exe | FileCheck %s -check-prefix STATIC
// STATIC-LABEL: Dynamic Relocations {
// STATIC-NEXT:  }
// STATIC-NEXT:  DynamicSymbols [
// STATIC-NEXT:  ]
// STATIC-NEXT:  CHERI __cap_relocs [
// STATIC-NEXT:     0x120020000 (callbacks)     Base: 0x120010000 (default_callback+0) Length: 12 Perms: Function
// STATIC-NEXT:     0x120020010 Base: 0x120010098 (static_callback+0) Length: 12 Perms: Function
// STATIC-NEXT:     0x120020020 Base: 0x120010000 (default_callback+0) Length: 12 Perms: Function
// STATIC-NEXT:     0x120020030 Base: 0x120010098 (static_callback+0) Length: 12 Perms: Function
// STATIC-NEXT:     0x120020040 (default_callback@CAPTABLE) Base: 0x120010000 (default_callback+0) Length: 12 Perms: Function
// STATIC-NEXT:     0x120020050 (check_if_default@CAPTABLE) Base: 0x120010270 (check_if_default+0) Length: 52 Perms: Function
// STATIC-NEXT:     0x120020060 (static_callback@CAPTABLE.2) Base: 0x120010098 (static_callback+0) Length: 12 Perms: Function
// STATIC-NEXT:     0x120020070 (callbacks@CAPTABLE.3) Base: 0x120020000 (callbacks+0) Length: 64 Perms: Constant
// STATIC-NEXT:  ]
// STATIC-NEXT:  CHERI .captable [
// STATIC-NEXT:    0x0      default_callback@CAPTABLE
// STATIC-NEXT:    0x10     check_if_default@CAPTABLE
// STATIC-NEXT:    0x20     static_callback@CAPTABLE.2
// STATIC-NEXT:    0x30     callbacks@CAPTABLE.3
// STATIC-NEXT:  ]

// Check that we don't crash when a version script marks a symbol as non-preemptible:
// RUN: echo "VERSION_1.0 { local: *; };" > %t.script
// RUN: ld.lld -shared %t.o %t2.o %t-empty-shlib.so --verbose-cap-relocs -o %t-version-script.so --version-script %t.script
// RUN: llvm-readobj --dyn-symbols --dyn-relocations --cap-relocs --cap-table %t-version-script.so | FileCheck %s -check-prefixes CHECK,CHECK-NONPREEMPTIBLE

// FIXME: this should also be a runtime test in rtld

typedef int (*func_ptr)(void);
__attribute__((visibility("protected"))) _Bool check_if_default(func_ptr ptr);
int default_callback(void);

#ifdef OTHER_FILE
_Bool check_if_default(func_ptr ptr) { return ptr == &default_callback; }
#else
__attribute__((noinline)) int default_callback(void) { return 1; }

__attribute__((visibility("hidden"))) int __start(void) {
  return check_if_default(&default_callback);
}

// All of these address-of operators should resolve to the same address:
// Previously we would synthesize a new symbol for every use
static __attribute__((noinline)) int static_callback(void) { return 1; }

__attribute__((used)) static _Bool test_static_fn_1(void) { return check_if_default(&static_callback); }
__attribute__((used)) static _Bool test_static_fn_2(void) { return check_if_default(&static_callback); }
// Two uses in the same function:
__attribute__((used)) static _Bool test_static_fn_3(void) {
  return check_if_default(&static_callback) && check_if_default(&static_callback);
}

typedef struct _CALLBACK {
  const func_ptr ptr;
} CALLBACK;

static const CALLBACK callbacks[] = {
    {default_callback},
    {static_callback},
    {default_callback},
    {static_callback},
};

__attribute__((used)) static _Bool test_nested_in_struct(void) {
  for (int i = 0; i <sizeof(callbacks)/sizeof(callbacks[0]); i++) {
    if (check_if_default(callbacks[i].ptr))
      return 1;
  }
  return 0;
}

#endif
