// REQUIRES: clang

// RUN: %clang_cheri_purecap %s -c -o %t.o
// RUN: llvm-objdump -C -r %t.o | FileCheck -check-prefix OBJ-CAPRELOCS %s

// RUN: ld.lld -process-cap-relocs %t.o -static -o %t-static.exe -verbose 2>&1 | FileCheck -check-prefixes LINKING-EXE %s
// RUN: llvm-objdump -C %t-static.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,STATIC %s

// same again for statically dynamically linked exe:
// RUN: %clang_cheri_purecap %S/Inputs/dummy_shlib.c -c -o %T/integrated_dummy_shlib.o
// RUN: ld.lld -process-cap-relocs -pie -Bdynamic %t.o -o %t-dynamic.exe -verbose 2>&1 | FileCheck -check-prefixes LINKING-DYNAMIC %s
// RUN: llvm-objdump -C %t-dynamic.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC %s
// RUN: llvm-readobj -r -s %t-dynamic.exe | FileCheck -check-prefixes DYNAMIC-RELOCS %s

// Look at shared libraries:
// RUN: ld.lld -process-cap-relocs %t.o -shared -o %t.so 2>&1 -verbose | FileCheck -check-prefixes LINKING-DYNAMIC %s
// RUN: llvm-readobj -r -s %t.so | FileCheck -check-prefixes DYNAMIC-RELOCS %s
// RUN: llvm-objdump -C -t %t.so | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC %s

// RUN: ld.lld %t.o -static -o %t-static-external-capsizefix.exe
// RUN: %capsizefix %t-static-external-capsizefix.exe
// RUN: llvm-objdump -C %t-static-external-capsizefix.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,STATIC-EXTERNAL-CAPSIZEFIX %s


// RUN: ld.lld %t.o -shared -o %t-external-capsizefix.so
// RUN: %capsizefix %t-external-capsizefix.so
// RUN: llvm-objdump -C %t-external-capsizefix.so | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC-EXTERNAL-CAPSIZEFIX %s
// RUN: llvm-readobj -r -s  %t-external-capsizefix.so | FileCheck -check-prefixes DYNAMIC-RELOCS %s



// FIXME: it would be good if we could set bounds here instead of having it as -1

int errno;

static int *
__error_unthreaded(void)
{

  return (&errno);
}

static int *(*__error_selector)(void) = __error_unthreaded;

void
__set_error_selector(int *(*arg)(void))
{

  __error_selector = arg;
}

__attribute__((visibility("protected"))) int *
__error(void)
{

  return (__error_selector());
}

void __start(void) {}

// OBJ-CAPRELOCS: CAPABILITY RELOCATION RECORDS:
// OBJ-CAPRELOCS: 0x0000000000000000      Base:  (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x00000000{{$}}
// OBJ-CAPRELOCS-SAME:{{[[:space:]]$}}

// LINKING-EXE-NOT: warning
// LINKING-DYNAMIC-NOT: warning


// dynamic should have 10 relocations against the load address
// DYNAMIC-RELOCS-LABEL: Relocations [
// DYNAMIC-RELOCS-NEXT:   Section (8) .rel.dyn {
// DYNAMIC-RELOCS-NEXT:     0x30000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30008 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:   }
// DYNAMIC-RELOCS-NEXT: ]


// DUMP-CAPRELOCS-LABEL: CAPABILITY RELOCATION RECORDS:
// STATIC-NEXT: 0x0000000120020000 Base: __error_unthreaded (0x00000001200100e8) Offset: 0x0000000000000000 Length: 0x0000000000000044 Permissions: 0x8000000000000000 (Function){{$}}
// PIE exe amd shlib should have dynamic relocations and only the offset values
// DYNAMIC-NEXT:0x0000000000020000 Base: __error_unthreaded (0x00000000000100e8) Offset: 0x0000000000000000 Length: 0x0000000000000044 Permissions: 0x8000000000000000 (Function){{$}}


// The external capsizefix does okay for both cases:
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: 0x0000000120020000 Base: __error_unthreaded (0x00000001200100e8) Offset: 0x0000000000000000 Length: 0x0000000000000044 Permissions: 0x8000000000000000 (Function){{$}}
// DYNAMIC-EXTERNAL-CAPSIZEFIX-NEXT: 0x0000000000020000 Base: __error_unthreaded (0x00000000000100e8) Offset: 0x0000000000000000 Length: 0x0000000000000044 Permissions: 0x8000000000000000 (Function){{$}}


// DUMP-CAPRELOCS-SAME:{{[[:space:]]$}}
