; RUN: opt -S -passes=instcombine %s -o - | FileCheck %s
; This used to produce a nonnull attribute on a null parameter in llvm.cheri.cap.address.set
; Check that we convert the set on NULL to a GEP instruction
target datalayout = "pf200:128:128:128:64-A200-P200-G200"

declare i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200)) addrspace(200)
declare ptr addrspace(200) @llvm.cheri.cap.address.set.i64(ptr addrspace(200), i64) addrspace(200)

define ptr addrspace(200) @inline_align_down(i64 %mask) addrspace(200) {
  %a = tail call ptr addrspace(200) @llvm.cheri.cap.address.set.i64(ptr addrspace(200) null, i64 100)
  %b = tail call i64 @llvm.cheri.cap.address.get.i64(ptr addrspace(200) nonnull %a)
  %c = and i64 %b, %mask
  %d = tail call ptr addrspace(200) @llvm.cheri.cap.address.set.i64(ptr addrspace(200) nonnull %a, i64 %c)
  ret ptr addrspace(200) %d
  ; CHECK-LABEL: @inline_align_down(i64 %mask)
  ; CHECK-NEXT: %c = and i64 %mask, 100
  ; CHECK-NEXT: %d = getelementptr i8, ptr addrspace(200) null, i64 %c
  ; CHECK-NEXT: ret ptr addrspace(200) %d
}
