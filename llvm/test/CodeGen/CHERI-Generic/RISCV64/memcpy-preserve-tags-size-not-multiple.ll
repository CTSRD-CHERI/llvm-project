; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --scrub-attributes --version 2
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/memcpy-preserve-tags-size-not-multiple.ll
; RUN: llc -mtriple=riscv64 --relocation-model=pic -target-abi l64pc128d -mattr=+xcheri,+cap-mode,+f,+d -o - -O0 -verify-machineinstrs %s | FileCheck %s -check-prefixes CHECK
; Check that we can inline memmove/memcpy despite having the must_preserve_cheri_tags property and the size not
; being a multiple of CAP_SIZE. Since the pointers are aligned we can start with capability copies and use
; word/byte copies for the trailing bytes.
declare void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* nocapture, i8 addrspace(200)* nocapture readonly, i64, i1) addrspace(200)
declare void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* nocapture, i8 addrspace(200)* nocapture readonly, i64, i1) addrspace(200)

define void @test_string_memmove(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) nounwind {
  ; Note: has must_preserve_cheri_tags, but this memmove can still be inlined since it's aligned
; CHECK-LABEL: test_string_memmove:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cmove ca5, ca1
; CHECK-NEXT:    cmove ca1, ca0
; CHECK-NEXT:    lc ca0, 0(ca5)
; CHECK-NEXT:    lc ca2, 16(ca5)
; CHECK-NEXT:    ld a3, 32(ca5)
; CHECK-NEXT:    lw a4, 40(ca5)
; CHECK-NEXT:    lb a5, 44(ca5)
; CHECK-NEXT:    sb a5, 44(ca1)
; CHECK-NEXT:    sw a4, 40(ca1)
; CHECK-NEXT:    sd a3, 32(ca1)
; CHECK-NEXT:    sc ca2, 16(ca1)
; CHECK-NEXT:    sc ca0, 0(ca1)
; CHECK-NEXT:    ret
  call void @llvm.memmove.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 addrspace(200)* align 16 %src, i64 45, i1 false) must_preserve_cheri_tags
  ret void
}

define void @test_string_memcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* %src) addrspace(200) nounwind {
  ; Note: has must_preserve_cheri_tags, but this memcpy can still be inlined since it's aligned
; CHECK-LABEL: test_string_memcpy:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cincoffset csp, csp, -16
; CHECK-NEXT:    sc ca1, 0(csp) # 16-byte Folded Spill
; CHECK-NEXT:    cmove ca1, ca0
; CHECK-NEXT:    lc ca0, 0(csp) # 16-byte Folded Reload
; CHECK-NEXT:    lb a2, 44(ca0)
; CHECK-NEXT:    sb a2, 44(ca1)
; CHECK-NEXT:    lw a2, 40(ca0)
; CHECK-NEXT:    sw a2, 40(ca1)
; CHECK-NEXT:    ld a2, 32(ca0)
; CHECK-NEXT:    sd a2, 32(ca1)
; CHECK-NEXT:    lc ca2, 16(ca0)
; CHECK-NEXT:    sc ca2, 16(ca1)
; CHECK-NEXT:    lc ca0, 0(ca0)
; CHECK-NEXT:    sc ca0, 0(ca1)
; CHECK-NEXT:    cincoffset csp, csp, 16
; CHECK-NEXT:    ret
  call void @llvm.memcpy.p200i8.p200i8.i64(i8 addrspace(200)* align 16 %dst, i8 addrspace(200)* align 16 %src, i64 45, i1 false) must_preserve_cheri_tags
  ret void
}
