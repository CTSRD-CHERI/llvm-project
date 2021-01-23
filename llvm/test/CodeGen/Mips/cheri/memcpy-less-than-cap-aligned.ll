; RUN: sed 's/align 16 %%src, i64 16/align %cheri_cap_bytes %%src, i64 %cheri_cap_bytes/g' %s > %t.ll
; RUN: %cheri_purecap_llc %t.ll -o - -O2 -verify-machineinstrs | FileCheck %s -check-prefixes CHECK,CHERI128

declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1) addrspace(200) #1
declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1) addrspace(200) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.address.set.i64(i8 addrspace(200)*, i64) addrspace(200) #1

define void @dst_align1_should_call_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) #0 {
entry:
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align1_should_call_memcpy:
  ; CHECK: clcbi $c12, %capcall20(memcpy)($c
  ; CHECK: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align2_should_call_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) #0 {
entry:
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 2 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 2 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align2_should_call_memcpy:
  ; CHECK: clcbi $c12, %capcall20(memcpy)($c
  ; CHECK: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align4_should_call_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) #0 {
entry:
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 4 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 4 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align4_should_call_memcpy:
  ; CHECK: clcbi $c12, %capcall20(memcpy)($c
  ; CHECK: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align8_should_call_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) #0 {
entry:
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 8 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 8 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align8_should_call_memcpy:
  ; CHECK: clcbi $c12, %capcall20(memcpy)($c
  ; CHECK: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align16_can_be_inlined_128(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) #0 {
entry:
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 addrspace(200)* align 16 %src, i64 16, i1 false)
  ret void
  ; CHECK-LABEL: dst_align16_can_be_inlined_128:
  ; CHERI128: clc	$c1, $zero, 0($c4)
  ; CHERI128: csc	$c1, $zero, 0($c3)
  ; CHERI128: clc	$c1, $zero, 0($c4)
  ; CHERI128: csc	$c1, $zero, 0($c3)
  ; CHERI256: clcbi $c12, %capcall20(memcpy)($c
  ; CHERI256: clcbi $c12, %capcall20(memmove)($c
}

define void @dst_align32_can_be_inlined_always(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) #0 {
entry:
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 32 %dst, i8 addrspace(200)* align 32 %src, i64 32, i1 false)
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 32 %dst, i8 addrspace(200)* align 32 %src, i64 32, i1 false)
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




attributes #0 = { nounwind "use-soft-float"="true" }
attributes #1 = { argmemonly nounwind }

