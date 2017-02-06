; RUN: llc -o - %s
; Test that this compiles without any errors
; ModuleID = 'cheri-static-init.c'
source_filename = "cheri-static-init.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@f = addrspace(200) global void (...) addrspace(200)* addrspacecast (void (...)* bitcast (void ()* @_none_mbrtowc to void (...)*) to void (...) addrspace(200)*), align 32

; Function Attrs: noinline nounwind
define void @_none_mbrtowc() #0 {
entry:
  ret void
}

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 5.0.0 (http://llvm.org/git/clang.git c3aa15f4357315da8260ac267b867257d9a49f2e) (git@github.com:RichardsonAlex/llvm-cheri.git 952c77070b3c9e77efab3ad3f04175e2c9425cda)"}
