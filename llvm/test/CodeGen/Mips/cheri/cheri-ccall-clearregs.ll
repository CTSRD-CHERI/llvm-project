; RUN: %cheri_llc %s -relocation-model=pic -o - -cheri-use-clearregs | FileCheck %s
; ModuleID = 'call.c'

; Function Attrs: nounwind
define void @a(i8 addrspace(200)* %a1, i8 addrspace(200)* %a2, i64 %foo, i64 %bar) {
  ; Move the argument registers into the ccall registers
  ; CHECK: cmove	$c2, $c4
  ; CHECK: cmove	$c1, $c3
  ; Move argument 0 from a0 to v0, arg 1 from a1 to a0.
  ; CHECK: move	$2, $4
  ; CHECK: move	$4, $5
  ; Then make sure that we've cleared the other argument registers, with one
  ; clear in the delay slot
  ; CHECK: cclearlo	2040
  ; CHECK-NEXT: jalr	$25
  ; CHECK-NEXT: clearlo	4064
  tail call chericcallcc void @b(i8 addrspace(200)* %a1, i8 addrspace(200)* %a2, i64 %foo, i64 %bar) #2
  ret void
}

declare chericcallcc void @b(i8 addrspace(200)*, i8 addrspace(200)*, i64, i64)

