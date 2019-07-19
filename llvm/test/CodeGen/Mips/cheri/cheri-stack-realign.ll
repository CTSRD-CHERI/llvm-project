; RUN: %cheri256_purecap_llc -O0 %s -o - | FileCheck %s
; ModuleID = 'cheri-stack.c'
source_filename = "cheri-stack.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define void @dynamic_alloca(i32 signext %x) local_unnamed_addr #0 {
entry:
  ; CHECK: cgetaddr	$[[SPOFFSET:([0-9]+|sp)]], $c11
  ; CHECK: daddiu	$[[MASK:([0-9]+|sp)]], $zero, -64
  ; CHECK: and	$[[NEWSPOFFSET:([0-9]+|sp)]], $[[SPOFFSET]], $[[MASK]]
  ; CHECK: csetaddr	$c11, $c11, $[[NEWSPOFFSET]]

  %0 = zext i32 %x to i64
  %vla = alloca i32, i64 %0, align 64, addrspace(200)
  call void @use_arg(i32 addrspace(200)* nonnull %vla) #2
  ret void
}

declare void @use_arg(i32 addrspace(200)*) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 5.0.0 "}
