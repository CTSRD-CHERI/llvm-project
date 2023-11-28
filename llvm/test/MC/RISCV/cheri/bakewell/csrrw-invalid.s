# RUN: not llvm-mc -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK
# RUN: not llvm-mc -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK
# RUN: not llvm-mc -triple=riscv32 -mattr=+zcheripurecap <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK
# RUN: not llvm-mc -triple=riscv64 -mattr=+zcheripurecap <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK


.option capmode

## integer CSRs with capability versions are removed in capmode
csrrw a0, mtvec, a0 # CHECK: <stdin>:[[#@LINE]]:11: error: system register use requires an option to be enabled


## Can't use a cheri system register with integer registers
csrrw a0, mtvecc, a0 # CHECK: <stdin>:[[#@LINE]]:11: error: operand must be a valid system register name or an integer in the range [0, 4095]

## Can't use integer CSRs with cheri registers
csrrw ca0, mcause, ca0 # CHECK: <stdin>:[[#@LINE]]:12: error: operand must be a valid cheri system register name

## Can't use integer constant as a cheri CSR
csrrw ca0, 4095, ca0 # CHECK:  <stdin>:[[#@LINE]]:12: error:  operand must be a valid cheri system register name

.option nocapmode 

## Can't use a cheri system register with integer registers
csrrw a0, mtvecc, a0 # CHECK: <stdin>:[[#@LINE]]:11: error: operand must be a valid system register name or an integer in the range [0, 4095]

## Can't use integer CSRs with cheri registers
csrrw ca0, mcause, ca0 # CHECK: <stdin>:[[#@LINE]]:12: error: operand must be a valid cheri system register name
.option nocapmode 
