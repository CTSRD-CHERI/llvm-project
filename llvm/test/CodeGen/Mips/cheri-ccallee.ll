; RUN: llc %s -O1 -mcpu=cheri -o - | FileCheck %s
; ModuleID = 'ccall.c'
target datalayout = "E-m:m-p200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define chericcallcce void @fish() #0 {
entry:
  ; CHECK: cfromptr $c3, $c0, $zero
  ; CHECK: daddiu	$2, $zero, 0
  ; CHECK: daddiu	$3, $zero, 0
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.6.0 "}
