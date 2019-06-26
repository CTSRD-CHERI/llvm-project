; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -o - -mxcaptable=true | %cheri_FileCheck %s -check-prefixes CHECK,BIG
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -o - -mxcaptable=false | %cheri_FileCheck %s -check-prefixes CHECK,SMALL
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/cap-table-call-extern.c"

@fn = common local_unnamed_addr addrspace(200) global void () addrspace(200)* null, align 32

; Function Attrs: nounwind
define void @test() local_unnamed_addr nounwind {
entry:
  %0 = load void () addrspace(200)*, void () addrspace(200)* addrspace(200)* @fn, align 32
  tail call void %0() #1
  ret void
  ; TODO: it would be nice if we could have function pointers inlined into the GOT
  ; CHECK:      csc	$c18, $zero, [[#CAP_SIZE]]($c11)
  ; CHECK-NEXT: csc	$c17, $zero, 0($c11)
  ; CHECK-NEXT: cmove	$c18, $c26
  ; BIG-NEXT:   lui	$1, %captab_hi(fn)
  ; BIG-NEXT:   daddiu	$1, $1, %captab_lo(fn)
  ; BIG-NEXT:   clc	$c1, $1, 0($c18)
  ; SMALL-NEXT: clcbi $c1, %captab20(fn)($c18)
  ; CHECK-NEXT: clc	$c12, $zero, 0($c1)
  ; CHECK-NEXT: cjalr	$c12, $c17
  ; CHECK-NEXT: nop
  ; CHECK-NEXT: cmove	$c26, $c18
  ; CHECK-NEXT: clc	$c17, $zero, 0($c11)
  ; CHECK-NEXT: clc	$c18, $zero, [[#CAP_SIZE]]($c11)
  ; CHECK-NEXT: cjr	$c17

  ; CHECK:        .type	fn,@object              # @fn
  ; FIXME: why is alignment always 32?
  ; CHECK-NEXT:   .comm	fn,[[#CAP_SIZE]],32
}
