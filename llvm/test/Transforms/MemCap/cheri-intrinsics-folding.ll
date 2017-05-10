; RUN: opt -S -cheri-fold-intrisics -mtriple=cheri-unknown-freebsd %s -mcpu=cheri -o - < %s | FileCheck %s
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"


declare i64 @llvm.cheri.cap.base.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.base.set(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.length.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.length.set(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.perms.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.perms.and(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.type.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.type.set(i8 addrspace(200)*, i64) #1
declare i64 @llvm.cheri.cap.sealed.get(i8 addrspace(200)*) #1
declare i64 @llvm.cheri.cap.tag.get(i8 addrspace(200)*) #1

define i64 @null_get_length() #1 {
entry:
  %ret = tail call i64 @llvm.cheri.cap.length.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_length()
  ; CHECK: ret i64 0
}

define i64 @null_get_offset() #1 {
entry:
  %ret = tail call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_offset()
  ; CHECK: ret i64 0
}

define i64 @null_get_base() #1 {
entry:
  %ret = tail call i64 @llvm.cheri.cap.base.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_base()
  ; CHECK: ret i64 0
}

define i64 @null_get_perms() #1 {
entry:
  %ret = tail call i64 @llvm.cheri.cap.perms.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_perms()
  ; CHECK: ret i64 0
}

define i64 @null_get_type() #1 {
entry:
  %ret = tail call i64 @llvm.cheri.cap.type.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_type()
  ; CHECK: ret i64 0
}

define i64 @null_get_sealed() #1 {
entry:
  %ret = tail call i64 @llvm.cheri.cap.sealed.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_sealed()
  ; CHECK: ret i64 0
}

define i64 @null_get_tag() #1 {
entry:
  %ret = tail call i64 @llvm.cheri.cap.tag.get(i8 addrspace(200)* null)
  ret i64 %ret
  ; CHECK-LABEL: @null_get_tag()
  ; CHECK: ret i64 0
}

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
