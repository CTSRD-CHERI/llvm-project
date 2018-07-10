; RUN: %cheri_purecap_llc -O0 %s -o - | FileCheck %s
; ModuleID = 'cmpcap.c'
source_filename = "cmpcap.c"
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

%struct.bigbuf = type { [5 x i8 addrspace(200)*] }

; Function Attrs: nounwind
define void @zero(%struct.bigbuf addrspace(200)* nocapture %out) local_unnamed_addr #0 {
entry:
  %.compoundliteral.sroa.0.0..sroa_cast1 = bitcast %struct.bigbuf addrspace(200)* %out to i8 addrspace(200)*
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 16 %.compoundliteral.sroa.0.0..sroa_cast1, i8 0, i64 29, i1 false)
; Check that the zero memset is not expanded to capability stores.
; It would be nice if we could expand it to capability stores for the aligned
; bit and pad it with other stores, but currently SelectionDAG expands it to
; two overlapping stores, which is not allowed because capability stores must
; be aligned.  We can probably catch some variants of this with some custom
; logic.
; CHECK: csd	$zero, $zero
; CHECK: csd	$zero, $zero
; CHECK: csd	$zero, $zero
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8, i64, i1) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+chericap,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 7.0.0 (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm/tools/clang f77d651cc0da68f4b1dfb357d7212fbadc5e1725) (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm 726657ee8ccc6ad3a8c0b2b189d9db24171c3f8f)"}
