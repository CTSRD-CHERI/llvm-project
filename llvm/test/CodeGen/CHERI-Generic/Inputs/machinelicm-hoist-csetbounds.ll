; Previously LLVM would hoist CSetBounds instructions out of if conditions/loops
; even if the source pointer could be NULL. On MIPS and RISC-V this results in a
; tag violation so we must ensure that the CSetBounds happens after the NULL check.

; Note: Opt correctly hoists the condition+csetbounds into a preheader, and LLC
; used to unconditionally hoist the csetbounds.
; RUN: opt -data-layout="@PURECAP_DATALAYOUT@" @PURECAP_HARDFLOAT_ARGS@ "-passes=default<O3>" -S < %s | FileCheck %s --check-prefix=HOIST-OPT
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -O3 < %s | FileCheck %s

; Generated from the following C code (with subobject bounds):
; struct foo {
;     int src;
;     int dst;
; };
;
; void call(int* src, int* dst);
;
; void hoist_csetbounds(int cond, struct foo* f) {
;     for (int i = 0; i < 100; i++) {
;         if (f) {
;             call(&f->src, &f->dst);
;         }
;     }
; }

%struct.foo = type { i32, i32 }
declare dso_local void @call(ptr addrspace(200), ptr addrspace(200)) local_unnamed_addr addrspace(200) nounwind
declare ptr addrspace(200) @llvm.cheri.cap.bounds.set.iCAPRANGE(ptr addrspace(200), iCAPRANGE) addrspace(200) nounwind readnone willreturn

define dso_local void @hoist_csetbounds(i32 signext %cond, ptr addrspace(200) %f) local_unnamed_addr addrspace(200) nounwind {
entry:
  %tobool = icmp eq ptr addrspace(200) %f, null
  %dst = getelementptr inbounds %struct.foo, ptr addrspace(200) %f, i64 0, i32 1
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %address.with.bounds = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.iCAPRANGE(ptr addrspace(200) nonnull %f, iCAPRANGE 4)
  %address.with.bounds1 = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.iCAPRANGE(ptr addrspace(200) nonnull %dst, iCAPRANGE 4)
  call void @call(ptr addrspace(200) %address.with.bounds, ptr addrspace(200) %address.with.bounds1)
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %inc = add nuw nsw i32 %i.06, 1
  %cmp = icmp ult i32 %i.06, 99
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}
