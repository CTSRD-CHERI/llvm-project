; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s | FileCheck %s --check-prefix=PURECAP
;; Hybrid baseline to compare against
; RUN: sed 's/addrspace(200)//g' %s | llc @HYBRID_HARDFLOAT_ARGS@ | FileCheck %s --check-prefix=HYBRID

;; If both offsets are known to be non-negative it is safe to commute them and
;; use an immediate load.
define i32 @nneg_nneg(ptr addrspace(200) %p, i16 %x) {
  %x.ext = zext i16 %x to i64
  %q = getelementptr [1 x i32], ptr addrspace(200) %p, i64 1, i64 %x.ext
  %ret = load i32, ptr addrspace(200) %q
  ret i32 %ret
}

;; If both offsets are known to be negative it is safe to commute them and use
;; an immediate load.
define i32 @neg_neg(ptr addrspace(200) %p, i16 %x) {
  %x.ext = zext i16 %x to i64
  %x.pos = or i64 %x.ext, 1
  %x.neg = sub i64 0, %x.pos
  %q = getelementptr [1 x i32], ptr addrspace(200) %p, i64 -1, i64 %x.neg
  %ret = load i32, ptr addrspace(200) %q
  ret i32 %ret
}

;; If one offset is known to be non-negative and the other negative it is not in
;; general safe to commute them and use an immediate load.
define i32 @nneg_neg(ptr addrspace(200) %p, i16 %x) {
  %x.ext = zext i16 %x to i64
  %x.pos = or i64 %x.ext, 1
  %x.neg = sub i64 0, %x.pos
  %q = getelementptr [1 x i32], ptr addrspace(200) %p, i64 1, i64 %x.neg
  %ret = load i32, ptr addrspace(200) %q
  ret i32 %ret
}

;; If one offset is known to be non-negative and the other negative it is not in
;; general safe to commute them and use an immediate load.
define i32 @neg_nneg(ptr addrspace(200) %p, i16 %x) {
  %x.ext = zext i16 %x to i64
  %q = getelementptr [1 x i32], ptr addrspace(200) %p, i64 -1, i64 %x.ext
  %ret = load i32, ptr addrspace(200) %q
  ret i32 %ret
}

;; If we do not know the sign of one offset it is not in general safe to
;; commute them and use an immediate load.
define i32 @nneg_unknown(ptr addrspace(200) %p, i64 %x) {
  %q = getelementptr [1 x i32], ptr addrspace(200) %p, i64 1, i64 %x
  %ret = load i32, ptr addrspace(200) %q
  ret i32 %ret
}

;; If we do not know the sign of one offset it is not in general safe to
;; commute them and use an immediate load.
define i32 @neg_unknown(ptr addrspace(200) %p, i64 %x) {
  %q = getelementptr [1 x i32], ptr addrspace(200) %p, i64 -1, i64 %x
  %ret = load i32, ptr addrspace(200) %q
  ret i32 %ret
}
