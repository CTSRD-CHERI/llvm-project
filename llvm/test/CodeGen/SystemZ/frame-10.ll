; Test the stacksave builtin.
;
; RUN: llc < %s -mtriple=s390x-linux-gnu | FileCheck %s

declare i8 *@llvm.stacksave.p0i8()

define void @f1(i8 **%dest) {
; CHECK-LABEL: f1:
; CHECK: stg %r15, 0(%r2)
; CHECK: br %r14
  %addr = call i8 *@llvm.stacksave.p0i8()
  store volatile i8 *%addr, i8 **%dest
  ret void
}
