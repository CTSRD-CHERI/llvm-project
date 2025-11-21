// RUN: mkdir -p %T/helloworld_src
// RUN: tar xJf %S/Inputs/helloworld.tar.xz -C %T/helloworld_src
// RUN: cd %T/helloworld_src/reproduce && ld.lld @%T/helloworld_src/reproduce/response.txt -o %t.exe
// RUN: llvm-objdump -t --cap-relocs %t.exe | FileCheck %s

// check that __libc_interposing table is initialized correctly
// I can't generate a smaller test case because if I only link the individual files it works

// CHECK: [[#%.16x,LIBC_INT_ADDR:]]  l     O .data           0000000000000540 .hidden __libc_interposing
// CHECK: [[#%.16x,LIBC_INT_CTAB:]]  l     O .captable       0000000000000020 __libc_interposing@CAPTABLE
// CHECK: [[#%.16x,WRAP_ACC_ADDR:]]  g     F .text           0000000000000038 __wrap_accept

// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// CHECK: [[#LIBC_INT_ADDR]] FUNC    [[#WRAP_ACC_ADDR]] [{{[0-9a-f]+}}-{{[0-9a-f]+}}]
// CHECK: [[#LIBC_INT_CTAB]] DATA    [[#LIBC_INT_ADDR]] {{\[}}[[#LIBC_INT_ADDR]]-[[#LIBC_INT_ADDR+0x540]]]
