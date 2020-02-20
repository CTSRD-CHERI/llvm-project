; RUN: %generic_cheri_hybrid_llc %s -o - | FileCheck %s --check-prefix=HYBRID
; RUN: %generic_cheri_purecap_llc %s -o - | FileCheck %s --check-prefix=PURECAP

define iCAPRANGE @subp(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) nounwind {
entry:
  %0 = tail call iCAPRANGE @llvm.cheri.cap.diff.iCAPRANGE(i8 addrspace(200)* %a, i8 addrspace(200)* %b)
  ret iCAPRANGE %0
}

declare iCAPRANGE @llvm.cheri.cap.diff.iCAPRANGE(i8 addrspace(200)*, i8 addrspace(200)*)
