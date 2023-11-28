;; Check that we can correctly generate code for llvm.cheri.cap.from.pointer()
;; This previously asserted on RISC-V due to a broken ISel pattern.
;; We pipe this input through instcombine first to ensure SelectionDAG sees canonical IR.
; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -passes=instcombine -S < %s | FileCheck %s --check-prefix=CHECK-IR
; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -passes=instcombine -S < %s | llc @PURECAP_HARDFLOAT_ARGS@ | FileCheck %s --check-prefix=PURECAP
; RUN: opt @HYBRID_HARDFLOAT_ARGS@ -passes=instcombine -S < %s | llc @HYBRID_HARDFLOAT_ARGS@ | FileCheck %s --check-prefix=HYBRID

define internal ptr addrspace(200) @test(ptr addrspace(200) %ptr, ptr addrspace(200) %cap, iCAPRANGE %offset) nounwind {
entry:
  %new = call ptr addrspace(200) @llvm.cheri.cap.from.pointer.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE %offset)
  store ptr addrspace(200) %new, ptr addrspace(200) %ptr, align 16
  ret ptr addrspace(200) %new
}

;; (int_cheri_cap_from_ptr x, 0) -> null
define internal ptr addrspace(200) @cap_from_ptr_zero(ptr addrspace(200) %ptr, ptr addrspace(200) %cap) nounwind {
entry:
  %new = call ptr addrspace(200) @llvm.cheri.cap.from.pointer.iCAPRANGE(ptr addrspace(200) %cap, iCAPRANGE 0)
  store ptr addrspace(200) %new, ptr addrspace(200) %ptr, align 16
  ret ptr addrspace(200) %new
}

#if !(defined(RISCV32BAKEWELL) | defined(RISCV64BAKEWELL))
;; Check that (int_cheri_cap_from_ptr ddc, x) can use the DDC register directly
define internal ptr addrspace(200) @cap_from_ptr_ddc(ptr addrspace(200) %ptr, iCAPRANGE %offset) nounwind {
entry:
  %ddc = call ptr addrspace(200) @llvm.cheri.ddc.get()
  %new = call ptr addrspace(200) @llvm.cheri.cap.from.pointer.iCAPRANGE(ptr addrspace(200) %ddc, iCAPRANGE %offset)
  store ptr addrspace(200) %new, ptr addrspace(200) %ptr, align 16
  ret ptr addrspace(200) %new
}

;; Check that (int_cheri_cap_from_ptr x, 0) -> null has priority over direct DDC usage
define internal ptr addrspace(200) @cap_from_ptr_ddc_zero(ptr addrspace(200) %ptr) nounwind {
entry:
  %ddc = call ptr addrspace(200) @llvm.cheri.ddc.get()
  %new = call ptr addrspace(200) @llvm.cheri.cap.from.pointer.iCAPRANGE(ptr addrspace(200) %ddc, iCAPRANGE 0)
  store ptr addrspace(200) %new, ptr addrspace(200) %ptr, align 16
  ret ptr addrspace(200) %new
}
#endif

;; Check that (int_cheri_cap_from_ptr null, x) does not use register zero (since that is DDC)
define internal ptr addrspace(200) @cap_from_ptr_null(ptr addrspace(200) %ptr, iCAPRANGE %offset) nounwind {
entry:
  %new = call ptr addrspace(200) @llvm.cheri.cap.from.pointer.iCAPRANGE(ptr addrspace(200) null, iCAPRANGE %offset)
  store ptr addrspace(200) %new, ptr addrspace(200) %ptr, align 16
  ret ptr addrspace(200) %new
}

declare ptr addrspace(200) @llvm.cheri.cap.from.pointer.iCAPRANGE(ptr addrspace(200), iCAPRANGE)
declare ptr addrspace(200) @llvm.cheri.ddc.get()
