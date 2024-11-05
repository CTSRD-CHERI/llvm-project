# RUN: not llvm-mc -triple riscv32 -mattr=+h,+zcheripurecap,+cap-mode < %s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK-OFFSET
# RUN: not llvm-mc -triple riscv32 < %s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK,CHECK-OFFSET

hfence.vvma zero, zero # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'H' (Hypervisor)

hlv.h   ca0, 0(ca1) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'H' (Hypervisor), The zcheripurecap Extension is Enabled, Capability Mode

hlv.wu  ca0, 0(ca1) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'H' (Hypervisor), The zcheripurecap Extension is Enabled, Capability Mode, RV64I Base Instruction Set

hlv.b   ca0, 100(ca1) # CHECK-OFFSET: :[[@LINE]]:14: error: optional integer offset must be 0
