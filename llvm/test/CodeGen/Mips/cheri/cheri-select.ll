; RUN: %cheri_llc %s -o -
; ModuleID = 'select.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define i8 addrspace(200)* @select(i8 addrspace(200)* %b, i8 addrspace(200)* %c) #0 {
entry:
  %b.addr = alloca i8 addrspace(200)*, align 32
  %c.addr = alloca i8 addrspace(200)*, align 32
  store i8 addrspace(200)* %b, i8 addrspace(200)** %b.addr, align 32
  store i8 addrspace(200)* %c, i8 addrspace(200)** %c.addr, align 32
  %0 = load i8 addrspace(200)*, i8 addrspace(200)** %b.addr, align 32
  %1 = load i8 addrspace(200)*, i8 addrspace(200)** %c.addr, align 32
  %cmp = icmp ult i8 addrspace(200)* %0, %1
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %2 = load i8 addrspace(200)*, i8 addrspace(200)** %b.addr, align 32
  br label %cond.end

cond.false:                                       ; preds = %entry
  %3 = load i8 addrspace(200)*, i8 addrspace(200)** %c.addr, align 32
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i8 addrspace(200)* [ %2, %cond.true ], [ %3, %cond.false ]
  ret i8 addrspace(200)* %cond
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 "}
