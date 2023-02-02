; RUN: llc --filetype=obj --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | llvm-objdump -d - | FileCheck %s
; ModuleID = 'static.c'
source_filename = "static.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local nonnull i32 () addrspace(200)* @num() local_unnamed_addr addrspace(200) align 8192 #0 {
entry:
; compartment_pccrel_hi relocation (2 * 4096) applied to auipcc
; CHECK: 0: 17 45 00 00   auipcc  ca0, 4
; compartment_pccrel_lo relocation (8) applied to cincoffset
; CHECK: 4: 5b 15 85 00   cincoffset      ca0, ca0, 8
  ret i32 () addrspace(200)* @ft
}

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local i32 @padding() addrspace(200) align 8192 #0 {
entry:
  ret i32 42
}

; Check that the target is where we expect it to be.
; CHECK: 00002008 <ft>:
; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define internal i32 @ft() addrspace(200) #0 {
entry:
  ret i32 42
}

attributes #0 = { minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 1}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 477cfe2b062105f29c0f7d48a87268d21ca2e3b6)"}
