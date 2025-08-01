; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --scrub-attributes --version 2
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/memcpy-from-constant.ll
;; Copying from a zero constant can be converted to a memset (even with the tag preservation flags)
; RUN: llc -mtriple=mips64 -mcpu=cheri128 -mattr=+cheri128 --relocation-model=pic -target-abi purecap < %s -o - | FileCheck %s

@a = internal addrspace(200) constant ptr addrspace(200) null
@b = internal addrspace(200) constant ptr addrspace(200) null
@zero_constant = internal addrspace(200) constant [5 x ptr addrspace(200)] zeroinitializer
@constant_ptrs = internal addrspace(200) constant [2 x ptr addrspace(200)] [ptr addrspace(200) @a, ptr addrspace(200) @b]

declare void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) noalias nocapture writeonly, ptr addrspace(200) noalias nocapture readonly, i64, i1 immarg) addrspace(200) #0

define linkonce_odr void @copy_from_zero_constant(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_zero_constant:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csc $cnull, $zero, 0($c3)
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 @zero_constant, i64 16, i1 false)
  ret void
}

define linkonce_odr void @copy_from_zero_constant_with_offset(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_zero_constant_with_offset:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csc $cnull, $zero, 0($c3)
do.body:
  %src = getelementptr inbounds i8, ptr addrspace(200) @zero_constant, i64 16
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  ret void
}

define linkonce_odr void @copy_from_large_zero_constant(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_large_zero_constant:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csd $zero, $zero, 0($c3)
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 @zero_constant, i64 8, i1 false)
  ret void
}

define linkonce_odr void @copy_from_ptr_constant(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_ptr_constant:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; CHECK-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; CHECK-NEXT:    cgetpccincoffset $c1, $1
; CHECK-NEXT:    clcbi $c1, %captab20(constant_ptrs)($c1)
; CHECK-NEXT:    clc $c1, $zero, 0($c1)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csc $c1, $zero, 0($c3)
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 @constant_ptrs, i64 16, i1 false)
  ret void
}

define linkonce_odr void @copy_from_ptr_constant_with_offset(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_ptr_constant_with_offset:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; CHECK-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; CHECK-NEXT:    cgetpccincoffset $c1, $1
; CHECK-NEXT:    clcbi $c1, %captab20(constant_ptrs)($c1)
; CHECK-NEXT:    clc $c1, $zero, 16($c1)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csc $c1, $zero, 0($c3)
do.body:
  %src = getelementptr inbounds i8, ptr addrspace(200) @constant_ptrs, i64 16
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false)
  ret void
}

;; Run the same tests again this time with must_preserve_tags to check that we don't call memcpy().

define linkonce_odr void @copy_from_zero_constant_preserve(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_zero_constant_preserve:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csc $cnull, $zero, 0($c3)
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 @zero_constant, i64 16, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_zero_constant_with_offset_preserve(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_zero_constant_with_offset_preserve:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csc $cnull, $zero, 0($c3)
do.body:
  %src = getelementptr inbounds i8, ptr addrspace(200) @zero_constant, i64 16
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_large_zero_constant_preserve(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_large_zero_constant_preserve:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csd $zero, $zero, 0($c3)
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 @zero_constant, i64 8, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_ptr_constant_preserve(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_ptr_constant_preserve:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; CHECK-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; CHECK-NEXT:    cgetpccincoffset $c1, $1
; CHECK-NEXT:    clcbi $c1, %captab20(constant_ptrs)($c1)
; CHECK-NEXT:    clc $c1, $zero, 0($c1)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csc $c1, $zero, 0($c3)
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 @constant_ptrs, i64 16, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_ptr_constant_with_offset_preserve(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_ptr_constant_with_offset_preserve:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; CHECK-NEXT:    daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; CHECK-NEXT:    cgetpccincoffset $c1, $1
; CHECK-NEXT:    clcbi $c1, %captab20(constant_ptrs)($c1)
; CHECK-NEXT:    clc $c1, $zero, 16($c1)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csc $c1, $zero, 0($c3)
do.body:
  %src = getelementptr inbounds i8, ptr addrspace(200) @constant_ptrs, i64 16
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 16 %dst, ptr addrspace(200) align 16 %src, i64 16, i1 false) #1
  ret void
}

;; Finally, check copying from a zero constant with insufficient known alignment.
;; We should be able to emit this inline since a zero constant source never has tags.

define linkonce_odr void @copy_from_underaligned_zero_constant(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_underaligned_zero_constant:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    csd $zero, $zero, 0($c3)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csd $zero, $zero, 8($c3)
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 8 %dst, ptr addrspace(200) align 8 @zero_constant, i64 16, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_underaligned_zero_constant_preserve(ptr addrspace(200) %dst) addrspace(200) {
; CHECK-LABEL: copy_from_underaligned_zero_constant_preserve:
; CHECK:       # %bb.0: # %do.body
; CHECK-NEXT:    csd $zero, $zero, 0($c3)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    csd $zero, $zero, 8($c3)
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align 8 %dst, ptr addrspace(200) align 8 @zero_constant, i64 16, i1 false) #1
  ret void
}

attributes #0 = { argmemonly nocallback nofree nounwind willreturn }
attributes #1 = { must_preserve_cheri_tags "frontend-memtransfer-type"="'const UChar * __capability' (aka 'const char16_t * __capability')" }
