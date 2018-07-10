; RUN: %cheri_llc -o /dev/null -relocation-model=pic -target-abi purecap -mips-ssection-threshold=0 %s
; XFAIL: * 
; https://github.com/CTSRD-CHERI/llvm/issues/239
; ModuleID = '/local/scratch/alr48/cheri/build/llvm-256-build/quota-8cee50-bugpoint-reduce.ll-reduced-simplified.bc'
source_filename = "quota-8cee50-bugpoint-reduce.ll-output-c4dba69.bc"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

module asm ".ident\09\22$FreeBSD$\22"

; Function Attrs: nounwind
define void @showquotas() #0 {
entry:
  %savedstack287 = call i8* @llvm.stacksave.p0i8()
  unreachable
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave.p0i8() #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 "}
