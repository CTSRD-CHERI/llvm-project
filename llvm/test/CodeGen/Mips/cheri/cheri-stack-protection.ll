; RUN: %cheri_llc -cheri-cfi -relocation-model=pic %s -o - | FileCheck %s

; CHECK-LABEL: bar
define i32 @bar(i32 signext %x) nounwind {
entry:
; Check that we're deriving a PCC-relative return capability and spilling it to the stack
; CHECK: cgetpcc	$c16
; CHECK: csetoffset	$c16, $c16, $ra
; CHECK: csc	$c16, $sp
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
; Check that there's a call in the middle
; CHECK: jalr	$25
  %call = call i32 @foo(i32 signext %0)
  %1 = load i32, i32* %x.addr, align 4
  %add = add nsw i32 %call, %1
; Check that we reload the return cap and jump to it
; CHECK: clc	$c16, $sp,
; CHECK: cjr	$c16
  ret i32 %add
}

declare i32 @foo(i32 signext) #1

; CHECK-LABEL: baz
define i32 @baz(i32 signext %x) nounwind {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
; Check that, if we've had no reason to spill $ra, we just use it for the
; return.
; CHECK: jr	$ra
  ret i32 %0
}
