; RUN: llc -mtriple=cheri-unknown-freebsd -cheri-test-mode -mattr=+cheri256 -mcpu=mips64 %s -o /dev/null

; ModuleID = '/local/scratch/alr48/cheri/llvm-master/tools/clang/test/CodeGen/cheriabi_machdep-crash.c'
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Generated from the following creduce source:
; #define a(c, d) (__intcap_t) * d
; #define e(f, b) g(*h, cap, i, j, k) {
; l;
; e(, ) if (l) m();
; if (*h)
;   return 4;
; return 0;
; }
; n() {
;   int o, p, error;
;   long i, j;
;   error = g(a(, &o), p, i, j, 1);
;   return error;
; }


@l = common local_unnamed_addr global i32 0, align 4

; Function Attrs: nounwind
define i32 @g(i32* nocapture readonly, i32 signext, i32 signext, i32 signext, i32 signext) local_unnamed_addr #0 {
  %6 = load i32, i32* @l, align 4, !tbaa !1
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %10, label %8

; <label>:8:                                      ; preds = %5
  %9 = tail call i32 bitcast (i32 (...)* @m to i32 ()*)() #3
  br label %10

; <label>:10:                                     ; preds = %5, %8
  %11 = load i32, i32* %0, align 4, !tbaa !1
  %12 = icmp eq i32 %11, 0
  %13 = select i1 %12, i32 0, i32 4
  ret i32 %13
}

declare i32 @m(...) local_unnamed_addr #1

; Function Attrs: nounwind
define i32 @n() local_unnamed_addr #0 {
  %1 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 0)
  %2 = ptrtoint i8 addrspace(200)* %1 to i64
  %3 = inttoptr i64 %2 to i32*
  %4 = load i32, i32* @l, align 4, !tbaa !1
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %8, label %6

; <label>:6:                                      ; preds = %0
  %7 = tail call i32 bitcast (i32 (...)* @m to i32 ()*)() #3
  br label %8

; <label>:8:                                      ; preds = %0, %6
  %9 = load i32, i32* %3, align 4, !tbaa !1
  %10 = icmp eq i32 %9, 0
  %11 = select i1 %10, i32 0, i32 4
  ret i32 %11
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #2

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="mips64" "target-features"="+mips64,+noabicalls,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="mips64" "target-features"="+mips64,+noabicalls,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #2 = { nounwind readnone }
attributes #3 = { nobuiltin nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 "}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
