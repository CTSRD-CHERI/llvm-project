; RUN: %cheri_llc %s -o - | FileCheck %s
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@ext_array = external addrspace(200) global [0 x i8], align 1
; CHECK: some_var:
; CHECK-NEXT: .8byte ext_array{{$}}
@some_var = addrspace(200) global i64 ptrtoint ([0 x i8] addrspace(200)* @ext_array to i64), align 8




