; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s 
; ModuleID = '/tmp/fnptr.c'
source_filename = "/tmp/fnptr.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn
define dso_local nonnull void () addrspace(200)* @x() local_unnamed_addr addrspace(200) #0 {
entry:
  ; CHECK: auipcc  ca0, %cheri_compartment_pccrel_hi(foo)
  ; CHECK: cincoffset      ca0, ca0, %cheri_compartment_pccrel_lo(.LBB0_1)
  ; CHECK-NOT: csetbounds
  ; CHECK: cret
  ret void () addrspace(200)* @foo
}

declare dso_local void @foo() addrspace(200) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-builtin-printf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,+xcheri-rvc,-64bit,-relax,-save-restore" }
attributes #1 = { "frame-pointer"="none" "no-builtin-printf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,+xcheri-rvc,-64bit,-relax,-save-restore" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 1}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM de1ff164fd92033343fe416e777c7c2edca47660)"}
