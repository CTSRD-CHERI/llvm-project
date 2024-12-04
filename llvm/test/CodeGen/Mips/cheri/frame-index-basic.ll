; RUN: %cheri_purecap_llc -o - -O2 -verify-machineinstrs %s | %cheri_FileCheck %s
; Should work just the same without the addressing mode folder
; RUN: %cheri_purecap_llc -o - -O2 -verify-machineinstrs %s -disable-cheri-addressing-mode-folder | %cheri_FileCheck %s

declare ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200), i64) addrspace(200)
declare ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200), i64) addrspace(200)
declare ptr addrspace(200) @foo(ptr addrspace(200)) addrspace(200)
declare ptr addrspace(200) @bar(ptr addrspace(200)) addrspace(200)

; Check that we can use the stack variable in a CSetOffset intrinsic
define ptr addrspace(200) @setoffset(i64 %arg) addrspace(200) nounwind {
  %stack_var = alloca ptr addrspace(200), align 32, addrspace(200)
  %stack_var_i8 = bitcast ptr addrspace(200) %stack_var to ptr addrspace(200)
  %with_bounds = tail call ptr addrspace(200) @llvm.cheri.cap.offset.set.i64(ptr addrspace(200) %stack_var_i8, i64 %arg)
  %result = tail call ptr addrspace(200) @foo(ptr addrspace(200) %with_bounds)
  ret ptr addrspace(200) %result
; CHECK-LABEL: setoffset:
; CHECK:      cincoffset $c11, $c11, -{{64|128}}
; CHECK:      lui $1, %pcrel_hi(_CHERI_CAPABILITY_TABLE_-8)
; CHECK-NEXT: daddiu $1, $1, %pcrel_lo(_CHERI_CAPABILITY_TABLE_-4)
; CHECK-NEXT: cgetpccincoffset $c1, $1
; After adding the stack-to-bounded-cap pseudo the cincoffset 0 should no longer be needed
; CHECK-NOT:  cincoffset $c2
; CHECK-NEXT: csetbounds	$c2, $c11, [[#CAP_SIZE]]
; CHECK-NEXT: clcbi	$c12, %capcall20(foo)($c1)
; CHECK-NEXT: cjalr	$c12, $c17
; CHECK-NEXT: csetoffset	$c3, $c2, $4
}

; We should also be able to pass it to  another function
define ptr addrspace(200) @pass_var(i64 %arg) addrspace(200) nounwind {
  %stack_var = alloca i64, align 8, addrspace(200)
  %result = tail call ptr addrspace(200) @bar(ptr addrspace(200) %stack_var)
  ret ptr addrspace(200) %result
  ; CHECK-LABEL: pass_var:
  ; CHECK:      cincoffset $c11, $c11, -{{32|64}}
  ; CHECK:      clcbi $c12, %capcall20(bar)($c1)
  ; CHECK-NEXT: cincoffset [[STACK_CAP:\$c[0-9]]], $c11, [[#CAP_SIZE - 8]]
  ; CHECK-NEXT: cjalr $c12, $c17
  ; CHECK-NEXT: csetbounds $c3, [[STACK_CAP]], 8
}
