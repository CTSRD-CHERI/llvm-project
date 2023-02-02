; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = 'cheri-export.c'
source_filename = "cheri-export.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local chericcallcce i32 @_Z2fnPvS_i(i8 addrspace(200)* nocapture readnone %a, i8 addrspace(200)* nocapture readnone %b, i32 returned %c) local_unnamed_addr addrspace(200) #0 {
entry:
  ret i32 %c
}

; Make sure that the flags field is set to 11 (3 argument registers used,
; interrupts disabled), even though we have used only one argument register
; (the third one).
; CHECK: __export_foo__Z2fnPvS_i:
; CHECK:        .half   _Z2fnPvS_i-__compartment_pcc_start
; CHECK:        .byte   0
; CHECK:        .byte   11

attributes #0 = { minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn "cheri-compartment"="foo" "frame-pointer"="none" "interrupt-state"="enabled" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,+xcheri-rvc,-64bit,-relax,-save-restore" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 0}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM adc819bf615e7d73e9accfcdbe646327aad75cde)"}
