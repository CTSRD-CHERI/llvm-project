; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --scrub-attributes --version 2
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/cheri-csub.ll
; RUN: llc -mtriple=riscv64 --relocation-model=pic -target-abi lp64d -mattr=+xcheri,+f,+d %s -o - | FileCheck %s --check-prefix=HYBRID
; RUN: llc -mtriple=riscv64 --relocation-model=pic -target-abi l64pc128d -mattr=+xcheri,+cap-mode,+f,+d %s -o - | FileCheck %s --check-prefix=PURECAP

define i64 @subp(i8 addrspace(200)* readnone %a, i8 addrspace(200)* readnone %b) nounwind {
; HYBRID-LABEL: subp:
; HYBRID:       # %bb.0:
; HYBRID-NEXT:    sub a0, a0, a1
; HYBRID-NEXT:    ret
;
; PURECAP-LABEL: subp:
; PURECAP:       # %bb.0:
; PURECAP-NEXT:    sub a0, a0, a1
; PURECAP-NEXT:    ret
  %1 = tail call i64 @llvm.cheri.cap.diff.i64(i8 addrspace(200)* %a, i8 addrspace(200)* %b)
  ret i64 %1
}

declare i64 @llvm.cheri.cap.diff.i64(i8 addrspace(200)*, i8 addrspace(200)*)
