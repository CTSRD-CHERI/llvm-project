; RUN: %cheri_purecap_llc -o - -cheri-cap-table-abi=plt -mxcaptable=false %s | FileCheck %s
; ModuleID = '/Users/alex/cheri/llvm/localtime-87e7dc-bugpoint-reduce.ll-reduced-simplified.bc'
source_filename = "localtime-87e7dc-bugpoint-reduce.ll-output-bc81d31.bc"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

%struct.state = type { i32, i32, i32, i32, i32, i32, [1200 x i64], [1200 x i8], [512 x i8]}

@lclmem = external addrspace(200) global %struct.state, align 8
@gmtmem = external addrspace(200) global %struct.state, align 8

define %struct.state addrspace(200)* @standard_load(i1 %arg) #0 {
  ; CHECK-LABEL: .ent standard_load
  ; CHECK: clcbi $c3, %captab20(lclmem)($c26)
  ; CHECK-NEXT: cjr $c17
  ret %struct.state addrspace(200)* @lclmem
}

; Function Attrs: noinline nounwind optnone
define %struct.state addrspace(200)* @ternary(i1 %arg) #0 {
  %cond = select i1 %arg, %struct.state addrspace(200)* @lclmem, %struct.state addrspace(200)* @gmtmem
  ret %struct.state addrspace(200)* %cond
  ; XXXAR: This is very inefficient but at least it looks like it will generate working code
  ; CHECK-LABEL: .ent ternary
  ; CHECK: sll	$1, $4, 0
  ; CHECK-NEXT: andi	$1, $1, 1
  ; CHECK-NEXT: daddiu	$2, $zero, %captab20(gmtmem)
  ; CHECK-NEXT: dsll	$2, $2, 4
  ; CHECK-NEXT: cincoffset	$c1, $c26, $2
  ; CHECK-NEXT: daddiu	$2, $zero, %captab20(lclmem)
  ; CHECK-NEXT: dsll	$2, $2, 4
  ; CHECK-NEXT: cincoffset	$c2, $c26, $2
  ; CHECK-NEXT: cmovn	$c1, $c2, $1
  ; CHECK-NEXT: clc	$c3, $zero, 0($c1)
  ; CHECK-NEXT: cjr	$c17
  ; CHECK-NEXT: nop
}

attributes #0 = { noinline nounwind optnone }
