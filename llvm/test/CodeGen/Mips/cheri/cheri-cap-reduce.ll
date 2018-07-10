; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'capfail.c'
target datalayout = "E-pf200:256:256:256-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f128:128:128-v64:64:64-n32"
target triple = "cheri-unknown-freebsd"

; Check that this doesn't crash SelectionDAG trying to fold the trunc into the
; load.

; CHECK: x
define i32 @x(i64 addrspace(200)* nocapture %capmem) #0 {
entry:
  %0 = load i64, i64 addrspace(200)* %capmem, align 8
  %conv = trunc i64 %0 to i32
  ret i32 %conv
}

attributes #0 = { nounwind readonly "target-cpu"="cheri" "target-features"="+n64,+cheri" }

