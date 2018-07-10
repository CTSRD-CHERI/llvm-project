; RUN: %cheri_purecap_llc %s -mxgot -cheri-cap-table-abi=plt -o - -O3  | FileCheck %s
; The optimze libcalls would previously replace constant strings with AS0 strings
; Seems like this can't be tested in LLVM, but let's still check that this code is sensible
; ModuleID = '/Users/alex/cheri/llvm/tools/clang/test/CodeGen/cap-table-printf.c'
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/cap-table-printf.c"

@.str = private unnamed_addr addrspace(200) constant [33 x i8] c" res_nquery retry without EDNS0\0A\00", align 1

define void @a() {
entry:
  %call = call i32 (i8 addrspace(200)*, ...) @printf(i8 addrspace(200)* getelementptr inbounds ([33 x i8], [33 x i8] addrspace(200)* @.str, i32 0, i32 0))
  ret void

  ; CHECK:      clcbi $c3, %captab20(.L.str)($c26)
  ; CHECK-NEXT: clcbi $c12, %capcall20(printf)($c26)
  ; CHECK-NEXT: cjalr	$c12, $c17
  ; CHECK-NEXT: nop
}

declare i32 @printf(i8 addrspace(200)*, ...)
