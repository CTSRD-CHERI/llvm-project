; Check that llvm.memcpy() and llvm.memmove() intrinisics with nobuiltin
; attribute are always lowered to libcalls
; TODO: should this be an X86 test to get more coverage?
; RUN: llc -mtriple=mips64-unknown-linux %s -o - | FileCheck %s

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #2
declare void @llvm.memmove.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #2

define void @memcpy_too_big_unaligned(i8* %dst, i8* %src) #0 {
entry:
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %dst, i8* align 16 %src, i64 2048, i1 false)
  ret void
  ; CHECK-LABEL: memcpy_too_big_unaligned:
  ; CHECK: jal memcpy
}

define void @memcpy_aligned(i8* %dst, i8* %src) #0 {
entry:
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %dst, i8* align 16 %src, i64 16, i1 false)
  ret void
  ; This can be inlined;
  ; CHECK-LABEL: memcpy_aligned:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: ld $1, 8($5)
  ; CHECK-NEXT: sd $1, 8($4)
  ; CHECK-NEXT: ld $1, 0($5)
  ; CHECK-NEXT: jr $ra
  ; CHECK-NEXT: sd $1, 0($4)
}

define void @memcpy_aligned_nobuiltin(i8* %dst, i8* %src) #0 {
entry:
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %dst, i8* align 8 %src, i64 16, i1 false) #3
  ret void
  ; The memcpy could be inlined but was tagged with must_preserve_cheri_tags -> should call memcpy()
  ; CHECK-LABEL: memcpy_aligned_nobuiltin:
  ; CHECK: jal memcpy
}

define void @memmove_too_big_unaligned(i8* %dst, i8* %src) #0 {
entry:
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 1 %dst, i8* align 16 %src, i64 2048, i1 false)
  ret void
  ; CHECK-LABEL: memmove_too_big_unaligned:
  ; CHECK: jal memmove
}

define void @memmove_aligned(i8* %dst, i8* %src) #0 {
entry:
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 16 %dst, i8* align 16 %src, i64 16, i1 false)
  ret void
  ; This can be inlined;
  ; CHECK-LABEL: memmove_aligned:
  ; CHECK: # %bb.0:
  ; CHECK-NEXT: ld $1, 0($5)
  ; CHECK-NEXT: sd $1, 0($4)
  ; CHECK-NEXT: ld $1, 8($5)
  ; CHECK-NEXT: jr $ra
  ; CHECK-NEXT: sd $1, 8($4)
}

define void @memmove_aligned_nobuiltin(i8* %dst, i8* %src) #0 {
entry:
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 16 %dst, i8* align 16 %src, i64 16, i1 false) #3
  ret void
  ; The memmove could be inlined but was tagged with nobuiltin -> should call memmove()
  ; CHECK-LABEL: memmove_aligned_nobuiltin:
  ; CHECK: jal memmove
}

attributes #0 = { noinline nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { argmemonly nounwind }
attributes #3 = { must_preserve_cheri_tags }
