// REQUIRES: clang

// RUN: %clang_cheri_purecap %S/Inputs/external_lib_user.c -c -o %t.o
// RUN: %clang_cheri_purecap %S/Inputs/external_lib.c -c -o %t-externs.o

// RUN: ld.lld -process-cap-relocs %t.o %t-externs.o -static -o %t-static.exe -e entry
// RUN: llvm-objdump -h -r -t -C %t-static.exe | FileCheck -check-prefixes DUMP-EXE,STATIC %s

// RUN: ld.lld -process-cap-relocs %t-externs.o -shared -o %t-externs.so
// RUN: llvm-readobj -r -s %t-externs.so | FileCheck -check-prefixes EXTERNALLIB %s
// RUN: llvm-objdump -C -t  %t-externs.so | FileCheck -check-prefixes DUMP-EXTERNALLIB %s

// RUN: ld.lld -process-cap-relocs %t-externs.so %t.o -o %t-dynamic.exe -e entry
// RUN: llvm-objdump -h -r -t -C %t-dynamic.exe | FileCheck -check-prefixes DUMP-EXE,DYNAMIC %s

// See what it looks like as a shared library
// RUN: ld.lld -process-cap-relocs %t-externs.so %t.o -shared -o %t.so
// RUN: llvm-readobj -r -s %t.so | FileCheck -check-prefixes SHLIB %s
// RUN: llvm-objdump -C -t %t.so | FileCheck -check-prefixes DUMP-SHLIB %s

// check that we get an undefined symbol error:
// RUN: not ld.lld -process-cap-relocs %t.o -Bdynamic -o /dev/null -e entry 2>&1 | FileCheck %s -check-prefix UNDEFINED
// RUN: not ld.lld -process-cap-relocs %t.o -static -o /dev/null -e entry 2>&1 | FileCheck %s -check-prefix UNDEFINED
// RUN: not ld.lld -process-cap-relocs %t.o -shared -o /dev/null 2>&1 | FileCheck %s -check-prefix UNDEFINED



// DUMP-EXE-NOT: foo
// DYNAMIC-NOT: foo
// STATIC-NOT: foo
// SHLIB-NOT: foo
// DUMP-SHLIB-NOT: foo
// EXTERNALLIB-NOT: foo
// DUMP-EXTERNALLIB-NOT: foo
// UNDEFINED:      error: undefined symbol: external_cap
// UNDEFINED-NEXT: >>> referenced by {{.+}}/external_lib_user.c
// UNDEFINED-NEXT: >>> {{.+}}/capreloc-externallib-integrated.c.tmp.o:(cap_ptr)
// UNDEFINED: error: undefined symbol: external_buffer
// UNDEFINED-NEXT: >>> referenced by {{.+}}/external_lib_user.c
// UNDEFINED-NEXT: >>> {{.+}}/capreloc-externallib-integrated.c.tmp.o:(buffer_ptr)
// UNDEFINED: error: undefined symbol: external_func
// UNDEFINED-NEXT: >>> referenced by {{.+}}/external_lib_user.c
// UNDEFINED-NEXT: >>> {{.+}}/capreloc-externallib-integrated.c.tmp.o:(func_ptr)

