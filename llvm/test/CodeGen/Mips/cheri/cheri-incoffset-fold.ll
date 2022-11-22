; RUN: %cheri_llc -O1 %s -o - | FileCheck %s

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

attributes #0 = { norecurse nounwind  }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 5.0.0 "}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
