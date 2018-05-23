; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'store.c'
target datalayout = "E-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f128:128:128-v64:64:64-n32"
target triple = "cheri-unknown-freebsd"

; CHECK: storeToPtr1
define void @storeToPtr1(i8 addrspace(200)* nocapture %a, i8 signext %v) nounwind {
entry:
  ; CHECK: csb
  store i8 %v, i8 addrspace(200)* %a, align 1
  ret void
}

; CHECK: storeToPtr2
define void @storeToPtr2(i16 addrspace(200)* nocapture %a, i16 signext %v) nounwind {
entry:
  ; CHECK: csh
  store i16 %v, i16 addrspace(200)* %a, align 2
  ret void
}

; CHECK: storeToPtr4
define void @storeToPtr4(i32 addrspace(200)* nocapture %a, i32 %v) nounwind {
entry:
  ; CHECK: csw
  store i32 %v, i32 addrspace(200)* %a, align 4
  ret void
}

; CHECK: storeToPtr8
define void @storeToPtr8(i64 addrspace(200)* nocapture %a, i64 %v) nounwind {
entry:
  ; CHECK: csd
  store i64 %v, i64 addrspace(200)* %a, align 8
  ret void
}

