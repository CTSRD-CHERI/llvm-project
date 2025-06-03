; Relative lookup tables are not compatible with tight bounds on the individual elements (unless we were
; to manually insert setbounds instructions after the relative load). Additionally, the relative load
; is also not compatible with the current CHERI globals initialization scheme since the generated
; global would require bounds that span all the individual elements.
; See https://github.com/CTSRD-CHERI/llvm-project/issues/572
; RUN: sed -e 's/addrspace(200)/addrspace(0)/g' -e 's/-A200-P200-G200//g' %s | \
; RUN:     opt -passes=rel-lookup-table-converter -relocation-model=pic -S | FileCheck %s --check-prefix=RISCV64
; RUN: sed -e 's/addrspace(200)/addrspace(0)/g' -e 's/-A200-P200-G200//g' %s | \
; RUN:     %riscv64_cheri_llc -relocation-model=pic | FileCheck %s --check-prefixes=RISCV64-ASM-NOREL
; RUN: sed -e 's/addrspace(200)/addrspace(0)/g' -e 's/-A200-P200-G200//g' %s | opt -passes=rel-lookup-table-converter \
; RUN:     -relocation-model=pic -S | %riscv64_cheri_llc -relocation-model=pic | FileCheck %s --check-prefixes=RISCV64-ASM-REL
; RUN: opt -o - -passes=rel-lookup-table-converter -relocation-model=pic -S %s | FileCheck %s --check-prefix=PURECAP
; RUN: opt -o - -passes=rel-lookup-table-converter -relocation-model=pic -S %s | \
; RUN:     %riscv64_cheri_purecap_llc -relocation-model=pic | FileCheck %s --check-prefix=PURECAP-ASM
; REQUIRES: riscv-registered-target

target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128-A200-P200-G200"
target triple = "riscv64-unknown-freebsd13.0"

@global = internal addrspace(200) constant [6 x ptr addrspace(200)] [ptr addrspace(200) @global.1, ptr addrspace(200) @global.2, ptr addrspace(200) @global.3, ptr addrspace(200) @global.4, ptr addrspace(200) @global.5, ptr addrspace(200) @global.6], align 16
@global.1 = private unnamed_addr addrspace(200) constant [13 x i8] c"RPC: Success\00", align 1
@global.2 = private unnamed_addr addrspace(200) constant [28 x i8] c"RPC: Can't encode arguments\00", align 1
@global.3 = private unnamed_addr addrspace(200) constant [25 x i8] c"RPC: Can't decode result\00", align 1
@global.4 = private unnamed_addr addrspace(200) constant [20 x i8] c"RPC: Unable to send\00", align 1
@global.5 = private unnamed_addr addrspace(200) constant [23 x i8] c"RPC: Unable to receive\00", align 1
@global.6 = private unnamed_addr addrspace(200) constant [15 x i8] c"RPC: Timed out\00", align 1

define ptr addrspace(200) @load_from_string_table(i64 %idx) addrspace(200) {
; RISCV64-LABEL: define {{[^@]+}}@load_from_string_table
; RISCV64-SAME: (i64 [[IDX:%.*]]) {
; RISCV64-NEXT:  bb:
; RISCV64-NEXT:    [[RELTABLE_SHIFT:%.*]] = shl i64 [[IDX]], 2
; RISCV64-NEXT:    [[RELTABLE_INTRINSIC:%.*]] = call ptr @llvm.load.relative.i64(ptr @reltable.load_from_string_table, i64 [[RELTABLE_SHIFT]])
; RISCV64-NEXT:    ret ptr [[RELTABLE_INTRINSIC]]
;
; PURECAP-LABEL: define {{[^@]+}}@load_from_string_table
; PURECAP-SAME: (i64 [[IDX:%.*]]) addrspace(200) {
; PURECAP-NEXT:  bb:
; PURECAP-NEXT:    [[PTR:%.*]] = getelementptr inbounds [6 x ptr addrspace(200)], ptr addrspace(200) @global, i64 0, i64 [[IDX]]
; PURECAP-NEXT:    [[STR:%.*]] = load ptr addrspace(200), ptr addrspace(200) [[PTR]], align 16
; PURECAP-NEXT:    ret ptr addrspace(200) [[STR]]
;
; RISCV64-ASM-NOREL-LABEL: load_from_string_table:
; RISCV64-ASM-NOREL:       # %bb.0: # %bb
; RISCV64-ASM-NOREL-NEXT:    slli a0, a0, 3
; RISCV64-ASM-NOREL-NEXT:  .Lpcrel_hi0:
; RISCV64-ASM-NOREL-NEXT:    auipc a1, %pcrel_hi(global)
; RISCV64-ASM-NOREL-NEXT:    addi a1, a1, %pcrel_lo(.Lpcrel_hi0)
; RISCV64-ASM-NOREL-NEXT:    add a0, a1, a0
; RISCV64-ASM-NOREL-NEXT:    ld a0, 0(a0)
; RISCV64-ASM-NOREL-NEXT:    ret
; RISCV64-ASM-REL-LABEL: load_from_string_table:
; RISCV64-ASM-REL:       # %bb.0: # %bb
; RISCV64-ASM-REL-NEXT:    slli a0, a0, 2
; RISCV64-ASM-REL-NEXT:  .Lpcrel_hi0:
; RISCV64-ASM-REL-NEXT:    auipc a1, %pcrel_hi(reltable.load_from_string_table)
; RISCV64-ASM-REL-NEXT:    addi a1, a1, %pcrel_lo(.Lpcrel_hi0)
; RISCV64-ASM-REL-NEXT:    add a0, a1, a0
; RISCV64-ASM-REL-NEXT:    lw a0, 0(a0)
; RISCV64-ASM-REL-NEXT:    add a0, a1, a0
; RISCV64-ASM-REL-NEXT:    ret
; PURECAP-ASM-LABEL: load_from_string_table:
; PURECAP-ASM:       # %bb.0: # %bb
; PURECAP-ASM-NEXT:  .LBB0_1: # %bb
; PURECAP-ASM-NEXT:    # Label of block must be emitted
; PURECAP-ASM-NEXT:    auipcc ca1, %got_pcrel_hi(global)
; PURECAP-ASM-NEXT:    lc ca1, %pcrel_lo(.LBB0_1)(ca1)
; PURECAP-ASM-NEXT:    slli a0, a0, 4
; PURECAP-ASM-NEXT:    cincoffset ca0, ca1, a0
; PURECAP-ASM-NEXT:    lc ca0, 0(ca0)
; PURECAP-ASM-NEXT:    ret
bb:
  %ptr = getelementptr inbounds [6 x ptr addrspace(200)], ptr addrspace(200) @global, i64 0, i64 %idx
  %str = load ptr addrspace(200), ptr addrspace(200) %ptr, align 16
  ret ptr addrspace(200) %str
}
