; RUN: %cheri_llc -cheri-cfi -relocation-model=pic %s -o - | FileCheck %s
; ModuleID = 'callret.c'
target datalayout = "E-m:m-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
; CHECK-LABEL: bar
define i32 @bar(i32 signext %x) #0 {
entry:
; Check that we're deriving a PCC-relative return capability and spilling it to the stack
; CHECK: cgetpcc	$c16
; CHECK: csetoffset	$c16, $c16, $ra
; CHECK: csc	$c16, $sp
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
; Check that there's a call in the middle
; CHECK: jalr	$25
  %call = call i32 @foo(i32 signext %0)
  %1 = load i32, i32* %x.addr, align 4
  %add = add nsw i32 %call, %1
; Check that we reload the return cap and jump to it
; CHECK: clc	$c16, $sp,
; CHECK: cjr	$c16
  ret i32 %add
}

declare i32 @foo(i32 signext) #1

; Function Attrs: nounwind
; CHECK-LABEL: baz
define i32 @baz(i32 signext %x) #0 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
; Check that, if we've had no reason to spill $ra, we just use it for the
; return.
; CHECK: jr	$ra
  ret i32 %0
}

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm/tools/clang 80929b3db96cf6409ddd5339c73b3616e9ec872c) (ssh://dc552@vica.cl.cam.ac.uk:/home/dc552/CHERISDK/llvm 7747414cd65fe72c94dd4219668e6bc025f92ad6)"}
