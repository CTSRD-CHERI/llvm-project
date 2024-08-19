; RUN: %riscv64_cheri_purecap_llc -stop-after=finalize-isel %s -o - | FileCheck %s

declare ptr addrspace(200) @llvm.ptrmask.p200.i64(ptr addrspace(200), i64);

; CHECK-LABEL: name: test1
; CHECK: %0:gpcr = COPY $c10
; CHECK-NEXT: %1:gpr = PseudoCGetAddr %0
; CHECK-NEXT: %2:gpr = ADDI $x0, -1793
; CHECK-NEXT: %3:gpr = SRLI killed %2, 8
; CHECK-NEXT: %4:gpr = AND killed %1, killed %3
; CHECK-NEXT: %5:gpcr = CSetAddr %0, killed %4
; CHECK-NEXT: $c10 = COPY %5
; CHECK-NEXT: PseudoCRET implicit $c10

define ptr addrspace(200) @test1(ptr addrspace(200) %src) {
  %ptr = call ptr addrspace(200) @llvm.ptrmask.p200.i64(ptr addrspace(200) %src, i64 72057594037927928)
  ret ptr addrspace(200) %ptr
}

declare ptr addrspace(200) @llvm.ptrmask.p200.i32(ptr addrspace(200), i32)

; CHECK-LABEL: name: test2
; CHECK: %0:gpcr = COPY $c10
; CHECK-NEXT: %1:gpr = PseudoCGetAddr %0
; CHECK-NEXT: %2:gpr = LUI 2
; CHECK-NEXT: %3:gpr = ADDIW killed %2, 1808
; CHECK-NEXT: %4:gpr = AND killed %1, killed %3
; CHECK-NEXT: %5:gpcr = CSetAddr %0, killed %4
; CHECK-NEXT: $c10 = COPY %5
; CHECK-NEXT: PseudoCRET implicit $c10

define ptr addrspace(200) @test2(ptr addrspace(200) %src) {
  %ptr = call ptr addrspace(200) @llvm.ptrmask.p200.i32(ptr addrspace(200) %src, i32 10000)
  ret ptr addrspace(200) %ptr
}
