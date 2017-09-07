; RUN: %cheri128_llc -O2 -o - -relocation-model=pic %s | FileCheck %s
; ModuleID = 'cheri128.c'
target datalayout = "E-m:m-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

@array = common global [2 x i8 addrspace(200)*] zeroinitializer, align 16

; Function Attrs: nounwind readonly
define i8 addrspace(200)* @get(i32 signext %x) #0 {
entry:
  %idxprom = sext i32 %x to i64
  ; Check that we convert the index to an offset by multiplying by 16:
  ; CHECK: dsll	$2, $4, 4
  %arrayidx = getelementptr inbounds [2 x i8 addrspace(200)*], [2 x i8 addrspace(200)*]* @array, i64 0, i64 %idxprom
  %0 = load i8 addrspace(200)*, i8 addrspace(200)** %arrayidx, align 16
  ret i8 addrspace(200)* %0
}

attributes #0 = { nounwind readonly "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 "}
