; RUN: llc @HYBRID_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=HYBRID
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=PURECAP

define iCAPRANGE @subp(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) nounwind {
  %1 = tail call iCAPRANGE @llvm.cheri.cap.diff.iCAPRANGE(i8 addrspace(200)* %a, i8 addrspace(200)* %b)
  ret iCAPRANGE %1
}

declare iCAPRANGE @llvm.cheri.cap.diff.iCAPRANGE(i8 addrspace(200)*, i8 addrspace(200)*)
