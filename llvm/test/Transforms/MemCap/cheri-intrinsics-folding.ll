; RUN: %cheri_opt -S -cheri-fold-intrisics %s -o - < %s | FileCheck %s
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

declare i64 @check_fold(i64) #1

declare i64 @llvm.cheri.cap.base.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.base.set(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #1
declare i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.length.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.length.set(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.perms.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.perms.and(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.type.get(i8 addrspace(200)*) #1
declare i64 @llvm.cheri.cap.sealed.get(i8 addrspace(200)*) #1
declare i64 @llvm.cheri.cap.tag.get(i8 addrspace(200)*) #1

define i64 @null_get_length() #1 {
  %ret = tail call i64 @llvm.cheri.cap.length.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_length()
  ; CHECK-NEXT: ret i64 0
}

define i64 @null_get_offset() #1 {
  %ret = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_offset()
  ; CHECK-NEXT: ret i64 0
}

define i64 @null_get_base() #1 {
  %ret = tail call i64 @llvm.cheri.cap.base.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_base()
  ; CHECK-NEXT: ret i64 0
}

define i64 @null_get_perms() #1 {
  %ret = tail call i64 @llvm.cheri.cap.perms.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_perms()
  ; CHECK-NEXT: ret i64 0
}

define i64 @null_get_type() #1 {
  %ret = tail call i64 @llvm.cheri.cap.type.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_type()
  ; CHECK-NEXT: ret i64 0
}

define i64 @null_get_sealed() #1 {
  %ret = tail call i64 @llvm.cheri.cap.sealed.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_sealed()
  ; CHECK-NEXT: ret i64 0
}

define i64 @null_get_tag() #1 {
  %ret = tail call i64 @llvm.cheri.cap.tag.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_tag()
  ; CHECK-NEXT: ret i64 0
}

define void @infer_values_from_null_set_offset() #1 {
  ; CHECK-LABEL: @infer_values_from_null_set_offset()
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 123456)

  %base = tail call i64 @llvm.cheri.cap.base.get(i8 addrspace(200)* %with_offset)
  %base_check = tail call i64 @check_fold(i64 %base)
  ; CHECK:  %base_check = tail call i64 @check_fold(i64 0)

  %length = tail call i64 @llvm.cheri.cap.length.get(i8 addrspace(200)* %with_offset)
  %length_check = tail call i64 @check_fold(i64 %length)
  ; CHECK:  %length_check = tail call i64 @check_fold(i64 0)

  %offset = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %with_offset)
  %offset_check = tail call i64 @check_fold(i64 %offset)
  ; CHECK:  %offset_check = tail call i64 @check_fold(i64 123456)

  %type = tail call i64 @llvm.cheri.cap.type.get(i8 addrspace(200)* %with_offset)
  %type_check = tail call i64 @check_fold(i64 %type)
  ; CHECK:  %type_check = tail call i64 @check_fold(i64 0)

  %sealed = tail call i64 @llvm.cheri.cap.sealed.get(i8 addrspace(200)* %with_offset)
  %sealed_check = tail call i64 @check_fold(i64 %sealed)
  ; CHECK:  %sealed_check = tail call i64 @check_fold(i64 0)

  %perms = tail call i64 @llvm.cheri.cap.perms.get(i8 addrspace(200)* %with_offset)
  %perms_check = tail call i64 @check_fold(i64 %perms)
  ; CHECK:  %perms_check = tail call i64 @check_fold(i64 0)

  %tag = tail call i64 @llvm.cheri.cap.tag.get(i8 addrspace(200)* %with_offset)
  %tag_check = tail call i64 @check_fold(i64 %tag)
  ; CHECK:  %tag_check = tail call i64 @check_fold(i64 0)

  ret void
  ; CHECK: ret void
}

define i64 @fold_set_offset_arg(i8 addrspace(200)* %arg) #1 {
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %arg, i64 42)
  %ret = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %with_offset)
  ret i64 %ret
  ; CHECK-LABEL: @fold_set_offset_arg(i8 addrspace(200)* %arg)
  ; CHECK: ret i64 42

}

define i64 @fold_null_inc_offset() #1 {
  %inc_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* null, i64 100)
  %ret = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %inc_offset)
  ret i64 %ret
  ; CHECK-LABEL: @fold_null_inc_offset()
  ; CHECK: ret i64 100
}

define i64 @fold_set_and_inc_offset(i8 addrspace(200)* %arg) #1 {
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %arg, i64 42)
  %inc_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %with_offset, i64 100)
  %ret = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %inc_offset)
  ret i64 %ret
  ; CHECK-LABEL: @fold_set_and_inc_offset(i8 addrspace(200)* %arg)
  ; CHECK: ret i64 142
}

define i64 @fold_set_and_multiple_inc_offset(i8 addrspace(200)* %arg) #1 {
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %arg, i64 42)
  %inc_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %with_offset, i64 100)
  %inc_offset2 = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.increment(i8 addrspace(200)* %inc_offset, i64 100)
  %ret = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %inc_offset2)
  ret i64 %ret
  ; CHECK-LABEL: @fold_set_and_multiple_inc_offset(i8 addrspace(200)* %arg)
  ; CHECK: ret i64 242
}


attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
