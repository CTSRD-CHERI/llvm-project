; RUN: %cheri_llc %s -mtriple=cheri-unknown-freebsd -target-abi purecap -mxgot -relocation-model=pic -cheri-cap-table -o - -O3  | FileCheck %s
; The optimze libcalls would previously replace constant strings with AS0 strings
; Seems like this can't be tested in LLVM, but let's still check that this code is sensible
; ModuleID = '/Users/alex/cheri/llvm/tools/clang/test/CodeGen/cap-table-printf.c'
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/cap-table-printf.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

@.str = private unnamed_addr addrspace(200) constant [33 x i8] c" res_nquery retry without EDNS0\0A\00", align 1

; Function Attrs: noinline nounwind optnone
define void @a() #0 {
entry:
  %call = call i32 (i8 addrspace(200)*, ...) addrspacecast (i32 (i8 addrspace(200)*, ...)* @printf to i32 (i8 addrspace(200)*, ...) addrspace(200)*)(i8 addrspace(200)* getelementptr inbounds ([33 x i8], [33 x i8] addrspace(200)* @.str, i32 0, i32 0))
  ret void

  ;CHECK:      lui	$1, %captab_hi(.L.str)
  ;CHECK-NEXT: daddiu	$1, $1, %captab_lo(.L.str)
  ;CHECK-NEXT: clc	$c3, $1, 0($c26)
  ;CHECK-NEXT: lui	$1, %capcall_hi(printf)
  ;CHECK-NEXT: daddiu	$1, $1, %capcall_lo(printf)
  ;CHECK-NEXT: clc	$c12, $1, 0($c26)
  ;CHECK-NEXT: daddiu	$1, $zero, 0
  ;CHECK-NEXT: cjalr	$c12, $c17
  ;CHECK-NEXT: cfromptr	$c13, $c0, $zero
}

declare i32 @printf(i8 addrspace(200)*, ...) #1

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 1}
!2 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 41c2892ed18fcb87c7a4c0dc1fb1e62d38ea3119) (https://github.com/llvm-mirror/llvm.git bac9cdb960b265f8085853678663d7b69c836286)"}
