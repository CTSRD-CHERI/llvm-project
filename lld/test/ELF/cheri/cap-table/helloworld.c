// RUN: mkdir -p %T/helloworld_src
// RUN: tar xJf %S/Inputs/helloworld.tar.xz -C %T/helloworld_src
// RUN: cd %T/helloworld_src/reproduce && ld.lld @%T/helloworld_src/reproduce/response.txt -o %t.exe
// RUN: llvm-objdump -t -cap-relocs %t.exe | FileCheck %s

// check that __libc_interposing table is initialized correctly
// I can't generate a smaller test case because if I only link the individual files it works

// CHECK: [[LIBC_INT_ADDR:[0-9a-f]+]]  l     O .data           00000540 .hidden __libc_interposing
// CHECK: [[LIBC_INT_CTAB:[0-9a-f]+]]  l     O .captable       00000020 __libc_interposing@CAPTABLE

// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// CHECK: 0x[[LIBC_INT_ADDR]]      Base: __wrap_accept (0x{{[0-9a-f]+}})            Offset: 0x0000000000000000      Length: 0x0000000000000038      Permissions: 0x8000000000000000 (Function)
// CHECK: 0x[[LIBC_INT_CTAB]]      Base: __libc_interposing (0x[[LIBC_INT_ADDR]])   Offset: 0x0000000000000000      Length: 0x0000000000000540      Permissions: 0x00000000

