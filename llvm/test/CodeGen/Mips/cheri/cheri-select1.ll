; RUN: %cheri_llc %s -o -
; ModuleID = 'select.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind readnone
define i8 addrspace(200)* @select(i8 addrspace(200)* readnone %b, i8 addrspace(200)* readnone %c) #0 {
entry:
  %cmp = icmp ult i8 addrspace(200)* %b, %c
  %cond = select i1 %cmp, i8 addrspace(200)* %b, i8 addrspace(200)* %c
  ret i8 addrspace(200)* %cond
}

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 "}
