; RUN: %cheri_purecap_llc -O0 %s -o - | FileCheck %s -check-prefixes CHECK
; ModuleID = 'cmpcap.c'
source_filename = "cmpcap.c"
target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"

%struct.bigbuf = type { [5 x i8 addrspace(200)*] }

; Function Attrs: nounwind
define void @zero(%struct.bigbuf addrspace(200)* nocapture %out) local_unnamed_addr #0 {
entry:
  %.compoundliteral.sroa.0.0..sroa_cast1 = bitcast %struct.bigbuf addrspace(200)* %out to i8 addrspace(200)*
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 32 %.compoundliteral.sroa.0.0..sroa_cast1, i8 0, i64 31, i1 false)
; Check that the zero memset is expanded to capability stores and a final overlapping store.
; CHECK-LABEL: zero:
; CHECK-DAG: csc $cnull, $zero, 0($c3)
; CHECK-DAG:    csd $zero, $zero, 16($c3)
; Note: no unaligned store anymore since CHERI doesn't support it -> three stores instead
; CHECK-DAG:    csw $zero, $zero, 24($c3)
; CHECK-DAG:    csh $zero, $zero, 28($c3)
; CHECK-DAG:    csb $zero, $zero, 30($c3)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8, i64, i1) #1
