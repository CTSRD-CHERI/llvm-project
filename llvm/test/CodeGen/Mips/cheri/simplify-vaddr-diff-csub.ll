; RUN: %cheri_purecap_opt -instcombine -S %s -o %t.ll
; RUN: FileCheck -input-file %t.ll -check-prefix INSTCOMBINE %s
; RUN: %cheri_purecap_llc -O2 %t.ll -o - | FileCheck %s

target datalayout = "A200-P200-G200"


declare i64 @llvm.cheri.cap.address.get(i8 addrspace(200)*) addrspace(200)

; Function Attrs: noinline nounwind optnone
; C source code: return ((vaddr_t)first - (vaddr_t)second) / 16;
define i64 @cap_subtract_with_vaddr_cast(i8 addrspace(200)* %first, i8 addrspace(200)* %second) addrspace(200) {
entry:
  %0 = call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* %first)
  %1 = call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* %second)
  %sub = sub i64 %0, %1
  %div = udiv i64 %sub, 16
  ret i64 %div
}

; INSTCOMBINE:      define i64 @cap_subtract_with_vaddr_cast(i8 addrspace(200)* %first, i8 addrspace(200)* %second) addrspace(200) #1 {
; INSTCOMBINE-NEXT: entry:
; INSTCOMBINE-NEXT:  %sub = call addrspace(200) i64 @llvm.cheri.cap.diff(i8 addrspace(200)* %first, i8 addrspace(200)* %second)
; INSTCOMBINE-NEXT:  %div = lshr i64 %sub, 4
; INSTCOMBINE-NEXT:  ret i64 %div
; INSTCOMBINE-NEXT:}


; CHECK: csub	$1, $c3, $c4
; CHECK: cjr	$c17
; CHECK: dsrl	$2, $1, 4
