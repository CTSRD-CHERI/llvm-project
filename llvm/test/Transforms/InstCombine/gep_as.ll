; RUN: opt -instcombine -S < %s | FileCheck %s
; ModuleID = '<stdin>'
target datalayout = "E-m:m-pf200:256:256:256:64-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "riscv64-unknown-freebsd"

define i32 @main() {
entry:
  ; Check that the address space cast happens before the GEP.
  ; CHECK: %1 = addrspacecast i32* %0 to i32 addrspace(200)*
  ; CHECK: %add.ptr = getelementptr inbounds i32, i32 addrspace(200)* %1, i64 41
  %call = call i8* @malloc(i64 zeroext 168)
  %0 = bitcast i8* %call to i32*
  %1 = addrspacecast i32* %0 to i32 addrspace(200)*
  call void @test(i32 addrspace(200)* %1)
  %add.ptr = getelementptr inbounds i32, i32 addrspace(200)* %1, i64 41
  call void @test(i32 addrspace(200)* %add.ptr)
  ret i32 0
}

declare i8* @malloc(i64 zeroext)

declare void @test(i32 addrspace(200)*)
