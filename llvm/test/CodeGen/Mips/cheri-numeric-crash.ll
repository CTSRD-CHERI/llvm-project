; This would previously crash attempting to optimize the constant multiplication by -10000
; TODO: check if this also crashes upstream and if it does submit the patch
; RUN: %cheri_llc -o - %s
; RUN: llc -mtriple=mips64-unknown-freebsd -o - %s
; ModuleID = 'numeric.creduce.ll-reduced-simplified.bc'
source_filename = "numeric.creduce.ll-output-a9bb50e.bc"
target datalayout = "E-m:e-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "mips64-unknown-freebsd"

; Function Attrs: nounwind
define fastcc void @int128_to_numericvar(i128 signext %val) unnamed_addr #0 {
entry:
  %sub = sub i128 0, %val
  %div = udiv i128 %sub, 10000
  %0 = mul i128 %div, -10000
  %sub7 = add i128 %0, %sub
  %conv = trunc i128 %sub7 to i16
  store i16 %conv, i16* undef, align 2
  unreachable
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="mips64r2" "target-features"="+mips64r2,+soft-float" "unsafe-fp-math"="false" "use-soft-float"="true" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 "}
