; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = 'test.cc'
source_filename = "test.cc"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local chericcallcce void @_Z4ret0v() local_unnamed_addr addrspace(200) #0 {
entry:
  ; CHECK-LABEL: _Z4ret0v
  ; CHECK: mv    a0, zero
  ; CHECK: mv    a1, zero
  ; CHECK: cret
  ret void
}

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local chericcallcce i32 @_Z4ret1v() local_unnamed_addr addrspace(200) #0 {
entry:
  ; CHECK-LABEL: _Z4ret1v
  ; CHECK: addi  a0, zero, 1
  ; CHECK: mv    a1, zero
  ; CHECK: cret
  ret i32 1
}

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local chericcallcce [2 x i32] @_Z4ret2v() local_unnamed_addr addrspace(200) #0 {
entry:
  ; CHECK-LABEL: _Z4ret2v
  ; CHECK: addi  a0, zero, 1
  ; CHECK: addi  a1, zero, 2
  ; CHECK: cret
  ret [2 x i32] [i32 1, i32 2]
}

attributes #0 = { minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn "cheri-compartment"="me" "frame-pointer"="none" "interrupt-state"="enabled" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"SmallDataLimit", i32 8}
!3 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 2329a3e445ef8deb947cbacde0b9bec36e7fbec3)"}
