; RUN: %cheri_llc %s -o - | FileCheck %s --check-prefixes=CHECK-INEXACT,CHECK
; RUN: %cheri_llc -cheri-exact-equals=true %s -o - | FileCheck %s --check-prefixes=CHECK-EXACT,CHECK
; RUN: %cheri_llc -cheri-exact-equals=false %s -o - | FileCheck %s --check-prefixes=CHECK-INEXACT,CHECK
; ModuleID = 'ptrcmp.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind readnone
; CHECK-LABEL: eq:
define i32 @eq(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK-INEXACT: ceq	$2, $c3, $c4
  ; CHECK-EXACT: cexeq	$2, $c3, $c4
  %cmp = icmp eq i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind readnone
; CHECK-LABEL: ne:
define i32 @ne(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK-INEXACT: cne	$2, $c3, $c4
  ; CHECK-EXACT: cnexeq	$2, $c3, $c4
  %cmp = icmp ne i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind readnone
; CHECK-LABEL: lt:
define i32 @lt(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK: clt	$2, $c3, $c4
  %cmp = icmp slt i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}
; Function Attrs: nounwind readnone
; CHECK-LABEL: ult:
define i32 @ult(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK: cltu	$2, $c3, $c4
  %cmp = icmp ult i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind readnone
; CHECK-LABEL: le:
define i32 @le(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK: cle	$2, $c3, $c4
  %cmp = icmp sle i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind readnone
; CHECK-LABEL: le:
define i32 @ule(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK: cleu	$2, $c3, $c4
  %cmp = icmp ule i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind readnone
; CHECK-LABEL: gt:
define i32 @gt(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK: clt	$2, $c4, $c3
  %cmp = icmp sgt i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind readnone
; CHECK-LABEL: ugt:
define i32 @ugt(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK: cltu	$2, $c4, $c3
  %cmp = icmp ugt i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind readnone
; CHECK-LABEL: ge:
define i32 @ge(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK: cle	$2, $c4, $c3
  %cmp = icmp sge i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind readnone
; CHECK-LABEL: uge:
define i32 @uge(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) #0 {
entry:
  ; CHECK: cleu	$2, $c4, $c3
  %cmp = icmp uge i8 addrspace(200)* %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 "}
