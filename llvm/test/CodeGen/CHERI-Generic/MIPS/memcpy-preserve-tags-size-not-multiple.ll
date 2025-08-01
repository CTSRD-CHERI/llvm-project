; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --scrub-attributes --version 2
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/memcpy-preserve-tags-size-not-multiple.ll
; RUN: llc -mtriple=mips64 -mcpu=cheri128 -mattr=+cheri128 --relocation-model=pic -target-abi purecap -o - -O0 -verify-machineinstrs %s | FileCheck %s -check-prefixes CHECK
; Check that we can inline memmove/memcpy despite having the must_preserve_cheri_tags property and the size not
; being a multiple of CAP_SIZE. Since the pointers are aligned we can start with capability copies and use
; word/byte copies for the trailing bytes.
declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture, i8 addrspace(200)* nocapture readonly, i64, i1) addrspace(200)
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture, i8 addrspace(200)* nocapture readonly, i64, i1) addrspace(200)

define void @test_string_memmove(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) nounwind {
  ; Note: has must_preserve_cheri_tags, but this memmove can still be inlined since it's aligned
; CHECK-LABEL: test_string_memmove:
; CHECK:       # %bb.0:
; CHECK-NEXT:    clc $c1, $zero, 0($c4)
; CHECK-NEXT:    clc $c2, $zero, 16($c4)
; CHECK-NEXT:    cld $1, $zero, 32($c4)
; CHECK-NEXT:    clw $2, $zero, 40($c4)
; CHECK-NEXT:    clb $3, $zero, 44($c4)
; CHECK-NEXT:    csb $3, $zero, 44($c3)
; CHECK-NEXT:    csw $2, $zero, 40($c3)
; CHECK-NEXT:    csd $1, $zero, 32($c3)
; CHECK-NEXT:    csc $c2, $zero, 16($c3)
; CHECK-NEXT:    csc $c1, $zero, 0($c3)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 addrspace(200)* align 16 %src, i64 45, i1 false) must_preserve_cheri_tags
  ret void
}

define void @test_string_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) nounwind {
  ; Note: has must_preserve_cheri_tags, but this memcpy can still be inlined since it's aligned
; CHECK-LABEL: test_string_memcpy:
; CHECK:       # %bb.0:
; CHECK-NEXT:    clc $c1, $zero, 0($c4)
; CHECK-NEXT:    csc $c1, $zero, 0($c3)
; CHECK-NEXT:    clb $1, $zero, 44($c4)
; CHECK-NEXT:    csb $1, $zero, 44($c3)
; CHECK-NEXT:    clw $1, $zero, 40($c4)
; CHECK-NEXT:    csw $1, $zero, 40($c3)
; CHECK-NEXT:    cld $1, $zero, 32($c4)
; CHECK-NEXT:    csd $1, $zero, 32($c3)
; CHECK-NEXT:    clc $c1, $zero, 16($c4)
; CHECK-NEXT:    csc $c1, $zero, 16($c3)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    nop
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 addrspace(200)* align 16 %src, i64 45, i1 false) must_preserve_cheri_tags
  ret void
}
