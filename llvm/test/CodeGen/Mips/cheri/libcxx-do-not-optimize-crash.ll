; RUN: not %cheri_purecap_llc -verify-machineinstrs %s 2>&1 | FileCheck %s -check-prefix=ERR
; ERR: error: couldn't allocate output register for constraint 'C'
; Check that this results in an error instead of a crash

$_Z13DoNotOptimizeIbEvU3capRT_ = comdat any

; Function Attrs: noinline nounwind optnone
define void @_Z13DoNotOptimizeIbEvU3capRT_() addrspace(200) #0 comdat {
entry:
  %0 = call i8 asm sideeffect "", "=C,0,~{memory},~{$1}"(i1 undef) #1
  ret void
}
