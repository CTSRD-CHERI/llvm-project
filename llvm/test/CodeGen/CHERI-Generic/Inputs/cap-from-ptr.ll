;; Check that we can correctly generate code for llvm.cheri.cap.from.pointer()
;; This previously asserted on RISC-V due to a broken ISel pattern.
;; We pipe this input through instcombine first to ensure SelectionDAG sees canonical IR.
; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -passes=instcombine -S < %s | FileCheck %s --check-prefix=CHECK-IR
@IFNOT-RISCV@; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -passes=instcombine -S < %s | llc @PURECAP_HARDFLOAT_ARGS@ | FileCheck %s --check-prefix=PURECAP
@IFNOT-RISCV@; RUN: opt @HYBRID_HARDFLOAT_ARGS@ -passes=instcombine -S < %s | llc @HYBRID_HARDFLOAT_ARGS@ | FileCheck %s --check-prefix=HYBRID
@IF-RISCV@; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -passes=instcombine -S < %s | not --crash llc @PURECAP_HARDFLOAT_ARGS@ 2>&1 | FileCheck %s --check-prefix BAD-COPY
@IF-RISCV@; RUN: opt @HYBRID_HARDFLOAT_ARGS@ -passes=instcombine -S < %s | not --crash llc @HYBRID_HARDFLOAT_ARGS@ 2>&1 | FileCheck %s --check-prefix BAD-COPY
@IF-RISCV@; BAD-COPY: Impossible reg-to-reg copy

define internal i8 addrspace(200)* @test(i8 addrspace(200)* addrspace(200)* %ptr, i8 addrspace(200)* %cap, iCAPRANGE %offset) nounwind {
entry:
  %new = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE %offset)
  store i8 addrspace(200)* %new, i8 addrspace(200)* addrspace(200)* %ptr, align 16
  ret i8 addrspace(200)* %new
}

;; (int_cheri_cap_from_ptr x, 0) -> null
define internal i8 addrspace(200)* @cap_from_ptr_zero(i8 addrspace(200)* addrspace(200)* %ptr, i8 addrspace(200)* %cap) nounwind {
entry:
  %new = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer.iCAPRANGE(i8 addrspace(200)* %cap, iCAPRANGE 0)
  store i8 addrspace(200)* %new, i8 addrspace(200)* addrspace(200)* %ptr, align 16
  ret i8 addrspace(200)* %new
}

;; Check that (int_cheri_cap_from_ptr ddc, x) can use the DDC register directly
define internal i8 addrspace(200)* @cap_from_ptr_ddc(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %offset) nounwind {
entry:
  %ddc = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %new = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer.iCAPRANGE(i8 addrspace(200)* %ddc, iCAPRANGE %offset)
  store i8 addrspace(200)* %new, i8 addrspace(200)* addrspace(200)* %ptr, align 16
  ret i8 addrspace(200)* %new
}

;; Check that (int_cheri_cap_from_ptr x, 0) -> null has priority over direct DDC usage
define internal i8 addrspace(200)* @cap_from_ptr_ddc_zero(i8 addrspace(200)* addrspace(200)* %ptr) nounwind {
entry:
  %ddc = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %new = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer.iCAPRANGE(i8 addrspace(200)* %ddc, iCAPRANGE 0)
  store i8 addrspace(200)* %new, i8 addrspace(200)* addrspace(200)* %ptr, align 16
  ret i8 addrspace(200)* %new
}

;; Check that (int_cheri_cap_from_ptr null, x) does not use register zero (since that is DDC)
define internal i8 addrspace(200)* @cap_from_ptr_null(i8 addrspace(200)* addrspace(200)* %ptr, iCAPRANGE %offset) nounwind {
entry:
  %new = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer.iCAPRANGE(i8 addrspace(200)* null, iCAPRANGE %offset)
  store i8 addrspace(200)* %new, i8 addrspace(200)* addrspace(200)* %ptr, align 16
  ret i8 addrspace(200)* %new
}

declare i8 addrspace(200)* @llvm.cheri.cap.from.pointer.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE)
declare i8 addrspace(200)* @llvm.cheri.ddc.get()
