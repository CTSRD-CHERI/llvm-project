; DO NOT EDIT -- This file was generated from test/CodeGen/CHERI-Generic/Inputs/global-capinit-hybrid.ll
; RUN: %riscv64_cheri_llc %s -o - | \
; RUN: FileCheck %s --check-prefix=ASM -DPTR_DIRECTIVE=.quad
; RUN: %riscv64_cheri_llc %s -filetype=obj -o - | llvm-objdump -r -t -

target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128"

declare void @extern_fn() #0
@extern_data = external global i8, align 1

@global_ptr_const = global i8* inttoptr (i64 1234 to i8*), align 8
; ASM-LABEL: .globl global_ptr_const
; ASM-NEXT:  .p2align 3
; ASM-NEXT: global_ptr_const:
; ASM-NEXT:  [[PTR_DIRECTIVE]] 1234
; ASM-NEXT:  .size global_ptr_const, 8
@global_cap_inttoptr = global i8 addrspace(200)* inttoptr (i64 1234 to i8 addrspace(200)*), align 16
; ASM-LABEL: .globl global_cap_inttoptr
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_cap_inttoptr:
; ASM-NEXT:  .chericap 1234
; ASM-NEXT:  .size global_cap_inttoptr, 16
@global_cap_addrspacecast = global i8 addrspace(200)* addrspacecast (i8* inttoptr (i64 1234 to i8*) to i8 addrspace(200)*), align 16
; ASM-LABEL: .globl global_cap_addrspacecast
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_cap_addrspacecast:
; ASM-NEXT:  .chericap 1234
; ASM-NEXT:  .size global_cap_addrspacecast, 16
@global_cap_nullgep = global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1234), align 16
; TODO: this should be untagged
; ASM-LABEL: .globl global_cap_nullgep
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_cap_nullgep:
; ASM-NEXT:  .chericap 1234
; ASM-NEXT:  .size global_cap_nullgep, 16

@global_ptr_data = global i8* @extern_data, align 8
; ASM-LABEL: .globl global_ptr_data
; ASM-NEXT:  .p2align 3
; ASM-NEXT: global_ptr_data:
; ASM-NEXT:  [[PTR_DIRECTIVE]] extern_data
; ASM-NEXT:  .size global_ptr_data, 8
@global_ptr_data_past_end = global i8* getelementptr inbounds (i8, i8* @extern_data, i64 1), align 8
; ASM-LABEL: .globl global_ptr_data_past_end
; ASM-NEXT:  .p2align 3
; ASM-NEXT: global_ptr_data_past_end:
; ASM-NEXT:  [[PTR_DIRECTIVE]] extern_data+1
; ASM-NEXT:  .size global_ptr_data_past_end, 8
@global_ptr_data_two_past_end = global i8* getelementptr (i8, i8* @extern_data, i64 2), align 8
; ASM-LABEL: .globl global_ptr_data_two_past_end
; ASM-NEXT:  .p2align 3
; ASM-NEXT: global_ptr_data_two_past_end:
; ASM-NEXT:  [[PTR_DIRECTIVE]] extern_data+2
; ASM-NEXT:  .size global_ptr_data_two_past_end, 8

@global_cap_data_addrspacecast = global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
; ASM-LABEL: .globl global_cap_data_addrspacecast
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_cap_data_addrspacecast:
; ASM-NEXT:  .chericap extern_data
; ASM-NEXT:  .size global_cap_data_addrspacecast, 16
@global_cap_data_addrspacecast_past_end = global i8 addrspace(200)* addrspacecast (i8* getelementptr inbounds (i8, i8* @extern_data, i64 1) to i8 addrspace(200)*), align 16
; ASM-LABEL: .globl global_cap_data_addrspacecast_past_end
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_cap_data_addrspacecast_past_end:
; ASM-NEXT:  .chericap extern_data+1
; ASM-NEXT:  .size global_cap_data_addrspacecast_past_end, 16
@global_cap_data_addrspacecast_two_past_end = global i8 addrspace(200)* addrspacecast (i8* getelementptr (i8, i8* @extern_data, i64 2) to i8 addrspace(200)*), align 16
; ASM-LABEL: .globl global_cap_data_addrspacecast_two_past_end
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_cap_data_addrspacecast_two_past_end:
; ASM-NEXT:  .chericap extern_data+2
; ASM-NEXT:  .size global_cap_data_addrspacecast_two_past_end, 16

@global_cap_data_nullgep = global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* @extern_data to i64)), align 16
; FIXME: this is completely wrong
; ASM-LABEL: .globl global_cap_data_nullgep
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_cap_data_nullgep:
; ASM-NEXT:  .chericap 0
; ASM-NEXT:  .size global_cap_data_nullgep, 16
@global_cap_data_nullgep_past_end = global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* getelementptr inbounds (i8, i8* @extern_data, i64 1) to i64)), align 16
; FIXME: this is completely wrong
; ASM-LABEL: .globl global_cap_data_nullgep_past_end
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_cap_data_nullgep_past_end:
; ASM-NEXT:  .chericap 0
; ASM-NEXT:  .size global_cap_data_nullgep_past_end, 16
@global_cap_data_nullgep_two_past_end = global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* getelementptr (i8, i8* @extern_data, i64 2) to i64)), align 16
; FIXME: this is completely wrong
; ASM-LABEL: .globl global_cap_data_nullgep_two_past_end
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_cap_data_nullgep_two_past_end:
; ASM-NEXT:  .chericap 0
; ASM-NEXT:  .size global_cap_data_nullgep_two_past_end, 16

@global_fnptr = global void ()* @extern_fn, align 8
; ASM-LABEL: .globl global_fnptr
; ASM-NEXT:  .p2align 3
; ASM-NEXT: global_fnptr:
; ASM-NEXT:  [[PTR_DIRECTIVE]] extern_fn
; ASM-NEXT:  .size global_fnptr, 8
@global_fncap_addrspacecast = global void () addrspace(200)* addrspacecast (void ()* @extern_fn to void () addrspace(200)*), align 16
; ASM-LABEL: .globl global_fncap_addrspacecast
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_fncap_addrspacecast:
; ASM-NEXT:  .chericap extern_fn
; ASM-NEXT:  .size global_fncap_addrspacecast, 16
@global_fncap_intcap_addrspacecast = global i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @extern_fn to i8*) to i8 addrspace(200)*), align 16
; ASM-LABEL: .globl global_fncap_intcap_addrspacecast
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_fncap_intcap_addrspacecast:
; ASM-NEXT:  .chericap extern_fn
; ASM-NEXT:  .size global_fncap_intcap_addrspacecast, 16
@global_fncap_intcap_nullgep = global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (void ()* @extern_fn to i64)), align 16
; ASM-LABEL: .globl global_fncap_intcap_nullgep
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_fncap_intcap_nullgep:
; FIXME: this is completely wrong
; ASM-NEXT:  .chericap 0
; ASM-NEXT:  .size global_fncap_intcap_nullgep, 16
@global_fncap_addrspacecast_plus_two = global i8 addrspace(200)* addrspacecast (i8* getelementptr (i8, i8* bitcast (void ()* @extern_fn to i8*), i64 2) to i8 addrspace(200)*), align 16
; ASM-LABEL: .globl global_fncap_addrspacecast_plus_two
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_fncap_addrspacecast_plus_two:
; ASM-NEXT:  .chericap extern_fn+2
; ASM-NEXT:  .size global_fncap_addrspacecast_plus_two, 16
@global_fncap_nullgep_plus_two = global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* getelementptr (i8, i8* bitcast (void ()* @extern_fn to i8*), i64 2) to i64)), align 16
; ASM-LABEL: .globl global_fncap_nullgep_plus_two
; ASM-NEXT:  .p2align	4
; ASM-NEXT: global_fncap_nullgep_plus_two:
; FIXME: this is completely wrong
; ASM-NEXT:  .chericap 0
; ASM-NEXT:  .size global_fncap_nullgep_plus_two, 16
