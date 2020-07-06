; RUN: %cheri_purecap_llc -filetype=obj %s -o -
; RUNs: %cheri_purecap_llc -filetype=obj %s -o - | llvm-objdump -t -
; RUN: llc -target-abi l64pc128d -mattr=+cap-mode -mtriple=riscv64-unknown-freebsd -mattr=+xcheri -verify-machineinstrs %s -o -
; RUN: llc -target-abi l64pc128d -mattr=+cap-mode -mtriple=riscv64-unknown-freebsd -mattr=+xcheri -verify-machineinstrs %s -filetype=obj -o %t.o
; RUN: llc -mtriple=x86_64-unknown-freebsd -verify-machineinstrs %s -filetype=obj -o - | llvm-objdump -t -

; ModuleID = 'jemalloc_rtree.c'
;source_filename = "jemalloc_rtree.c"
;target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128-A200-P200-G200"
;target triple = "riscv64-unknown-freebsd13.0"

define internal void @empty_fn_1() addrspace(200) norecurse noreturn nounwind readnone {
entry:
  unreachable
}

define internal void @empty_fn_2() addrspace(200) nounwind {
entry:
  unreachable
}
