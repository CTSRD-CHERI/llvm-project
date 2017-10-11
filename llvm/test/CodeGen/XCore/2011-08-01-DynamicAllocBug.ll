; RUN: llc < %s -march=xcore | FileCheck %s

declare void @g()
declare i8* @llvm.stacksave.p0i8() nounwind
declare void @llvm.stackrestore.p0i8(i8*) nounwind

define void @f(i32** %p, i32 %size) {
allocas:
  %0 = call i8* @llvm.stacksave.p0i8()
  %a = alloca i32, i32 %size
  store i32* %a, i32** %p
  call void @g()
  call void @llvm.stackrestore.p0i8(i8* %0)
  ret void
}
; CHECK-LABEL: f:
; CHECK: ldaw [[REGISTER:r[0-9]+]], {{r[0-9]+}}[-r1]
; CHECK: set sp, [[REGISTER]]
; CHECK: extsp 1
; CHECK: bl g
