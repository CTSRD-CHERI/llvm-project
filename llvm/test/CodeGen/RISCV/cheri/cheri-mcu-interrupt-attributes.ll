; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = '../llvm-trunk-release/inline.c'
source_filename = "../llvm-trunk-release/inline.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Callees with explicit interrupt states

; Function Attrs: noinline nounwind
define dso_local i32 @inner_disabled() local_unnamed_addr addrspace(200) #0 {
entry:
  %call = call i32 @foo() #7
  ret i32 %call
}

declare dso_local i32 @foo() local_unnamed_addr addrspace(200) #1

; Function Attrs: noinline nounwind
define dso_local i32 @inner_enabled() local_unnamed_addr addrspace(200) #2 {
entry:
  %call = call i32 @foo() #7
  ret i32 %call
}

; Function Attrs: noinline nounwind
define dso_local i32 @inner_inherit() local_unnamed_addr addrspace(200) #3 {
entry:
  %call = call i32 @foo() #7
  ret i32 %call
}



; Function Attrs: nounwind
define dso_local i32 @outer_disabled_d() local_unnamed_addr addrspace(200) #4 {
entry:
  ; Same state, should be a direct call:
  ; CHECK-LABEL: outer_disabled_d:
  ; CHECK: ccall   inner_disabled
  %call = call i32 @inner_disabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind
define dso_local i32 @outer_disabled_e() local_unnamed_addr addrspace(200) #4 {
entry:
  ; Different state, should call via import table
  ; CHECK-LABEL: outer_disabled_e
  ; CHECK: %cheri_compartment_pccrel_hi(__library_import__inner_enabled)
  ; CHECK: cjalr
  %call = call i32 @inner_enabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind
define dso_local i32 @outer_disabled_i() local_unnamed_addr addrspace(200) #4 {
entry:
  ; Callee inherits, should be a direct call:
  ; CHECK-LABEL: outer_disabled_i
  ; CHECK: ccall inner_inherit
  %call = call i32 @inner_inherit()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind
define dso_local i32 @outer_enabled_d() local_unnamed_addr addrspace(200) #5 {
entry:
  ; Different state, should call via import table
  ; CHECK-LABEL: outer_enabled_d
  ; CHECK: %cheri_compartment_pccrel_hi(__library_import__inner_disabled)
  ; CHECK: cjalr
  %call = call i32 @inner_disabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind
define dso_local i32 @outer_enabled_e() local_unnamed_addr addrspace(200) #5 {
entry:
  ; Same state, should be a direct call:
  ; CHECK-LABEL: outer_enabled_e
  ; CHECK: ccall inner_enabled
  %call = call i32 @inner_enabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind
define dso_local i32 @outer_enabled_i() local_unnamed_addr addrspace(200) #5 {
entry:
  ; Callee inherits, should be a direct call:
  ; CHECK-LABEL: outer_enabled_i
  ; CHECK: ccall inner_inherit
  %call = call i32 @inner_inherit()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind
define dso_local i32 @outer_inherit_d() local_unnamed_addr addrspace(200) #6 {
entry:
  ; Different state, should call via import table
  ; CHECK-LABEL: outer_inherit_d
  ; CHECK: %cheri_compartment_pccrel_hi(__library_import__inner_disabled)
  ; CHECK: cjalr
  %call = call i32 @inner_disabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind
define dso_local i32 @outer_inherit_e() local_unnamed_addr addrspace(200) #6 {
entry:
  ; Different state, should call via import table
  ; CHECK-LABEL: outer_inherit_e
  ; CHECK: %cheri_compartment_pccrel_hi(__library_import__inner_enabled)
  ; CHECK: cjalr
  %call = call i32 @inner_enabled()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

; Function Attrs: nounwind
define dso_local i32 @outer_inherit_i() local_unnamed_addr addrspace(200) #6 {
entry:
  ; Same state and callee inherits, should be a direct call:
  ; CHECK-LABEL: outer_inherit_i
  ; CHECK: ccall inner_inherit
  %call = call i32 @inner_inherit()
  %add = add nsw i32 %call, 12
  ret i32 %add
}

attributes #0 = { noinline nounwind "frame-pointer"="none" "interrupt-state"="disabled" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #1 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #2 = { noinline nounwind "frame-pointer"="none" "interrupt-state"="enabled" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #3 = { noinline nounwind "frame-pointer"="none" "interrupt-state"="inherit" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #4 = { nounwind "frame-pointer"="none" "interrupt-state"="disabled" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #5 = { nounwind "frame-pointer"="none" "interrupt-state"="enabled" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #6 = { nounwind "frame-pointer"="none" "interrupt-state"="inherit" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #7 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"SmallDataLimit", i32 8}
!3 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 83ddcec972c6a81bda278ebdb3791041fc8ce47c)"}

; CHECK-NOT: .globl __library_export__inner_enabled
; CHECK: __library_export__inner_disabled:
; CHECK-NEXT:        .half   inner_disabled-__compartment_pcc_start
; CHECK-NEXT:        .byte   0
; Check that this has the correct bit set to indicate interrupts are disabled
; CHECK-NEXT:        .byte   16
; CHECK: __library_export__inner_enabled:
; CHECK-NEXT:        .half   inner_enabled-__compartment_pcc_start
; CHECK-NEXT:        .byte   0
; Check that this has the correct bit set to indicate interrupts are enabled
; CHECK-NEXT:        .byte   8
