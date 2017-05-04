; This test was crashing llvm with the -mxgot flag (even at -O0)
; RUN: llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mcpu=mips64r2 -O0 -o /dev/null %s
; RUN: llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mcpu=mips64r2 -O0 -mxgot -o /dev/null %s
; RUN: llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mcpu=mips64r2 -O1 -mxgot -o /dev/null %s
; RUN: llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mcpu=mips64r2 -O2 -mxgot -o /dev/null %s
; RUN: llc -mtriple=mips64-unknown-freebsd -relocation-model=pic -mcpu=mips64r2 -O3 -mxgot -o /dev/null %s

; ModuleID = '/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/cheri-unwind-dw2-crash.creduce.c'
source_filename = "/local/scratch/alr48/cheri/llvm/tools/clang/test/CodeGen/cheri-unwind-dw2-crash.creduce.c"
target datalayout = "E-m:e-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "mips64-unknown-freebsd12.0"

; Function Attrs: noinline
define i32 @fde_mixed_encoding_compare() #0 {
entry:
  %call = call i32 @get_cie_encoding()
  %call1 = call i32 @get_cie_encoding()
  ret i32 0
}

; Function Attrs: noinline
define internal i32 @get_cie_encoding() #0 {
  ret i32 0
}


; These here are not part of the reduced test case. Only there to see whether it only happens with two calls

; Function Attrs: noinline
; define i32 @fde_mixed_encoding_compare_only_1_call() #0 {
; entry:
;   %retval = alloca i32, align 4
;   %call = call i32 @get_cie_encoding()
;   %0 = load i32, i32* %retval, align 4
;   ret i32 %0
; }
; ; Function Attrs: noinline
; define i32 @fde_mixed_encoding_compare_with_3_calls() #0 {
; entry:
;   %retval = alloca i32, align 4
;   %call = call i32 @get_cie_encoding()
;   %call1 = call i32 @get_cie_encoding()
;   %call2 = call i32 @get_cie_encoding()
;   %0 = load i32, i32* %retval, align 4
;   ret i32 %0
; }


attributes #0 = { noinline "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="mips64r2" "target-features"="+mips64r2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 5.0.0 (https://github.com/llvm-mirror/clang.git 2c8254bcca9f1b2cebf0db80136518156d82e6d5) (https://github.com/llvm-mirror/llvm.git 3f78469f811ac40ba4edc6530dc8303cb2a96bd6)"}
