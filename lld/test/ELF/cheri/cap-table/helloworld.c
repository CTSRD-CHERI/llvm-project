// RUN: mkdir -p %T/helloworld_src
// RUN: tar xJf %S/Inputs/helloworld.tar.xz -C %T/helloworld_src
// RUN: cd %T/helloworld_src/reproduce && ld.lld @%T/helloworld_src/reproduce/response.txt -o %t.exe
// RUN: llvm-objdump -t -C -s %t.exe | FileCheck %s

// check that __libc_interposing table is initialized correctly
// I can't generate a smaller test case because if I only link the individual files it works
// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// CHECK: 0x00000001200d7480      Base: __wrap_accept (0x000000012008bd00)   Offset: 0x0000000000000000      Length: 0x0000000000000038      Permissions: 0x8000000000000000 (Function)
// CHECK: 0x00000001200f1080      Base: __libc_interposing (0x00000001200d7480)   Offset: 0x0000000000000000      Length: 0x0000000000000540      Permissions: 0x00000000

// CHECK: 00000001200d7480 l       .data           00000540 .hidden __libc_interposing
