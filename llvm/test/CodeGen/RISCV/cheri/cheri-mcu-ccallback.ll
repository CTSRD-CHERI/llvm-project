; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = 'ccallback.c'
source_filename = "ccallback.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

@x = internal unnamed_addr addrspace(200) global i32 0, align 4

; Function Attrs: nounwind
define dso_local void @call_callback(i32 (i8 addrspace(200)*) addrspace(200)* nocapture %cb) local_unnamed_addr addrspace(200) #0 {
entry:
; Make sure that calling the callback has a sensible call sequence:
; CHECK-LABEL: call_callback:
; Move the argument register to the compartment switcher target
; CHECK: cmove   ct1, ca0
; Allocate 42 bytes of stack storage and store it in ca0
; CHECK: cincoffset      ca0, csp,
; CHECK: csetbounds      ca0, ca0, 42
; Load the compartment switcher into $ct2 and jump there
; CHECK: auipcc  ct2, %cheri_compartment_pccrel_hi(.compartment_switcher)
; CHECK: clc     ct2, %cheri_compartment_pccrel_lo(.LBB0_1)(ct2)
; CHECK: cjalr   ct2
  %buf = alloca [42 x i8], align 1, addrspace(200)
  %0 = getelementptr inbounds [42 x i8], [42 x i8] addrspace(200)* %buf, i32 0, i32 0
  %call = notail call chericcallcc i32 %cb(i8 addrspace(200)* nonnull %0) #4
  ret void
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p200i8(i64 immarg, i8 addrspace(200)* nocapture) addrspace(200) #1

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p200i8(i64 immarg, i8 addrspace(200)* nocapture) addrspace(200) #1

; Function Attrs: nounwind
define dso_local void @pass_callback() local_unnamed_addr addrspace(200) #0 {
entry:
; CHECK-LABEL: pass_callback:
; Check that this call is loading the import table entry, not the function
; address
; CHECK:         auipcc  ca0, %cheri_compartment_pccrel_hi(__import_comp_cb)
; CHECK:         cincoffset      ca0, ca0, %cheri_compartment_pccrel_lo(.LBB1_1)
; CHECK:         clc     ca0, 0(ca0)
; And make sure that it's really jumping to the right function.
; CHECK: ccall   take_callback
  call void @take_callback(i32 (i8 addrspace(200)*) addrspace(200)* nonnull @cb) #4
  call void @take_callback(i32 (i8 addrspace(200)*) addrspace(200)* nonnull @ecb) #4
  ret void
}

declare dso_local void @take_callback(i32 (i8 addrspace(200)*) addrspace(200)*) local_unnamed_addr addrspace(200) #2

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn
declare chericcallcc i32 @ecb(i8 addrspace(200)* nocapture readnone %buf) addrspace(200) #3

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn
define internal chericcallcc i32 @cb(i8 addrspace(200)* nocapture readnone %buf) addrspace(200) #3 {
entry:
  %0 = load i32, i32 addrspace(200)* @x, align 4, !tbaa !4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32 addrspace(200)* @x, align 4, !tbaa !4
  ret i32 %0
}

; Make sure that the import table entry a COMDAT for the external function
; CHECK:        .type   __import_comp_ecb,@object       # @__import_comp_ecb
; CHECK:        .section        .compartment_imports,"aG",@progbits,__import_comp_ecb,comdat
; CHECK:        .weak   __import_comp_ecb
; CHECK:        .p2align        3
; CHECK:__import_comp_ecb:
; CHECK:        .word   __export_comp_ecb
; CHECK:        .word   0
; CHECK:        .size   __import_comp_ecb, 8
; Make sure that the import and export table entries are local for the internal function.
; CHECK: .section        .compartment_imports,"a",@progbits
; CHECK-NOT: comdat
; CHECK-NOT: .globl __import_comp_cb
; CHECK: __import_comp_cb:
; CHECK: .section        .compartment_exports,"a",@progbits
; CHECK-NOT: comdat
; CHECK-NOT: .globl __export_comp_cb
; CHECK-NOT: .globl __export_comp_ecb
; CHECK: __export_comp_cb:
; CHECK-NOT: __export_comp_ecb:

attributes #0 = { nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" "cheri-compartment"="comp" }
attributes #1 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #2 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }
attributes #3 = { mustprogress nofree norecurse nosync nounwind willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" "cheri-compartment"="comp" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"SmallDataLimit", i32 8}
!3 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM c932702871f30deca612e0f2918db8127f487bda)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
