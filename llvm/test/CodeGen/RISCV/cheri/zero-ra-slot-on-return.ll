; Check that we clear the cra stack slot prior to return if -zero-return-slot-after-restore
; is passed.
; RUN: %riscv64_cheri_purecap_llc -O3 < %s \
; RUN:   | FileCheck --check-prefixes=CHECK,NOZERO %s
; RUN: %riscv64_cheri_purecap_llc -O3 -zero-return-slot-after-restore < %s \
; RUN:   | FileCheck --check-prefixes=CHECK,ZERO %s

; Also check that hybrid mode clears the ra slot:
; RUN: sed 's/addrspace(200)/addrspace(0)/g' %s | %riscv64_cheri_llc -O3 \
; RUN:   | FileCheck --check-prefixes=HYBRID,HYBRID-NOZERO %s
; RUN: sed 's/addrspace(200)/addrspace(0)/g' %s | %riscv64_cheri_llc -O3 -zero-return-slot-after-restore \
; RUN:   | FileCheck --check-prefixes=HYBRID,HYBRID-ZERO %s

declare i8 addrspace(200)* @notdead(i8 addrspace(200)*) addrspace(200)

; If ra is not saved, we don't need to zero it:
define i8 addrspace(200)* @direct_return() addrspace(200) nounwind {
; CHECK-LABEL: direct_return:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cmove ca0, cnull
; CHECK-NEXT:    cret
; HYBRID-LABEL: direct_return:
; HYBRID:       # %bb.0:
; HYBRID-NEXT:    mv a0, zero
; HYBRID-NEXT:    ret
  ret i8 addrspace(200)* null
}

; If "frame-pointer"="all" is specified ra is always saved, so check that we clear it here:
define i8 addrspace(200)* @direct_return_with_fp() addrspace(200) nounwind "frame-pointer"="all" {
; CHECK-LABEL: direct_return_with_fp:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cincoffset csp, csp, -32
; CHECK-NEXT:    csc cra, 16(csp)
; CHECK-NEXT:    csc cs0, 0(csp)
; CHECK-NEXT:    cincoffset cs0, csp, 32
; CHECK-NEXT:    cmove ca0, cnull
; CHECK-NEXT:    clc cs0, 0(csp)
; CHECK-NEXT:    clc cra, 16(csp)
; ZERO-NEXT:     csc cnull, 16(csp)
; CHECK-NEXT:    cincoffset csp, csp, 32
; CHECK-NEXT:    cret
; HYBRID-LABEL:   direct_return_with_fp:
; HYBRID:         # %bb.0:
; HYBRID-NEXT:      addi sp, sp, -16
; HYBRID-NEXT:      sd ra, 8(sp)
; HYBRID-NEXT:      sd s0, 0(sp)
; HYBRID-NEXT:      addi s0, sp, 16
; HYBRID-NEXT:      mv a0, zero
; HYBRID-NEXT:      ld s0, 0(sp)
; HYBRID-NEXT:      ld ra, 8(sp)
; HYBRID-ZERO-NEXT: sd zero, 8(sp)
; HYBRID-NEXT:      addi sp, sp, 16
; HYBRID-NEXT:      ret
  ret i8 addrspace(200)* null
}

define i8 addrspace(200)* @must_save_ra() addrspace(200) nounwind {
; CHECK-LABEL: must_save_ra:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cincoffset csp, csp, -16
; CHECK-NEXT:    csc cra, 0(csp)
; CHECK-NEXT:  .LBB2_1: # Label of block must be emitted
; CHECK-NEXT:    auipcc ca1, %captab_pcrel_hi(notdead)
; CHECK-NEXT:    clc ca1, %pcrel_lo(.LBB2_1)(ca1)
; CHECK-NEXT:    cmove ca0, cnull
; CHECK-NEXT:    cjalr ca1
; CHECK-NEXT:    clc cra, 0(csp)
; ZERO-NEXT:     csc cnull, 0(csp)
; CHECK-NEXT:    cincoffset csp, csp, 16
; CHECK-NEXT:    cret
; HYBRID-LABEL:   must_save_ra:
; HYBRID:         # %bb.0:
; HYBRID-NEXT:      addi sp, sp, -16
; HYBRID-NEXT:      sd ra, 8(sp)
; HYBRID-NEXT:      mv a0, zero
; HYBRID-NEXT:      call notdead
; HYBRID-NEXT:      ld ra, 8(sp)
; HYBRID-ZERO-NEXT: sd zero, 8(sp)
; HYBRID-NEXT:      addi sp, sp, 16
; HYBRID-NEXT:      ret
  %ret = call addrspace(200) i8 addrspace(200)* @notdead(i8 addrspace(200)* null)
  ret i8 addrspace(200)* %ret
}


define i8 addrspace(200)* @must_save_ra_with_alloca() addrspace(200) nounwind {
; CHECK-LABEL: must_save_ra_with_alloca:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cincoffset csp, csp, -128
; CHECK-NEXT:    csc cra, 112(csp)
; CHECK-NEXT:    addi a0, zero, 100
; CHECK-NEXT:    cincoffset ca1, csp, 12
; CHECK-NEXT:    csetbounds ca0, ca1, a0
; CHECK-NEXT:  .LBB3_1: # Label of block must be emitted
; CHECK-NEXT:    auipcc ca1, %captab_pcrel_hi(notdead)
; CHECK-NEXT:    clc ca1, %pcrel_lo(.LBB3_1)(ca1)
; CHECK-NEXT:    cjalr ca1
; CHECK-NEXT:    clc cra, 112(csp)
; ZERO-NEXT:     csc cnull, 112(csp)
; CHECK-NEXT:    cincoffset csp, csp, 128
; CHECK-NEXT:    cret
; HYBRID-LABEL:   must_save_ra_with_alloca:
; HYBRID:         # %bb.0:
; HYBRID-NEXT:      addi sp, sp, -112
; HYBRID-NEXT:      sd ra, 104(sp)
; HYBRID-NEXT:      addi a0, sp, 4
; HYBRID-NEXT:      call notdead
; HYBRID-NEXT:      ld ra, 104(sp)
; HYBRID-ZERO-NEXT: sd zero, 104(sp)
; HYBRID-NEXT:      addi sp, sp, 112
; HYBRID-NEXT:      ret
  %1 = alloca [100 x i8], addrspace(200)
  %2 = bitcast [100 x i8] addrspace(200)* %1 to i8 addrspace(200)*
  %ret = call addrspace(200) i8 addrspace(200)* @notdead(i8 addrspace(200)* %2)
  ret i8 addrspace(200)* %ret
}
