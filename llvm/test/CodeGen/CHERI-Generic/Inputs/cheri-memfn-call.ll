; Check that we call memset_c/memmove_c/memcpy_c in hybrid mode.
; RUN: llc -opaque-pointers @PURECAP_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=PURECAP
; RUN: llc -opaque-pointers @HYBRID_HARDFLOAT_ARGS@ %s -o - | FileCheck %s --check-prefix=HYBRID
%struct.x = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }

declare void @llvm.memmove.p200i8.p200i8.i64(ptr addrspace(200) nocapture, ptr addrspace(200) nocapture readonly, i64, i1)
declare void @llvm.memset.p200i8.i64(ptr addrspace(200) nocapture, i8, i64, i1)
declare void @llvm.memcpy.p200i8.p200i8.i64(ptr addrspace(200) nocapture, ptr addrspace(200) nocapture readonly, i64, i1)

define void @call_memset(ptr addrspace(200) align 4 %dst) nounwind {
entry:
  call void @llvm.memset.p200i8.i64(ptr addrspace(200) align 4 %dst, i8 0, i64 40, i1 false)
  ret void
}

define void @call_memcpy(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 4 %src) nounwind {
entry:
  call void @llvm.memcpy.p200i8.p200i8.i64(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 4 %src, i64 40, i1 false)
  ret void
}

define void @call_memmove(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 4 %src) nounwind {
entry:
  call void @llvm.memmove.p200i8.p200i8.i64(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 4 %src, i64 40, i1 false)
  ret void
}

