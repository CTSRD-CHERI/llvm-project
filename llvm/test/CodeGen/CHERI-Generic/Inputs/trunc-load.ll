; RUN: llc @PURECAP_HARDFLOAT_ARGS@ %s -o - < %s | FileCheck %s --check-prefix=PURECAP
; RUN: llc @HYBRID_HARDFLOAT_ARGS@ -o - < %s | FileCheck %s --check-prefix=HYBRID

define zeroext i16 @trunc_load_zext(i32 addrspace(200)* %p) {
  %1 = load i32, i32 addrspace(200)* %p
  %2 = trunc i32 %1 to i16
  ret i16 %2
}

define signext i16 @trunc_load_sext(i32 addrspace(200)* %p) {
  %1 = load i32, i32 addrspace(200)* %p
  %2 = trunc i32 %1 to i16
  ret i16 %2
}

define zeroext i16 @trunc_load_gep_zext(i32 addrspace(200)* %p) {
  %1 = getelementptr i32, i32 addrspace(200)* %p, iCAPRANGE 1
  %2 = load i32, i32 addrspace(200)* %1
  %3 = trunc i32 %2 to i16
  ret i16 %3
}

define signext i16 @trunc_load_gep_sext(i32 addrspace(200)* %p) {
  %1 = getelementptr i32, i32 addrspace(200)* %p, iCAPRANGE 1
  %2 = load i32, i32 addrspace(200)* %1
  %3 = trunc i32 %2 to i16
  ret i16 %3
}

define zeroext i16 @trunc_lshr_load_zext(i32 addrspace(200)* %p) {
  %1 = load i32, i32 addrspace(200)* %p
  %2 = lshr i32 %1, 16
  %3 = trunc i32 %2 to i16
  ret i16 %3
}

define signext i16 @trunc_lshr_load_sext(i32 addrspace(200)* %p) {
  %1 = load i32, i32 addrspace(200)* %p
  %2 = lshr i32 %1, 16
  %3 = trunc i32 %2 to i16
  ret i16 %3
}
