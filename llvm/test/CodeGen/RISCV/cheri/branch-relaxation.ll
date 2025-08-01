; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: %riscv32_cheri_purecap_llc -verify-machineinstrs -filetype=obj < %s \
; RUN:   -o /dev/null 2>&1
; RUN: %riscv32_cheri_purecap_llc -relocation-model=pic -verify-machineinstrs \
; RUN:   -filetype=obj < %s -o /dev/null 2>&1
; RUN: %riscv32_cheri_purecap_llc -verify-machineinstrs < %s | FileCheck %s
; RUN: %riscv32_cheri_purecap_llc -relocation-model=pic -verify-machineinstrs < %s \
; RUN:   | FileCheck %s

; Looks identical to the plain RISC-V version, but the j here is the alias for
; CJAL (and -verify-machineinstrs will verify it's that and not JAL).
define void @relax_bcc(i1 %a) nounwind {
; CHECK-LABEL: relax_bcc:
; CHECK:       # %bb.0:
; CHECK-NEXT:    andi a0, a0, 1
; CHECK-NEXT:    bnez a0, .LBB0_1
; CHECK-NEXT:    j .LBB0_2
; CHECK-NEXT:  .LBB0_1: # %iftrue
; CHECK-NEXT:    #APP
; CHECK-NEXT:    .zero 4096
; CHECK-NEXT:    #NO_APP
; CHECK-NEXT:  .LBB0_2: # %tail
; CHECK-NEXT:    ret
  br i1 %a, label %iftrue, label %tail

iftrue:
  call void asm sideeffect ".space 4096", ""()
  br label %tail

tail:
  ret void
}

define i32 @relax_cjal(i1 %a) nounwind {
; CHECK-LABEL: relax_cjal:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cincoffset csp, csp, -16
; CHECK-NEXT:    andi a0, a0, 1
; CHECK-NEXT:    bnez a0, .LBB1_1
; CHECK-NEXT:  # %bb.4:
; CHECK-NEXT:    cjump .LBB1_2, ca0
; CHECK-NEXT:  .LBB1_1: # %iftrue
; CHECK-NEXT:    #APP
; CHECK-NEXT:    #NO_APP
; CHECK-NEXT:    #APP
; CHECK-NEXT:    .zero 1048576
; CHECK-NEXT:    #NO_APP
; CHECK-NEXT:    j .LBB1_3
; CHECK-NEXT:  .LBB1_2: # %jmp
; CHECK-NEXT:    #APP
; CHECK-NEXT:    #NO_APP
; CHECK-NEXT:  .LBB1_3: # %tail
; CHECK-NEXT:    li a0, 1
; CHECK-NEXT:    cincoffset csp, csp, 16
; CHECK-NEXT:    ret
  br i1 %a, label %iftrue, label %jmp

jmp:
  call void asm sideeffect "", ""()
  br label %tail

iftrue:
  call void asm sideeffect "", ""()
  br label %space

space:
  call void asm sideeffect ".space 1048576", ""()
  br label %tail

tail:
  ret i32 1
}
