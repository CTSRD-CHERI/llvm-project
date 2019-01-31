; RUN: %cheri_purecap_llc -o - -O2 -verify-machineinstrs %s | %cheri_FileCheck %s
; Should work just the same without the addressing mode folder
; RUN: %cheri_purecap_llc -o - -O2 -verify-machineinstrs %s -disable-cheri-addressing-mode-folder | %cheri_FileCheck %s

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.bounds.set(i8 addrspace(200)*, i64) addrspace(200) #1
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) addrspace(200) #1
declare i8 addrspace(200)* @foo(i8 addrspace(200)*) addrspace(200)
declare i8 addrspace(200)* @bar(i64 addrspace(200)*) addrspace(200)


; Check that we can use the stack variable in a CSetOffset intrinsic
define i8 addrspace(200)* @setoffset(i64 %arg) addrspace(200) nounwind {
  %stack_var = alloca i8 addrspace(200)*, align 32, addrspace(200)
  %stack_var_i8 = bitcast i8 addrspace(200)* addrspace(200)* %stack_var to i8 addrspace(200)*
  %with_bounds = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* %stack_var_i8, i64 %arg)
  %result = tail call i8 addrspace(200)* @foo(i8 addrspace(200)* %with_bounds)
  ret i8 addrspace(200)* %result
; CHECK-LABEL: setoffset:
; CHECK:      cincoffset	$c2, $c11, 0
; CHECK-NEXT: csetbounds	$c2, $c2, [[$CAP_SIZE]]
; CHECK-NEXT: clcbi	$c12, %capcall20(foo)($c1)
; CHECK-NEXT: cjalr	$c12, $c17
; CHECK-NEXT: csetoffset	$c3, $c2, $4
}

; We should also be able to pass it to  another function
define i8 addrspace(200)* @pass_var(i64 %arg) addrspace(200) nounwind {
  %stack_var = alloca i64, align 8, addrspace(200)
  %result = tail call i8 addrspace(200)* @bar(i64 addrspace(200)* %stack_var)
  ret i8 addrspace(200)* %result
  ; CHECK-LABEL: pass_var
  ; CHECK:      cincoffset $c2, $c11, [[@EXPR $CAP_SIZE - 8]]
  ; CHECK-NEXT: clcbi $c12, %capcall20(bar)($c1)
  ; CHECK-NEXT: cjalr $c12, $c17
  ; CHECK-NEXT: csetbounds $c3, $c2, 8
}
