; RUN: %cheri_purecap_llc %s -o - | FileCheck %s

@ext_array = external addrspace(200) global [0 x i8], align 1
; CHECK: some_var:
; CHECK-NEXT: .8byte ext_array{{$}}
@some_var = addrspace(200) global i64 ptrtoint ([0 x i8] addrspace(200)* @ext_array to i64), align 8




