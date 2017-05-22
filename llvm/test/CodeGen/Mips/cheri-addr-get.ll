; TODO: would be nice to run this test on the IR instead of the generated ASM
; RUN: %cheri_llc -O3 %s -filetype=asm -o - | FileCheck %s -check-prefix OPT
; RUN: %cheri_llc -O0 %s -filetype=asm -o - | FileCheck %s -check-prefix NOOPT
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"
target triple = "cheri-unknown-freebsd"

declare i64 @check_fold(i64) #0
declare i64 @llvm.cheri.cap.address.get(i8 addrspace(200)*) #1
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #1

define i64 @null_get_vaddr() #1 {
  %ret = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* null)
  %ret_check = tail call i64 @check_fold(i64 %ret)
  ret i64 %ret
  ; XXXAR: The backend will remove the unnecessary add, maybe running the passes in a different order will remove it?
  ; IROPT-LABEL: @null_get_vaddr()
  ; IROPT-NEXT: %1 = add i64 0, 0
  ; IROPT-NEXT: ret i64 %1

  ; OPT-LABEL: null_get_vaddr: # @null_get_vaddr
  ; OPT:      jal check_fold
  ; OPT-NEXT: daddiu  $4, $zero, 0

  ; NOOPT-LABEL: null_get_vaddr: # @null_get_vaddr
  ; NOOPT: cfromptr        $c1, $c0, $zero
  ; NOOPT: cgetbase $2, $c1
  ; NOOPT: cgetoffset      $3, $c1
  ; NOOPT: daddu   $2, $2, $3


}

define void @infer_values_from_null_set_offset() #1 {
  %with_offset = tail call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 50)
  %vaddr = tail call i64 @llvm.cheri.cap.address.get(i8 addrspace(200)* %with_offset)
  %vaddr_check = tail call i64 @check_fold(i64 %vaddr)
  ret void
  ; IROPT-LABEL: @infer_values_from_null_set_offset()
  ; IROPT-NEXT: %1 = add i64 0, 50
  ; IROPT-NEXT: %vaddr_check = tail call i64 @check_fold(i64 %1)
  ; IROPT-NEXT: ret void

  ; OPT-LABEL: infer_values_from_null_set_offset: # @infer_values_from_null_set_offset
  ; OPT:      jal     check_fold
  ; OPT-NEXT: daddiu  $4, $zero, 50

  ; NOOPT-LABEL: infer_values_from_null_set_offset: # @infer_values_from_null_set_offset
  ; NOOPT: cfromptr        $c1, $c0, $zero
  ; NOOPT: daddiu  $2, $zero, 50
  ; NOOPT: csetoffset      $c1, $c1, $2
  ; NOOPT: cgetbase $2, $c1
  ; NOOPT: cgetoffset      $3, $c1
  ; NOOPT: daddu   $4, $2, $3
}


attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
