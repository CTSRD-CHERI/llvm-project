; RUN: llc < %s -march=xcore | FileCheck %s

declare void @g()
declare ptr @llvm.stacksave.p0() nounwind
declare void @llvm.stackrestore.p0(ptr) nounwind

define void @f(ptr %p, i32 %size) {
allocas:
  %0 = call ptr @llvm.stacksave.p0()
  %a = alloca i32, i32 %size
  store ptr %a, ptr %p
  call void @g()
  call void @llvm.stackrestore.p0(ptr %0)
  ret void
}
; CHECK-LABEL: f:
; CHECK: ldaw [[REGISTER:r[0-9]+]], {{r[0-9]+}}[-r1]
; CHECK: set sp, [[REGISTER]]
; CHECK: extsp 1
; CHECK: bl g
