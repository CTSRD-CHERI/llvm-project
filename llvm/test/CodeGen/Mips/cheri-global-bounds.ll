; RUN: llc -mtriple=cheri-unknown-freebsd -mcpu=cheri %s -o - | FileCheck -check-prefix=CHERI %s
; RUN: llc -mtriple=cheri-unknown-freebsd -cheri-no-global-bounds -mcpu=cheri %s -o - | FileCheck -check-prefix=MIPS %s

; ModuleID = 'global.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@x = common addrspace(200) global i32 0, align 4

; Function Attrs: nounwind
define void @foo(i32 signext %y) #0 {
entry:
  ; MIPS: sw	$4, 0($1)
  ; CHERI: cfromptr	$c1, $c0, $1
  ; CHERI: csetbounds	$c1, $c1, $2
  ; CHERI: csw	$4, $zero, 0($c1)

  store i32 %y, i32 addrspace(200)* @x, align 4, !tbaa !1
  ret void
}

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm/tools/clang 4afe7540389c3cb256e3ada0812effe189b19d48)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
