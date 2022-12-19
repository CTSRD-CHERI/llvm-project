; RUN: llc < %s -mtriple=arm-linux-gnueabi | FileCheck %s

; CHECK: .globl	test

; CHECK: .Lstructvar:
; CHECK: .size .Lstructvar, 8

; CHECK: .globl	foo1
; CHECK-NEXT: .set foo1, bar
; CHECK-NEXT: .size foo1, 4

; CHECK: .globl	foo2
; CHECK-NEXT: .set foo2, bar
; CHECK-NEXT: .size foo2, 4

; CHECK: .weak	bar_f
; CHECK-NEXT: .type bar_f,%function
; CHECK-NEXT: .set bar_f, foo_f
; CHECK-NEXT: .size bar_f, .Lfunc_end0-foo_f

; CHECK: .set bar_i, bar
; CHECK-NEXT: .size bar_i, 4

; CHECK: .globl	A
; CHECK-NEXT: .set A, bar
; CHECK-NEXT: .size A, 4

; CHECK: .globl elem0
; CHECK-NEXT: .set elem0, .Lstructvar
; CHECK-NEXT: .size elem0, 4

; CHECK: .globl elem1
; CHECK-NEXT: .set elem1, .Lstructvar+4
; CHECK-NEXT: .size elem1, 4

@bar = global i32 42
@foo1 = alias i32, ptr @bar
@foo2 = alias i32, ptr @bar

%FunTy = type i32()

define i32 @foo_f() {
  ret i32 0
}
@bar_f = weak alias %FunTy, ptr @foo_f

@bar_i = internal alias i32, ptr @bar

@A = alias i64, ptr @bar

@structvar = private global {i32, i32} {i32 1, i32 2}
@elem0 = alias i32, getelementptr({i32, i32}, ptr  @structvar, i32 0, i32 0)
@elem1 = alias i32, getelementptr({i32, i32}, ptr  @structvar, i32 0, i32 1)

define i32 @test() {
entry:
   %tmp = load i32, ptr @foo1
   %tmp1 = load i32, ptr @foo2
   %tmp0 = load i32, ptr @bar_i
   %tmp2 = call i32 @foo_f()
   %tmp3 = add i32 %tmp, %tmp2
   %tmp4 = call i32 @bar_f()
   %tmp5 = add i32 %tmp3, %tmp4
   %tmp6 = add i32 %tmp1, %tmp5
   %tmp7 = add i32 %tmp6, %tmp0
   ret i32 %tmp7
}
