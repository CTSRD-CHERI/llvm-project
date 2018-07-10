; RUN: %cheri_purecap_llc -o - -float-abi=soft -O0 -mips-ssection-threshold=0 %s | %cheri_FileCheck %s
; ModuleID = '/home/alr48/obj/build/llvm-build/fnmatch-ccd909-bugpoint-reduce.ll-reduced-simplified.bc'
source_filename = "fnmatch-ccd909-bugpoint-reduce.ll-output-0ec4103.bc"
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

module asm ".ident\09\22$FreeBSD$\22"

%union.__mbstate_t.0.8.16.24.32.40.48.56.64 = type { i64, [120 x i8] }
%struct._xlocale.4.12.20.28.36.44.52.60.68 = type { %struct.xlocale_refcounted.1.9.17.25.33.41.49.57.65, [6 x %struct.xlocale_component.2.10.18.26.34.42.50.58.66 addrspace(200)*], i32, i32, i32, i32, i32, i32, %struct.lconv.3.11.19.27.35.43.51.59.67, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, %union.__mbstate_t.0.8.16.24.32.40.48.56.64, i8 addrspace(200)* }
%struct.xlocale_refcounted.1.9.17.25.33.41.49.57.65 = type { i64, void (i8 addrspace(200)*) addrspace(200)* }
%struct.xlocale_component.2.10.18.26.34.42.50.58.66 = type { %struct.xlocale_refcounted.1.9.17.25.33.41.49.57.65, [32 x i8] }
%struct.lconv.3.11.19.27.35.43.51.59.67 = type { i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i8 addrspace(200)*, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8 }
%struct._RuneLocale.7.15.23.31.39.47.55.63.71 = type { [8 x i8], [32 x i8], i32 (i8 addrspace(200)*, i64, i8 addrspace(200)* addrspace(200)*) addrspace(200)*, i32 (i32, i8 addrspace(200)*, i64, i8 addrspace(200)* addrspace(200)*) addrspace(200)*, i32, [256 x i64], [256 x i32], [256 x i32], %struct._RuneRange.6.14.22.30.38.46.54.62.70, %struct._RuneRange.6.14.22.30.38.46.54.62.70, %struct._RuneRange.6.14.22.30.38.46.54.62.70, i8 addrspace(200)*, i32, [12 x i8] }
%struct._RuneRange.6.14.22.30.38.46.54.62.70 = type { i32, %struct._RuneEntry.5.13.21.29.37.45.53.61.69 addrspace(200)* }
%struct._RuneEntry.5.13.21.29.37.45.53.61.69 = type { i32, i32, i32, i64 addrspace(200)* }

@fnmatch.initial = external addrspace(200) constant %union.__mbstate_t.0.8.16.24.32.40.48.56.64, align 8

; Function Attrs: noinline nounwind optnone
declare i32 @fnmatch(i8 addrspace(200)*, i8 addrspace(200)*, i32 signext) #0

; Function Attrs: noinline nounwind optnone
define internal i32 @fnmatch1(i8 addrspace(200)* %pattern, i8 addrspace(200)* %string, i8 addrspace(200)* %stringstart, i32 signext %flags, i64 inreg %patmbs.coerce0, i64 inreg %patmbs.coerce1, i64 inreg %patmbs.coerce2, i64 inreg %patmbs.coerce3, i64 inreg %patmbs.coerce4, i64 inreg %patmbs.coerce5, i64 inreg %patmbs.coerce6, i64 inreg %patmbs.coerce7, i64 inreg %patmbs.coerce8, i64 inreg %patmbs.coerce9, i64 inreg %patmbs.coerce10, i64 inreg %patmbs.coerce11, i64 inreg %patmbs.coerce12, i64 inreg %patmbs.coerce13, i64 inreg %patmbs.coerce14, i64 inreg %patmbs.coerce15, i64 inreg %strmbs.coerce0, i64 inreg %strmbs.coerce1, i64 inreg %strmbs.coerce2, i64 inreg %strmbs.coerce3, i64 inreg %strmbs.coerce4, i64 inreg %strmbs.coerce5, i64 inreg %strmbs.coerce6, i64 inreg %strmbs.coerce7, i64 inreg %strmbs.coerce8, i64 inreg %strmbs.coerce9, i64 inreg %strmbs.coerce10, i64 inreg %strmbs.coerce11, i64 inreg %strmbs.coerce12, i64 inreg %strmbs.coerce13, i64 inreg %strmbs.coerce14, i64 inreg %strmbs.coerce15) #0 {
entry:
  store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* undef, align 16
  unreachable
}
; CHECK-LABEL: fnmatch1:


; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.pcc.get() #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #1

declare i64 @mbrtowc(i32 addrspace(200)*, i8 addrspace(200)*, i64 zeroext, %union.__mbstate_t.0.8.16.24.32.40.48.56.64 addrspace(200)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8, i64, i32, i1) #3

; Function Attrs: nounwind readonly
declare i8 addrspace(200)* @strchr(i8 addrspace(200)*, i32 signext) #4

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8 addrspace(200)* nocapture readonly, i64, i32, i1) #3

; Function Attrs: noinline nounwind optnone
declare i32 @rangematch(i8 addrspace(200)*, i32 signext, i32 signext, i8 addrspace(200)* addrspace(200)*, %union.__mbstate_t.0.8.16.24.32.40.48.56.64 addrspace(200)*) #0

; Function Attrs: noinline nounwind optnone
declare i32 @__tolower(i32 signext) #0

declare %struct._xlocale.4.12.20.28.36.44.52.60.68 addrspace(200)* @__get_locale() #2

declare i32 @__wcollate_range_cmp(i32 signext, i32 signext) #2

; Function Attrs: nounwind readonly
declare i32 @___tolower(i32 signext) #4

declare %struct._RuneLocale.7.15.23.31.39.47.55.63.71 addrspace(200)* @__getCurrentRuneLocale() #2

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="true" }
attributes #3 = { argmemonly nounwind }
attributes #4 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="true" }

!llvm.ident = !{!0}

!0 = !{!"clang version 7.0.0 (https://github.com/llvm-mirror/clang.git 407982bdd68f598f7afdc16acb46deba1b59070a) (https://github.com/llvm-mirror/llvm.git 960f90bedf5ab44be8a48a74cdcec9cdd2eed62f)"}
