; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --scrub-attributes --version 2
; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/unaligned-loads-stores-hybrid.ll
; RUN: llc -mtriple=riscv32 --relocation-model=pic -target-abi ilp32f -mattr=+xcheri,+f %s -o - | FileCheck %s

@a1 = global i64 0, align 1
@a2 = global i64 0, align 2
@a4 = global i64 0, align 4
@a8 = global i64 0, align 8

define i64 @load_global_i64_align_1(i64 %y) addrspace(200) nounwind {
; CHECK-LABEL: load_global_i64_align_1:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .Lpcrel_hi0:
; CHECK-NEXT:    auipc a0, %got_pcrel_hi(a1)
; CHECK-NEXT:    lw a0, %pcrel_lo(.Lpcrel_hi0)(a0)
; CHECK-NEXT:    cspecialr ca1, ddc
; CHECK-NEXT:    bnez a0, .LBB0_2
; CHECK-NEXT:  # %bb.1:
; CHECK-NEXT:    cmove ca1, cnull
; CHECK-NEXT:    j .LBB0_3
; CHECK-NEXT:  .LBB0_2:
; CHECK-NEXT:    csetaddr ca1, ca1, a0
; CHECK-NEXT:  .LBB0_3:
; CHECK-NEXT:    lbu.cap a0, (ca1)
; CHECK-NEXT:    cincoffset ca2, ca1, 1
; CHECK-NEXT:    lbu.cap a2, (ca2)
; CHECK-NEXT:    cincoffset ca3, ca1, 2
; CHECK-NEXT:    lbu.cap a3, (ca3)
; CHECK-NEXT:    cincoffset ca4, ca1, 3
; CHECK-NEXT:    lbu.cap a4, (ca4)
; CHECK-NEXT:    slli a2, a2, 8
; CHECK-NEXT:    or a0, a2, a0
; CHECK-NEXT:    slli a3, a3, 16
; CHECK-NEXT:    slli a4, a4, 24
; CHECK-NEXT:    or a3, a4, a3
; CHECK-NEXT:    or a0, a3, a0
; CHECK-NEXT:    cincoffset ca2, ca1, 4
; CHECK-NEXT:    lbu.cap a2, (ca2)
; CHECK-NEXT:    cincoffset ca3, ca1, 5
; CHECK-NEXT:    lbu.cap a3, (ca3)
; CHECK-NEXT:    cincoffset ca4, ca1, 6
; CHECK-NEXT:    lbu.cap a4, (ca4)
; CHECK-NEXT:    cincoffset ca1, ca1, 7
; CHECK-NEXT:    lbu.cap a1, (ca1)
; CHECK-NEXT:    slli a3, a3, 8
; CHECK-NEXT:    or a2, a3, a2
; CHECK-NEXT:    slli a4, a4, 16
; CHECK-NEXT:    slli a1, a1, 24
; CHECK-NEXT:    or a1, a1, a4
; CHECK-NEXT:    or a1, a1, a2
; CHECK-NEXT:    ret
  %ret = load i64, i64 addrspace(200)* addrspacecast(i64* @a1 to i64 addrspace(200)*), align 1
  ret i64 %ret
}

define i64 @load_global_i64_align_2(i64 %y) addrspace(200) nounwind {
; CHECK-LABEL: load_global_i64_align_2:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .Lpcrel_hi1:
; CHECK-NEXT:    auipc a0, %got_pcrel_hi(a2)
; CHECK-NEXT:    lw a0, %pcrel_lo(.Lpcrel_hi1)(a0)
; CHECK-NEXT:    cspecialr ca1, ddc
; CHECK-NEXT:    bnez a0, .LBB1_2
; CHECK-NEXT:  # %bb.1:
; CHECK-NEXT:    cmove ca0, cnull
; CHECK-NEXT:    j .LBB1_3
; CHECK-NEXT:  .LBB1_2:
; CHECK-NEXT:    csetaddr ca0, ca1, a0
; CHECK-NEXT:  .LBB1_3:
; CHECK-NEXT:    lhu.cap a1, (ca0)
; CHECK-NEXT:    cincoffset ca2, ca0, 2
; CHECK-NEXT:    lhu.cap a2, (ca2)
; CHECK-NEXT:    cincoffset ca3, ca0, 4
; CHECK-NEXT:    cincoffset ca0, ca0, 6
; CHECK-NEXT:    lhu.cap a4, (ca0)
; CHECK-NEXT:    lhu.cap a3, (ca3)
; CHECK-NEXT:    slli a0, a2, 16
; CHECK-NEXT:    or a0, a0, a1
; CHECK-NEXT:    slli a1, a4, 16
; CHECK-NEXT:    or a1, a1, a3
; CHECK-NEXT:    ret
  %ret = load i64, i64 addrspace(200)* addrspacecast(i64* @a2 to i64 addrspace(200)*), align 2
  ret i64 %ret
}

define i64 @load_global_i64_align_4(i64 %y) addrspace(200) nounwind {
; CHECK-LABEL: load_global_i64_align_4:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .Lpcrel_hi2:
; CHECK-NEXT:    auipc a0, %got_pcrel_hi(a4)
; CHECK-NEXT:    lw a0, %pcrel_lo(.Lpcrel_hi2)(a0)
; CHECK-NEXT:    cspecialr ca1, ddc
; CHECK-NEXT:    bnez a0, .LBB2_2
; CHECK-NEXT:  # %bb.1:
; CHECK-NEXT:    cmove ca1, cnull
; CHECK-NEXT:    j .LBB2_3
; CHECK-NEXT:  .LBB2_2:
; CHECK-NEXT:    csetaddr ca1, ca1, a0
; CHECK-NEXT:  .LBB2_3:
; CHECK-NEXT:    lw.cap a0, (ca1)
; CHECK-NEXT:    cincoffset ca1, ca1, 4
; CHECK-NEXT:    lw.cap a1, (ca1)
; CHECK-NEXT:    ret
  %ret = load i64, i64 addrspace(200)* addrspacecast(i64* @a4 to i64 addrspace(200)*), align 4
  ret i64 %ret
}

define i64 @load_global_i64_align_8(i64 %y) addrspace(200) nounwind {
; CHECK-LABEL: load_global_i64_align_8:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .Lpcrel_hi3:
; CHECK-NEXT:    auipc a0, %got_pcrel_hi(a8)
; CHECK-NEXT:    lw a0, %pcrel_lo(.Lpcrel_hi3)(a0)
; CHECK-NEXT:    cspecialr ca1, ddc
; CHECK-NEXT:    bnez a0, .LBB3_2
; CHECK-NEXT:  # %bb.1:
; CHECK-NEXT:    cmove ca1, cnull
; CHECK-NEXT:    j .LBB3_3
; CHECK-NEXT:  .LBB3_2:
; CHECK-NEXT:    csetaddr ca1, ca1, a0
; CHECK-NEXT:  .LBB3_3:
; CHECK-NEXT:    lw.cap a0, (ca1)
; CHECK-NEXT:    cincoffset ca1, ca1, 4
; CHECK-NEXT:    lw.cap a1, (ca1)
; CHECK-NEXT:    ret
  %ret = load i64, i64 addrspace(200)* addrspacecast(i64* @a8 to i64 addrspace(200)*), align 8
  ret i64 %ret
}

define void @store_global_i64_align_1(i64 %y) addrspace(200) nounwind {
; CHECK-LABEL: store_global_i64_align_1:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .Lpcrel_hi4:
; CHECK-NEXT:    auipc a2, %got_pcrel_hi(a1)
; CHECK-NEXT:    lw a2, %pcrel_lo(.Lpcrel_hi4)(a2)
; CHECK-NEXT:    cspecialr ca3, ddc
; CHECK-NEXT:    bnez a2, .LBB4_2
; CHECK-NEXT:  # %bb.1:
; CHECK-NEXT:    cmove ca2, cnull
; CHECK-NEXT:    j .LBB4_3
; CHECK-NEXT:  .LBB4_2:
; CHECK-NEXT:    csetaddr ca2, ca3, a2
; CHECK-NEXT:  .LBB4_3:
; CHECK-NEXT:    sb.cap a0, (ca2)
; CHECK-NEXT:    cincoffset ca3, ca2, 7
; CHECK-NEXT:    srli a4, a1, 24
; CHECK-NEXT:    sb.cap a4, (ca3)
; CHECK-NEXT:    cincoffset ca3, ca2, 6
; CHECK-NEXT:    srli a4, a1, 16
; CHECK-NEXT:    sb.cap a4, (ca3)
; CHECK-NEXT:    cincoffset ca3, ca2, 5
; CHECK-NEXT:    srli a4, a1, 8
; CHECK-NEXT:    sb.cap a4, (ca3)
; CHECK-NEXT:    cincoffset ca3, ca2, 4
; CHECK-NEXT:    sb.cap a1, (ca3)
; CHECK-NEXT:    cincoffset ca1, ca2, 3
; CHECK-NEXT:    srli a3, a0, 24
; CHECK-NEXT:    sb.cap a3, (ca1)
; CHECK-NEXT:    cincoffset ca1, ca2, 2
; CHECK-NEXT:    srli a3, a0, 16
; CHECK-NEXT:    sb.cap a3, (ca1)
; CHECK-NEXT:    cincoffset ca1, ca2, 1
; CHECK-NEXT:    srli a0, a0, 8
; CHECK-NEXT:    sb.cap a0, (ca1)
; CHECK-NEXT:    ret
  store i64 %y, i64 addrspace(200)* addrspacecast(i64* @a1 to i64 addrspace(200)*), align 1
  ret void
}

define void @store_global_i64_align_2(i64 %y) addrspace(200) nounwind {
; CHECK-LABEL: store_global_i64_align_2:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .Lpcrel_hi5:
; CHECK-NEXT:    auipc a2, %got_pcrel_hi(a2)
; CHECK-NEXT:    lw a2, %pcrel_lo(.Lpcrel_hi5)(a2)
; CHECK-NEXT:    cspecialr ca3, ddc
; CHECK-NEXT:    bnez a2, .LBB5_2
; CHECK-NEXT:  # %bb.1:
; CHECK-NEXT:    cmove ca2, cnull
; CHECK-NEXT:    j .LBB5_3
; CHECK-NEXT:  .LBB5_2:
; CHECK-NEXT:    csetaddr ca2, ca3, a2
; CHECK-NEXT:  .LBB5_3:
; CHECK-NEXT:    sh.cap a0, (ca2)
; CHECK-NEXT:    cincoffset ca3, ca2, 6
; CHECK-NEXT:    srli a4, a1, 16
; CHECK-NEXT:    sh.cap a4, (ca3)
; CHECK-NEXT:    cincoffset ca3, ca2, 4
; CHECK-NEXT:    sh.cap a1, (ca3)
; CHECK-NEXT:    cincoffset ca1, ca2, 2
; CHECK-NEXT:    srli a0, a0, 16
; CHECK-NEXT:    sh.cap a0, (ca1)
; CHECK-NEXT:    ret
  store i64 %y, i64 addrspace(200)* addrspacecast(i64* @a2 to i64 addrspace(200)*), align 2
  ret void
}

define void @store_global_i64_align_4(i64 %y) addrspace(200) nounwind {
; CHECK-LABEL: store_global_i64_align_4:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .Lpcrel_hi6:
; CHECK-NEXT:    auipc a2, %got_pcrel_hi(a4)
; CHECK-NEXT:    lw a2, %pcrel_lo(.Lpcrel_hi6)(a2)
; CHECK-NEXT:    cspecialr ca3, ddc
; CHECK-NEXT:    bnez a2, .LBB6_2
; CHECK-NEXT:  # %bb.1:
; CHECK-NEXT:    cmove ca2, cnull
; CHECK-NEXT:    j .LBB6_3
; CHECK-NEXT:  .LBB6_2:
; CHECK-NEXT:    csetaddr ca2, ca3, a2
; CHECK-NEXT:  .LBB6_3:
; CHECK-NEXT:    sw.cap a0, (ca2)
; CHECK-NEXT:    cincoffset ca0, ca2, 4
; CHECK-NEXT:    sw.cap a1, (ca0)
; CHECK-NEXT:    ret
  store i64 %y, i64 addrspace(200)* addrspacecast(i64* @a4 to i64 addrspace(200)*), align 4
  ret void
}

define void @store_global_i64_align_8(i64 %y) addrspace(200) nounwind {
; CHECK-LABEL: store_global_i64_align_8:
; CHECK:       # %bb.0:
; CHECK-NEXT:  .Lpcrel_hi7:
; CHECK-NEXT:    auipc a2, %got_pcrel_hi(a8)
; CHECK-NEXT:    lw a2, %pcrel_lo(.Lpcrel_hi7)(a2)
; CHECK-NEXT:    cspecialr ca3, ddc
; CHECK-NEXT:    bnez a2, .LBB7_2
; CHECK-NEXT:  # %bb.1:
; CHECK-NEXT:    cmove ca2, cnull
; CHECK-NEXT:    j .LBB7_3
; CHECK-NEXT:  .LBB7_2:
; CHECK-NEXT:    csetaddr ca2, ca3, a2
; CHECK-NEXT:  .LBB7_3:
; CHECK-NEXT:    sw.cap a0, (ca2)
; CHECK-NEXT:    cincoffset ca0, ca2, 4
; CHECK-NEXT:    sw.cap a1, (ca0)
; CHECK-NEXT:    ret
  store i64 %y, i64 addrspace(200)* addrspacecast(i64* @a8 to i64 addrspace(200)*), align 8
  ret void
}
