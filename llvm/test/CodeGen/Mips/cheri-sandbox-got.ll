; RUN: llc %s -mtriple=cheri-unknown-freebsd -mattr=sandbox -o - | FileCheck %s

; ModuleID = 'global.c'
target datalayout = "E-m:m-p200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@x = common addrspace(200) global i32 0, align 4

; Function Attrs: nounwind
define void @foo(i32 signext %y) #0 {
entry:
  ; CHECK: 	daddiu	$1, $1, %got_disp(x)
  ; CHECK: 	cfromptr $c1, $c0, $1
  ; CHECK: 	cld	$1, $zero, 0($c1)
  store i32 %y, i32 addrspace(200)* @x, align 4, !tbaa !1
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.6.0 "}
!1 = metadata !{metadata !2, metadata !2, i64 0}
!2 = metadata !{metadata !"int", metadata !3, i64 0}
!3 = metadata !{metadata !"omnipotent char", metadata !4, i64 0}
!4 = metadata !{metadata !"Simple C/C++ TBAA"}
