; REQUIRES: asserts
; Check that we can hoist the csetbounds for a local alloca outside of loops
; We know that it's always tagged and unsealed so machinelicm should be able to
; to hoist the csetbounds instructions.
; TODO: for MIPS "simple-register-coalescing" moves the CheriBoundedStackPseudoImm back into the loop.
; In general this will be faster than loading from the stack, but it's probably worse
; than using a callee-saved register for loops with many iterations.

; Generated from this code:
; void call(int *src, int *dst);
;
; void hoist_alloca_uncond(int cond) {
;   int buf1[123];
;   int buf2[22];
;   for (int i = 0; i < 100; i++) {
;     call(buf1, buf2);
;   }
; }
;
; void hoist_alloca_cond(int cond) {
;   int buf1[123];
;   int buf2[22];
;   for (int i = 0; i < 100; i++) {
;     if (cond) {
;       call(buf1, buf2);
;     }
;   }
; }

; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -o %t.mir -stop-before=early-machinelicm < %s
; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -run-pass=early-machinelicm -debug-only=machinelicm %t.mir -o /dev/null 2>%t.dbg
; RUN: FileCheck --input-file=%t.dbg --check-prefix=MACHINELICM-DBG %s
; Check that MachineLICM hoists the CheriBoundedStackPseudoImm (MIPS) / IncOffset+SetBoundsImm (RISCV) instructions
; MACHINELICM-DBG-LABEL: ******** Pre-regalloc Machine LICM: hoist_alloca_uncond
@IF-MIPS@; MACHINELICM-DBG: Hoisting %{{[0-9]+}}:cherigpr = CheriBoundedStackPseudoImm %stack.0.buf1, 0, 492
@IF-RISCV@; MACHINELICM-DBG: Hoisting [[INC:%[0-9]+]]:gpcr = CIncOffsetImm %stack.0.buf1, 0
; MACHINELICM-DBG-NEXT:  from %bb.2 to %bb.0
@IF-RISCV32@; MACHINELICM-DBG: Hoisting [[BOUNDS:%[0-9]+]]:gpcr = CSetBoundsImm [[INC]]:gpcr, 512
@IF-RISCV64@; MACHINELICM-DBG: Hoisting [[BOUNDS:%[0-9]+]]:gpcr = CSetBoundsImm [[INC]]:gpcr, 492
@IF-RISCV@; MACHINELICM-DBG-NEXT:  from %bb.2 to %bb.0
@IF-MIPS@; MACHINELICM-DBG: Hoisting %{{[0-9]+}}:cherigpr = CheriBoundedStackPseudoImm %stack.1.buf2, 0, 88
@IF-RISCV@; MACHINELICM-DBG: Hoisting [[INC:%[0-9]+]]:gpcr = CIncOffsetImm %stack.1.buf2, 0
; MACHINELICM-DBG-NEXT:  from %bb.2 to %bb.0
@IF-RISCV@; MACHINELICM-DBG: Hoisting [[BOUNDS:%[0-9]+]]:gpcr = CSetBoundsImm [[INC]]:gpcr, 88
@IF-RISCV@; MACHINELICM-DBG-NEXT:  from %bb.2 to %bb.0
; MACHINELICM-DBG-LABEL: ******** Pre-regalloc Machine LICM: hoist_alloca_cond
@IF-MIPS@; MACHINELICM-DBG: Hoisting %{{[0-9]+}}:cherigpr = CheriBoundedStackPseudoImm %stack.0.buf1, 0, 492
@IF-RISCV@; MACHINELICM-DBG: Hoisting [[INC:%[0-9]+]]:gpcr = CIncOffsetImm %stack.0.buf1, 0
; MACHINELICM-DBG-NEXT:  from %bb.3 to %bb.0
@IF-RISCV32@; MACHINELICM-DBG: Hoisting [[BOUNDS:%[0-9]+]]:gpcr = CSetBoundsImm [[INC]]:gpcr, 512
@IF-RISCV64@; MACHINELICM-DBG: Hoisting [[BOUNDS:%[0-9]+]]:gpcr = CSetBoundsImm [[INC]]:gpcr, 492
@IF-RISCV@; MACHINELICM-DBG-NEXT:  from %bb.3 to %bb.0
@IF-MIPS@; MACHINELICM-DBG: Hoisting %{{[0-9]+}}:cherigpr = CheriBoundedStackPseudoImm %stack.1.buf2, 0, 88
@IF-RISCV@; MACHINELICM-DBG: Hoisting [[INC:%[0-9]+]]:gpcr = CIncOffsetImm %stack.1.buf2, 0
; MACHINELICM-DBG-NEXT:  from %bb.3 to %bb.0
@IF-RISCV@; MACHINELICM-DBG: Hoisting [[BOUNDS:%[0-9]+]]:gpcr = CSetBoundsImm [[INC]]:gpcr, 88
@IF-RISCV@; MACHINELICM-DBG-NEXT:  from %bb.3 to %bb.0

; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -O1 -o - < %s | FileCheck %s

define void @hoist_alloca_uncond(i32 signext %cond) local_unnamed_addr addrspace(200) nounwind {
entry:
  %buf1 = alloca [123 x i32], align 4, addrspace(200)
  %buf2 = alloca [22 x i32], align 4, addrspace(200)
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.04 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arraydecay = getelementptr inbounds [123 x i32], [123 x i32] addrspace(200)* %buf1, i64 0, i64 0
  %arraydecay1 = getelementptr inbounds [22 x i32], [22 x i32] addrspace(200)* %buf2, i64 0, i64 0
  call void @call(i32 addrspace(200)* nonnull %arraydecay, i32 addrspace(200)* nonnull %arraydecay1)
  %inc = add nuw nsw i32 %i.04, 1
  %exitcond.not = icmp eq i32 %inc, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

declare void @call(i32 addrspace(200)*, i32 addrspace(200)*) local_unnamed_addr addrspace(200) nounwind

define void @hoist_alloca_cond(i32 signext %cond) local_unnamed_addr addrspace(200) nounwind {
entry:
  %buf1 = alloca [123 x i32], align 4, addrspace(200)
  %buf2 = alloca [22 x i32], align 4, addrspace(200)
  %tobool.not = icmp eq i32 %cond, 0
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %i.04 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arraydecay = getelementptr inbounds [123 x i32], [123 x i32] addrspace(200)* %buf1, i64 0, i64 0
  %arraydecay1 = getelementptr inbounds [22 x i32], [22 x i32] addrspace(200)* %buf2, i64 0, i64 0
  call void @call(i32 addrspace(200)* nonnull %arraydecay, i32 addrspace(200)* nonnull %arraydecay1)
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw nsw i32 %i.04, 1
  %exitcond.not = icmp eq i32 %inc, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}
