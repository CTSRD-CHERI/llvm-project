; RUN: opt -S -passes=instcombine -o - %s | FileCheck %s
; RUN: opt -S -passes=instcombine -o - %s | llc @PURECAP_HARDFLOAT_ARGS@ -O1 - -o - | %cheri_FileCheck %s --check-prefix ASM

target datalayout = "@PURECAP_DATALAYOUT@"
; Reduced test case for a crash in the new optimization to fold multiple setoffset calls (orignally found when compiling libunwind)

declare iCAPRANGE @check_fold(iCAPRANGE) addrspace(200)
declare void @check_fold_i8ptr(i8 addrspace(200)*) addrspace(200)
declare iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(i8 addrspace(200)*) addrspace(200)
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)*, iCAPRANGE) addrspace(200)

define void @infer_values_from_null_set_offset() addrspace(200) nounwind {
  %with_offset = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)* null, iCAPRANGE 123456)
  %offset = call iCAPRANGE @llvm.cheri.cap.offset.get.iCAPRANGE(i8 addrspace(200)* nonnull %with_offset)
  %offset_check = call iCAPRANGE @check_fold(iCAPRANGE %offset)
  ret void
}

define void @multiple_uses_big_constant() addrspace(200) nounwind {
  %with_offset = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)* null, iCAPRANGE 123456)
  call void @check_fold_i8ptr(i8 addrspace(200)* %with_offset)
  call void @check_fold_i8ptr(i8 addrspace(200)* %with_offset)
  call void @check_fold_i8ptr(i8 addrspace(200)* %with_offset)
  ret void
}

; Here we should use an immediate cincoffset:
define void @multiple_uses_small_constant() addrspace(200) nounwind {
  %with_offset = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.iCAPRANGE(i8 addrspace(200)* null, iCAPRANGE 123)
  call void @check_fold_i8ptr(i8 addrspace(200)* %with_offset)
  call void @check_fold_i8ptr(i8 addrspace(200)* %with_offset)
  call void @check_fold_i8ptr(i8 addrspace(200)* %with_offset)
  ret void
}
