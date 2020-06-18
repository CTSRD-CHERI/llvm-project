; RUN: %generic_cheri_purecap_llc < %s -o - | FileCheck %s --check-prefix=PURECAP
; RUN: not --crash %generic_cheri_hybrid_llc < %s -o - 2>&1 | FileCheck %s --check-prefix HYBRID-ERROR
; This test checks target-independent CHERI intrinsics that are only available for purecap code

; Currently the only purecap-only intrinsic is llvm.cheri.stack.cap.get()
declare i8 addrspace(200)* @llvm.cheri.stack.cap.get()

define i8 addrspace(200)* @stack_get() nounwind {
  %cap = call i8 addrspace(200)* @llvm.cheri.stack.cap.get()
  ret i8 addrspace(200)* %cap
}
; HYBRID-ERROR: LLVM ERROR: Cannot select: intrinsic %llvm.cheri.stack.cap.get
