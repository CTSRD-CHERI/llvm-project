; RUN: llc -mtriple=cheri-unknown-freebsd %s -target-abi n64 -relocation-model pic -o - | FileCheck %s
; Check that we do the right thing here in the hybrid ABI (the new cap-table code introduced an assert)

%struct.b = type { i64 }

@.str = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@c = local_unnamed_addr global %struct.b { i64 ptrtoint (ptr @.str to i64) }, align 8

; CHECK-LABEL: .globl	c
; CHECK-NEXT: .p2align	3
; CHECK-NEXT: c:
; CHECK-NEXT: .8byte	.L.str
; CHECK-NEXT: .size	c, 8
