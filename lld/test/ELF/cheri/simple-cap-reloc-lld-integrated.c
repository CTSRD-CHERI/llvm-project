// REQUIRES: clang

// RUN: %clang_cheri_purecap %S/simple-cap-reloc.c -c -o %t.o
// RUN: llvm-readobj -s -r %t.o | FileCheck -check-prefix READOBJ %S/simple-cap-reloc-common.check
// RUN: llvm-objdump -C -r %t.o | FileCheck -check-prefix DUMP-OBJ %S/simple-cap-reloc-common.check

// We need the symbol table before the capabiilty relocation record -> dump it to a temporary file first
// RUN: ld.lld -process-cap-relocs %t.o -static -o %t-static.exe
// RUN: llvm-objdump -h -r -t -C %t-static.exe | FileCheck -check-prefixes DUMP-EXE,STATIC %S/simple-cap-reloc-common.check

// same again for statically dynamically linked exe:
// Create a dummy shared library otherwise bfd won't create a dynamic executable
// RUN: %clang_cheri_purecap %S/Inputs/dummy_shlib.c -c -o %T/bfd_dummy_shlib.o
// RUN: ld.lld -process-cap-relocs  %T/bfd_dummy_shlib.o -shared -o %T/libbfd_dummy_shlib.so
// RUN: ld.lld -process-cap-relocs -L%T -lbfd_dummy_shlib %t.o -o %t-dynamic.exe
// RUN: llvm-objdump -h -r -t -C %t-dynamic.exe | FileCheck -check-prefixes DUMP-EXE,DYNAMIC %S/simple-cap-reloc-common.check

// Look at shared libraries:
// RUN: ld.lld -process-cap-relocs %t.o -shared -o %t.so
// RUN: llvm-readobj -r -s %t.so | FileCheck -check-prefixes SHLIB,SHLIB-INTEGRATED %S/simple-cap-reloc-common.check
// RUN: llvm-objdump -C -t %t.so | FileCheck -check-prefixes DUMP-SHLIB,DUMP-SHLIB-INTERNAL %S/simple-cap-reloc-common.check
