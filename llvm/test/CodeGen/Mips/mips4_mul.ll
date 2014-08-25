; RUN: llc -mtriple=mips64-unknown-freebsd -mcpu=mips4 %s -o - | FileCheck %s
; ModuleID = 'mul.c'
target datalayout = "E-m:m-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "mips64-unknown-freebsd"

; Function Attrs: nounwind readnone
define i32 @multiply(i32 %a, i32 %b) #0 {
entry:
  ; CHECK: mult
  ; CHECK: mflo
  %mul = mul nsw i32 %b, %a
  ret i32 %mul
}

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.6.0 "}
