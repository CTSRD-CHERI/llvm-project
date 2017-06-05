// REQUIRES: bigarreau_clang

// RUN: %bigarreau_clang_cheri_purecap %S/simple-cap-reloc.c -c -o %t.o
// RUN: llvm-readobj -s -r %t.o | FileCheck -check-prefix READOBJ %S/simple-cap-reloc-common.check
// RUN: llvm-objdump -C -r %t.o | FileCheck -check-prefix DUMP-OBJ %S/simple-cap-reloc-common.check

// We need the symbol table before the capabiilty relocation record -> dump it to a temporary file first
// RUN: %bigarreau_clang_link_lld %t.o -static -o %t-static.exe
// RUN: llvm-objdump -h -r -t -C %t-static.exe | FileCheck -check-prefixes DUMP-EXE,STATIC %S/simple-cap-reloc-common.check

// same again for statically dynamically linked exe:
// Create a dummy shared library otherwise bfd won't create a dynamic executable
// RUN: %bigarreau_clang_cheri_purecap %S/Inputs/dummy_shlib.c -c -o %T/dummy_shlib.o
// RUN: %bigarreau_clang_link_lld  %T/dummy_shlib.o -shared -o %T/libdummy_shlib.so
// RUN: %bigarreau_clang_link_lld -L%T -ldummy_shlib %t.o -o %t-dynamic.exe
// RUN: llvm-objdump -h -r -t -C %t-dynamic.exe | FileCheck -check-prefixes DUMP-EXE,DYNAMIC %S/simple-cap-reloc-common.check

// RUN: %bigarreau_clang_link_lld %t.o -shared -o %t.so
// RUN: llvm-readobj -r -s %t.so | FileCheck -check-prefixes SHLIB,SHLIB-CAPSIZEFIX %S/simple-cap-reloc-common.check
// RUN: llvm-objdump -C -t %t.so | FileCheck -check-prefixes DUMP-SHLIB,DUMP-SHLIB-EXTERNAL %S/simple-cap-reloc-common.check

