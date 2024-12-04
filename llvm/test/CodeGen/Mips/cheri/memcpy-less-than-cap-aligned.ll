; RUN: %cheri_purecap_llc -O2 -verify-machineinstrs < %s | FileCheck %s -check-prefixes CHECK,CHERI128

declare void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) noalias nocapture writeonly, ptr addrspace(200) noalias nocapture readonly, i64, i1 immarg) addrspace(200)
declare void @llvm.memmove.p200.p200.i64(ptr addrspace(200) nocapture writeonly, ptr addrspace(200) nocapture readonly, i64, i1 immarg) addrspace(200)

declare ptr addrspace(200) @llvm.cheri.cap.address.set.i64(ptr addrspace(200), i64) addrspace(200)

define void @dst_align1_should_call_memcpy(ptr addrspace(200) %dst, ptr addrspace(200) %src) addrspace(200) nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 1 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 1 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align1_should_call_memcpy:
  ; CHECK: clcbi $c12, %capcall20(memcpy)($c
  ; CHECK: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align2_should_call_memcpy(ptr addrspace(200) %dst, ptr addrspace(200) %src) addrspace(200) nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 2 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 2 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align2_should_call_memcpy:
  ; CHECK: clcbi $c12, %capcall20(memcpy)($c
  ; CHECK: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align4_should_call_memcpy(ptr addrspace(200) %dst, ptr addrspace(200) %src) addrspace(200) nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 4 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align4_should_call_memcpy:
  ; CHECK: clcbi $c12, %capcall20(memcpy)($c
  ; CHECK: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align8_should_call_memcpy(ptr addrspace(200) %dst, ptr addrspace(200) %src) addrspace(200) nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 8 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 8 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align8_should_call_memcpy:
  ; CHECK: clcbi $c12, %capcall20(memcpy)($c
  ; CHECK: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align16_can_be_inlined_128(ptr addrspace(200) %dst, ptr addrspace(200) %src) addrspace(200) nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align16_can_be_inlined_128:
  ; CHERI128: clc	$c1, $zero, 0($c4)
  ; CHERI128: csc	$c1, $zero, 0($c3)
  ; CHERI128: clc	$c1, $zero, 0($c4)
  ; CHERI128: csc	$c1, $zero, 0($c3)
  ; CHERI256: clcbi $c12, %capcall20(memcpy)($c
  ; CHERI256: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align32_can_be_inlined_always(ptr addrspace(200) %dst, ptr addrspace(200) %src) addrspace(200) nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 32 %dst, ptr addrspace(200) align 32 %src, i64 32, i1 false)
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 32 %dst, ptr addrspace(200) align 32 %src, i64 32, i1 false)
  ret void
  ; CHECK-LABEL: dst_align32_can_be_inlined_always:
  ; CHECK: clc	$c1, $zero, 0($c4)
  ; CHECK: csc	$c1, $zero, 0($c3)
  ; CHERI128: clc	$c1, $zero, 16($c4)
  ; CHERI128: csc	$c1, $zero, 16($c3)
  ; CHECK: clc	$c1, $zero, 0($c4)
  ; CHECK: csc	$c1, $zero, 0($c3)
  ; CHERI128: clc	$c1, $zero, 16($c4)
  ; CHERI128: csc	$c1, $zero, 16($c3)
}
