; RUN: llc -mcpu=cheri -O2 -o - -mattr=+cheri128 %s | FileCheck %s
; ModuleID = 'cheri128.c'
target datalayout = "E-m:m-p200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

@array = common global [2 x i8 addrspace(200)*] zeroinitializer, align 16

; Function Attrs: nounwind readonly
define i8 addrspace(200)* @get(i32 signext %x) #0 {
entry:
  %idxprom = sext i32 %x to i64
  ; Check that we convert the index to an offset by multiplying by 16:
  ; CHECK: dsll	$2, $4, 4
  %arrayidx = getelementptr inbounds [2 x i8 addrspace(200)*]* @array, i64 0, i64 %idxprom
  %0 = load i8 addrspace(200)** %arrayidx, align 16, !tbaa !1
  ret i8 addrspace(200)* %0
}

attributes #0 = { nounwind readonly "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.6.0 "}
!1 = metadata !{metadata !2, metadata !2, i64 0}
!2 = metadata !{metadata !"any pointer", metadata !3, i64 0}
!3 = metadata !{metadata !"omnipotent char", metadata !4, i64 0}
!4 = metadata !{metadata !"Simple C/C++ TBAA"}
