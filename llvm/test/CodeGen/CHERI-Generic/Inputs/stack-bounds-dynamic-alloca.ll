; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -cheri-bound-allocas -o - -S %s | FileCheck %s
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -O0 %s -o - | FileCheck %s -check-prefix ASM
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -O2 %s -o - | FileCheck %s -check-prefix ASM-OPT

; reduced C test case:
; __builtin_va_list a;
; char *b;
; void c() {
;   while (__builtin_va_arg(a, char))
;     b = __builtin_alloca(sizeof(b));
;   d(b);
; }
target datalayout = "@PURECAP_DATALAYOUT@"

declare i32 @use_alloca(ptr addrspace(200)) local_unnamed_addr addrspace(200)

define i32 @alloca_in_entry(i1 %arg) local_unnamed_addr addrspace(200) nounwind {
entry:
  %alloca = alloca [16 x i8], align 16, addrspace(200)
  br i1 %arg, label %do_alloca, label %exit

do_alloca:                                        ; preds = %entry
  br label %use_alloca_no_bounds

use_alloca_no_bounds:                             ; preds = %do_alloca
  %ptr_plus_one = getelementptr i64, ptr addrspace(200) %alloca, i64 1
  store i64 1234, ptr addrspace(200) %ptr_plus_one, align 8
  br label %use_alloca_need_bounds

use_alloca_need_bounds:                           ; preds = %use_alloca_no_bounds
  %.sub.le = getelementptr inbounds [16 x i8], ptr addrspace(200) %alloca, i64 0, i64 0
  %call = call signext i32 @use_alloca(ptr addrspace(200) %.sub.le)
  br label %exit

exit:                                             ; preds = %use_alloca_need_bounds, %entry
  ret i32 123
}

define i32 @alloca_not_in_entry(i1 %arg) local_unnamed_addr addrspace(200) nounwind {
entry:
  br i1 %arg, label %do_alloca, label %exit

do_alloca:                                        ; preds = %entry
  %alloca = alloca [16 x i8], align 16, addrspace(200)
  br label %use_alloca_no_bounds

use_alloca_no_bounds:                             ; preds = %do_alloca
  %ptr_plus_one = getelementptr i64, ptr addrspace(200) %alloca, i64 1
  store i64 1234, ptr addrspace(200) %ptr_plus_one, align 8
  br label %use_alloca_need_bounds

use_alloca_need_bounds:                           ; preds = %use_alloca_no_bounds
  %.sub.le = getelementptr inbounds [16 x i8], ptr addrspace(200) %alloca, i64 0, i64 0
  %call = call signext i32 @use_alloca(ptr addrspace(200) %.sub.le)
  br label %exit

exit:                                             ; preds = %use_alloca_need_bounds, %entry
  ret i32 123
}

; The original reduced test case from libc/gen/exec.c
; We can't use llvm.cheri.bounded.stack.cap.i64 here, since that only works for static allocas:
define i32 @crash_reproducer(i1 %arg) local_unnamed_addr addrspace(200) nounwind {
entry:
  br i1 %arg, label %entry.while.end_crit_edge, label %while.body

entry.while.end_crit_edge:                        ; preds = %entry
  unreachable

while.body:                                       ; preds = %entry
  %0 = alloca [16 x i8], align 16, addrspace(200)
  br label %while.end.loopexit

while.end.loopexit:                               ; preds = %while.body
  %.sub.le = getelementptr inbounds [16 x i8], ptr addrspace(200) %0, i64 0, i64 0
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit
  %call = call signext i32 @use_alloca(ptr addrspace(200) %.sub.le)
  %result = add i32 %call, 1234
  ret i32 %result
}
