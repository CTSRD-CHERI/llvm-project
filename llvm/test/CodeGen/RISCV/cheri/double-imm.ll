; Check that .LCPI symbols have a size associated with them:
; RUN: %riscv64_cheri_purecap_llc -mattr=+d -verify-machineinstrs < %s
; RUN: %riscv64_cheri_purecap_llc -mattr=+d -verify-machineinstrs < %s | FileCheck %s

; CHECK-LABEL: .section .sdata,"aw",@progbits
; CHECK-NEXT:  .p2align 3
; CHECK-NEXT: .LCPI0_0:
; CHECK-NEXT:  .quad 4607182418800017408 # double 1
; CHECK-NEXT:  .size .LCPI0_0, 8
; CHECK-NEXT:  .text

define double @double_add_const(double %a) nounwind {
; CHECK-LABEL: double_add_const:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .LBB0_1: # Label of block must be emitted
; CHECK-NEXT:    auipcc ca1, %captab_pcrel_hi(.LCPI0_0)
; CHECK-NEXT:    clc ca1, %pcrel_lo(.LBB0_1)(ca1)
; CHECK-NEXT:    cfld ft0, 0(ca1)
; CHECK-NEXT:    fmv.d.x ft1, a0
; CHECK-NEXT:    fadd.d ft0, ft1, ft0
; CHECK-NEXT:    fmv.x.d a0, ft0
; CHECK-NEXT:    cret
  %1 = fadd double %a, 1.0
  ret double %1
}

; CHECK-LABEL: .section .sdata,"aw",@progbits
; CHECK-NEXT:  .p2align 2
; CHECK-NEXT: .LCPI1_0:
; CHECK-NEXT:  .word 1065353216 # float 1
; CHECK-NEXT:  .size .LCPI1_0, 4
; CHECK-NEXT:  .text

define float @float_add_const(float %a) nounwind {
; CHECK-LABEL: float_add_const:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .LBB1_1: # Label of block must be emitted
; CHECK-NEXT:    auipcc ca1, %captab_pcrel_hi(.LCPI1_0)
; CHECK-NEXT:    clc ca1, %pcrel_lo(.LBB1_1)(ca1)
; CHECK-NEXT:    cflw ft0, 0(ca1)
; CHECK-NEXT:    fmv.w.x ft1, a0
; CHECK-NEXT:    fadd.s ft0, ft1, ft0
; CHECK-NEXT:    fmv.x.w a0, ft0
; CHECK-NEXT:    cret
  %1 = fadd float %a, 1.0
  ret float %1
}
