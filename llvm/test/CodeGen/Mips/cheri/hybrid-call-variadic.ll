; RUN: %cheri128_llc -o - %s | FileCheck %s
; The capability argument was not being passed on the stack but in $c3 instead
; Note: passing capabilities to variadics in the hybrid ABI is completely broken.
; https://github.com/CTSRD-CHERI/llvm/issues/271
; ModuleID = 'test.c'

@.str = private unnamed_addr constant [28 x i8] c"test 7=%td 8=%d p=%tx 9=%d\0A\00", align 1

; Function Attrs: nounwind
define i32 @test(ptr %a, ...) local_unnamed_addr nounwind {
entry:
  %ap = alloca ptr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %ap) nounwind
  call void @llvm.va_start.p0(ptr nonnull %ap)
  %argp.cur = load ptr, ptr %ap, align 8
  %argp.next = getelementptr inbounds i8, ptr %argp.cur, i64 8
  store ptr %argp.next, ptr %ap, align 8
  %0 = load i64, ptr %argp.cur, align 8
  %1 = trunc i64 %0 to i32
  %2 = ptrtoint ptr %argp.next to i64
  %3 = add i64 %2, 15
  %4 = and i64 %3, -16
  %argp.cur2.aligned = inttoptr i64 %4 to ptr
  %argp.next3 = getelementptr inbounds i8, ptr %argp.cur2.aligned, i64 16
  %5 = inttoptr i64 %4 to ptr
  %6 = load ptr addrspace(200), ptr %5, align 16
  %argp.next5 = getelementptr inbounds i8, ptr %argp.cur2.aligned, i64 24
  store ptr %argp.next5, ptr %ap, align 8
  %7 = load i64, ptr %argp.next3, align 16
  %8 = trunc i64 %7 to i32
  %9 = ptrtoint ptr %a to i64
  %10 = addrspacecast ptr addrspace(200) %6 to ptr
  %11 = ptrtoint ptr %10 to i64
  %call = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i64 signext %9, i32 signext %1, i64 signext %11, i32 signext %8)
  call void @llvm.va_end.p0(ptr nonnull %ap)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %ap) nounwind
  ret i32 0

  ; The capability argument should be loaded from the stack:
  ; CHECK: daddiu	$2, $sp, 24
  ; CHECK-NEXT: daddiu	$2, $2, 23
  ; CHECK-NEXT: daddiu	$3, $zero, -16
  ; CHECK-NEXT: and	$2, $2, $3
  ; load from stack:
  ; CHECK-NEXT: clc	$c1, $2, 0($ddc)
}

declare i32 @test_non_variadic(ptr, i32, ptr addrspace(200) nonnull, i32) local_unnamed_addr #0

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare void @llvm.va_start.p0(ptr)

declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr nounwind

declare void @llvm.va_end.p0(ptr)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

define i32 @main(ptr addrspace(200) %arg1, ptr addrspace(200) %arg2) {
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
  %call1 = call i32 (ptr, ...) @test(ptr inttoptr (i64 7 to ptr), i32 signext 8, ptr addrspace(200) nonnull %arg2, i32 signext 9)
  ret i32 0
}

define i32 @call_non_variadic(ptr addrspace(200) %arg1, ptr addrspace(200) %arg2) {
entry:
  ; CHECK-LABEL: call_non_variadic:
  ; CHECK:      cmove	$c3,  $c4
  ; CHECK-NEXT: daddiu	$4, $zero, 7
  ; CHECK-NEXT: daddiu	$5, $zero, 8
  ; CHECK-NEXT: jal	test_non_variadic
  ; CHECK-NEXT: daddiu	$6, $zero, 9
  ; CHECK: .end call_non_variadic
  %call1 = call i32 @test_non_variadic(ptr inttoptr (i64 7 to ptr), i32 signext 8, ptr addrspace(200) nonnull %arg2, i32 signext 9)
  ret i32 0
}
