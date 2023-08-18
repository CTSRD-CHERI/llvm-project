; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - < %s | FileCheck %s --check-prefix=PURECAP
; RUN: sed 's/addrspace(200)/addrspace(0)/g' %s | llc @HYBRID_HARDFLOAT_ARGS@ | FileCheck %s --check-prefix HYBRID
; Check that we can lower llvm.frameaddress/llvm.returnaddress

; Capability-Inspection Instructions

define dso_local ptr addrspace(200) @frameaddr() addrspace(200) nounwind {
entry:
  %b = alloca ptr addrspace(200), align 16, addrspace(200)
  %0 = call ptr addrspace(200) @llvm.frameaddress.p200(i32 0)
  store ptr addrspace(200) %0, ptr addrspace(200) %b, align 16
  %1 = load ptr addrspace(200), ptr addrspace(200) %b, align 16
  ret ptr addrspace(200) %1
}

declare ptr addrspace(200) @llvm.frameaddress.p200(i32 immarg) addrspace(200)

define dso_local ptr addrspace(200) @retaddr() addrspace(200) nounwind {
entry:
  %b = alloca ptr addrspace(200), align 16, addrspace(200)
  %0 = call ptr addrspace(200) @llvm.returnaddress.p200(i32 0)
  store ptr addrspace(200) %0, ptr addrspace(200) %b, align 16
  %1 = load ptr addrspace(200), ptr addrspace(200) %b, align 16
  ret ptr addrspace(200) %1
}

declare ptr addrspace(200) @llvm.returnaddress.p200(i32 immarg) addrspace(200)
