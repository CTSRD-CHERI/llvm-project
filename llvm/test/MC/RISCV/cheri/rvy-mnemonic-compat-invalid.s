# RUN: not llvm-mc %s -triple=riscv32 -mattr=+a,+xcheri -riscv-no-aliases 2>&1 | FileCheck %s
# RUN: not llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri -riscv-no-aliases 2>&1 | FileCheck %s

## Check that RVY mnemonics reject DDC as an operand for compatibility with the RVY specification.
# CHECK: :[[@LINE+1]]:10: error: invalid operand for instruction
ybld x1, ddc, c3
# CHECK: :[[@LINE+1]]:14: error: invalid operand for instruction
ybld x1, x2, ddc
# CHECK: :[[@LINE+1]]:9: error: invalid operand for instruction
yss x1, ddc, x3
# CHECK: :[[@LINE+1]]:13: error: invalid operand for instruction
yss x1, x2, ddc

## Check that ypermc rejects rd == rs1 since expansion requires a temporary register.
# CHECK: :[[@LINE+1]]:8: error: expanding RVY compatible mnemonic requires destination and source capability registers to be different since this needs a temporary register to negate the mask
ypermc x1, x1, x3

## Check that ymodew rejects rd == rs1 since expansion requires a temporary register.
# CHECK: :[[@LINE+1]]:8: error: expanding RVY compatible mnemonic requires destination and source capability registers to be different since this needs a temporary register to negate the mask
ymodew x1, x1, x3

## Check that in hybrid mode capability base registers are rejected for capability loads.
# CHECK: :[[@LINE+1]]:1: error: instruction requires the following: Capability Mode
ly x1, 0(c2)

## Check that in hybrid mode jumps reject capability registers.
# CHECK: :[[@LINE+1]]:1: error: instruction requires the following: Capability Mode
jr c1
# CHECK: :[[@LINE+1]]:1: error: instruction requires the following: Capability Mode
jalr c1, c2
# CHECK: :[[@LINE+1]]:1: error: instruction requires the following: Capability Mode
jal c1, 20
