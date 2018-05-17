; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -o - -mxcaptable | %cheri_FileCheck %s
; ModuleID = '/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/cap-table-call-extern.c'

source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/cap-table-call-extern.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define void @a() local_unnamed_addr #0 {
entry:
  %call = tail call i32 (...) @b() #2
  ret void
; Make sure we don't use $gp
; CHECK: 	    cincoffset	$c11, $c11, -[[$CAP_SIZE]]
; CHECK-NEXT:	csc	$c17, $zero, 0($c11)    # [[$CAP_SIZE]]-byte Folded Spill
; CHECK-NEXT:	lui	$1, %capcall_hi(b)
; CHECK-NEXT:	daddiu	$1, $1, %capcall_lo(b)
; CHECK-NEXT:	clc	$c12, $1, 0($c26)
; CHECK-NEXT:	cjalr	$c12, $c17
; CHECK-NEXT:	nop
; CHECK-NEXT:	clc	$c17, $zero, 0($c11)    # [[$CAP_SIZE]]-byte Folded Reload
; CHECK-NEXT:	cjr	$c17
; CHECK-NEXT:	cincoffset	$c11, $c11, [[$CAP_SIZE]]
}

declare i32 @b(...) local_unnamed_addr #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri,-noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 1}
!2 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git e2ff9176d9192cfbbf64adc4189992f67cdbf13b) (https://github.com/llvm-mirror/llvm.git c9555a2a44d458d7b63aa114334e2a2d4a1b8f1e)"}
