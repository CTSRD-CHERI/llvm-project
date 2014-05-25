; RUN: llc -mtriple=cheri-unknown-freebsd %s -o - | FileCheck %s
; ModuleID = 'unaligneStore.c'
target datalayout = "E-m:m-p200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define void @st(i32 addrspace(200)* nocapture %c, i32 %i) #0 {
entry:
  ; Check that we emit four byte stores for an unaligned 4-byte store.
  ; CHECK: csb
  ; CHECK: csb
  ; CHECK: csb
  ; CHECK: csb
  store i32 %i, i32 addrspace(200)* %c, align 1, !tbaa !1
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5 "}
!1 = metadata !{metadata !2, metadata !2, i64 0}
!2 = metadata !{metadata !"int", metadata !3, i64 0}
!3 = metadata !{metadata !"omnipotent char", metadata !4, i64 0}
!4 = metadata !{metadata !"Simple C/C++ TBAA"}
