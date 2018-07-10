; RUN: %cheri_purecap_llc -O0 %s -o -
; This was crashing after https://reviews.llvm.org/D40095 was merged
source_filename = "/Users/alex/cheri/llvm/test/CodeGen/Mips/cheri-stack-reduce.test.ll"
target datalayout = "E-m:e-pf200:256:256-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"

declare i32 @a(i32 addrspace(200)*)

define i32 @d(i64 %i) {
entry:
  %e = alloca i32, i64 %i, addrspace(200)
  %call = call i32 @a(i32 addrspace(200)* %e)
  ret i32 %call
}
