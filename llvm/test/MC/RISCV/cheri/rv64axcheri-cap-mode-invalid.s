# RUN: not llvm-mc -triple riscv64 -mattr=+a,+xcheri,+cap-mode < %s 2>&1 | FileCheck %s

# Final operand must have parentheses
camoswap.d a1, a2, ca3 # CHECK: :[[@LINE]]:20: error: expected '(' or optional integer offset
camomin.d a1, a2, 1 # CHECK: :[[@LINE]]:21: error: expected '(' after optional integer offset
camomin.d a1, a2, 1(ca3) # CHECK: :[[@LINE]]:19: error: optional integer offset must be 0
clr.d a4, a5 # CHECK: :[[@LINE]]:11: error: expected '(' or optional integer offset
clr.d a4, (a5) # CHECK: :[[@LINE]]:12: error: invalid operand for instruction

# Only .aq, .rl, and .aqrl suffixes are valid
camoxor.d.rlqa a2, a3, (ca4) # CHECK: :[[@LINE]]:1: error: unrecognized instruction mnemonic
camoor.d.aq.rl a4, a5, (ca6) # CHECK: :[[@LINE]]:1: error: unrecognized instruction mnemonic
camoor.d. a4, a5, (ca6) # CHECK: :[[@LINE]]:1: error: unrecognized instruction mnemonic

# lr only takes two operands
clr.d s0, (cs1), s2 # CHECK: :[[@LINE]]:18: error: invalid operand for instruction
