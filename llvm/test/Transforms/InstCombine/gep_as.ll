; RUN: opt -passes=instcombine -S < %s | FileCheck %s
target datalayout = "pf200:128:128:128:64"
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
