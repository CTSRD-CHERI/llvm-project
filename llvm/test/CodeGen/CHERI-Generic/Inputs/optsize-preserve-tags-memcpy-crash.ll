; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s -o - | FileCheck %s
; The following code copying 31 bytes (with capability alignment) using the
; must_preserve_tags attribute used to trigger a "(Align < CapSize)" assertion
; inside diagnoseInefficientCheriMemOp() when compiling with -Oz.
; This function should not be called since the reason we are falling back to memcpy
; is that the load/store limit is reached (and not the alignment).
; However, the code was checking for limit reached using a simple `(CapSize * Limit) < Size`
; check which fails here since the last 15 bytes need four (8 + 4 + 2 + 1 bytes) copies on
; architectures where LLVM does not emit misaligned loads/stores.

define hidden void @optnone_preserve_tags_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) optnone noinline nounwind {
  tail call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %dst, i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %src, i64 31, i1 false) must_preserve_cheri_tags
  ret void
}

define hidden void @optsize_preserve_tags_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) optsize nounwind {
  tail call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %dst, i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %src, i64 31, i1 false) must_preserve_cheri_tags
  ret void
}

define hidden void @default_preserve_tags_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) nounwind {
  tail call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %dst, i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %src, i64 31, i1 false) must_preserve_cheri_tags
  ret void
}

define hidden void @optnone_preserve_tags_memmove(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) optnone noinline nounwind {
  tail call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %dst, i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %src, i64 31, i1 false) must_preserve_cheri_tags
  ret void
}

define hidden void @optsize_preserve_tags_memmove(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) optsize nounwind {
  tail call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %dst, i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %src, i64 31, i1 false) must_preserve_cheri_tags
  ret void
}

define hidden void @default_preserve_tags_memmove(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) nounwind{
  tail call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %dst, i8 addrspace(200)* noundef nonnull align 16 dereferenceable(31) %src, i64 31, i1 false) must_preserve_cheri_tags
  ret void
}

declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* noalias nocapture writeonly, i8 addrspace(200)* noalias nocapture readonly, i64, i1 immarg) addrspace(200)
declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* noalias nocapture writeonly, i8 addrspace(200)* noalias nocapture readonly, i64, i1 immarg) addrspace(200)
