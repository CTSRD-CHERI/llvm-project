; RUN: llc %s -mcpu=cheri -o - | FileCheck %s
; XFAIL: *
; ModuleID = 'brtest.c'
target datalayout = "E-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f128:128:128-v64:64:64-n32"
target triple = "cheri-unknown-freebsd"

; Check whether we're optimising a branch on a condition flag to a BC2F.

; CHECK: isValid
define void @isValid(i8 addrspace(200)* %x) nounwind {
entry:
  %0 = tail call i32 @llvm.mips.cheri.get.cap.tag(i8 addrspace(200)* %x)
  %cmp = icmp eq i32 %0, 0
  ; CHECK: BC2F
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @f1() nounwind
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  tail call void @f2() nounwind
  ret void
}

declare i32 @llvm.mips.cheri.get.cap.tag(i8 addrspace(200)*) nounwind readnone

declare void @f1()

declare void @f2()
