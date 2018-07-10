; RUN: %cheri128_llc -o - %s | FileCheck %s
; The capability argument was not being passed on the stack but in $c3 instead
; Note: passing capabilities to variadics in the pure ABI is completely broken.
; https://github.com/CTSRD-CHERI/llvm/issues/271
; ModuleID = 'test.c'

@.str = private unnamed_addr constant [28 x i8] c"test 7=%td 8=%d p=%tx 9=%d\0A\00", align 1

; Function Attrs: nounwind
define i32 @test(i8* %a, ...) local_unnamed_addr #0 {
entry:
  %ap = alloca i8*, align 8
  %0 = bitcast i8** %ap to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %0) #2
  call void @llvm.va_start.p0i8(i8* nonnull %0)
  %argp.cur = load i8*, i8** %ap, align 8
  %argp.next = getelementptr inbounds i8, i8* %argp.cur, i64 8
  store i8* %argp.next, i8** %ap, align 8
  %1 = bitcast i8* %argp.cur to i64*
  %2 = load i64, i64* %1, align 8
  %3 = trunc i64 %2 to i32
  %4 = ptrtoint i8* %argp.next to i64
  %5 = add i64 %4, 15
  %6 = and i64 %5, -16
  %argp.cur2.aligned = inttoptr i64 %6 to i8*
  %argp.next3 = getelementptr inbounds i8, i8* %argp.cur2.aligned, i64 16
  %7 = inttoptr i64 %6 to i8 addrspace(200)**
  %8 = load i8 addrspace(200)*, i8 addrspace(200)** %7, align 16
  %argp.next5 = getelementptr inbounds i8, i8* %argp.cur2.aligned, i64 24
  store i8* %argp.next5, i8** %ap, align 8
  %9 = bitcast i8* %argp.next3 to i64*
  %10 = load i64, i64* %9, align 16
  %11 = trunc i64 %10 to i32
  %12 = ptrtoint i8* %a to i64
  %13 = addrspacecast i8 addrspace(200)* %8 to i8*
  %14 = ptrtoint i8* %13 to i64
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str, i64 0, i64 0), i64 signext %12, i32 signext %3, i64 signext %14, i32 signext %11)
  call void @llvm.va_end.p0i8(i8* nonnull %0)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #2
  ret i32 0

  ; The capability argument should be loaded from the stack:
  ; CHECK: daddiu	$2, $sp, 24
  ; CHECK: sd	$2, 8($sp)
  ; CHECK: daddiu	$3, $2, 8
  ; CHECK: sd	$3, 8($sp)
  ; CHECK: lw	$6, 28($sp)
  ; CHECK: daddiu	$2, $2, 23
  ; CHECK: daddiu	$3, $zero, -16
  ; CHECK: and	$2, $2, $3
  ; load from stack:
  ; CHECK: clc	$c1, $2, 0($ddc)
}

declare i32 @test_non_variadic(i8* %i1, i32 %i2, i8 addrspace(200)* nonnull %cap1, i32 %i3) local_unnamed_addr #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.va_start.p0i8(i8*) #2

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: nounwind
declare void @llvm.va_end.p0i8(i8*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

define i32 @main(i8 addrspace(200)* %arg1, i8 addrspace(200)* %arg2) {
entry:
  ; Verify that the capabilty argument is passed on the stack and not in a register
  ; for variadic calls:
  ; CHECK-LABEL: main:
  ; CHECK-NOT: cmove	$c3,  $c4
  ; CHECK:      daddiu	$4, $zero, 7
  ; CHECK-NEXT: daddiu	$5, $zero, 8
  ; CHECK-NEXT: daddiu	$6, $zero, 9
  ; CHECK-NEXT: jal	test
  ; CHECK-NEXT: csc	$c4, $sp, 0($ddc)
  ; CHECK: .end main
  %call1 = call i32 (i8*, ...) @test(i8* inttoptr (i64 7 to i8*), i32 signext 8, i8 addrspace(200)* nonnull %arg2, i32 signext 9)
  ret i32 0
}

define i32 @call_non_variadic(i8 addrspace(200)* %arg1, i8 addrspace(200)* %arg2) {
entry:
  ; CHECK-LABEL: call_non_variadic:
  ; CHECK:      cmove	$c3,  $c4
  ; CHECK-NEXT: daddiu	$4, $zero, 7
  ; CHECK-NEXT: daddiu	$5, $zero, 8
  ; CHECK-NEXT: jal	test_non_variadic
  ; CHECK-NEXT: daddiu	$6, $zero, 9
  ; CHECK: .end call_non_variadic
  %call1 = call i32 @test_non_variadic(i8* inttoptr (i64 7 to i8*), i32 signext 8, i8 addrspace(200)* nonnull %arg2, i32 signext 9)
  ret i32 0
}

attributes #0 = { nounwind }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { nounwind }
