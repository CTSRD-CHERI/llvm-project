; RUN: %cheri_llc -mtriple=cheri-unknown-freebsd %s -o - -relocation-model=static | FileCheck %s
; RUN: %cheri_llc -mtriple=cheri-unknown-freebsd %s -o - -relocation-model=pic | FileCheck %s
; ModuleID = 'global_init.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@a = common addrspace(200) global [5 x i32] zeroinitializer, align 4
@b = addrspace(200) global [3 x i32 addrspace(200)*] [
    i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 8) to i32 addrspace(200)*),
    i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 4) to i32 addrspace(200)*),
    i32 addrspace(200)* getelementptr inbounds ([5 x i32], [5 x i32] addrspace(200)* @a, i32 0, i32 0)
  ], align 32
@c = addrspace(200) constant [3 x i32 addrspace(200)*] [
    i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 16) to i32 addrspace(200)*),
    i32 addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast ([5 x i32] addrspace(200)* @a to i8 addrspace(200)*), i64 12) to i32 addrspace(200)*),
    i32 addrspace(200)* getelementptr inbounds ([5 x i32], [5 x i32] addrspace(200)* @a, i32 0, i32 0)
  ], align 32

; CHECK:      .data
; CHECK-NEXT: .globl  b
; CHECK-NEXT: .p2align        {{5|4}}
; CHECK-NEXT: b:
; CHECK-NEXT: .Ltmp0:
; CHECK-NEXT: .space [[$CAP_SIZE:32|16]]
; CHECK-NEXT: .Ltmp1:
; CHECK-NEXT: .space [[$CAP_SIZE]]
; CHECK-NEXT: .Ltmp2:
; CHECK-NEXT: .space [[$CAP_SIZE]]
; CHECK-NEXT: .size   b, {{96|48}}

; CHECK:      .section	.data.rel.ro,"aw",@progbits
; CHECK-NEXT: .globl  c
; CHECK-NEXT: .p2align        {{5|4}}
; CHECK-NEXT: c:
; CHECK-NEXT: .Ltmp3:
; CHECK-NEXT: .space [[$CAP_SIZE]]
; CHECK-NEXT: .Ltmp4:
; CHECK-NEXT: .space [[$CAP_SIZE]]
; CHECK-NEXT: .Ltmp5:
; CHECK-NEXT: .space [[$CAP_SIZE]]
; CHECK-NEXT: .size   c, {{96|48}}

; CHECK:	.section	__cap_relocs,"a",@progbits
; CHECK:	.8byte	.Ltmp0
; CHECK:	.8byte	a
; CHECK:	.8byte	8
; CHECK:	.space	16
; CHECK:	.8byte	.Ltmp1
; CHECK:	.8byte	a
; CHECK:	.8byte	4
; CHECK:	.space	16
; CHECK:	.8byte	.Ltmp2
; CHECK:	.8byte	a
; CHECK:	.space	8
; CHECK:	.space	16
; CHECK:	.8byte	.Ltmp3
; CHECK:	.8byte	a
; CHECK:	.8byte	16
; CHECK:	.space	16
; CHECK:	.8byte	.Ltmp4
; CHECK:	.8byte	a
; CHECK:	.8byte	12
; CHECK:	.space	16
; CHECK:	.8byte	.Ltmp5
; CHECK:	.8byte	a
; CHECK:	.space	8
; CHECK:	.space	16
