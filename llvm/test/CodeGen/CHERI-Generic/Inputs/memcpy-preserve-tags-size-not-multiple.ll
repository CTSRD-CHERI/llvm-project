; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -o - -O0 -verify-machineinstrs %s | FileCheck %s -check-prefixes CHECK
; Check that we can inline memmove/memcpy despite having the must_preserve_cheri_tags property and the size not
; being a multiple of CAP_SIZE. Since the pointers are aligned we can start with capability copies and use
; word/byte copies for the trailing bytes.
declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture, i8 addrspace(200)* nocapture readonly, i64, i1) addrspace(200) #1
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture, i8 addrspace(200)* nocapture readonly, i64, i1) addrspace(200) #1

define void @test_string_memmove(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) nounwind {
entry:
  ; Note: has must_preserve_cheri_tags, but this memmove can still be inlined since it's aligned
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 addrspace(200)* align 16 %src, i64 45, i1 false) #0
  ret void
}

define void @test_string_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) nounwind {
entry:
  ; Note: has must_preserve_cheri_tags, but this memcpy can still be inlined since it's aligned
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 addrspace(200)* align 16 %src, i64 45, i1 false) #0
  ret void
}

attributes #0 = { "frontend-memtransfer-type"="'struct Test'" must_preserve_cheri_tags }
