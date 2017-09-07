; RUN: %cheri_llc %s -o - | FileCheck %s

; ModuleID = 'load.c'
target datalayout = "E-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f128:128:128-v64:64:64-n32"
target triple = "cheri-unknown-freebsd"

; CHECK: loadFromPtr1U
define zeroext i8 @loadFromPtr1U(i8 addrspace(200)* nocapture %a) nounwind readonly {
entry:
  ; CHECK: clb
  %0 = load i8, i8 addrspace(200)* %a, align 1
  ; CHECK: addiu
  %conv = zext i8 %0 to i32
  %add = add nsw i32 %conv, 12
  ; CHECK: andi
  %conv1 = trunc i32 %add to i8
  ret i8 %conv1
}

; CHECK: loadFromPtr1
define signext i8 @loadFromPtr1(i8 addrspace(200)* nocapture %a) nounwind readonly {
entry:
  ; CHECK: clb
  %0 = load i8, i8 addrspace(200)* %a, align 1
  %conv2 = zext i8 %0 to i32
  %add = add nsw i32 %conv2, 12
  %conv1 = trunc i32 %add to i8
  ret i8 %conv1
}

; CHECK: loadFromPtr2U
define zeroext i16 @loadFromPtr2U(i16 addrspace(200)* nocapture %a) nounwind readonly {
entry:
  ; CHECK: clh
  %0 = load i16, i16 addrspace(200)* %a, align 2
  %conv = zext i16 %0 to i32
  %add = add nsw i32 %conv, 12
  %conv1 = trunc i32 %add to i16
  ret i16 %conv1
}

; CHECK: loadFromPtr2
define signext i16 @loadFromPtr2(i16 addrspace(200)* nocapture %a) nounwind readonly {
entry:
  ; CHECK: clh
  %0 = load i16, i16 addrspace(200)* %a, align 2
  %conv2 = zext i16 %0 to i32
  %add = add nsw i32 %conv2, 12
  %conv1 = trunc i32 %add to i16
  ret i16 %conv1
}

; CHECK: loadFromPtr4U
define i32 @loadFromPtr4U(i32 addrspace(200)* nocapture %a) nounwind readonly {
entry:
  ; CHECK: clw
  %0 = load i32, i32 addrspace(200)* %a, align 4
  %add = add i32 %0, 12
  ret i32 %add
}

; CHECK: loadFromPtr4
define i32 @loadFromPtr4(i32 addrspace(200)* nocapture %a) nounwind readonly {
entry:
  ; CHECK: clw
  %0 = load i32, i32 addrspace(200)* %a, align 4
  %add = add nsw i32 %0, 12
  ret i32 %add
}

; CHECK: loadFromPtr8U
define i64 @loadFromPtr8U(i64 addrspace(200)* nocapture %a) nounwind readonly {
entry:
  ; CHECK: cld
  %0 = load i64, i64 addrspace(200)* %a, align 8
  %add = add i64 %0, 12
  ret i64 %add
}

; CHECK: loadFromPtr8
define i64 @loadFromPtr8(i64 addrspace(200)* nocapture %a) nounwind readonly {
entry:
  ; CHECK: cld
  %0 = load i64, i64 addrspace(200)* %a, align 8
  %add = add nsw i64 %0, 12
  ret i64 %add
}
