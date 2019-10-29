; RUN: opt < %s -basicaa -gvn -S | FileCheck -check-prefix=CHECK-GVN %s
; RUN: opt < %s -basicaa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"

; Check that we don't propagate the value of 42 from the setbounds call.

;
;
; struct A { virtual void f(); int n; };
;
; int h() {
;     A a;
;     a.n = 42;
;     return __builtin_cheri_bounds_set(&a, 4)->n;
; }

%struct.A = type <{ i8*, i8 }>


declare i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)*, i64)
declare i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)*, i64)

define i8 @testBoundsSetAlias() {
; CHECK-LABEL: Function: testBoundsSetAlias: 5 pointers, 1 call sites
; CHECK-DAG:   MustAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %a.bitcast
; CHECK-DAG:   PartialAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %n
; CHECK-DAG:   NoAlias:	i8 addrspace(200)* %a.bitcast, i8 addrspace(200)* %n
; CHECK-DAG:   MayAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %a.laundered
; CHECK-DAG:   MayAlias:	i8 addrspace(200)* %a.bitcast, i8 addrspace(200)* %a.laundered
; CHECK-DAG:   MayAlias:	i8 addrspace(200)* %a.laundered, i8 addrspace(200)* %n
; CHECK-DAG:   MayAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %n.laundered
; CHECK-DAG:   NoAlias:	i8 addrspace(200)* %a.bitcast, i8 addrspace(200)* %n.laundered
; CHECK-DAG:   MayAlias:	i8 addrspace(200)* %n, i8 addrspace(200)* %n.laundered
; CHECK-DAG:   NoAlias:	i8 addrspace(200)* %a.laundered, i8 addrspace(200)* %n.laundered
; CHECK-DAG:   NoModRef:  Ptr: %struct.A addrspace(200)* %a	<->  %a.laundered = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* nonnull %a.bitcast, i64 4)
; CHECK-DAG:   NoModRef:  Ptr: i8 addrspace(200)* %a.bitcast	<->  %a.laundered = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* nonnull %a.bitcast, i64 4)
; CHECK-DAG:   NoModRef:  Ptr: i8 addrspace(200)* %n	<->  %a.laundered = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* nonnull %a.bitcast, i64 4)
; CHECK-DAG:   NoModRef:  Ptr: i8 addrspace(200)* %a.laundered	<->  %a.laundered = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* nonnull %a.bitcast, i64 4)
; CHECK-DAG:   NoModRef:  Ptr: i8 addrspace(200)* %n.laundered	<->  %a.laundered = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* nonnull %a.bitcast, i64 4)

entry:
  %a = alloca %struct.A, align 8, addrspace(200)
  %a.bitcast = bitcast %struct.A addrspace(200)* %a to i8 addrspace(200)*
  %n = getelementptr inbounds %struct.A, %struct.A addrspace(200)* %a, i64 0, i32 1
  store i8 42, i8 addrspace(200)* %n
  %a.laundered = call i8 addrspace(200)* @llvm.cheri.cap.bounds.set.i64(i8 addrspace(200)* nonnull %a.bitcast, i64 4)
  %n.laundered = getelementptr inbounds i8, i8 addrspace(200)* %a.laundered, i64 8
  %v = load i8, i8 addrspace(200)* %n.laundered
; make sure that the load from %n.laundered to %v is performed even though the actual pointer aliases!
; Due to the bounds shrinking performed by setbounds the value is not accessible!
; CHECK-GVN-LABEL: @testBoundsSetAlias(
; CHECK-GVN:  %v = load i8, i8 addrspace(200)* %n.laundered
; CHECK-GVN-NEXT:  ret i8 %v
  ret i8 %v
}

define i8 @testIncOffsetAlias() {
; CHECK-LABEL: Function: testIncOffsetAlias: 5 pointers, 0 call sites
; CHECK-DAG:     MustAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %a.bitcast
; CHECK-DAG:     PartialAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %n
; CHECK-DAG:     NoAlias:	i8 addrspace(200)* %a.bitcast, i8 addrspace(200)* %n
; CHECK-DAG:     PartialAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %a.laundered
; CHECK-DAG:     NoAlias:	i8 addrspace(200)* %a.bitcast, i8 addrspace(200)* %a.laundered
; CHECK-DAG:     NoAlias:	i8 addrspace(200)* %a.laundered, i8 addrspace(200)* %n
; CHECK-DAG:     NoAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %n.laundered
; CHECK-DAG:     NoAlias:	i8 addrspace(200)* %a.bitcast, i8 addrspace(200)* %n.laundered
; CHECK-DAG:     NoAlias:	i8 addrspace(200)* %n, i8 addrspace(200)* %n.laundered
; CHECK-DAG:     NoAlias:	i8 addrspace(200)* %a.laundered, i8 addrspace(200)* %n.laundered
entry:
  %a = alloca %struct.A, align 8, addrspace(200)
  %a.bitcast = bitcast %struct.A addrspace(200)* %a to i8 addrspace(200)*
  %n = getelementptr inbounds %struct.A, %struct.A addrspace(200)* %a, i64 0, i32 1
  store i8 42, i8 addrspace(200)* %n
  %a.laundered = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* nonnull %a.bitcast, i64 4)
  %n.laundered = getelementptr inbounds i8, i8 addrspace(200)* %a.laundered, i64 8
  %v = load i8, i8 addrspace(200)* %n.laundered
; Also check that incoffset does not alias (except when offset is zero)
; Due to the bounds shrinking performed by setbounds the value is not accessible!
; CHECK-GVN-LABEL: @testIncOffsetAlias(
; uninitialized load:
; CHECK-GVN:  ret i8 undef
  ret i8 %v
}

define i8 @testIncOffsetZeroAlias() {
; CHECK-LABEL: Function: testIncOffsetZeroAlias: 5 pointers, 0 call sites
; CHECK-DAG:  MustAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %a.bitcast
; CHECK-DAG:  PartialAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %n
; CHECK-DAG:  NoAlias:	i8 addrspace(200)* %a.bitcast, i8 addrspace(200)* %n
; CHECK-DAG:  MustAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %a.laundered
; CHECK-DAG:  MustAlias:	i8 addrspace(200)* %a.bitcast, i8 addrspace(200)* %a.laundered
; CHECK-DAG:  NoAlias:	i8 addrspace(200)* %a.laundered, i8 addrspace(200)* %n
; CHECK-DAG:  PartialAlias:	%struct.A addrspace(200)* %a, i8 addrspace(200)* %n.laundered
; CHECK-DAG:  NoAlias:	i8 addrspace(200)* %a.bitcast, i8 addrspace(200)* %n.laundered
; CHECK-DAG:  MustAlias:	i8 addrspace(200)* %n, i8 addrspace(200)* %n.laundered
; CHECK-DAG:  NoAlias:	i8 addrspace(200)* %a.laundered, i8 addrspace(200)* %n.laundered
entry:
  %a = alloca %struct.A, align 8, addrspace(200)
  %a.bitcast = bitcast %struct.A addrspace(200)* %a to i8 addrspace(200)*
  %n = getelementptr inbounds %struct.A, %struct.A addrspace(200)* %a, i64 0, i32 1
  store i8 42, i8 addrspace(200)* %n
  %a.laundered = call i8 addrspace(200)* @llvm.cheri.cap.offset.increment.i64(i8 addrspace(200)* nonnull %a.bitcast, i64 0)
  %n.laundered = getelementptr inbounds i8, i8 addrspace(200)* %a.laundered, i64 8
  %v = load i8, i8 addrspace(200)* %n.laundered
; Also check that incoffset does not alias (except when offset is zero)
; Due to the bounds shrinking performed by setbounds the value is not accessible!
; CHECK-GVN-LABEL: @testIncOffsetZeroAlias(
; CHECK-GVN-NOT:   load i8, i8 addrspace(200)* %n.laundered
; CHECK-GVN:       ret i8 42
  ret i8 %v
}
