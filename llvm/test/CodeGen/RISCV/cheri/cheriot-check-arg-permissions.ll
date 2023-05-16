; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = 'test.cc'
source_filename = "test.cc"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

%struct.ret = type { i8 addrspace(200)*, i8 addrspace(200)* }

; A function with an sret argument.  This must be on the caller's stack.
; Note: This might not be quite true for C++ guaranteed copy elision, but that
; introduces some complications into the programmer model because the callee
; doesn't know if it's safe to store on-stack things there (storing on-stack
; pointers there at the *end* of the call is a bug, but temporary values
; may be on the stack).

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize willreturn writeonly
define dso_local chericcallcce void @_Z4ret2v(%struct.ret addrspace(200)* noalias nocapture sret(%struct.ret) align 8 %agg.result) local_unnamed_addr addrspace(200) #0 {
entry:
  ; CHECK-LABEL: _Z4ret2v:
  ; Make sure that the base points to the current address
  ; CHECK:      cgetaddr        t1, ca0
  ; CHECK:      cgetbase        t2, ca0
  ; CHECK:      bne     t1, t2,
  ; Make sure that the base is above the current stack pointer
  ; CHECK:      blt     t1, sp,
  ; Make sure the length is sufficient for the returned structure
  ; CHECK:      cgetlen t1, ca0
  ; CHECK:      addi    t2, zero, 16
  ; CHECK:      blt     t1, t2,
  ; Make sure that the permissions are as expected
  ; CHECK:      cgetperm t1, ca0
  ; CHECK:      addi    t2, zero, 126
  ; CHECK:      bne     t1, t2,
  %a = getelementptr inbounds %struct.ret, %struct.ret addrspace(200)* %agg.result, i32 0, i32 0
  store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %a, align 8, !tbaa !4
  %b = getelementptr inbounds %struct.ret, %struct.ret addrspace(200)* %agg.result, i32 0, i32 1
  store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %b, align 8, !tbaa !9
  ret void
}

; A function with on-stack arguments.  These are passed in ct0.  As with sret
; pointers, they are not visible in the C/C++ abstract machine and so the
; compiler must insert the check that they are valid.

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local chericcallcce i32 @_Z7bigargsiiiiiiii(i32 %a0, i32 %a1, i32 %a2, i32 %a3, i32 %a4, i32 %a5, i32 %a6, i32 %a7) local_unnamed_addr addrspace(200) #1 {
entry:
  ; CHECK-LABEL: _Z7bigargsiiiiiiii:
  ; Make sure that the base points to the current address
  ; CHECK:      cgetaddr        t1, ct0
  ; CHECK:      cgetbase        t2, ct0
  ; CHECK:      bne     t1, t2,
  ; Make sure that the base is above the current stack pointer
  ; CHECK:      blt     t1, sp,
  ; Make sure the length is sufficient for the returned structure
  ; CHECK:      cgetlen t1, ct0
  ; CHECK:      addi    t2, zero, 8
  ; CHECK:      blt     t1, t2,
  ; Make sure that the permissions are as expected
  ; CHECK:      cgetperm t1, ct0
  ; CHECK:      addi    t2, zero, 126
  ; CHECK:      bne     t1, t2,
  %add = add nsw i32 %a7, %a6
  ret i32 %add
}

attributes #0 = { minsize mustprogress nofree norecurse nosync nounwind optsize willreturn writeonly "cheri-compartment"="me" "frame-pointer"="none" "interrupt-state"="enabled" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }
attributes #1 = { minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn "cheri-compartment"="me" "frame-pointer"="none" "interrupt-state"="enabled" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"SmallDataLimit", i32 8}
!3 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 198968b29f2621a055c4b8a190164a9650c23ff1)"}
!4 = !{!5, !6, i64 0}
!5 = !{!"_ZTS3ret", !6, i64 0, !6, i64 8}
!6 = !{!"any pointer", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!5, !6, i64 8}
