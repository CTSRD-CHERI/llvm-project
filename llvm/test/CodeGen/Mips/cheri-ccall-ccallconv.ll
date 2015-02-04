; RUN: llc %s -mcpu=cheri -o - | FileCheck %s
; ModuleID = 'call.c'
target datalayout = "E-p200:256:256:256-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32:64-S256"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define void @a(i8 addrspace(200)* %a1, i8 addrspace(200)* %a2, i64 %foo, i64 %bar) #0 {
  ; Move the argument registers into the ccall registers
  ; CHECK: cincbase	$c1, $c3, $zero
  ; CHECK: cincbase	$c2, $c4, $zero
  ; Move argument 0 from a0 to v0, arg 1 from a1 to a0.
  ; CHECK: move	$2, $4
  ; CHECK: move	$4, $1
  ; Then do the function call
  ; CHECK: jalr	$25
  tail call chericcallcc void @b(i8 addrspace(200)* %a1, i8 addrspace(200)* %a2, i64 %foo, i64 %bar) #2
  ret void
}

declare chericcallcc void @b(i8 addrspace(200)*, i8 addrspace(200)*, i64, i64) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.4 "}
