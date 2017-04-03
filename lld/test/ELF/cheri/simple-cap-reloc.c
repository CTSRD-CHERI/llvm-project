// REQUIRES: clang

// RUN: %clang_cheri_purecap %s -c -o %t.o
// RUN: llvm-readobj -s -r %t.o | FileCheck -check-prefix READOBJ %S/simple-cap-reloc-common.check
// RUN: llvm-objdump -C -r %t.o | FileCheck -check-prefix DUMP-OBJ %S/simple-cap-reloc-common.check

// We need the symbol table before the capabiilty relocation record -> dump it to a temporary file first
// RUN: %clang_link_purecap_bfd %t.o -static -o %t-static.exe
// RUN: llvm-objdump -h -r -t -C %t-static.exe | FileCheck -check-prefixes DUMP-EXE,STATIC %S/simple-cap-reloc-common.check

// same a gain for statically dynamically linked exe:
// Create a dummy shared library otherwise bfd won't create a dynamic executable
// RUN: %clang_cheri_purecap %S/Inputs/dummy_shlib.c -c -o %T/dummy_shlib.o
// RUN: %clang_link_purecap_bfd  %T/dummy_shlib.o -shared -o %T/libdummy_shlib.so
// RUN: %clang_link_purecap_bfd -L%T -ldummy_shlib %t.o -o %t-dynamic.exe
// RUN: llvm-objdump -h -r -t -C %t-dynamic.exe | FileCheck -check-prefixes DUMP-EXE,DYNAMIC %S/simple-cap-reloc-common.check

// RUN: %clang_link_purecap %t.o -shared -o %t.so
// RUN: llvm-readobj -r -s %t.so | FileCheck -check-prefix SHLIB %S/simple-cap-reloc-common.check

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
