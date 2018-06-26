// Check that capability relocations in .rodata sections cause errors (since they
// will trap when being processed at runtime). This is true both for static as well
// as dynamic binaries. Currently the kernel just gives us a read-write mapping to
// work around this but we really should just make this an error
//
// RUN: %cheri_purecap_cc1 %s -emit-obj -o %t.o
// RUNNOT: llvm-readobj -r %t.o
// RUN: not ld.lld -shared %t.o -o %t.so 2>&1 | FileCheck %s
// RUN: ld.lld -shared %t.o -o %t.so -z notext
// RUN: llvm-objdump -C -s -t %t.so | FileCheck %s -check-prefix SHLIB
// RUN: not ld.lld -static %t.o -o %t.exe 2>&1 | FileCheck %s
// RUN: ld.lld -static %t.o -o %t.exe -z notext
// RUN: llvm-objdump -C -s -t %t.exe | FileCheck %s -check-prefix EXE


__attribute((visibility("protected"))) int __start(int x) {
  return x + 1;
}

const char space[16] = {0};
__attribute((section(".rodata"))) const int(*foo)(int) = &__start + 4;

// CHECK: error: attempting to add a capability relocation against symbol __start in a read-only section; pass -Wl,-z,notext if you really want to do this
// CHECK-NEXT: >>> referenced by object foo
// CHECK-NEXT: >>> defined in capability-in-rodata2.c ({{.+}}capability-in-rodata2.c.tmp.o:(foo))

// EXE: CAPABILITY RELOCATION RECORDS:
// EXE: 0x0000000120000{{1c0|1a0}} Base: __start (0x0000000120010000)	Offset: 0x0000000000000004	Length: 0x0000000000000028	Permissions: 0x8000000000000000 (Function)

// SHLIB: CAPABILITY RELOCATION RECORDS:
// SHLIB: 0x0000000000000{{250|240}} Base: __start (0x0000000000010000)	Offset: 0x0000000000000004	Length: 0x0000000000000028	Permissions: 0x8000000000000000 (Function)

