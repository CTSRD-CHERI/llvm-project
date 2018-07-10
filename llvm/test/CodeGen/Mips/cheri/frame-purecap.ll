; RUN: %cheri_purecap_llc %s -o - | FileCheck %s 

; Check that the frame directive sets $c17, not $ra, as the return address register.
; CHECK: .frame	$c11,0,$c17
; Function Attrs: noinline nounwind optnone
define void @func()  {
entry:
  ret void
}
