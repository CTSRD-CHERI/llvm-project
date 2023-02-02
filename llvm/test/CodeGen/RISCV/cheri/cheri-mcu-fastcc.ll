; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s 
; ModuleID = 'ccall.c'
source_filename = "ccall.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

@testcall8.stack_arg = internal addrspace(200) global i32 0, align 4

; Function Attrs: minsize nounwind optsize
define dso_local i32 @testcall8() local_unnamed_addr addrspace(200) #2 {
entry:
  ; Check that we have the right relocations and stack layout.
  %args = alloca [8 x i32], align 4, addrspace(200)
  %0 = bitcast [8 x i32] addrspace(200)* %args to i8 addrspace(200)*
  %arrayidx = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 0
  %arrayidx1 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 1
  %arrayidx2 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 2
  %arrayidx3 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 3
  %arrayidx4 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 4
  %arrayidx5 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 5
  %arrayidx6 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 6
  ; Make sure that we're not trying to use nonexistent registers in fastcc mode.
  ; CHECK-NOT: ca6
  ; CHECK-NOT: ca7
  %call = call fastcc i32 @test8callee(i32 addrspace(200)* nonnull %arrayidx, i32 addrspace(200)* nonnull %arrayidx1, i32 addrspace(200)* nonnull %arrayidx2, i32 addrspace(200)* nonnull %arrayidx3, i32 addrspace(200)* nonnull %arrayidx4, i32 addrspace(200)* nonnull %arrayidx5, i32 addrspace(200)* nonnull %arrayidx6, i32 addrspace(200)* nonnull @testcall8.stack_arg) #6
  ret i32 %call
}

; Function Attrs: minsize optsize
declare dso_local fastcc i32 @test8callee(i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*) local_unnamed_addr addrspace(200) #4

attributes #0 = { minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn "cheri-compartment"="example" "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #1 = { minsize mustprogress nofree norecurse nosync nounwind optsize readonly willreturn "cheri-compartment"="example" "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #2 = { minsize nounwind optsize "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #3 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #4 = { minsize optsize "cheri-compartment"="other" "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+xcheri,-64bit,-relax,-save-restore,-xcheri-rvc" }
attributes #5 = { nounwind }
attributes #6 = { minsize nounwind optsize }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 1}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 510563e51e3d10ea94e5d2f31575f8cd00dc5857)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}

; Check that we have the right import and export tables.

; BOTH:        .type   __import_other_test8callee,@object # @__import_other_test8callee
; BOTH:        .section        .compartment_imports,"aG",@progbits,__import_other_test8callee,comdat
; BOTH:        .weak  __import_other_test8callee
; BOTH:        .p2align        3
; BOTH:__import_other_test8callee:
; BOTH:        .word   __export_other_test8callee
; This is not a libcall, so it shouldn't have any add to set low bits in this
; pointer.
; BOTH-NOT: +
; BOTH:        .word   0
; BOTH:        .size   __import_other_test8callee, 8
; BOTH:        .section        .compartment_exports,"a",@progbits
; BOTH:        .type   __export_example_test2,@object
; BOTH:        .globl  __export_example_test2
; BOTH:        .p2align        2
; BOTH:__export_example_test2:
; BOTH:        .half   test2-__compartment_pcc_start
; BOTH:        .byte   0
; BOTH:        .byte   2
; BOTH:        .size   __export_example_test2, 4
; BOTH:        .type   __export_example_test6,@object
; BOTH:        .globl  __export_example_test6
; BOTH:        .p2align        2
; BOTH:__export_example_test6:
; BOTH:        .half   test6-__compartment_pcc_start
; BOTH:        .byte   0
; BOTH:        .byte   6
; BOTH:        .size   __export_example_test6, 4
; BOTH:        .type   __export_example_test8,@object
; BOTH:        .globl  __export_example_test8
; BOTH:        .p2align        2
; BOTH:__export_example_test8:
; BOTH:        .half   test8-__compartment_pcc_start
; BOTH:        .byte   0
; BOTH:        .byte   7
; BOTH:        .size   __export_example_test8, 4
