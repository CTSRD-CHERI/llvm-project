; Check that -mgpopt doesn't crash on unsized externals
; RUN: llc -mtriple=mips64-unknown-freebsd -target-abi n64 -mgpopt -o - %s | FileCheck %s

; ModuleID = '/Users/alex/cheri/llvm/tools/clang/test/CodeGen/uart_subr-fd65f8-reduce.test.c'
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/uart_subr-fd65f8-reduce.test.c"
target datalayout = "E-m:e-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "mips64-unknown-freebsd"

%struct.a = type opaque

@b = external global %struct.a, align 1

; Function Attrs: norecurse nounwind readnone
define i32 @d() local_unnamed_addr #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  br i1 icmp ugt (%struct.a* @b, %struct.a* null), label %for.cond, label %for.end

  ; we should use a sequence of HIGHEST/HIGHER/HI/LO here:
  ; CHECK:      lui     $1, %highest(b)
  ; CHECK-NEXT: daddiu  $1, $1, %higher(b)
  ; CHECK-NEXT: dsll    $1, $1, 16
  ; CHECK-NEXT: daddiu  $1, $1, %hi(b)
  ; CHECK-NEXT: dsll    $1, $1, 16
  ; CHECK-NEXT: daddiu  $2, $1, %lo(b)


for.end:                                          ; preds = %for.cond
  ret i32 undef
}

attributes #0 = { norecurse nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="mips64" "target-features"="+mips64,+noabicalls" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 1131af2e23396b93b1004df1751bfa3fef7990be) (https://github.com/llvm-mirror/llvm.git a2788ab93c15f82a12423c907426f602a4fdc5a3)"}
