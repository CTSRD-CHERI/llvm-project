; RUN: opt -S -passes=instcombine %s -o - | FileCheck %s
; This used to produce a nonnull attribute on a null parameter in llvm.cheri.cap.address.set
; Check that we convert the set on NULL to a GEP instruction
target datalayout = "pf200:128:128:128:64-A200-P200-G200"

declare i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)*, i64)

define i8 addrspace(200)* @inline_align_down(i64 %mask) {
  %a = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* null, i64 100)
  %b = tail call i64 @llvm.cheri.cap.address.get.i64(i8 addrspace(200)* nonnull %a) #3
  %c = and i64 %b, %mask
  %d = tail call i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)* nonnull %a, i64 %c) #3
  ret i8 addrspace(200)* %d
  ; CHECK-LABEL: @inline_align_down(i64 %mask)
  ; CHECK-NEXT: %c = and i64 %mask, 100
  ; CHECK-NEXT: %d = getelementptr i8, i8 addrspace(200)* null, i64 %c
  ; CHECK-NEXT: ret i8 addrspace(200)* %d
}
