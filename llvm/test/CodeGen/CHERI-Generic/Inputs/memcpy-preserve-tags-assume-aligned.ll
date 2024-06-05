; Check that __builtin_assume_aligned does the right thing and allows us to elide the memcpy
; call even with must_preserve_cheri_tags attribute (run instcombine to propagate assume information)
; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -S -passes=instcombine < %s | llc @PURECAP_HARDFLOAT_ARGS@ -O2 -o - | FileCheck %s
target datalayout = "@PURECAP_DATALAYOUT@"

declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1)
declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1)
declare void @llvm.assume(i1) addrspace(200)

define void @memcpy_assume(i8 addrspace(200)* addrspace(200)* %local_cap_ptr, i8 addrspace(200)* %align1) addrspace(200) nounwind {
  %ptrint = ptrtoint i8 addrspace(200)* %align1 to i64
  %maskedptr = and i64 %ptrint, 15
  %maskcond = icmp eq i64 %maskedptr, 0
  tail call void @llvm.assume(i1 %maskcond)
  %1 = bitcast i8 addrspace(200)* addrspace(200)* %local_cap_ptr to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %align1, i8 addrspace(200)* align 16 %1, i64 32, i1 false) must_preserve_cheri_tags
  ret void
}

define void @memmove_assume(i8 addrspace(200)* addrspace(200)* %local_cap_ptr, i8 addrspace(200)* %align1) addrspace(200) nounwind {
  %ptrint = ptrtoint i8 addrspace(200)* %align1 to i64
  %maskedptr = and i64 %ptrint, 15
  %maskcond = icmp eq i64 %maskedptr, 0
  tail call void @llvm.assume(i1 %maskcond)
  %1 = bitcast i8 addrspace(200)* addrspace(200)* %local_cap_ptr to i8 addrspace(200)*
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %align1, i8 addrspace(200)* align 16 %1, i64 32, i1 false) must_preserve_cheri_tags
  ret void
}

