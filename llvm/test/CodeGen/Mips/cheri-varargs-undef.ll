; RUN: %cheri_llc -mtriple=cheri-unknown-freebsd -target-abi purecap -O0 %s -o - | FileCheck %s --check-prefixes=UNDEF
; RUN: %cheri_llc -mtriple=cheri-unknown-freebsd -target-abi purecap -O0 %s -o - | FileCheck %s --check-prefix=ALIGNED-ARG
; RUN: %cheri_llc -mtriple=cheri-unknown-freebsd -target-abi purecap -O0 %s -o - | FileCheck %s --check-prefix=ALIGNED-VARARG
; ModuleID = 'unions.c'
; XFAIL: *
; This test is horribly fragile and it doesn't document what it's supposed to
; be checking, so no one except Khilan can fix it.
source_filename = "unions.c"
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%union.meh = type { i8 addrspace(200)* }

@undef.meh = private unnamed_addr addrspace(200) constant { i32, [12 x i8] } { i32 42, [12 x i8] undef }, align 16
@aligned_arg.meh = private unnamed_addr addrspace(200) constant { i32, [12 x i8] } { i32 42, [12 x i8] undef }, align 16
@aligned_vararg.meh = private unnamed_addr addrspace(200) constant { i32, [12 x i8] } { i32 42, [12 x i8] undef }, align 16

; Function Attrs: noinline nounwind optnone
define void @a(i32 signext %cmd, ...) #0 {
entry:
  %cmd.addr = alloca i32, align 4, addrspace(200)
  %ap = alloca i8 addrspace(200)*, align 16, addrspace(200)
  %meh = alloca %union.meh, align 16, addrspace(200)
  store i32 %cmd, i32 addrspace(200)* %cmd.addr, align 4
  %ap1 = addrspacecast i8 addrspace(200)* addrspace(200)* %ap to i8*
  call void @llvm.va_start.p200i8(i8* %ap1)
  %argp.cur = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %ap, align 16
  %0 = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %argp.cur)
  %1 = add i64 %0, 15
  %2 = and i64 %1, -16
  %3 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %argp.cur, i64 %2)
  %argp.next = getelementptr inbounds i8, i8 addrspace(200)* %3, i64 16
  store i8 addrspace(200)* %argp.next, i8 addrspace(200)* addrspace(200)* %ap, align 16
  %4 = bitcast i8 addrspace(200)* %3 to %union.meh addrspace(200)*
  %5 = bitcast %union.meh addrspace(200)* %meh to i8 addrspace(200)*
  %6 = bitcast %union.meh addrspace(200)* %4 to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* %5, i8 addrspace(200)* %6, i64 16, i32 16, i1 false)
  %ap2 = addrspacecast i8 addrspace(200)* addrspace(200)* %ap to i8*
  call void @llvm.va_end.p200i8(i8* %ap2)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.va_start.p200i8(i8*) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)*) #2

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i32, i1) #3

; Function Attrs: nounwind
declare void @llvm.va_end.p200i8(i8*) #1

; Function Attrs: noinline nounwind optnone
define void @b(i32 signext %cmd, i32 signext %x, ...) #0 {
entry:
  %cmd.addr = alloca i32, align 4, addrspace(200)
  %x.addr = alloca i32, align 4, addrspace(200)
  %ap = alloca i8 addrspace(200)*, align 16, addrspace(200)
  %meh = alloca %union.meh, align 16, addrspace(200)
  store i32 %cmd, i32 addrspace(200)* %cmd.addr, align 4
  store i32 %x, i32 addrspace(200)* %x.addr, align 4
  %ap1 = addrspacecast i8 addrspace(200)* addrspace(200)* %ap to i8*
  call void @llvm.va_start.p200i8(i8* %ap1)
  %argp.cur = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %ap, align 16
  %0 = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %argp.cur)
  %1 = add i64 %0, 15
  %2 = and i64 %1, -16
  %3 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %argp.cur, i64 %2)
  %argp.next = getelementptr inbounds i8, i8 addrspace(200)* %3, i64 16
  store i8 addrspace(200)* %argp.next, i8 addrspace(200)* addrspace(200)* %ap, align 16
  %4 = bitcast i8 addrspace(200)* %3 to %union.meh addrspace(200)*
  %5 = bitcast %union.meh addrspace(200)* %meh to i8 addrspace(200)*
  %6 = bitcast %union.meh addrspace(200)* %4 to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* %5, i8 addrspace(200)* %6, i64 16, i32 16, i1 false)
  %ap2 = addrspacecast i8 addrspace(200)* addrspace(200)* %ap to i8*
  call void @llvm.va_end.p200i8(i8* %ap2)
  ret void
}

; Function Attrs: noinline nounwind optnone
define void @c(i32 signext %cmd, ...) #0 {
entry:
  %cmd.addr = alloca i32, align 4, addrspace(200)
  %ap = alloca i8 addrspace(200)*, align 16, addrspace(200)
  %meh = alloca %union.meh, align 16, addrspace(200)
  %j = alloca i32, align 4, addrspace(200)
  %vaarg.promotion-temp = alloca i32, align 4, addrspace(200)
  store i32 %cmd, i32 addrspace(200)* %cmd.addr, align 4
  %ap1 = addrspacecast i8 addrspace(200)* addrspace(200)* %ap to i8*
  call void @llvm.va_start.p200i8(i8* %ap1)
  %argp.cur = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %ap, align 16
  %argp.next = getelementptr inbounds i8, i8 addrspace(200)* %argp.cur, i64 8
  store i8 addrspace(200)* %argp.next, i8 addrspace(200)* addrspace(200)* %ap, align 16
  %0 = bitcast i8 addrspace(200)* %argp.cur to i64 addrspace(200)*
  %1 = load i64, i64 addrspace(200)* %0, align 8
  %2 = trunc i64 %1 to i32
  store i32 %2, i32 addrspace(200)* %vaarg.promotion-temp, align 4
  %3 = load i32, i32 addrspace(200)* %vaarg.promotion-temp, align 4
  store i32 %3, i32 addrspace(200)* %j, align 4
  %argp.cur2 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %ap, align 16
  %4 = call i64 @llvm.cheri.cap.offset.get(i8 addrspace(200)* %argp.cur2)
  %5 = add i64 %4, 15
  %6 = and i64 %5, -16
  %7 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %argp.cur2, i64 %6)
  %argp.next3 = getelementptr inbounds i8, i8 addrspace(200)* %7, i64 16
  store i8 addrspace(200)* %argp.next3, i8 addrspace(200)* addrspace(200)* %ap, align 16
  %8 = bitcast i8 addrspace(200)* %7 to %union.meh addrspace(200)*
  %9 = bitcast %union.meh addrspace(200)* %meh to i8 addrspace(200)*
  %10 = bitcast %union.meh addrspace(200)* %8 to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* %9, i8 addrspace(200)* %10, i64 16, i32 16, i1 false)
  %ap4 = addrspacecast i8 addrspace(200)* addrspace(200)* %ap to i8*
  call void @llvm.va_end.p200i8(i8* %ap4)
  ret void
}

; Function Attrs: noinline nounwind optnone
define void @undef() #0 {
entry:
  %meh = alloca %union.meh, align 16, addrspace(200)
  %0 = bitcast %union.meh addrspace(200)* %meh to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* %0, i8 addrspace(200)* bitcast ({ i32, [12 x i8] } addrspace(200)* @undef.meh to i8 addrspace(200)*), i64 16, i32 16, i1 false)
  %coerce.dive = getelementptr inbounds %union.meh, %union.meh addrspace(200)* %meh, i32 0, i32 0
  %1 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %coerce.dive, align 16
  ; UNDEF-LABEL: undef:
  ; UNDEF: cincoffset $c24, $c11, $zero
  ; UNDEF: cincoffset [[TMPSP:\$c[0-9]+]], $c11, {{128|80}}
  ; UNDEF-NEXT: csetbounds	{{.*}}, [[TMPSP]], {{32|16}}
  call void (i32, ...) @a(i32 signext 99, i64 undef, i8 addrspace(200)* inreg %1)
  ret void
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.pcc.get() #2

; Function Attrs: noinline nounwind optnone
define void @aligned_arg() #0 {
entry:
  %meh = alloca %union.meh, align 16, addrspace(200)
  %0 = bitcast %union.meh addrspace(200)* %meh to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* %0, i8 addrspace(200)* bitcast ({ i32, [12 x i8] } addrspace(200)* @aligned_arg.meh to i8 addrspace(200)*), i64 16, i32 16, i1 false)
  %coerce.dive = getelementptr inbounds %union.meh, %union.meh addrspace(200)* %meh, i32 0, i32 0
  %1 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %coerce.dive, align 16
  ; ALIGNED-ARG-LABEL: aligned_arg:
  ; ALIGNED-ARG: cincoffset	$c[[TMP:[0-9]+]], $c11, {{128|80}}
  ; ALIGNED-ARG-NEXT:csetbounds	$c{{[0-9]+}}, $c[[TMP]], {{32|16}}

  call void (i32, i32, ...) @b(i32 signext 13, i32 signext 37, i8 addrspace(200)* inreg %1)
  ret void
}

; Function Attrs: noinline nounwind optnone
define void @aligned_vararg() #0 {
entry:
  %meh = alloca %union.meh, align 16, addrspace(200)
  %0 = bitcast %union.meh addrspace(200)* %meh to i8 addrspace(200)*
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* %0, i8 addrspace(200)* bitcast ({ i32, [12 x i8] } addrspace(200)* @aligned_vararg.meh to i8 addrspace(200)*), i64 16, i32 16, i1 false)
  %coerce.dive = getelementptr inbounds %union.meh, %union.meh addrspace(200)* %meh, i32 0, i32 0
  %1 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %coerce.dive, align 16
  ; ALIGNED-VARARG-LABEL: aligned_vararg:
  ; ALIGNED-VARARG: cincoffset	{{.*}}, $c11, 12
  call void (i32, ...) @c(i32 signext 99, i32 signext 37, i8 addrspace(200)* inreg %1)
  ret void
}

; Function Attrs: noinline nounwind optnone
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4, addrspace(200)
  store i32 0, i32 addrspace(200)* %retval, align 4
  call void @undef()
  call void @aligned_arg()
  call void @aligned_vararg()
  ret i32 0
}

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone }
attributes #3 = { argmemonly nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 5.0.0 (git@github.com:CTSRD-CHERI/clang.git f592f4eab1e8d9994f233fcf13c984caa3b4b913) (https://github.com/CTSRD-CHERI/llvm.git 3f41fd54c7337547f976f82705594c3e2287f346)"}
