# Make sure that using - as an output file uses stdout
# RUN: llvm-mc -filetype=obj -triple=x86_64-unknown-linux %s -o %t.o
# RUN: ld.lld %t.o -o - | llvm-readobj -h | FileCheck %s
# REQUIRES: x86

.globl _start
_start:
  ret

# CHECK: File: <stdin>
# CHECK: Format: elf64-x86-64
# CHECK: Type: Executable (0x2)
# CHECK: Machine: EM_X86_64 (0x3E)
