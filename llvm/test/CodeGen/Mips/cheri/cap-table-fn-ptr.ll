; RUN: %cheri_purecap_llc -cheri-cap-table %s -o - -mxcaptable=true | %cheri_FileCheck %s -check-prefixes CHECK,BIG
; RUN: %cheri_purecap_llc -cheri-cap-table %s -o - -mxcaptable=false | %cheri_FileCheck %s -check-prefixes CHECK,SMALL
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/cap-table-call-extern.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@fn = common local_unnamed_addr addrspace(200) global void () addrspace(200)* null, align 32

; Function Attrs: nounwind
define void @test() local_unnamed_addr #0 {
entry:
  %0 = load void () addrspace(200)*, void () addrspace(200)* addrspace(200)* @fn, align 32, !tbaa !3
  tail call void %0() #1
  ret void
  ; TODO: it would be nice if we could have function pointers inlined into the GOT
  ; CHECK:      csc	$c17, $zero, 0($c11)
  ; BIG-NEXT: lui	$1, %captab_hi(fn)
  ; BIG-NEXT: daddiu	$1, $1, %captab_lo(fn)
  ; BIG-NEXT: clc	$c1, $1, 0($c26)
  ; SMALL-NEXT: clcbi $c1, %captab20(fn)($c26)
  ; CHECK-NEXT: clc	$c12, $zero, 0($c1)
  ; CHECK-NEXT: cjalr	$c12, $c17
  ; CHECK-NEXT: cfromptr	$c13, $c0, $zero
  ; CHECK-NEXT: clc	$c17, $zero, 0($c11)
  ; CHECK-NEXT: cjr	$c17

  ;	CHECK:        .type	fn,@object              # @fn
  ; FIXME: why is alignment always 32?
  ; CHECK-NEXT: 	.comm	fn,[[$CAP_SIZE]],32
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 1}
!2 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git c5f3638ca2170161f8769db4a5886c2a206dba1b) (https://github.com/llvm-mirror/llvm.git 28c5d377cdc9e35a9dc5e9e435c8974159c122b6)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"any pointer", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
