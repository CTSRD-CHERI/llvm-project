; RUN: %cheri_purecap_llc -verify-machineinstrs -cheri-cap-table-abi=pcrel %s -o - -O2 | FileCheck %s

; The Cheri Addressing Mode Folder pass would previously turn this clw of NULL (which always traps with a tag violation)
; into a lw of zero (which is relative to $ddc and might not trap)

define i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i32, i32 addrspace(200)* null, align 536870912
  ret i32 %0
}

; This was previously turned into a MIPS lw which is not the same:
; CHECK-LABEL: main:
; CHECK:      cgetnull	$c1
; CHECK-NEXT: clw	$2, $zero, 0($c1)
; CHECK-NEXT: cjr	$c17
; CHECK-NEXT: nop
; CHECK:     .end main
