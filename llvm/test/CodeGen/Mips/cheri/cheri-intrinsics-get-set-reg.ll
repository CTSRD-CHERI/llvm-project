; RUN: %cheri_llc %s -o - | FileCheck %s
; RUN: %cheri_llc %s -o - -filetype=obj | llvm-objdump -d -
; RUN: %cheri_llc %s -o - -filetype=obj | llvm-objdump -d - | FileCheck %s -check-prefix DUMP
; ModuleID = 'cheriintrinsic.c'
target datalayout = "E-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f128:128:128-v64:64:64-n32"
target triple = "cheri-unknown-freebsd"

define i8 addrspace(200)* @getddc() nounwind {
entry:
  ; CHECK-LABEL: getddc:
  ; CHECK: cgetdefault	$c3
  %0 = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  ret i8 addrspace(200)* %0
}
declare i8 addrspace(200)* @llvm.cheri.ddc.get()

define i8 addrspace(200)* @getstack() nounwind {
entry:
  ; CHECK-LABEL: getstack:
  ; CHECK: cincoffset	$c3, $c11, $zero
  %0 = call i8 addrspace(200)* @llvm.cheri.stack.cap.get()
  ret i8 addrspace(200)* %0
}
declare i8 addrspace(200)* @llvm.cheri.stack.cap.get()


define i8 addrspace(200)* @getpcc() nounwind {
entry:
  ; CHECK-LABEL: getpcc:
  ; CHECK: cgetpcc	$c3
  %0 = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  ret i8 addrspace(200)* %0
}
declare i8 addrspace(200)* @llvm.cheri.pcc.get()


define i64 @getcause() nounwind {
entry:
  ; CHECK-LABEL: getcause:
  ; CHECK: cgetcause	$2
  %0 = call i64 @llvm.mips.cap.cause.get()
  ret i64 %0
}
declare i64 @llvm.mips.cap.cause.get()


define void @setcause() nounwind {
entry:
  ; CHECK-LABEL: setcause:
  ; CHECK: csetcause	$1
  call void @llvm.mips.cap.cause.set(i64 1)
  ret void
}
declare void @llvm.mips.cap.cause.set(i64)


; Now get the special registers (currently this reads GPRs but that will change soon)

define i8 addrspace(200)* @getidc() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.idc.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getidc:
; CHECK: cincoffset	$c3, $c26, $zero

define i8 addrspace(200)* @getkr1c() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.kr1c.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getkr1c:
; CHECK: cgetkr1c	$c3

define i8 addrspace(200)* @getkr2c() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.kr2c.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getkr2c:
; CHECK: cgetkr2c	$c3

define i8 addrspace(200)* @getkcc() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.kcc.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getkcc:
; CHECK: cgetkcc	$c3

define i8 addrspace(200)* @getkdc() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.kdc.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getkdc:
; CHECK: cgetkdc	$c3

define i8 addrspace(200)* @getepcc() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.epcc.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getepcc:
; CHECK: creadhwr $c3, $chwr_epcc

declare i8 addrspace(200)* @llvm.mips.idc.get()
declare i8 addrspace(200)* @llvm.mips.kr1c.get()
declare i8 addrspace(200)* @llvm.mips.kr2c.get()
declare i8 addrspace(200)* @llvm.mips.kcc.get()
declare i8 addrspace(200)* @llvm.mips.kdc.get()
declare i8 addrspace(200)* @llvm.mips.epcc.get()


; DUMP-LABEL: getddc:
; DUMP-NEXT:        0:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:        4:	48 03 00 11 	cgetdefault	$c3

; DUMP-LABEL: getstack:
; DUMP-NEXT:        8:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:        c:	48 03 58 11 	cincoffset	$c3, $c11, $zero

; DUMP-LABEL: getpcc:
; DUMP-NEXT:       10:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:       14:	48 03 07 ff 	cgetpcc	$c3

; DUMP-LABEL: getcause:
; DUMP-NEXT:       18:	48 02 0f ff 	cgetcause	$2
; DUMP-NEXT:       1c:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:       20:	00 00 00 00 	nop
; DUMP-NEXT:       24:	00 00 00 00 	nop

; DUMP-LABEL: setcause:
; DUMP-NEXT:       28:	64 01 00 01 	daddiu	$1, $zero, 1
; DUMP-NEXT:       2c:	48 01 17 ff 	csetcause	$1
; DUMP-NEXT:       30:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:       34:	00 00 00 00 	nop

; DUMP-LABEL: getidc:
; DUMP-NEXT:       38:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:       3c:	48 03 d0 11 	cincoffset	$c3, $c26, $zero

; DUMP-LABEL: getkr1c:
; DUMP-NEXT:       40:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:       44:	48 03 d8 11 	cgetkr1c	$c3

; DUMP-LABEL: getkr2c:
; DUMP-NEXT:       48:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:       4c:	48 03 e0 11 	cgetkr2c	$c3

; DUMP-LABEL: getkcc:
; DUMP-NEXT:       50:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:       54:	48 03 e8 11 	cgetkcc	$c3

; DUMP-LABEL: getkdc:
; DUMP-NEXT:       58:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:       5c:	48 03 f0 11 	cgetkdc	$c3

; DUMP-LABEL: getepcc:
; DUMP-NEXT:       60:	03 e0 00 08 	jr	$ra
; DUMP-NEXT:       64:	48 03 fb 7f 	creadhwr	$c3, $chwr_epcc
