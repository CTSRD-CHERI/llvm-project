; RUN: llc -mtriple=cheri-unknown-freebsd -mcpu=cheri %s -o - | FileCheck %s
; ModuleID = 'cheri-ccall-zero-return.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; CHECK: cgetdata
; Function Attrs: nounwind readnone
define chericcallcc i8 addrspace(200)* @cgetdata(i8 addrspace(200)* inreg readnone %c.coerce0, i8 addrspace(200)* inreg nocapture readnone %c.coerce1) #0 {
entry:
; The real return value goes in $c3, check that $v0 and $v1 are zeroed:
; CHECK: daddiu	$2, $zero, 0
; CHECK: daddiu	$3, $zero, 0
  ret i8 addrspace(200)* %c.coerce0
}

; Function Attrs: nounwind readnone
define chericcallcc i32 @cgetnumber(i8 addrspace(200)* inreg nocapture readnone %c.coerce0, i8 addrspace(200)* inreg nocapture readnone %c.coerce1) #0 {
entry:
; The real return value goes in $v0, check that $c3 and $v1 are zeroed:
; CHECK: cfromptr $c3, $c0, $zero
; CHECK: daddiu	$3, $zero, 0
  ret i32 42
}

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="true" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 "}
