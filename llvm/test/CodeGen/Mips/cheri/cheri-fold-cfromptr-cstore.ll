; RUN: %cheri_llc %s -O1 -o - | FileCheck %s
; ModuleID = 'fold-cfromptr-cstore.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

%struct.int_struct = type { i32 }

@b = common global i32 0, align 4
@a = common global %struct.int_struct zeroinitializer, align 4

; Function Attrs: nounwind
; CHECK-LABEL: fn1:
define void @fn1() #0 {
entry:
  %0 = load i32, i32* @b, align 4, !tbaa !1
  store i32 %0, i32 addrspace(200)* getelementptr (%struct.int_struct, %struct.int_struct addrspace(200)* addrspacecast (%struct.int_struct* @a to %struct.int_struct addrspace(200)*), i64 0, i32 0), align 4, !tbaa !5
  ; CHECK-NOT: cfromptr
  ; CHECK-NOT: cfromddc
  ; CHECK-NOT: csw
  ret void
}

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (git@github.com:CTSRD-CHERI/clang 05f124e1657de7fd881aec2ad65f265f657b1edb) (git@github.com:CTSRD-CHERI/llvm 77789fd21f998d693008119df8076f71c7d48bd6)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !2, i64 0}
!6 = !{!"", !2, i64 0}
