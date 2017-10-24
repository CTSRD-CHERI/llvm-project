// REQUIRES: clang

// RUN: %clang_cheri_purecap %s -c -o %t.o
// RUN: llvm-readobj -s -r %t.o | FileCheck -check-prefix READOBJ %S/simple-cap-reloc-common.check
// RUN: llvm-objdump -C -r %t.o | FileCheck -check-prefix DUMP-OBJ %S/simple-cap-reloc-common.check

// We need the symbol table before the capabiilty relocation record -> dump it to a temporary file first
// RUN: %clang_link_purecap %t.o -static -o %t-static.exe
// RUN: llvm-objdump -h -r -t -C %t-static.exe | FileCheck -check-prefixes DUMP-EXE,STATIC %S/simple-cap-reloc-common.check

// same a gain for statically dynamically linked exe:
// Create a dummy shared library otherwise bfd won't create a dynamic executable
// RUN: %clang_cheri_purecap %S/Inputs/dummy_shlib.c -c -o %T/dummy_shlib.o
// RUN: %clang_link_purecap  %T/dummy_shlib.o -shared -o %T/libdummy_shlib.so
// RUN: %clang_link_purecap -L%T -ldummy_shlib %t.o -o %t-dynamic.exe
// RUN: llvm-objdump -h -r -t -C %t-dynamic.exe | FileCheck -check-prefixes DUMP-EXE,DYNAMIC %S/simple-cap-reloc-common.check

// RUN: %clang_link_purecap %t.o -shared -o %t.so
// RUN: llvm-readobj -r -s %t.so | FileCheck -check-prefixes SHLIB,SHLIB-CAPSIZEFIX %S/simple-cap-reloc-common.check
// RUNNOT: llvm-objdump -C -t %t.so | FileCheck -check-prefixes DUMP-SHLIB,DUMP-SHLIB-EXTERNAL %S/simple-cap-reloc-common.check



// now invoke lld directly and check whether it matches:
// RUN: %cheri_lld_static %t.o -o %t-direct-static.exe
// RUN: %capsizefix %t-direct-static.exe
// RUN: llvm-objdump -h -r -t -C %t-direct-static.exe | FileCheck -check-prefixes DUMP-EXE,STATIC %S/simple-cap-reloc-common.check

// RUN: %cheri_lld_dynamic %t.o -L%T -ldummy_shlib -o %t-direct-dynamic.exe
// RUN: %capsizefix %t-direct-dynamic.exe
// RUN: llvm-objdump -h -r -t -C %t-direct-dynamic.exe | FileCheck -check-prefixes DUMP-EXE,DYNAMIC %S/simple-cap-reloc-common.check

// RUN: %cheri_lld_shlib %t.o -o %t.so2
// RUN: %capsizefix %t.so2
// RUN: llvm-readobj -r -s %t.so2 | FileCheck -check-prefixes SHLIB,SHLIB-CAPSIZEFIX %S/simple-cap-reloc-common.check
// RUNNOT: llvm-objdump -C -t %t.so2 | FileCheck -check-prefixes DUMP-SHLIB,DUMP-SHLIB-EXTERNAL %S/simple-cap-reloc-common.check


// now try linking with the new --process-cap-relocs flag and compare the output

int a = 0;
int b = 1;
const int c = 2;
int x = 3;
int *xptr = &x;

char buffer[42];
const char* const bufptr = &buffer[0];

int __start(void) {
  return  (void*)xptr != (void*)bufptr;
}

int (*fn_ptr)(void) = &__start;
