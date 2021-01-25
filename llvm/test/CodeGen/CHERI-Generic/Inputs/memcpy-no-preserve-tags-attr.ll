; Check that the no_preserve_tags annotation on memcpy/memmove intrinsics allows
; use to inline struct copies >= capability size.
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -o - < %s | FileCheck %s

%struct.pair = type { iCAPRANGE, iCAPRANGE }

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1)
declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1)

; Without a no_preserve_tags attribute we always call memcpy. In this case we
; don't know whether the type might actually contain capabilities (e.g. unions).
define void @memcpy_no_attr(%struct.pair addrspace(200)* %a, %struct.pair addrspace(200)* %b) addrspace(200) nounwind {
entry:
  %a_i8 = bitcast %struct.pair addrspace(200)* %a to i8 addrspace(200)*
  %b_i8 = bitcast %struct.pair addrspace(200)* %b to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %a_i8, i8 addrspace(200)* align @CAP_RANGE_BYTES@ %b_i8, i64 16, i1 false)
  ret void
}

define void @memmove_no_attr(%struct.pair addrspace(200)* %a, %struct.pair addrspace(200)* %b) addrspace(200) nounwind {
entry:
  %a_i8 = bitcast %struct.pair addrspace(200)* %a to i8 addrspace(200)*
  %b_i8 = bitcast %struct.pair addrspace(200)* %b to i8 addrspace(200)*
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %a_i8, i8 addrspace(200)* align @CAP_RANGE_BYTES@ %b_i8, i64 16, i1 false)
  ret void
}

; We have to emit a call if the intrinsic has must_preserve_cheri_tags:
define void @memcpy_must_preserve(%struct.pair addrspace(200)* %a, %struct.pair addrspace(200)* %b) addrspace(200) nounwind {
entry:
  %a_i8 = bitcast %struct.pair addrspace(200)* %a to i8 addrspace(200)*
  %b_i8 = bitcast %struct.pair addrspace(200)* %b to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %a_i8, i8 addrspace(200)* align @CAP_RANGE_BYTES@ %b_i8, i64 16, i1 false) must_preserve_cheri_tags
  ret void
}

define void @memmove_must_preserve(%struct.pair addrspace(200)* %a, %struct.pair addrspace(200)* %b) addrspace(200) nounwind {
entry:
  %a_i8 = bitcast %struct.pair addrspace(200)* %a to i8 addrspace(200)*
  %b_i8 = bitcast %struct.pair addrspace(200)* %b to i8 addrspace(200)*
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %a_i8, i8 addrspace(200)* align @CAP_RANGE_BYTES@ %b_i8, i64 16, i1 false) must_preserve_cheri_tags
  ret void
}

; We should be able to inline the call memcpy/memmove if the intrinsic has no_preserve_cheri_tags:
; TODO: we should be able to elide this memcpy call
define void @memcpy_no_preserve(%struct.pair addrspace(200)* %a, %struct.pair addrspace(200)* %b) addrspace(200) nounwind {
entry:
  %a_i8 = bitcast %struct.pair addrspace(200)* %a to i8 addrspace(200)*
  %b_i8 = bitcast %struct.pair addrspace(200)* %b to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %a_i8, i8 addrspace(200)* align @CAP_RANGE_BYTES@ %b_i8, i64 16, i1 false) no_preserve_cheri_tags
  ret void
}

define void @memmove_no_preserve(%struct.pair addrspace(200)* %a, %struct.pair addrspace(200)* %b) addrspace(200) nounwind {
entry:
  %a_i8 = bitcast %struct.pair addrspace(200)* %a to i8 addrspace(200)*
  %b_i8 = bitcast %struct.pair addrspace(200)* %b to i8 addrspace(200)*
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %a_i8, i8 addrspace(200)* align @CAP_RANGE_BYTES@ %b_i8, i64 16, i1 false) no_preserve_cheri_tags
  ret void
}
