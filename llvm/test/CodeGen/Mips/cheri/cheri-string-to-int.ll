; RUN: llc -mtriple=cheri-unknown-freebsd %s -target-abi n64 -relocation-model pic -o - | FileCheck %s
; Check that we do the right thing here in the hybrid ABI (the new cap-table code introduced an assert)

; ModuleID = '/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/cheri-string-to-int.c'
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/cheri-string-to-int.c"
target datalayout = "E-m:e-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "mips64-unknown-freebsd12.0"

%struct.b = type { i64 }

@.str = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@c = local_unnamed_addr global %struct.b { i64 ptrtoint ([1 x i8]* @.str to i64) }, align 8




; CHECK-LABEL: .globl	c
; CHECK-NEXT: .p2align	3
; CHECK-NEXT: c:
; CHECK-NEXT: .8byte	.L.str
; CHECK-NEXT: .size	c, 8




!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 41c2892ed18fcb87c7a4c0dc1fb1e62d38ea3119) (https://github.com/llvm-mirror/llvm.git 9fca120cea84d71524a8b1e7738e9788666571e2)"}
