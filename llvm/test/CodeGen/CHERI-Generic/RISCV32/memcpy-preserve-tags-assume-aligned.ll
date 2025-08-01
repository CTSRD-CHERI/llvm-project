; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --scrub-attributes --version 2
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/memcpy-preserve-tags-assume-aligned.ll
; Check that __builtin_assume_aligned does the right thing and allows us to elide the memcpy
; call even with must_preserve_cheri_tags attribute (run instcombine to propagate assume information)
; RUN: opt -mtriple=riscv32 --relocation-model=pic -target-abi il32pc64f -mattr=+xcheri,+cap-mode,+f -S -passes=instcombine < %s | llc -mtriple=riscv32 --relocation-model=pic -target-abi il32pc64f -mattr=+xcheri,+cap-mode,+f -O2 -o - | FileCheck %s
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"

declare void @llvm.memcpy.p200i8.p200i8.i32(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i32, i1)
declare void @llvm.memmove.p200i8.p200i8.i32(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i32, i1)
declare void @llvm.assume(i1) addrspace(200)

define void @memcpy_assume(i8 addrspace(200)* addrspace(200)* %local_cap_ptr, i8 addrspace(200)* %align1) addrspace(200) nounwind {
; CHECK-LABEL: memcpy_assume:
; CHECK:       # %bb.0:
; CHECK-NEXT:    lc ca2, 24(ca0)
; CHECK-NEXT:    sc ca2, 24(ca1)
; CHECK-NEXT:    lc ca2, 16(ca0)
; CHECK-NEXT:    sc ca2, 16(ca1)
; CHECK-NEXT:    lc ca2, 8(ca0)
; CHECK-NEXT:    sc ca2, 8(ca1)
; CHECK-NEXT:    lc ca0, 0(ca0)
; CHECK-NEXT:    sc ca0, 0(ca1)
; CHECK-NEXT:    ret
  %ptrint = ptrtoint i8 addrspace(200)* %align1 to i32
  %maskedptr = and i32 %ptrint, 15
  %maskcond = icmp eq i32 %maskedptr, 0
  tail call void @llvm.assume(i1 %maskcond)
  %1 = bitcast i8 addrspace(200)* addrspace(200)* %local_cap_ptr to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i32(i8 addrspace(200)* align 1 %align1, i8 addrspace(200)* align 16 %1, i32 32, i1 false) must_preserve_cheri_tags
  ret void
}

define void @memmove_assume(i8 addrspace(200)* addrspace(200)* %local_cap_ptr, i8 addrspace(200)* %align1) addrspace(200) nounwind {
; CHECK-LABEL: memmove_assume:
; CHECK:       # %bb.0:
; CHECK-NEXT:    lc ca2, 0(ca0)
; CHECK-NEXT:    lc ca3, 16(ca0)
; CHECK-NEXT:    sc ca2, 0(ca1)
; CHECK-NEXT:    lc ca2, 8(ca0)
; CHECK-NEXT:    lc ca0, 24(ca0)
; CHECK-NEXT:    sc ca2, 8(ca1)
; CHECK-NEXT:    sc ca3, 16(ca1)
; CHECK-NEXT:    sc ca0, 24(ca1)
; CHECK-NEXT:    ret
  %ptrint = ptrtoint i8 addrspace(200)* %align1 to i32
  %maskedptr = and i32 %ptrint, 15
  %maskcond = icmp eq i32 %maskedptr, 0
  tail call void @llvm.assume(i1 %maskcond)
  %1 = bitcast i8 addrspace(200)* addrspace(200)* %local_cap_ptr to i8 addrspace(200)*
  call void @llvm.memmove.p200i8.p200i8.i32(i8 addrspace(200)* align 1 %align1, i8 addrspace(200)* align 16 %1, i32 32, i1 false) must_preserve_cheri_tags
  ret void
}

