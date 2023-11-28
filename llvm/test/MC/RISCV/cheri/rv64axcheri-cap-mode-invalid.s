# RUN: not llvm-mc -triple riscv64 -mattr=+a,+xcheri,+cap-mode < %s 2>&1 | FileCheck %s

# Final operand must have parentheses
amoswap.d a1, a2, ca3 # CHECK: :[[@LINE]]:19: error: expected '(' or optional integer offset
amomin.d a1, a2, 1 # CHECK: :[[@LINE]]:20: error: expected '(' after optional integer offset
amomin.d a1, a2, 1(ca3) # CHECK: :[[@LINE]]:18: error: optional integer offset must be 0
lr.d a4, a5 # CHECK: :[[@LINE]]:10: error: expected '(' or optional integer offset
lr.d a4, (a5) # CHECK: :[[@LINE]]:1: error: instruction requires the following: Not Capability Mode

# Only .aq, .rl, and .aqrl suffixes are valid
amoxor.d.rlqa a2, a3, (ca4) # CHECK: :[[@LINE]]:1: error: unrecognized instruction mnemonic
amoor.d.aq.rl a4, a5, (ca6) # CHECK: :[[@LINE]]:1: error: unrecognized instruction mnemonic
amoor.d. a4, a5, (ca6) # CHECK: :[[@LINE]]:1: error: unrecognized instruction mnemonic

# lr only takes two operands
lr.d s0, (cs1), s2 # CHECK: :[[@LINE]]:17: error: invalid operand for instruction
