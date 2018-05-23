; RUN: %cheri_llc -O1 %s -o - | FileCheck %s
; ModuleID = 'incoffset.c'
source_filename = "incoffset.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Function Attrs: norecurse nounwind
define void @doThing(i8 addrspace(200)* nocapture readonly %in, i8 addrspace(200)* nocapture %out) local_unnamed_addr #0 {
entry:
  ; Check that the +8 is folded into the load / store and isn't a separate cincoffset
  ; CHECK-NOT: cincoffset
  ; CHECK: 	clb	$[[REG:[0-9]+]], $zero, 8($c3)
  ; CHECK: 	csb	$[[REG]], $zero, 6($c4)
  ; CHECK-NOT: cincoffset
  ; CHECK-NOT: cincoffset
  %arrayidx = getelementptr inbounds i8, i8 addrspace(200)* %in, i64 8
  %0 = load i8, i8 addrspace(200)* %arrayidx, align 1, !tbaa !3
  %arrayidx1 = getelementptr inbounds i8, i8 addrspace(200)* %out, i64 6
  store i8 %0, i8 addrspace(200)* %arrayidx1, align 1, !tbaa !3
  ret void
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 5.0.0 "}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
