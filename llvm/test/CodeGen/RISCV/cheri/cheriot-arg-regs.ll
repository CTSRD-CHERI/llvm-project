; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = 'big.c'

; Smoke test the calling convention.  Isn't actually checking that things go in
; the right registers, only that the right number of spill slots are used on the
; caller side.

source_filename = "big.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn
define dso_local i32 @manyInts(i32 %a0, i32 %a1, i32 %a2, i32 %a3, i32 %a4, i32 %a5, i32 %a6, i32 %a7, i32 %a8, i32 %a9, i32 %a10) local_unnamed_addr addrspace(200) #0 {
entry:
  ; CHECK-LABEL: manyInts:
  %add = add nsw i32 %a1, %a0
  %add1 = add nsw i32 %add, %a2
  %add2 = add nsw i32 %add1, %a3
  %add3 = add nsw i32 %add2, %a4
  %add4 = add nsw i32 %add3, %a5
  %add5 = add nsw i32 %add4, %a6
  %add6 = add nsw i32 %add5, %a7
  %add7 = add nsw i32 %add6, %a8
  %add8 = add nsw i32 %add7, %a9
  %add9 = add nsw i32 %add8, %a10
  ; Check that we at least got the right number of arguments
  ; Check that we at least got the right number of arguments
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: cret
  ret i32 %add9
}

; Function Attrs: minsize nounwind optsize
define dso_local i32 @callBigInts() local_unnamed_addr addrspace(200) #1 {
entry:
  ; CHECK-LABEL: callBigInts:
  ; 11 arguments, 6 in registers, 5 on the stack.
  ; CHECK: csw
  ; CHECK: csw
  ; CHECK: csw
  ; CHECK: csw
  ; CHECK: csw
  ; Call the function
  ; CHECK: manyIntsExt
  ; CHECK: cret
  %call = tail call i32 @manyIntsExt(i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10) #5
  ret i32 %call
}

; Function Attrs: minsize optsize
declare dso_local i32 @manyIntsExt(i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32) local_unnamed_addr addrspace(200) #2

; Function Attrs: minsize mustprogress nofree norecurse nosync nounwind optsize readonly willreturn
define dso_local i32 @manyCaps(i32 addrspace(200)* nocapture readonly %a0, i32 addrspace(200)* nocapture readonly %a1, i32 addrspace(200)* nocapture readonly %a2, i32 addrspace(200)* nocapture readonly %a3, i32 addrspace(200)* nocapture readonly %a4, i32 addrspace(200)* nocapture readonly %a5, i32 addrspace(200)* nocapture readonly %a6, i32 addrspace(200)* nocapture readonly %a7, i32 addrspace(200)* nocapture readonly %a8, i32 addrspace(200)* nocapture readonly %a9, i32 addrspace(200)* nocapture readonly %a10) local_unnamed_addr addrspace(200) #3 {
entry:
  ; CHECK-LABEL: manyCaps:
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
  %add6 = add nsw i32 %add5, %7
  %8 = load i32, i32 addrspace(200)* %a8, align 4, !tbaa !5
  %add7 = add nsw i32 %add6, %8
  %9 = load i32, i32 addrspace(200)* %a9, align 4, !tbaa !5
  %add8 = add nsw i32 %add7, %9
  %10 = load i32, i32 addrspace(200)* %a10, align 4, !tbaa !5
  %add9 = add nsw i32 %add8, %10
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: add
  ; CHECK: cret
  ret i32 %add9
}

; Function Attrs: minsize nounwind optsize
define dso_local i32 @callBigCaps() local_unnamed_addr addrspace(200) #1 {
entry:
  ; CHECK-LABEL: callBigCaps:
  %buffer = alloca [11 x i32], align 4, addrspace(200)
  %0 = bitcast [11 x i32] addrspace(200)* %buffer to i8 addrspace(200)*
  call void @llvm.lifetime.start.p200i8(i64 44, i8 addrspace(200)* nonnull %0) #6
  %arraydecay = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 0
  %add.ptr = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 1
  %add.ptr3 = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 2
  %add.ptr5 = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 3
  %add.ptr7 = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 4
  %add.ptr9 = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 5
  %add.ptr11 = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 6
  %add.ptr13 = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 7
  %add.ptr15 = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 8
  %add.ptr17 = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 9
  %add.ptr19 = getelementptr inbounds [11 x i32], [11 x i32] addrspace(200)* %buffer, i32 0, i32 10
  ; Useful point after all of the spills in the prolog
  ; CHECK: csetbounds
  ; 6 argument registers, 11 arguments, so 5 spills
  ; CHECK: csc
  ; CHECK: csc
  ; CHECK: csc
  ; CHECK: csc
  ; CHECK: csc
  ; Call the function (call instruction may change soon)
  ; CHECK: manyCapsExt
  ; cret
  %call = call i32 @manyCapsExt(i32 addrspace(200)* nonnull %arraydecay, i32 addrspace(200)* nonnull %add.ptr, i32 addrspace(200)* nonnull %add.ptr3, i32 addrspace(200)* nonnull %add.ptr5, i32 addrspace(200)* nonnull %add.ptr7, i32 addrspace(200)* nonnull %add.ptr9, i32 addrspace(200)* nonnull %add.ptr11, i32 addrspace(200)* nonnull %add.ptr13, i32 addrspace(200)* nonnull %add.ptr15, i32 addrspace(200)* nonnull %add.ptr17, i32 addrspace(200)* nonnull %add.ptr19) #5
  call void @llvm.lifetime.end.p200i8(i64 44, i8 addrspace(200)* nonnull %0) #6
  ret i32 %call
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p200i8(i64 immarg, i8 addrspace(200)* nocapture) addrspace(200) #4

; Function Attrs: minsize optsize
declare dso_local i32 @manyCapsExt(i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*, i32 addrspace(200)*) local_unnamed_addr addrspace(200) #2

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p200i8(i64 immarg, i8 addrspace(200)* nocapture) addrspace(200) #4

attributes #0 = { minsize mustprogress nofree norecurse nosync nounwind optsize readnone willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #1 = { minsize nounwind optsize "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #2 = { minsize optsize "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #3 = { minsize mustprogress nofree norecurse nosync nounwind optsize readonly willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,-64bit,-save-restore,-xcheri-rvc" }
attributes #4 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #5 = { minsize nounwind optsize }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"Code Model", i32 0}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"clang version 13.0.0 (git@ssh.dev.azure.com:v3/Portmeirion/CHERI-MCU/LLVM 0d1665aea68c45b9d65c0dd019b1591dc827b17d)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
