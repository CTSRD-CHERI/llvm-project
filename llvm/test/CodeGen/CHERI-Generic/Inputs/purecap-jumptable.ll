; RUN: llc @PURECAP_HARDFLOAT_ARGS@ < %s -o - | FileCheck %s
@IFNOT-MIPS@; RUN: llc @PURECAP_HARDFLOAT_ARGS@ -relocation-model=static < %s -o - | FileCheck %s
; Check that we can generate jump tables for switch statements.
; TODO: this is currently not implemented for CHERI-RISC-V

define void @below_threshold(i32 %in, i32 addrspace(200)* %out) nounwind {
entry:
  switch i32 %in, label %exit [
    i32 1, label %bb1
    i32 2, label %bb2
  ]
bb1:
  store i32 4, i32 addrspace(200)* %out
  br label %exit
bb2:
  store i32 3, i32 addrspace(200)* %out
  br label %exit
exit:
  ret void
}

; For RISC-V the jump table threshold is set to 5 cases, but MIPS uses the default
; value of 4 (set in llvm/lib/CodeGen/TargetLoweringBase.cpp).
define void @above_threshold_mips(i32 %in, i32 addrspace(200)* %out) nounwind {
entry:
  switch i32 %in, label %exit [
    i32 1, label %bb1
    i32 2, label %bb2
    i32 3, label %bb3
    i32 4, label %bb4
  ]
bb1:
  store i32 4, i32 addrspace(200)* %out
  br label %exit
bb2:
  store i32 3, i32 addrspace(200)* %out
  br label %exit
bb3:
  store i32 2, i32 addrspace(200)* %out
  br label %exit
bb4:
  store i32 1, i32 addrspace(200)* %out
  br label %exit
exit:
  ret void
}

; UTC_ARGS: --disable
@IF-MIPS@; CHECK-LABEL: .LJTI1_0:
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB1_2-.LJTI1_0
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB1_3-.LJTI1_0
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB1_4-.LJTI1_0
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB1_5-.LJTI1_0
; UTC_ARGS: --enable

define void @above_threshold_all(i32 %in, i32 addrspace(200)* %out) nounwind {
entry:
  switch i32 %in, label %exit [
    i32 1, label %bb1
    i32 2, label %bb2
    i32 3, label %bb3
    i32 4, label %bb4
    i32 5, label %bb5
    i32 6, label %bb6
  ]
bb1:
  store i32 4, i32 addrspace(200)* %out
  br label %exit
bb2:
  store i32 3, i32 addrspace(200)* %out
  br label %exit
bb3:
  store i32 2, i32 addrspace(200)* %out
  br label %exit
bb4:
  store i32 1, i32 addrspace(200)* %out
  br label %exit
bb5:
  store i32 100, i32 addrspace(200)* %out
  br label %exit
bb6:
  store i32 200, i32 addrspace(200)* %out
  br label %exit
exit:
  ret void
}

; UTC_ARGS: --disable
@IF-MIPS@; CHECK-LABEL: .LJTI2_0:
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB2_2-.LJTI2_0
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB2_3-.LJTI2_0
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB2_4-.LJTI2_0
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB2_5-.LJTI2_0
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB2_6-.LJTI2_0
@IF-MIPS@; CHECK-NEXT:    .4byte .LBB2_7-.LJTI2_0
@IF-RISCV@; CHECK-LABEL: .LJTI2_0:
@IF-RISCV@; CHECK-NEXT:    .word .LBB2_2-.Labove_threshold_all$jump_table_base
@IF-RISCV@; CHECK-NEXT:    .word .LBB2_3-.Labove_threshold_all$jump_table_base
@IF-RISCV@; CHECK-NEXT:    .word .LBB2_4-.Labove_threshold_all$jump_table_base
@IF-RISCV@; CHECK-NEXT:    .word .LBB2_5-.Labove_threshold_all$jump_table_base
@IF-RISCV@; CHECK-NEXT:    .word .LBB2_6-.Labove_threshold_all$jump_table_base
@IF-RISCV@; CHECK-NEXT:    .word .LBB2_7-.Labove_threshold_all$jump_table_base
; UTC_ARGS: --enable
