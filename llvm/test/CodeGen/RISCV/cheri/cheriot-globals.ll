; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = 'glob.c'
source_filename = "glob.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

@y = dso_local local_unnamed_addr addrspace(200) global i32 12, align 4
@x = dso_local addrspace(200) global i32 0, align 4
@z = external dso_local local_unnamed_addr addrspace(200) constant i32, align 4

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local nonnull i32 addrspace(200)* @a() local_unnamed_addr addrspace(200) #0 {
entry:
  ; CHECK-LABEL: a:
  ; If we're generating a $cgp-relative offset and a size for address-taken accesses to globals
  ; CHECK: cincoffset      ca0, cgp, %cheri_compartment_rel(x)
  ; CHECK: csetbounds ca0, ca0, %cheri_size(x) 
  ret i32 addrspace(200)* @x
}

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readonly willreturn
define dso_local i32 @v() addrspace(200) #1 {
entry:
  ; Check that we're not setting bounds on direct accesses to globals
  ; CHECK-LABEL: v:
  ; CHECK: clw     a0, %cheri_compartment_rel(x)(cgp)
  ; CHECK-NEXT: cret
  %0 = load i32, i32 addrspace(200)* @x, align 4, !tbaa !5
  ret i32 %0
}

attributes #0 = { minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #1 = { minsize mustprogress nofree norecurse nosync nounwind optsize readonly willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #2 = { minsize mustprogress nofree norecurse nosync nounwind optsize willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 0}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 18e025b950174e35f2345e10f667bb442611895b)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
