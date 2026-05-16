# RUN: not llvm-mc %s -triple=riscv32 -mattr=+a,+xcheri -riscv-no-aliases 2>&1 | FileCheck %s
# RUN: not llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri -riscv-no-aliases 2>&1 | FileCheck %s

## Check that RVY mnemonics reject DDC as an operand for compatibility with the RVY specification.
# CHECK: :[[@LINE+1]]:10: error: invalid operand for instruction
ybld c1, ddc, c3
# CHECK: :[[@LINE+1]]:14: error: invalid operand for instruction
ybld c1, c2, ddc
# CHECK: :[[@LINE+1]]:9: error: invalid operand for instruction
yss x1, ddc, c3
# CHECK: :[[@LINE+1]]:13: error: invalid operand for instruction
yss x1, c2, ddc
