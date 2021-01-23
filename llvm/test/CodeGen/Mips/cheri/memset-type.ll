; RUN: %cheri_purecap_llc -O1 %s -o - | FileCheck %s -check-prefixes CHECK,CHERI128
; RUN: llc -mtriple=mips64-unknown-freebsd -mattr=+cheri256 -mcpu=cheri256 -target-abi purecap -O1 %s -o - | FileCheck %s -check-prefixes CHECK,CHERI256

declare void @llvm.memset.p200i8.i64(i8 addrspace(200)* nocapture writeonly, i8, i64, i1) addrspace(200)

define void @align4(i8 addrspace(200)* nocapture %out) addrspace(200) {
entry:
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 4 %out, i8 0, i64 36, i1 false)
; Check that the zero memset is expanded to stores.
; CHECK-LABEL: align4:
; Cannot do 36 bytes with only a few stores
; CHECK: clcbi	$c12, %capcall20(memset)
  ret void
}

define void @align8(i8 addrspace(200)* nocapture %out) addrspace(200) {
entry:
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 8 %out, i8 0, i64 36, i1 false)
; Check that the zero memset is expanded to stores.
; CHECK-LABEL: align8:
; CHECK-DAG: csd $zero, $zero, 0($c3)
; CHECK-DAG: csd $zero, $zero, 8($c3)
; CHECK-DAG: csd $zero, $zero, 16($c3)
; CHECK-DAG: csd $zero, $zero, 24($c3)
; CHECK-DAG: csw $zero, $zero, 32($c3)
  ret void
}

define void @align16(i8 addrspace(200)* nocapture %out) addrspace(200) {
entry:
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 16 %out, i8 0, i64 36, i1 false)
; Check that the zero memset is expanded to csc + a single csw for 128 and csd for 256
; CHECK-LABEL: align16:
; CHERI128-DAG: csc $cnull, $zero, 0($c3)
; CHERI128-DAG: csc $cnull, $zero, 16($c3)
; CHERI256-DAG: csd $zero, $zero, 0($c3)
; CHERI256-DAG: csd $zero, $zero, 8($c3)
; CHERI256-DAG: csd $zero, $zero, 16($c3)
; CHERI256-DAG: csd $zero, $zero, 24($c3)
; CHECK-DAG:    csw $zero, $zero, 32($c3)
  ret void
}

define void @align32(i8 addrspace(200)* nocapture %out) addrspace(200) {
entry:
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 32 %out, i8 0, i64 36, i1 false)
; Check that the zero memset is expanded to csc (x2 for Cheri128) + a single csw
; CHECK-LABEL: align32:
; CHECK-DAG:    csc $cnull, $zero, 0($c3)
; CHERI128-DAG: csc $cnull, $zero, 16($c3)
; CHECK-DAG:    csw $zero, $zero, 32($c3)
  ret void
}

define void @align64(i8 addrspace(200)* nocapture %out) addrspace(200) {
entry:
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 64 %out, i8 0, i64 36, i1 false)
; Check that the zero memset is expanded to csc (x2 for Cheri128) + a single csw
; CHECK-LABEL: align64:
; CHECK-DAG:    csc $cnull, $zero, 0($c3)
; CHERI128-DAG: csc $cnull, $zero, 16($c3)
; CHECK-DAG:    csw $zero, $zero, 32($c3)
  ret void
}

; should just use csc here (even though align is more than 64
define void @align64_multiple_of_capsize(i8 addrspace(200)* nocapture %out) addrspace(200) {
entry:
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 64 %out, i8 0, i64 64, i1 false)
; Check that the zero memset is expanded to csc
; CHECK-LABEL: align64_multiple_of_capsize:
; CHECK-DAG:    csc $cnull, $zero, 0($c3)
; CHERI128-DAG: csc $cnull, $zero, 16($c3)
; CHECK-DAG:    csc $cnull, $zero, 32($c3)
; CHERI128-DAG: csc $cnull, $zero, 48($c3)
  ret void
}

; Can't use csc here since generating the constant is expensive
define void @align64_nonzero_value(i8 addrspace(200)* nocapture %out) addrspace(200) {
entry:
  call void @llvm.memset.p200i8.i64(i8 addrspace(200)* align 64 %out, i8 1, i64 36, i1 false)
; Check that the non-zero memset is expanded to stores.
; CHECK-LABEL: align64_nonzero_value:
; generate and use 32-bit constant:
; CHECK:      lui	$1, 257
; CHECK-NEXT: ori	$1, $1, 257
; CHECK-NEXT: csw $1, $zero, 32($c3)
; generate the 64-bit constant:
; CHECK-NEXT: lui	$1, 257
; CHECK-NEXT: daddiu	$1, $1, 257
; CHECK-NEXT: dsll	$1, $1, 16
; CHECK-NEXT: daddiu	$1, $1, 257
; CHECK-NEXT: dsll	$1, $1, 16
; CHECK-NEXT: daddiu	$1, $1, 257
; CHECK-DAG:  csd $1, $zero, 0($c3)
; CHECK-DAG:  csd $1, $zero, 8($c3)
; CHECK-DAG:  csd $1, $zero, 16($c3)
; CHECK-DAG:  csd $1, $zero, 24($c3)
  ret void
}
