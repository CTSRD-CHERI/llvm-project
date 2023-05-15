; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = 'test.cc'
source_filename = "test.cc"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local chericcallcc void @_Z8callbackv() addrspace(200) #0 {
entry:
  ret void
}

; Function Attrs: minsize mustprogress nounwind optsize
define dso_local void @_Z4testv() local_unnamed_addr addrspace(200) #1 {
entry:
  ; Make sure that we *do* look up an import table relocation for this, but we don't look up a relocation for the direct call.
  ; CHECK-NOT: cheri_compartment_pccrel_hi(_Z8callbackv)
  ; CHECK: cheri_compartment_pccrel_hi(__import_me__Z8callbackv)
  ; CHECK-NOT: cheri_compartment_pccrel_hi(_Z8callbackv)
  notail call chericcallcc void @_Z22cross_compartment_callPU10chericcallFvvE(void () addrspace(200)* nonnull @_Z8callbackv) #3
  notail call chericcallcc void @_Z22cross_compartment_callPU10chericcallFvvE(void () addrspace(200)* nonnull @_Z8callbackv) #3
  ret void
}

; Function Attrs: minsize optsize
declare dso_local chericcallcc void @_Z22cross_compartment_callPU10chericcallFvvE(void () addrspace(200)*) local_unnamed_addr addrspace(200) #2

attributes #0 = { minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn "cheri-compartment"="me" "frame-pointer"="none" "interrupt-state"="enabled" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }
attributes #1 = { minsize mustprogress nounwind optsize "cheri-compartment"="me" "frame-pointer"="none" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }
attributes #2 = { minsize optsize "cheri-compartment"="other" "frame-pointer"="none" "interrupt-state"="enabled" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }
attributes #3 = { minsize nobuiltin nounwind optsize "no-builtins" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"SmallDataLimit", i32 8}
!3 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 2329a3e445ef8deb947cbacde0b9bec36e7fbec3)"}
