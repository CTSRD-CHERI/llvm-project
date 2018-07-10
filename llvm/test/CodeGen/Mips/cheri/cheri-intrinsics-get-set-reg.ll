; RUN: %cheri_llc %s -o - -verify-machineinstrs | FileCheck %s
; RUN: %cheri_purecap_llc %s -o - -verify-machineinstrs | FileCheck %s
; RUN: %cheri_llc %s -o - -filetype=obj | llvm-objdump -d - | FileCheck %s -check-prefix DUMP
; ModuleID = 'cheriintrinsic.c'
target datalayout = "E-p:64:64:64-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f128:128:128-v64:64:64-n32"
target triple = "cheri-unknown-freebsd"

define i8 addrspace(200)* @getddc() nounwind {
entry:
  ; CHECK-LABEL: getddc:
  ; CHECK: creadhwr $c3, $chwr_ddc
  ; CHECK: .end getddc
  %0 = call i8 addrspace(200)* @llvm.cheri.ddc.get()
  ret i8 addrspace(200)* %0
}
declare i8 addrspace(200)* @llvm.cheri.ddc.get()

define i8 addrspace(200)* @getpcc() nounwind {
entry:
  ; CHECK-LABEL: getpcc:
  ; CHECK: cgetpcc	$c3
  ; CHECK: .end getpcc
  %0 = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  ret i8 addrspace(200)* %0
}
declare i8 addrspace(200)* @llvm.cheri.pcc.get()


define i64 @getcause() nounwind {
entry:
  ; CHECK-LABEL: getcause:
  ; CHECK: cgetcause	$2
  ; CHECK: .end getcause
  %0 = call i64 @llvm.mips.cap.cause.get()
  ret i64 %0
}
declare i64 @llvm.mips.cap.cause.get()


define void @setcause() nounwind {
entry:
  ; CHECK-LABEL: setcause:
  ; CHECK: csetcause	$1
  ; CHECK: .end setcause

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
; CHECK: .end getidc

define i8 addrspace(200)* @getkr1c() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.kr1c.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getkr1c:
; CHECK: creadhwr $c3, $chwr_kr1c
; CHECK: .end getkr1c

define i8 addrspace(200)* @getkr2c() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.kr2c.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getkr2c:
; CHECK: creadhwr $c3, $chwr_kr2c
; CHECK: .end getkr2c

define i8 addrspace(200)* @getkcc() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.kcc.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getkcc:
; CHECK: creadhwr $c3, $chwr_kcc
; CHECK: .end getkcc

define i8 addrspace(200)* @getkdc() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.kdc.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getkdc:
; CHECK: creadhwr $c3, $chwr_kdc
; CHECK: .end getkdc

define i8 addrspace(200)* @getepcc() nounwind {
entry:
  %0 = call i8 addrspace(200)* @llvm.mips.epcc.get()
  ret i8 addrspace(200)* %0
}
; CHECK-LABEL: getepcc:
; CHECK: creadhwr $c3, $chwr_epcc
; CHECK: .end getepcc

declare i8 addrspace(200)* @llvm.mips.idc.get()
declare i8 addrspace(200)* @llvm.mips.kr1c.get()
declare i8 addrspace(200)* @llvm.mips.kr2c.get()
declare i8 addrspace(200)* @llvm.mips.kcc.get()
declare i8 addrspace(200)* @llvm.mips.kdc.get()
declare i8 addrspace(200)* @llvm.mips.epcc.get()


; DUMP-LABEL: getddc:
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       48 03 03 7f creadhwr $c3, $chwr_ddc

; DUMP-LABEL: getpcc:
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       48 03 07 ff 	cgetpcc	$c3

; DUMP-LABEL: getcause:
; DUMP-NEXT:       48 02 0f ff 	cgetcause	$2
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       00 00 00 00 	nop
; DUMP-NEXT:       00 00 00 00 	nop

; DUMP-LABEL: setcause:
; DUMP-NEXT:       64 01 00 01 	daddiu	$1, $zero, 1
; DUMP-NEXT:       48 01 17 ff 	csetcause	$1
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       00 00 00 00 	nop

; DUMP-LABEL: getidc:
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       48 03 d0 11 	cincoffset	$c3, $c26, $zero

; DUMP-LABEL: getkr1c:
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       48 03 b3 7f creadhwr $c3, $chwr_kr1c

; DUMP-LABEL: getkr2c:
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       48 03 bb 7f creadhwr $c3, $chwr_kr2c

; DUMP-LABEL: getkcc:
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       48 03 eb 7f creadhwr $c3, $chwr_kcc

; DUMP-LABEL: getkdc:
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       48 03 f3 7f creadhwr $c3, $chwr_kdc

; DUMP-LABEL: getepcc:
; DUMP-NEXT:       03 e0 00 08 	jr	$ra
; DUMP-NEXT:       48 03 fb 7f 	creadhwr	$c3, $chwr_epcc
