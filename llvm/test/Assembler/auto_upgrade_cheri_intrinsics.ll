; Test to make sure cincoffset intrinsics are automatically upgraded to GEPs.
; RUN: llvm-as < %s | llvm-dis | FileCheck %s

declare ptr addrspace(200) @llvm.cheri.cap.offset.increment.i64(ptr addrspace(200), i64)
declare ptr addrspace(200) @llvm.cheri.cap.offset.increment.i32(ptr addrspace(200), i32)


define void @test.cincoffset(ptr addrspace(200) %cap, i64 %off64, i32 %off32) {
; CHECK: @test.cincoffset(
entry:
; CHECK-NEXT:  entry:
  call ptr addrspace(200) @llvm.cheri.cap.offset.increment.i64(ptr addrspace(200) %cap, i64 %off64)
; CHECK-NEXT:    %0 = getelementptr i8, ptr addrspace(200) %cap, i64 %off64
  call ptr addrspace(200) @llvm.cheri.cap.offset.increment.i32(ptr addrspace(200) %cap, i32 %off32)
; CHECK-NEXT:    %1 = getelementptr i8, ptr addrspace(200) %cap, i32 %off32
  call ptr addrspace(200) @llvm.cheri.cap.offset.increment.i64(ptr addrspace(200) %cap, i64 64)
; CHECK-NEXT:    %2 = getelementptr i8, ptr addrspace(200) %cap, i64 64
  call ptr addrspace(200) @llvm.cheri.cap.offset.increment.i32(ptr addrspace(200) %cap, i32 32)
; CHECK-NEXT:    %3 = getelementptr i8, ptr addrspace(200) %cap, i32 32
  ret void
; CHECK-NEXT:    ret void
}
