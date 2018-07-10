; RUN: %cheri_llc %s -relocation-model=pic -o - | FileCheck %s
; ModuleID = '/Users/theraven/Documents/Research/ctsrd/cheritest/trunk/tests/c/test_clang_struct.c'
target datalayout = "E-pf200:256:256:256-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f128:128:128-v64:64:64-n32-A200"
target triple = "cheri-unknown-freebsd"

%struct.example = type { i32 }

@example_object = internal addrspace(200) global %struct.example zeroinitializer, align 4


; Check that the length is set when casting a static to a capability.
; CHECK: csetbounds
define %struct.example addrspace(200)* @example_constructor() #2 {
entry:
  ret %struct.example addrspace(200)* @example_object
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noreturn "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn nounwind }

