; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --scrub-attributes --version 2
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/intrinsics-purecap-only.ll
; RUN: llc -mtriple=mips64 -mcpu=cheri128 -mattr=+cheri128 --relocation-model=pic -target-abi purecap < %s -o - | FileCheck %s --check-prefix=PURECAP
; RUN: not --crash llc -mtriple=mips64 -mcpu=cheri128 -mattr=+cheri128 --relocation-model=pic -target-abi n64 < %s -o - 2>&1 | FileCheck %s --check-prefix HYBRID-ERROR
; This test checks target-independent CHERI intrinsics that are only available for purecap code

; Currently the only purecap-only intrinsic is llvm.cheri.stack.cap.get()
declare i8 addrspace(200)* @llvm.cheri.stack.cap.get()

define i8 addrspace(200)* @stack_get() nounwind {
; PURECAP-LABEL: stack_get:
; PURECAP:       # %bb.0:
; PURECAP-NEXT:    cjr $c17
; PURECAP-NEXT:    cmove $c3, $c11
  %cap = call i8 addrspace(200)* @llvm.cheri.stack.cap.get()
  ret i8 addrspace(200)* %cap
}
; HYBRID-ERROR: LLVM ERROR: Cannot select: intrinsic %llvm.cheri.stack.cap.get
