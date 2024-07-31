;; Copying from a zero constant can be converted to a memset (even with the tag preservation flags)
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s -o - | FileCheck %s

@a = internal addrspace(200) constant ptr addrspace(200) null
@b = internal addrspace(200) constant ptr addrspace(200) null
@zero_constant = internal addrspace(200) constant [5 x ptr addrspace(200)] zeroinitializer
@constant_ptrs = internal addrspace(200) constant [2 x ptr addrspace(200)] [ptr addrspace(200) @a, ptr addrspace(200) @b]

declare void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) noalias nocapture writeonly, ptr addrspace(200) noalias nocapture readonly, i64, i1 immarg) addrspace(200) #0

define linkonce_odr void @copy_from_zero_constant(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ @zero_constant, i64 @CAP_BYTES@, i1 false)
  ret void
}

define linkonce_odr void @copy_from_zero_constant_with_offset(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  %src = getelementptr inbounds i8, ptr addrspace(200) @zero_constant, i64 @CAP_BYTES@
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ %src, i64 @CAP_BYTES@, i1 false)
  ret void
}

define linkonce_odr void @copy_from_large_zero_constant(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ @zero_constant, i64 @CAP_RANGE_BYTES@, i1 false)
  ret void
}

define linkonce_odr void @copy_from_ptr_constant(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ @constant_ptrs, i64 @CAP_BYTES@, i1 false)
  ret void
}

define linkonce_odr void @copy_from_ptr_constant_with_offset(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  %src = getelementptr inbounds i8, ptr addrspace(200) @constant_ptrs, i64 @CAP_BYTES@
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ %src, i64 @CAP_BYTES@, i1 false)
  ret void
}

;; Run the same tests again this time with must_preserve_tags to check that we don't call memcpy().

define linkonce_odr void @copy_from_zero_constant_preserve(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ @zero_constant, i64 @CAP_BYTES@, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_zero_constant_with_offset_preserve(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  %src = getelementptr inbounds i8, ptr addrspace(200) @zero_constant, i64 @CAP_BYTES@
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ %src, i64 @CAP_BYTES@, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_large_zero_constant_preserve(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ @zero_constant, i64 @CAP_RANGE_BYTES@, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_ptr_constant_preserve(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ @constant_ptrs, i64 @CAP_BYTES@, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_ptr_constant_with_offset_preserve(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  %src = getelementptr inbounds i8, ptr addrspace(200) @constant_ptrs, i64 @CAP_BYTES@
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_BYTES@ %dst, ptr addrspace(200) align @CAP_BYTES@ %src, i64 @CAP_BYTES@, i1 false) #1
  ret void
}

;; Finally, check copying from a zero constant with insufficient known alignment.
;; We should be able to emit this inline since a zero constant source never has tags.

define linkonce_odr void @copy_from_underaligned_zero_constant(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_RANGE_BYTES@ %dst, ptr addrspace(200) align @CAP_RANGE_BYTES@ @zero_constant, i64 @CAP_BYTES@, i1 false) #1
  ret void
}

define linkonce_odr void @copy_from_underaligned_zero_constant_preserve(ptr addrspace(200) %dst) addrspace(200) {
do.body:
  call void @llvm.memcpy.p200.p200.i64(ptr addrspace(200) align @CAP_RANGE_BYTES@ %dst, ptr addrspace(200) align @CAP_RANGE_BYTES@ @zero_constant, i64 @CAP_BYTES@, i1 false) #1
  ret void
}

attributes #0 = { argmemonly nocallback nofree nounwind willreturn }
attributes #1 = { must_preserve_cheri_tags "frontend-memtransfer-type"="'const UChar * __capability' (aka 'const char16_t * __capability')" }
