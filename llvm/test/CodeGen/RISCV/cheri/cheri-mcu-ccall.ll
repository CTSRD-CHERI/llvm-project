; RUN: llc --filetype=asm --code-model=tiny --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s -check-prefixes BOTH
; RUN: llc --filetype=asm --code-model=small --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s -check-prefixes BOTH
; ModuleID = 'ccall.c'
source_filename = "ccall.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

@testcall6.stack_arg = internal addrspace(200) global i32 0, align 4
@testcall8.stack_arg = internal addrspace(200) global i32 0, align 4

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local chericcallcce i32 @test2(i32 %a0, i32 %a1) local_unnamed_addr addrspace(200) #0 {
entry:
  ; Check that we have the first two arguments in the right registers.
  ; BOTH-LABEL: test2:
  ; BOTH: add     a0, a0, a1
  ; BOTH: cret
  %add = add nsw i32 %a1, %a0
  ret i32 %add
}

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readonly willreturn
define dso_local chericcallcce i32 @test6(i32 addrspace(200)* nocapture readonly %a0, i32 addrspace(200)* nocapture readonly %a1, i32 addrspace(200)* nocapture readonly %a2, i32 addrspace(200)* nocapture readonly %a3, i32 addrspace(200)* nocapture readonly %a4, i32 addrspace(200)* nocapture readonly %a5) local_unnamed_addr addrspace(200) #1 {
entry:
  ; Check that we are loading the last register argument
  ; BOTH-LABEL: test6:
  ; BOTH: clw     a4, 0(ca5)
  %0 = load i32, i32 addrspace(200)* %a0, align 4, !tbaa !5
  %1 = load i32, i32 addrspace(200)* %a1, align 4, !tbaa !5
  %add = add nsw i32 %1, %0
  %2 = load i32, i32 addrspace(200)* %a2, align 4, !tbaa !5
  %add1 = add nsw i32 %add, %2
  %3 = load i32, i32 addrspace(200)* %a3, align 4, !tbaa !5
  %add2 = add nsw i32 %add1, %3
  %4 = load i32, i32 addrspace(200)* %a4, align 4, !tbaa !5
  %add3 = add nsw i32 %add2, %4
  %5 = load i32, i32 addrspace(200)* %a5, align 4, !tbaa !5
  %add4 = add nsw i32 %add3, %5
  ret i32 %add4
}

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readonly willreturn
define dso_local chericcallcce i32 @test8(i32 addrspace(200)* nocapture readonly %a0, i32 addrspace(200)* nocapture readonly %a1, i32 addrspace(200)* nocapture readonly %a2, i32 addrspace(200)* nocapture readonly %a3, i32 addrspace(200)* nocapture readonly %a4, i32 addrspace(200)* nocapture readonly %a5, i32 addrspace(200)* nocapture readonly %a6, i32 addrspace(200)* nocapture readonly %a7) local_unnamed_addr addrspace(200) #1 {
entry:
  ; Check that the last argument (used in the multiply) is loaded from offset 8
  ; in the stack-argument capability.
  ; BOTH-LABEL: test8:
  ; BOTH: clc     [[CREG:[a-z]+[0-9]+]], 8(ct0)
  ; BOTH: clw     [[IREG:[a-z]+[0-9]+]], 0([[CREG]])
  ; BOTH: mul
  ; BOTH-SAME: [[IREG]]
  %0 = load i32, i32 addrspace(200)* %a0, align 4, !tbaa !5
  %1 = load i32, i32 addrspace(200)* %a1, align 4, !tbaa !5
  %add = add nsw i32 %1, %0
  %2 = load i32, i32 addrspace(200)* %a2, align 4, !tbaa !5
  %add1 = add nsw i32 %add, %2
  %3 = load i32, i32 addrspace(200)* %a3, align 4, !tbaa !5
  %add2 = add nsw i32 %add1, %3
  %4 = load i32, i32 addrspace(200)* %a4, align 4, !tbaa !5
  %add3 = add nsw i32 %add2, %4
  %5 = load i32, i32 addrspace(200)* %a5, align 4, !tbaa !5
  %add4 = add nsw i32 %add3, %5
  %6 = load i32, i32 addrspace(200)* %a6, align 4, !tbaa !5
  %add5 = add nsw i32 %add4, %6
  %7 = load i32, i32 addrspace(200)* %a7, align 4, !tbaa !5
  %mul = mul nsw i32 %add5, %7
  ret i32 %mul
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p200i8(i64 immarg, i8 addrspace(200)* nocapture) addrspace(200) #3

; Function Attrs: minsize optsize
declare dso_local chericcallcc i32 @test6callee(i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*) local_unnamed_addr addrspace(200) #4

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p200i8(i64 immarg, i8 addrspace(200)* nocapture) addrspace(200) #3

; Function Attrs: minsize nounwind optsize
define dso_local i32 @testcall8() local_unnamed_addr addrspace(200) #2 {
entry:
  ; Check that we have the right relocations and stack layout.
  ; BOTH-LABEL: testcall8:
  ; BOTH:  auicgp  ct1, %cheri_compartment_cgprel_hi(testcall8.stack_arg)
  ; BOTH:  cincoffset      ct1, ct1, %cheri_compartment_cgprel_lo_i(testcall8.stack_arg)
  ; BOTH:  csetbounds      ct1, ct1, %cheri_compartment_size(testcall8.stack_arg)
  ; BOTH:  csc     ct1, 8(csp)
  ; BOTH:  auipcc  ct0, %cheri_compartment_pccrel_hi(__import_other_test8callee)
  ; BOTH:  cincoffset      ct0, ct0, %cheri_compartment_pccrel_lo(.LBB3_1)
  ; BOTH:  auipcc  ct2, %cheri_compartment_pccrel_hi(.compartment_switcher)
  ; BOTH:  clc     ct2, %cheri_compartment_pccrel_lo(.LBB3_2)(ct2)
  ; BOTH:  c.cjalr ct2
  %args = alloca [8 x i32], align 4, addrspace(200)
  %0 = bitcast [8 x i32] addrspace(200)* %args to i8 addrspace(200)*
  call void @llvm.lifetime.start.p200i8(i64 32, i8 addrspace(200)* nonnull %0) #5
  %arrayidx = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 0
  %arrayidx1 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 1
  %arrayidx2 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 2
  %arrayidx3 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 3
  %arrayidx4 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 4
  %arrayidx5 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 5
  %arrayidx6 = getelementptr inbounds [8 x i32], [8 x i32] addrspace(200)* %args, i32 0, i32 6
  %call = call chericcallcc i32 @test8callee(i32 addrspace(200)* nonnull %arrayidx, i32 addrspace(200)* nonnull %arrayidx1, i32 addrspace(200)* nonnull %arrayidx2, i32 addrspace(200)* nonnull %arrayidx3, i32 addrspace(200)* nonnull %arrayidx4, i32 addrspace(200)* nonnull %arrayidx5, i32 addrspace(200)* nonnull %arrayidx6, i32 addrspace(200)* nonnull @testcall8.stack_arg) #6
  call void @llvm.lifetime.end.p200i8(i64 32, i8 addrspace(200)* nonnull %0) #5
  ret i32 %call
}

; Function Attrs: minsize optsize
declare dso_local chericcallcc i32 @test8callee(i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*) local_unnamed_addr addrspace(200) #4

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
