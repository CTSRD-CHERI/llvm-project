; Check that llvm.memcpy() and llvm.memmove() intrinisics with must_preserve_cheri_tags
; attribute are always lowered to libcalls
; RUN: %cheri128_purecap_llc %s -o - | FileCheck %s -check-prefixes=CHECK,PURECAP
; RUN: %cheri128_llc %s -o - | FileCheck %s -check-prefixes=CHECK,HYBRID

declare void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) noalias nocapture writeonly, ptr addrspace(200) noalias nocapture readonly, i64, i1 immarg) #1
declare void @llvm.memmove.p200.p200.i64(ptr addrspace(200) nocapture writeonly, ptr addrspace(200) nocapture readonly, i64, i1 immarg) #1

define void @memcpy_too_big_unaligned(ptr addrspace(200) %dst, ptr addrspace(200) %src) noinline nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 1 %dst, ptr addrspace(200) align 16 %src, i64 2048, i1 false)
  ret void
  ; CHECK-LABEL: memcpy_too_big_unaligned:
  ; PURECAP: clcbi $c12, %capcall20(memcpy)
  ; HYBRID: jal	memcpy_c
}

define void @memcpy_aligned(ptr addrspace(200) %dst, ptr addrspace(200) %src) noinline nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  ret void
  ; This can be inlined;
  ; CHECK-LABEL: memcpy_aligned:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: clc $c1, $zero, 0($c4)
  ; PURECAP-NEXT: cjr $c17
  ; HYBRID-NEXT:  jr $ra
  ; CHECK-NEXT: csc $c1, $zero, 0($c3)
}

define void @memcpy_aligned_must_preserve_tags(ptr addrspace(200) %dst, ptr addrspace(200) %src) noinline nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false) #2
  ret void
  ; Correctly aligned -> no need for libcall (even with attribute)
  ; CHECK-LABEL: memcpy_aligned_must_preserve_tags:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: clc $c1, $zero, 0($c4)
  ; PURECAP-NEXT: cjr $c17
  ; HYBRID-NEXT:  jr $ra
  ; CHECK-NEXT: csc $c1, $zero, 0($c3)
}

define void @memcpy_underaligned_must_preserve_tags(ptr addrspace(200) %dst, ptr addrspace(200) %src) noinline nounwind {
entry:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 8 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false) #2
  ret void
  ; The memmove could be inlined but was tagged with nobuiltin -> should call memmove()
  ; CHECK-LABEL: memcpy_underaligned_must_preserve_tags:
  ; PURECAP: clcbi $c12, %capcall20(memcpy)
  ; HYBRID: jal	memcpy_c
}


; Same again in hybrid:

define void @memmove_too_big_unaligned(ptr addrspace(200) %dst, ptr addrspace(200) %src) noinline nounwind {
entry:
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 1 %dst, ptr addrspace(200) align 16 %src, i64 2048, i1 false)
  ret void
  ; CHECK-LABEL: memmove_too_big_unaligned:
  ; PURECAP: clcbi $c12, %capcall20(memmove)
  ; HYBRID: jal	memmove_c
}

define void @memmove_aligned(ptr addrspace(200) %dst, ptr addrspace(200) %src) noinline nounwind {
entry:
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  ret void
  ; This can be inlined;
  ; CHECK-LABEL: memmove_aligned:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: clc $c1, $zero, 0($c4)
  ; PURECAP-NEXT: cjr $c17
  ; HYBRID-NEXT:  jr $ra
  ; CHECK-NEXT: csc $c1, $zero, 0($c3)
}

define void @memmove_underaligned_must_preserve_tags(ptr addrspace(200) %dst, ptr addrspace(200) %src) noinline nounwind {
entry:
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 8 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false) #2
  ret void
  ; The memmove could be inlined but was tagged with nobuiltin -> should call memmove()
  ; CHECK-LABEL: memmove_underaligned_must_preserve_tags:
  ; PURECAP: clcbi $c12, %capcall20(memmove)
  ; HYBRID: jal	memmove_c
}

define void @memmove_aligned_must_preserve_tags(ptr addrspace(200) %dst, ptr addrspace(200) %src) noinline nounwind {
entry:
  call void @llvm.memmove.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false) #2
  ret void
  ; Correctly aligned -> no need for libcall (even with attribute)
  ; CHECK-LABEL: memmove_aligned_must_preserve_tags:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: clc $c1, $zero, 0($c4)
  ; PURECAP-NEXT: cjr $c17
  ; HYBRID-NEXT:  jr $ra
  ; CHECK-NEXT: csc $c1, $zero, 0($c3)
}
