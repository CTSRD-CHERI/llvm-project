; RUNs: %cheri128_purecap_opt -data-layout="E-m:m-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200" -atomic-expand -S -cheri-cap-table-abi=pcrel %s -o - -debug-only=atomic-expand -print-before-all
; RUNs: %cheri128_purecap_opt -data-layout="E-m:m-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200" -atomic-expand -S -cheri-cap-table-abi=pcrel %s -o - -debug-only=atomic-expand -print-before-all
; RUsN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - -O0 -print-before=atomic-expand -print-after=atomic-expand -debug-only=atomic-expand
; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=pcrel %s -o - -O2 | FileCheck %s
; TODO: | FileCheck %s -enable-var-scope
; ModuleID = 'atomic.c'

@cap = common addrspace(200) global i32 addrspace(200)* null, align 32

declare void @test(i32 addrspace(200)* nocapture %cap, i1 %bool)

; CHECK-LABEL: cmpxchg_cap:
define i32 @cmpxchg_cap(i32 addrspace(200)* nocapture %exp, i32 addrspace(200)* %newval) {
entry:
; CHECK: clcbi   [[GLOBAL:\$c[0-9]+]], %captab20(cap)($c26)
; CHECK: [[BB0:(\$|\.L)[A-Z_0-9]+]]:
; CHECK: cllc    [[DST:\$c[0-9]+]], [[GLOBAL]]
; $c3 is the expected value (arg 1)
; CHECK: ceq     $1, [[DST]], $c3
; CHECK: beqz    $1, [[BB1:(\$|\.L)[A-Z_0-9]+]]
; try to store arg2 ($c4)
; CHECK: cscc    $1, $c4, [[GLOBAL]]
; CHECK: beqz    $1, [[BB0]]
; CHECK: [[BB1]]:
; CHECK-NEXT: ceq	$4, [[DST]], $c3
; CHECK-NEXT:	sync
; CHECK-NEXT:	clcbi	$c12, %capcall20(test)($c26)
; CHECK-NEXT:	cjalr	$c12, $c17
; CHECK-NEXT:	cmove	$c3,  [[DST]]
; CHECK: .end cmpxchg_cap
  %0 = cmpxchg i32 addrspace(200)* addrspace(200)* @cap, i32 addrspace(200)* %exp, i32 addrspace(200)* %newval seq_cst seq_cst
  %1 = extractvalue { i32 addrspace(200)*, i1 } %0, 0
  %2 = extractvalue { i32 addrspace(200)*, i1 } %0, 1
  call void @test(i32 addrspace(200)* nocapture %1, i1 %2)
  ret i32 42
}

define i32 addrspace(200)* @atomic_fetch_swap_cap(i32 addrspace(200)* %x) nounwind {
; CHECK-LABEL:   atomic_fetch_swap_cap:
; CHECK: clcbi   [[GLOBAL:\$c[0-9]+]], %captab20(cap)($c26)
; CHECK-NEXT: [[BB0:(\$|\.L)[A-Z_0-9]+]]:
; Load into $2 (return value)
; CHECK-NEXT: cllc    [[DST:\$c[0-9]+]], [[GLOBAL]]
; CHECK-NEXT: cscc    $1, $c3, [[GLOBAL]]
; CHECK-NEXT: beqz    $1, [[BB0]]
; CHECK-NEXT: nop
; CHECK:      cjr     $c17
; CHECK-NEXT: cmove   $c3, [[DST]]

  %t1 = atomicrmw xchg i32 addrspace(200)* addrspace(200)* @cap, i32 addrspace(200)* %x acquire
  ret i32 addrspace(200)* %t1
}
