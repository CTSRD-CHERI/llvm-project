; RUN: opt -inline %s -S | FileCheck %s
; ModuleID = '../llvm-trunk-release/inline.c'
target datalayout = "e-m:e-p:32:32-i64:64-n32-S128"
target triple = "riscv32-unknown-unknown"

; Check that inlining respects the interrupt state.  Interrupt state is one out
; of:
;  - Enabled
;  - Disabled
;  - Inherited
;
; It is safe to inline any function if the interrupt state of the callee is
; inherited, or if the caller and callee's interrupt state are the same.


; Function Attrs: nounwind optnone
define dso_local i32 @inner_disabled() #0 {
entry:
  %call = call i32 @foo()
  ret i32 %call
}

declare dso_local i32 @foo() #1

; Function Attrs: nounwind optnone
define dso_local i32 @inner_enabled() #2 {
entry:
  %call = call i32 @foo()
  ret i32 %call
}

; Function Attrs: nounwind optnone
define dso_local i32 @inner_inherit() #3 {
entry:
  %call = call i32 @foo()
  ret i32 %call
}

; Function Attrs: nounwind optnone
define dso_local i32 @outer_disabled_d() #0 {
entry:
  ; CHECK-LABEL: define dso_local i32 @outer_disabled_d
  ; CHECK: call i32 @foo()
  %call = call i32 @inner_disabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind optnone
define dso_local i32 @outer_disabled_e() #0 {
entry:
  ; CHECK-LABEL: define dso_local i32 @outer_disabled_e
  ; CHECK: call i32 @inner_enabled()
  %call = call i32 @inner_enabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind optnone
define dso_local i32 @outer_disabled_i() #0 {
entry:
  ; CHECK-LABEL: define dso_local i32 @outer_disabled_i
  ; CHECK: call i32 @foo()
  %call = call i32 @inner_inherit()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind optnone
define dso_local i32 @outer_enabled_d() #2 {
entry:
  ; CHECK-LABEL: define dso_local i32 @outer_enabled_d
  ; CHECK: call i32 @inner_disabled()
  %call = call i32 @inner_disabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind optnone
define dso_local i32 @outer_enabled_e() #2 {
entry:
  ; CHECK-LABEL: define dso_local i32 @outer_enabled_e
  ; CHECK: call i32 @foo()
  %call = call i32 @inner_enabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind optnone
define dso_local i32 @outer_enabled_i() #2 {
entry:
  ; CHECK-LABEL: define dso_local i32 @outer_enabled_i
  ; CHECK: call i32 @foo()
  %call = call i32 @inner_inherit()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind optnone
define dso_local i32 @outer_inherit_d() #3 {
entry:
  ; CHECK-LABEL: define dso_local i32 @outer_inherit_d
  ; CHECK: call i32 @inner_disabled()
  %call = call i32 @inner_disabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind optnone
define dso_local i32 @outer_inherit_e() #3 {
entry:
  ; CHECK-LABEL: define dso_local i32 @outer_inherit_e
  ; CHECK: call i32 @inner_enabled()
  %call = call i32 @inner_enabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind optnone
define dso_local i32 @outer_inherit_i() #3 {
entry:
  ; CHECK-LABEL: define dso_local i32 @outer_inherit_i
  %call = call i32 @inner_inherit()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

attributes #0 = { nounwind "frame-pointer"="all" "interrupt-state"="disabled" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+a,+c,+m,+relax,-save-restore,-xcheri-rvc" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+a,+c,+m,+relax,-save-restore,-xcheri-rvc" }
attributes #2 = { nounwind "frame-pointer"="all" "interrupt-state"="enabled" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+a,+c,+m,+relax,-save-restore,-xcheri-rvc" }
attributes #3 = { nounwind "frame-pointer"="all" "interrupt-state"="inherit" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+a,+c,+m,+relax,-save-restore,-xcheri-rvc" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"ilp32"}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 83ddcec972c6a81bda278ebdb3791041fc8ce47c)"}
