; RUN: llc @HYBRID_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=HYBRID
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=PURECAP

define iCAPRANGE @subp(ptr addrspace(200) readnone %a, ptr addrspace(200) readnone %b) nounwind {
  %1 = tail call iCAPRANGE @llvm.cheri.cap.diff.iCAPRANGE(ptr addrspace(200) %a, ptr addrspace(200) %b)
  ret iCAPRANGE %1
}

declare iCAPRANGE @llvm.cheri.cap.diff.iCAPRANGE(ptr addrspace(200), ptr addrspace(200))
