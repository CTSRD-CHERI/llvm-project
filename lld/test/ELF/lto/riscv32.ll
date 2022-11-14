; REQUIRES: riscv

; RUN: llvm-as %s -o %t.o
; RUN: ld.lld %t.o -o %t
; RUN: llvm-readelf --syms %t | FileCheck %s
; CHECK: Symbol table '.symtab' contains 3 entries:
; CHECK: FUNC    GLOBAL DEFAULT     1 _start
; CHECK-EMPTY:

target datalayout = "e-m:e-p:32:32-i64:64-n32-S128"
target triple = "riscv32-unknown-elf"

define void @_start() {
  ret void
}

!llvm.module.flags = !{!0}
!0 = !{i32 1, !"target-abi", !"ilp32"}
