; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = 'test.cc'
source_filename = "test.cc"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: minsize mustprogress nounwind optsize
define dso_local void @_Z15internal_callerv() local_unnamed_addr addrspace(200) #0 {
entry:
  tail call chericcallcce void @_Z5emptyv() #2
  ret void
}

; Make sure that, if we are calling an exported function, but have to call it
; indirectly to use the correct sentries, that we call it with an import that
; doesn't conflict with the normal import.
; CHECK: _Z15internal_callerv:
; CHECK:         auipcc  ca0, %cheri_compartment_pccrel_hi(__library_import_example__Z5emptyv)
; CHECK:         cincoffset      ca0, ca0, %cheri_compartment_pccrel_lo(_Z15internal_callerv)
; CHECK:         cjr     ca0
; CHECK: __library_import_example__Z5emptyv:
; Although the import is in the library call namespace, the export should not be.
; CHECK:         .word   __export_example__Z5emptyv+1

; Function Attrs: minsize optsize
declare dso_local chericcallcce void @_Z5emptyv() local_unnamed_addr addrspace(200) #1

attributes #0 = { minsize mustprogress nounwind optsize "cheri-compartment"="example" "frame-pointer"="none" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,+xcheri-rvc,-64bit,-relax,-save-restore" }
attributes #1 = { minsize optsize "cheri-compartment"="example" "frame-pointer"="none" "interrupt-state"="enabled" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,+xcheri-rvc,-64bit,-relax,-save-restore" }
attributes #2 = { minsize nobuiltin nounwind optsize "no-builtins" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 0}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 646abd81c6ddb64fa9ebf684785ae59439ea7a3c)"}
