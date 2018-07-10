; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -mxcaptable=true  -o - -O0 | FileCheck %s -check-prefixes CHECK,BIGTABLE
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -mxcaptable=false -o - -O0 | FileCheck %s -check-prefixes CHECK,SMALLTABLE

; ModuleID = '/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/cap-table-call-extern.c'
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/cap-table-call-extern.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@global = local_unnamed_addr addrspace(200) global i64 123, align 8

declare void @extern_func() #0

; Function Attrs: nounwind
define i64 @test() local_unnamed_addr #1 {
entry:
  %loaded = load i64, i64 addrspace(200)* @global, align 8
  ; CHECK-LABEL: test:
  ; CHECK:         cmove   $c1,  $c26
  ; BIGTABLE-NEXT:      lui  $1, %captab_hi(global)
  ; BIGTABLE-NEXT: daddiu $1, $1, %captab_lo(global)
  ; BIGTABLE-NEXT: clc $c1, $1, 0($c1)
  ; SMALLTABLE-NEXT: clcbi $c1, %captab20(global)($c1)
  ; CHECK-NEXT:    cld $2, $zero, 0($c1)
  ; CHECK-NEXT:    cjr $c17
  ; CHECK-NEXT: nop
  ret i64 %loaded
}


; CHECK-LABEL:        .type   global,@object          # @global
; CHECK-NEXT:         .data
; CHECK-NEXT:         .globl  global
; CHECK-NEXT:         .p2align        3
; CHECK-NEXT: global:
; CHECK-NEXT:         .8byte  123                     # 0x7b
; CHECK-NEXT:         .size   global, 8

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 1}
!2 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git c5f3638ca2170161f8769db4a5886c2a206dba1b) (https://github.com/llvm-mirror/llvm.git 28c5d377cdc9e35a9dc5e9e435c8974159c122b6)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"any pointer", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
