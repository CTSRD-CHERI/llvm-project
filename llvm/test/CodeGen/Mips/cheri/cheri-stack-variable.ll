; RUN: %cheri128_purecap_llc -O1 %s -o - | FileCheck %s --check-prefixes=CHECK,CHERI128
; RUN: %cheri256_purecap_llc -O1 %s -o - | FileCheck %s --check-prefixes=CHECK,CHERI256
; ModuleID = 'cheri-stack.c'
source_filename = "cheri-stack.c"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

; Function Attrs: nounwind
define void @dynamic_alloca(i32 signext %x) local_unnamed_addr #0 {
entry:
; CHECK: cincoffset	$c24, $c11, $zero
  %cmp4 = icmp sgt i32 %x, 0
  br i1 %cmp4, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %0 = zext i32 %x to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  ; Save the old $csp
  ; CHECK: cmove	$c[[SPSAVE:[0-9]+]], $c11
  ; Adjust $csp and allocate a new bounded stack object
  ; CHECK: cgetaddr	$[[SPADDR:[0-9]+]], $c11
  ; CHERI128-NEXT: dsubu	$[[SUBSPADDR:[0-9]+]], $[[SPADDR]], $[[SIZE:[0-9]+]]
  ; CHERI128-NEXT: crepresentablealignmentmask	$[[MASK:[0-9]+]], ${{[0-9]+}}
  ; CHERI128-NEXT: and	$[[NEWSPADDR:[0-9]+]], $[[SUBSPADDR]], $[[MASK]]
  ; CHERI256-NEXT: dsubu	$[[NEWSPADDR:[0-9]+]], $[[SPADDR]], $[[SIZE:[0-9]+]]
  ; CHECK-NEXT: csetaddr	$c[[TMPSP:[0-9]+]], $c11, $[[NEWSPADDR]]
  ; CHECK-NEXT: csetbounds [[BOUNDED_ALLOCA:\$c[0-9]+]], $c[[TMPSP]], $[[SIZE]]
  ; CHECK-NEXT: cmove	$c11, $c[[TMPSP]]
  ; TODO: this is not needed
  ; CHECK-NEXT: csetbounds $c{{[0-9]+}}, [[BOUNDED_ALLOCA]], ${{[0-9]+}}

  %i.05 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %1 = call i8 addrspace(200)* @llvm.stacksave.p200i8()
  %vla = alloca i32, i64 %0, align 4, addrspace(200)
  call void @use_arg(i32 addrspace(200)* nonnull %vla) #1
  call void @llvm.stackrestore.p200i8(i8 addrspace(200)* %1)
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %x
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
  ; Restore old $csp
  ; CHECK: cmove	$c11, $c[[SPSAVE]]
}

; Function Attrs: nounwind
declare i8 addrspace(200)* @llvm.stacksave.p200i8() #1

declare void @use_arg(i32 addrspace(200)*) #2

; Function Attrs: nounwind
declare void @llvm.stackrestore.p200i8(i8 addrspace(200)*) #1

attributes #0 = { nounwind  }
attributes #1 = { nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 5.0.0 "}
