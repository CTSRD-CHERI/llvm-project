; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = 'test.c'
source_filename = "../Release/test.c"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

@ca = dso_local local_unnamed_addr addrspace(200) global [3 x i8] zeroinitializer, align 1
@cca = external dso_local local_unnamed_addr addrspace(200) constant [0 x i8], align 1
@ia = dso_local local_unnamed_addr addrspace(200) global [3 x i32] zeroinitializer, align 4
@la = dso_local local_unnamed_addr addrspace(200) global [3 x i32] zeroinitializer, align 4
@pa = dso_local local_unnamed_addr addrspace(200) global [3 x i8 addrspace(200)*] zeroinitializer, align 8

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly willreturn
define dso_local zeroext i8 @getCharArrayOutOfBounds() local_unnamed_addr addrspace(200) #0 {
entry:
  ; CHECK-LABEL: getCharArrayOutOfBounds
  ; CHECK: csetbounds
  ; CHECK-SAME: (ca)
  %0 = load i8, i8 addrspace(200)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(200)* @ca, i32 1, i32 0), align 1, !tbaa !4
  ret i8 %0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly willreturn
define dso_local zeroext i8 @getCharArrayInbounds() local_unnamed_addr addrspace(200) #0 {
entry:
  ; CHECK-LABEL: getCharArrayInbounds
  ; CHECK-NOT: csetbounds
  %0 = load i8, i8 addrspace(200)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(200)* @ca, i32 0, i32 2), align 1, !tbaa !4
  ret i8 %0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn
define dso_local zeroext i8 @getCharArray() local_unnamed_addr addrspace(200) #1 {
entry:
  ; We don't know the bounds of this global, so we should do bounds checks
  ; CHECK-LABEL: getCharArray
  ; CHECK: csetbounds
  ; CHECK-SAME: (cca)
  %0 = load i8, i8 addrspace(200)* getelementptr inbounds ([0 x i8], [0 x i8] addrspace(200)* @cca, i32 0, i32 3), align 1, !tbaa !4
  ret i8 %0
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn writeonly
define dso_local void @storeInBounds() local_unnamed_addr addrspace(200) #2 {
entry:
  ; CHECK-LABEL: storeInBounds
  ; CHECK-NOT: csetbounds
  store i8 0, i8 addrspace(200)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(200)* @ca, i32 0, i32 0), align 1, !tbaa !4
  store i32 0, i32 addrspace(200)* getelementptr inbounds ([3 x i32], [3 x i32] addrspace(200)* @ia, i32 0, i32 0), align 4, !tbaa !7
  store i32 0, i32 addrspace(200)* getelementptr inbounds ([3 x i32], [3 x i32] addrspace(200)* @la, i32 0, i32 0), align 4, !tbaa !9
  store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* getelementptr inbounds ([3 x i8 addrspace(200)*], [3 x i8 addrspace(200)*] addrspace(200)* @pa, i32 0, i32 0), align 8, !tbaa !11
  store i8 0, i8 addrspace(200)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(200)* @ca, i32 0, i32 1), align 1, !tbaa !4
  store i32 0, i32 addrspace(200)* getelementptr inbounds ([3 x i32], [3 x i32] addrspace(200)* @ia, i32 0, i32 1), align 4, !tbaa !7
  store i32 0, i32 addrspace(200)* getelementptr inbounds ([3 x i32], [3 x i32] addrspace(200)* @la, i32 0, i32 1), align 4, !tbaa !9
  store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* getelementptr inbounds ([3 x i8 addrspace(200)*], [3 x i8 addrspace(200)*] addrspace(200)* @pa, i32 0, i32 1), align 8, !tbaa !11
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn writeonly
define dso_local void @storeOutOfBounds() local_unnamed_addr addrspace(200) #2 {
entry:
  ; CHECK-LABEL: storeOutOfBounds
  ; CHECK: csetbounds
  ; CHECK-SAME: (ca)
  ; CHECK: csetbounds
  ; CHECK-SAME: (ia)
  ; CHECK: csetbounds
  ; CHECK-SAME: (la)
  ; CHECK: csetbounds
  ; CHECK-SAME: (pa)
  store i8 0, i8 addrspace(200)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(200)* @ca, i32 1, i32 0), align 1, !tbaa !4
  store i32 0, i32 addrspace(200)* getelementptr inbounds ([3 x i32], [3 x i32] addrspace(200)* @ia, i32 1, i32 0), align 4, !tbaa !7
  store i32 0, i32 addrspace(200)* getelementptr inbounds ([3 x i32], [3 x i32] addrspace(200)* @la, i32 1, i32 0), align 4, !tbaa !9
  store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* getelementptr inbounds ([3 x i8 addrspace(200)*], [3 x i8 addrspace(200)*] addrspace(200)* @pa, i32 1, i32 0), align 8, !tbaa !11
  ret void
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind readnone willreturn "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }
attributes #2 = { mustprogress nofree norecurse nosync nounwind willreturn writeonly "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheriot" "target-features"="+relax,+xcheri,+xcheri-rvc,-64bit,-save-restore" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 1, !"SmallDataLimit", i32 8}
!3 = !{!"clang version 13.0.0 (ssh://git@github.com/ctsrd-cheri/llvm-project 2645d8a86d7f0cb9239052be3f9a3c366382b929)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !5, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"long", !5, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"any pointer", !5, i64 0}

