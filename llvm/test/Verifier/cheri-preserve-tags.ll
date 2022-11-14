; RUN: not opt -S %s -verify 2>&1 | FileCheck %s --check-prefixes=CHECK,PURECAP
; RUN: not opt -data-layout="p0:64:64:64" -S %s -verify 2>&1 | FileCheck %s --check-prefixes=CHECK,NON-CHERI

target datalayout = "pf200:128:128:128:64"

declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1)
declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i1)
declare void @llvm.memset.p200i8.i64(i8 addrspace(200)* nocapture, i8, i64, i1) nounwind

define void @too_small(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) #0 {
entry:

  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %dst, i8 addrspace(200)* align 16 %src, i64 15, i1 false) must_preserve_cheri_tags
  ; PURECAP: Attribute 'must_preserve_cheri_tags' cannot be set on copies of less than capability size
  ; NON-CHERI: Attribute 'must_preserve_cheri_tags' requires a CHERI target
  ; CHECK-NEXT: call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %dst, i8 addrspace(200)* align 16 %src, i64 15, i1 false)
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %dst, i8 addrspace(200)* align 16 %src, i64 15, i1 false) must_preserve_cheri_tags
  ; PURECAP-NEXT: Attribute 'must_preserve_cheri_tags' cannot be set on copies of less than capability size
  ; NON-CHERI-NEXT: Attribute 'must_preserve_cheri_tags' requires a CHERI target
  ; CHECK-NEXT: call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 1 %dst, i8 addrspace(200)* align 16 %src, i64 15, i1 false)
  ret void
}

define void @memset_invalid(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) #0 {
entry:
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 0, i64 16, i1 false) must_preserve_cheri_tags
  ; NON-CHERI-NEXT: Attribute 'must_preserve_cheri_tags' requires a CHERI target
  ; NON-CHERI-NEXT: call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 0, i64 16, i1 false)
  ; CHECK-NEXT: Attribute 'must_preserve_cheri_tags' cannot be used with memset
  ; CHECK-NEXT: call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 0, i64 16, i1 false)
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 0, i64 16, i1 false) no_preserve_cheri_tags
  ; NON-CHERI-NEXT: Attribute 'no_preserve_cheri_tags' requires a CHERI target
  ; NON-CHERI-NEXT: call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 0, i64 16, i1 false)
  ; CHECK-NEXT: Attribute 'no_preserve_cheri_tags' cannot be used with memset
  ; CHECK-NEXT: call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 0, i64 16, i1 false)
  ret void
}

; CHECK-NEXT: error: input module is broken!
