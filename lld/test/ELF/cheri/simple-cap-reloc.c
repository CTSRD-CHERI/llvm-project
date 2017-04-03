// REQUIRES: clang

// RUN: %clang_cheri_purecap %s -c -o %t.o
// RUN: llvm-readobj -s -r %t.o | FileCheck -check-prefix READOBJ %S/simple-cap-reloc-common.check
// RUN: llvm-objdump -C -r %t.o | FileCheck -check-prefix DUMP-OBJ %S/simple-cap-reloc-common.check

// We need the symbol table before the capabiilty relocation record -> dump it to a temporary file first
// RUN: %clang_link_purecap_bfd %t.o -static -o %t-static.exe
// RUN: llvm-objdump -t %t-static.exe > %t-static.dump
// RUN: llvm-objdump -h -r -C %t-static.exe >> %t-static.dump
// RUN: FileCheck -check-prefixes DUMP-EXE,STATIC %S/simple-cap-reloc-common.check < %t-static.dump

// same a gain for statically dynamically linked exe:
// Create a dummy shared library otherwise bfd won't create a dynamic executable
// RUN: %clang_cheri_purecap %S/Inputs/dummy_shlib.c -c -o %T/dummy_shlib.o
// RUN: %clang_link_purecap_bfd  %T/dummy_shlib.o -shared -o %T/libdummy_shlib.so
// RUN: %clang_link_purecap_bfd -L%T -ldummy_shlib %t.o -o %t-dynamic.exe
// RUN: llvm-objdump -t %t-dynamic.exe > %t-dynamic.dump
// RUN: llvm-objdump -h -r -C %t-dynamic.exe >> %t-dynamic.dump
// RUN: FileCheck -check-prefixes DUMP-EXE,DYNAMIC %S/simple-cap-reloc-common.check < %t-dynamic.dump

// RUN: %clang_link_purecap %t.o -shared -o %t.so
// RUN: llvm-readobj -s -r %t.so | FileCheck -check-prefix SHLIB %S/simple-cap-reloc-common.check
// RsUN: llvm-objdump -C -r %t.so | FileCheck -check-prefix DUMP-SHLIB %s


// Check runtime relocation info:
// RUsN: llvm-rtdyld -triple=mips64-unknown-linux -verify -map-section test_ELF_Mips64N64.o,.text=0x1000 -map-section test_ELF_ExternalFunction_Mips64N64.o,.text=0x10000 -check=%s %/T/test_ELF_Mips64N64.o %T/test_ELF_ExternalFunction_Mips64N64.o
// RsUN: llvm-rtdyld -triple=cheri-unknown-freebsd -verify -check=%S/simple-cap-relocs-rtdyld.check %t-dynamic.exe -debug

// 4831903744

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


// DUMP-SHLIB: CAPABILITY RELOCATION RECORDS:
// DUMP-SHLIB-NEXT: 0x0000000120020020      Base: x (0x0000000120020000)    Offset: 0000000000000000        Length: 0000000000000004        Permissions: 00000000
// DUMP-SHLIB-NEXT: 0x0000000120030000      Base: buffer (0x0000000120040000)       Offset: 0000000000000000        Length: 0000000000000042        Permissions: 00000000
// DUMP-SHLIB-NEXT: 0x0000000120020040      Base: __start (0x0000000120010000)      Offset: 0000000000000000        Length: 0000000000000108        Permissions: 9223372036854775808 (Function)

