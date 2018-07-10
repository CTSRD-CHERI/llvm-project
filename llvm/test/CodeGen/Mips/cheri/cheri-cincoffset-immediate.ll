; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'ptradd.c'
source_filename = "ptradd.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: norecurse nounwind readnone
define i8 addrspace(200)* @imm(i8 addrspace(200)* readnone %a) local_unnamed_addr #0 {
entry:
  ; CHECK: 	cincoffset	$c3, $c3, 1023
  %add.ptr = getelementptr inbounds i8, i8 addrspace(200)* %a, i64 1023
  ret i8 addrspace(200)* %add.ptr
}

; Function Attrs: norecurse nounwind readnone
define i8 addrspace(200)* @imm1(i8 addrspace(200)* readnone %a) local_unnamed_addr #0 {
entry:
  ; CHECK: 	cincoffset	$c3, $c3, -1024
  %add.ptr = getelementptr inbounds i8, i8 addrspace(200)* %a, i64 -1024
  ret i8 addrspace(200)* %add.ptr
}

; Function Attrs: norecurse nounwind readnone
define i8 addrspace(200)* @reg(i8 addrspace(200)* readnone %a) local_unnamed_addr #0 {
entry:
  ; CHECK: daddiu	$[[TMP:[0-9]+]], $zero, 1024
  ; CHECK: 	cincoffset	$c3, $c3, $[[TMP]]
  %add.ptr = getelementptr inbounds i8, i8 addrspace(200)* %a, i64 1024
  ret i8 addrspace(200)* %add.ptr
}

; Function Attrs: norecurse nounwind readnone
define i8 addrspace(200)* @reg1(i8 addrspace(200)* readnone %a) local_unnamed_addr #0 {
entry:
  ; CHECK: daddiu	$[[TMP:[0-9]+]], $zero, -1025
  ; CHECK: 	cincoffset	$c3, $c3, $[[TMP]]
  %add.ptr = getelementptr inbounds i8, i8 addrspace(200)* %a, i64 -1025
  ret i8 addrspace(200)* %add.ptr
}

attributes #0 = { norecurse nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 5.0.0 "}
