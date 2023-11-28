; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -o - < %s | FileCheck %s --check-prefix=HYBRID
; This test checks target-independent CHERI intrinsics that are only available for hybrid code

; Pointer-Arithmetic Instructions

declare iCAPRANGE @llvm.cheri.cap.to.pointer(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)*, iCAPRANGE)
declare iCAPRANGE @llvm.cheri.cap.diff(i8 addrspace(200)*, i8 addrspace(200)*)
declare i8 addrspace(200)* @llvm.cheri.ddc.get()

; Dummy declaration so that the file is not empty for some targets
define i8 addrspace(200)* @dummy(i8 addrspace(200)* %ptr) nounwind {
  ret i8 addrspace(200)* %ptr
}

#if defined(RISCV32BAKEWELL) | defined(RISCV64BAKEWELL)
define iCAPRANGE @to_pointer_ddc_relative(i8 addrspace(200)* %cap) nounwind {
  %ddc = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %ptr = call iCAPRANGE @llvm.cheri.cap.to.pointer(i8 addrspace(200)* %ddc, i8 addrspace(200)* %cap)
  ret iCAPRANGE %ptr
}

define i8 addrspace(200)* @from_ddc(iCAPRANGE %ptr) nounwind {
  %ddc = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  %cap = call i8 addrspace(200)* @llvm.cheri.cap.from.pointer(i8 addrspace(200)* %ddc, iCAPRANGE %ptr)
  ret i8 addrspace(200)* %cap
}

define i8 addrspace(200)* @ddc_get() nounwind {
  %cap = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  ret i8 addrspace(200)* %cap
}

; PURECAP-ERROR: LLVM ERROR: Cannot select: intrinsic %llvm.cheri.ddc.get
#endif

