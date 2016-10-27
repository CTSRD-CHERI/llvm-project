; RUN: llc %s -mcpu=cheri -o - | FileCheck %s
; ModuleID = 'cheriintrinsic.c'
target datalayout = "E-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f128:128:128-v64:64:64-n32"
target triple = "cheri-unknown-freebsd"

; CHECK: cmalloc
define i8 addrspace(200)* @cmalloc(i64 %s) nounwind {
entry:
  %call = tail call i8* @malloc(i64 %s) nounwind
  %0 = ptrtoint i8* %call to i64
  %1 = inttoptr i64 %0 to i8 addrspace(200)*
  ; CHECK: csetbounds
  %2 = tail call i8 addrspace(200)* @llvm.memcap.cap.bounds.set(i8 addrspace(200)* %1, i64 %s)
  ret i8 addrspace(200)* %2
}

declare noalias i8* @malloc(i64) nounwind

; CHECK-LABEL: addBase
define i8 addrspace(200)* @addBase(i8 addrspace(200)* %p) nounwind readnone {
entry:
  ; CHECK: cincoffset
  %incdec.ptr = getelementptr inbounds i8, i8 addrspace(200)* %p, i64 1
  ret i8 addrspace(200)* %incdec.ptr
}

; CHECK-LABEL: getLength
define i64 @getLength(i8 addrspace(200)* %c) nounwind readnone {
entry:
  ; CHECK: cgetlen
  %0 = tail call i64 @llvm.memcap.cap.length.get(i8 addrspace(200)* %c)
  ret i64 %0
}

declare i64 @llvm.memcap.cap.length.get(i8 addrspace(200)*) nounwind readnone

; CHECK-LABEL: getPerms
define signext i16 @getPerms(i8 addrspace(200)* %c) nounwind readnone {
entry:
  ; CHECK: cgetperm
  %0 = tail call i64 @llvm.memcap.cap.perms.get(i8 addrspace(200)* %c)
  %1 = trunc i64 %0 to i16
  ret i16 %1
}

declare i64 @llvm.memcap.cap.perms.get(i8 addrspace(200)*) nounwind readnone

; CHECK-LABEL: andPerms
define i8 addrspace(200)* @andPerms(i8 addrspace(200)* %c, i16 signext %perms) nounwind readnone {
entry:
  ; CHECK: candperm
  %0 = zext i16 %perms to i64
  %1 = tail call i8 addrspace(200)* @llvm.memcap.cap.perms.and(i8 addrspace(200)* %c, i64 %0)
  ret i8 addrspace(200)* %1
}

declare i8 addrspace(200)* @llvm.memcap.cap.perms.and(i8 addrspace(200)*, i64) nounwind readnone

; CHECK-LABEL: gettype
define i64 @gettype(i8 addrspace(200)* %c) nounwind readnone {
entry:
  ; CHECK: cgettype
  %0 = tail call i64 @llvm.memcap.cap.type.get(i8 addrspace(200)* %c)
  ret i64 %0
}

declare i64 @llvm.memcap.cap.type.get(i8 addrspace(200)*) nounwind readnone

; CHECK-LABEL: setBounds
define i8 addrspace(200)* @setBounds(i8 addrspace(200)* %c, i64 %bounds) nounwind readnone {
entry:
  ; CHECK: csetbounds
  %0 = tail call i8 addrspace(200)* @llvm.memcap.cap.bounds.set(i8 addrspace(200)* %c, i64 %bounds)
  ret i8 addrspace(200)* %0
}

declare i8 addrspace(200)* @llvm.memcap.cap.bounds.set(i8 addrspace(200)*, i64) nounwind readnone
