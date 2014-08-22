; RUN: llc %s -mcpu=cheri -o - | FileCheck %s
; ModuleID = 'builtin.c'
target datalayout = "E-p200:256:256:256-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-n32:64-S256"
target triple = "cheri-unknown-freebsd"

@results = common global [12 x i8 addrspace(200)*] zeroinitializer, align 32

; CHECK: test
; Function Attrs: nounwind
define i64 @test(i8 addrspace(200)* %foo) #0 {
  %1 = alloca i8 addrspace(200)*, align 32
  %x = alloca i64, align 8
  store i8 addrspace(200)* %foo, i8 addrspace(200)** %1, align 32
  %2 = load i8 addrspace(200)** %1, align 32
  ; CHECK: cgetlen
  %3 = call i64 @llvm.mips.get.cap.length(i8 addrspace(200)* %2)
  %4 = load i64* %x, align 8
  %5 = and i64 %4, %3
  store i64 %5, i64* %x, align 8
  %6 = load i8 addrspace(200)** %1, align 32
  ; CHECK: cgetperm
  %7 = call i64 @llvm.mips.get.cap.perms(i8 addrspace(200)* %6)
  %8 = trunc i64 %7 to i16
  %9 = sext i16 %8 to i64
  %10 = load i64* %x, align 8
  %11 = and i64 %10, %9
  store i64 %11, i64* %x, align 8
  %12 = load i8 addrspace(200)** %1, align 32
  ; CHECK: cgettype
  %13 = call i64 @llvm.mips.get.cap.type(i8 addrspace(200)* %12)
  %14 = load i64* %x, align 8
  %15 = and i64 %14, %13
  store i64 %15, i64* %x, align 8
  %16 = load i8 addrspace(200)** %1, align 32
  ; CHECK: cgettag
  %17 = call i64 @llvm.mips.get.cap.tag(i8 addrspace(200)* %16)
  %18 = trunc i64 %17 to i1
  %19 = zext i1 %18 to i64
  %20 = load i64* %x, align 8
  %21 = and i64 %20, %19
  store i64 %21, i64* %x, align 8
  %22 = load i8 addrspace(200)** %1, align 32
  ; CHECK: csetlen
  %23 = call i8 addrspace(200)* @llvm.mips.set.cap.length(i8 addrspace(200)* %22, i64 42)
  store i8 addrspace(200)* %23, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*]* @results, i32 0, i64 0), align 32
  %24 = load i8 addrspace(200)** %1, align 32
  ; CHECK: candperm
  %25 = call i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)* %24, i64 12)
  store i8 addrspace(200)* %25, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*]* @results, i32 0, i64 1), align 32
  %26 = load i8 addrspace(200)** %1, align 32
  ; This comes later, but the instruction scheduler puts it in here to avoid a
  ; spill
  ; CHECK: cgetunsealed
  ; CHECK: csettype
  %27 = call i8 addrspace(200)* @llvm.mips.set.cap.type(i8 addrspace(200)* %26, i64 35)
  store i8 addrspace(200)* %27, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*]* @results, i32 0, i64 2), align 32
  %28 = load i8 addrspace(200)** %1, align 32
  ; CHECK: csealcode
  %29 = call i8 addrspace(200)* @llvm.mips.seal.cap.code(i8 addrspace(200)* %28)
  store i8 addrspace(200)* %29, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*]* @results, i32 0, i64 3), align 32
  %30 = load i8 addrspace(200)** %1, align 32
  %31 = load i8 addrspace(200)** %1, align 32
  ; CHECK: csealdata
  %32 = call i8 addrspace(200)* @llvm.mips.seal.cap.data(i8 addrspace(200)* %30, i8 addrspace(200)* %31)
  store i8 addrspace(200)* %32, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*]* @results, i32 0, i64 4), align 32
  %33 = load i8 addrspace(200)** %1, align 32
  %34 = load i8 addrspace(200)** %1, align 32
  ; CHECK: cunseal
  %35 = call i8 addrspace(200)* @llvm.mips.unseal.cap(i8 addrspace(200)* %33, i8 addrspace(200)* %34)
  store i8 addrspace(200)* %35, i8 addrspace(200)** getelementptr inbounds ([12 x i8 addrspace(200)*]* @results, i32 0, i64 5), align 32
  ; CHECK: csetcause
  call void @llvm.mips.set.cause(i64 42)
  %36 = load i64* %x, align 8
  ; CHECK: cgetcause
  %37 = call i64 @llvm.mips.get.cause()
  %38 = and i64 %36, %37
  %39 = call i64 @llvm.mips.get.cap.unsealed(i8 addrspace(200)* %16)
  %40 = trunc i64 %39 to i1
  %41 = zext i1 %40 to i64
  %42 = and i64 %41, %38
  ; CHECK: ccheckperm
  call void @llvm.mips.check.perms(i8 addrspace(200)* %30, i64 12)
  ; CHECK: cchecktype
  call void @llvm.mips.check.type(i8 addrspace(200)* %30, i8 addrspace(200)* %31)
  ; CHECK: jr
  ret i64 %42
}

; Function Attrs: nounwind readnone
declare i64 @llvm.mips.get.cap.length(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.mips.get.cap.perms(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.mips.get.cap.type(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.mips.get.cap.unsealed(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.mips.get.cap.tag(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.mips.set.cap.length(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.mips.and.cap.perms(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.mips.set.cap.type(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.mips.seal.cap.code(i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.mips.seal.cap.data(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.mips.unseal.cap(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind readnone
declare void @llvm.mips.check.perms(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare void @llvm.mips.check.type(i8 addrspace(200)*, i8 addrspace(200)*) #1

; Function Attrs: nounwind
declare void @llvm.mips.set.cause(i64) #2

; Function Attrs: nounwind
declare i64 @llvm.mips.get.cause() #2

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.4 "}
