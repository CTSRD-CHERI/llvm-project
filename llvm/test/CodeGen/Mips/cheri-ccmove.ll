; RUN: llc -mtriple=cheri-unknown-freebsd -mcpu=cheri %s -o - | FileCheck %s
; ModuleID = 'cbez.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind readnone
define i8 addrspace(200)* @ez(i32 signext %a, i8 addrspace(200)* readnone %b, i8 addrspace(200)* readnone %c) #0 {
entry:
  %tobool = icmp eq i32 %a, 0
  ; CHECK: ccmovz	$c3, $c4, $4
  %c.b = select i1 %tobool, i8 addrspace(200)* %c, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %c.b
}

; Function Attrs: nounwind readnone
define i8 addrspace(200)* @nz(i32 signext %a, i8 addrspace(200)* readnone %b, i8 addrspace(200)* readnone %c) #0 {
entry:
  %tobool = icmp ne i32 %a, 0
  ; CHECK: ccmovn	$c3, $c4, $4
  %c.b = select i1 %tobool, i8 addrspace(200)* %c, i8 addrspace(200)* %b
  ret i8 addrspace(200)* %c.b
}


attributes #0 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm/tools/clang 5cb14ce28a05af60f3a31b8dc14403511ddaac80) (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm 54548ddd0faf42ec91fba879fb03b9ce410c4091)"}
