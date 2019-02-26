; RUN: %cheri_purecap_llc -O0 %s -o - | FileCheck %s -check-prefixes CHECK,%cheri_type
; ModuleID = 'cmpcap.c'
source_filename = "cmpcap.c"
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

%struct.bigbuf = type { [5 x i8 addrspace(200)*] }

; Function Attrs: nounwind
define void @zero(%struct.bigbuf addrspace(200)* nocapture %out) local_unnamed_addr #0 {
entry:
  %.compoundliteral.sroa.0.0..sroa_cast1 = bitcast %struct.bigbuf addrspace(200)* %out to i8 addrspace(200)*
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 32 %.compoundliteral.sroa.0.0..sroa_cast1, i8 0, i64 29, i1 false)
; Check that the zero memset is not expanded to capability stores.
; It would be nice if we could expand it to capability stores for the aligned
; bit and pad it with other stores, but currently SelectionDAG expands it to
; two overlapping stores, which is not allowed because capability stores must
; be aligned.  We can probably catch some variants of this with some custom
; logic.
; CHECK-LABEL: zero:
; CHERI128-DAG: csc $cnull, $zero, 0($c3)
; CHERI256-DAG: csd $zero, $zero, 0($c3)
; CHERI256-DAG: csd $zero, $zero, 8($c3)
; CHECK-DAG:    csd $zero, $zero, 16($c3)
; CHECK-DAG:    csw $zero, $zero, 24($c3)
; CHECK-DAG:    csb $zero, $zero, 28($c3)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8, i64, i1) #1
