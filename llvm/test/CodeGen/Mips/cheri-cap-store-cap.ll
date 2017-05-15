; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'jon.c'
target datalayout = "E-pf200:256:256:256-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32:64-S256"
target triple = "cheri-unknown-freebsd"

%struct.node = type opaque

@n = common global %struct.node addrspace(200)* addrspace(200)* null, align 32

; Function Attrs: nounwind
; CHECK: set
define void @set(%struct.node addrspace(200)* %h) #0 {
; CHECK: clc
  %1 = load %struct.node addrspace(200)* addrspace(200)*, %struct.node addrspace(200)* addrspace(200)** @n, align 32
; CHECK: csc
  store %struct.node addrspace(200)* %h, %struct.node addrspace(200)* addrspace(200)* %1, align 32
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
