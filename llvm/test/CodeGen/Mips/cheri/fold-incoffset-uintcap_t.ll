; REQUIRES: clang
; RUN: %cheri_opt %s -S -O3 | FileCheck %s -check-prefix IR
; RUN: %cheri_opt %s -S -O3 | %cheri_llc -o - | FileCheck %s
target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128"
target triple = "cheri-unknown-freebsd"


; Compiled from this source code:
;
; void* __capability add_cap() {
;   char* __capability cap = (char* __capability)100;
;   return cap + 924;
; }
;
; __uintcap_t add_uintcap_t() {
;   __uintcap_t cap = (__uintcap_t)100;
;   return cap + 924;
; }

define i8 addrspace(200)* @add_cap() #0 {
entry:
  %cap = alloca i8 addrspace(200)*, align 16
  store i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 100), i8 addrspace(200)** %cap, align 16
  %0 = load i8 addrspace(200)*, i8 addrspace(200)** %cap, align 16
  %add.ptr = getelementptr inbounds i8, i8 addrspace(200)* %0, i64 924
  ret i8 addrspace(200)* %add.ptr
  ; This creates a potentially valid capability:
  ; IR-LABEL: @add_cap()
  ; IR:       ret i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1024)
  ; CHECK-LABEL: add_cap
  ; CHECK: daddiu $1, $zero, 1024
  ; CHECK: jr $ra
  ; CHECK: cincoffset $c3, $cnull, $1
}

define i8 addrspace(200)* @add_uintcap_t() #0 {
entry:
  %cap = alloca i8 addrspace(200)*, align 16
  %0 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* null, i64 100)
  store i8 addrspace(200)* %0, i8 addrspace(200)** %cap, align 16
  %1 = load i8 addrspace(200)*, i8 addrspace(200)** %cap, align 16
  %2 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* null, i64 924)
  %3 = call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %1)
  %4 = call i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)* %2)
  %add = add i64 %3, %4
  %5 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %1, i64 %add)
  ret i8 addrspace(200)* %5
  ; This always creates an untagged capability:
  ; IR-LABEL: @add_uintcap_t()
  ; IR: ret i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1024)

  ; CHECK-LABEL: add_uintcap_t
  ; CHECK: daddiu	$1, $zero, 1024
  ; CHECK: jr	$ra
  ; CHECK: cincoffset	$c3, $cnull, $1
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)*, i64) #1

; Function Attrs: nounwind readnone
declare i64 @llvm.cheri.cap.offset.get.i64(i8 addrspace(200)*) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
