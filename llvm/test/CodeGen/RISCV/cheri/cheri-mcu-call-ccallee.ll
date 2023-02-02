; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = '/tmp/ccall-size.c'
source_filename = "/tmp/ccall-size.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: nounwind
define dso_local void @func() local_unnamed_addr addrspace(200) #0 {
entry:
  ; Make sure that there's a 4-byte stack argument passed in ct0.
  ; CHECK:         csetbounds      ct0, csp, 4
  call chericcallcce void @ccall(i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6) #2
  ret void
}

declare dso_local chericcallcce void @ccall(i32, i32, i32, i32, i32, i32, i32) local_unnamed_addr addrspace(200) #1

attributes #0 = { nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-builtin-printf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,+xcheri-rvc,-64bit,-relax,-save-restore" }
attributes #1 = { "frame-pointer"="none" "no-builtin-printf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,+xcheri-rvc,-64bit,-relax,-save-restore" }
attributes #2 = { nounwind "no-builtin-printf" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 1}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 0a5300f9fb85a014dcf8239e1813fae0fcc6526d)"}
