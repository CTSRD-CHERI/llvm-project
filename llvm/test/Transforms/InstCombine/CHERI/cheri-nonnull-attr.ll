; RUN: opt -S -passes=instcombine %s | FileCheck %s
target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"

@char_ptr = external local_unnamed_addr addrspace(200) global i8 addrspace(200)*, align 32
@char_data = external addrspace(200) global i8, align 1
@.str = private unnamed_addr addrspace(200) constant [5 x i8] c"1234\00", align 1

; Function Attrs: nounwind
define void @foo() local_unnamed_addr #0 {
  %vla = alloca i8, align 1, addrspace(200)

  ; Allocas are nonnull unless you target AMDGPU
  call void @test(i8 addrspace(200)* %vla)
  ; CHECK: call void @test(i8 addrspace(200)* nonnull %vla)

  ; This one is not nonnull because we are loading an unknown value
  %1 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* @char_ptr, align 32
  call void @test(i8 addrspace(200)* %1) #1
  ; CHECK: call void @test(i8 addrspace(200)* %1)

  ; this one is nonnull because we are taking the address of a global:
  call void @test(i8 addrspace(200)* @char_data) #1
  ; CHECK: call void @test(i8 addrspace(200)* nonnull @char_data)

  ; Probably this one should be nonnnull too because it is a GEP into a known constant
  ; Other targets don't do so leave it like this
  call void @test(i8 addrspace(200)* getelementptr inbounds ([5 x i8], [5 x i8] addrspace(200)* @.str, i64 0, i64 0)) #1
  ; CHECK-NOTYET: call void @test(i8 addrspace(200)* nonnull getelementptr inbounds ([5 x i8], [5 x i8] addrspace(200)* @.str, i64 0, i64 0))
  ; CHECK: call void @test(i8 addrspace(200)* nonnull getelementptr inbounds ([5 x i8], [5 x i8] addrspace(200)* @.str, i64 0, i64 0))
  ret void
}

declare void @test(i8 addrspace(200)*) local_unnamed_addr #2

attributes #0 = { nounwind }
attributes #1 = { nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git ceb1d6da4e92e547484fdf484f023f40b8e6be36) (https://github.com/llvm-mirror/llvm.git bf0a21de41c48e4f781b9082e964cad51217b14b)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
