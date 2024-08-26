# RUN: not llvm-mc -triple riscv32 -mattr=+a,+xcheri,+cap-mode < %s 2>&1 | FileCheck %s

# Final operand must have parentheses
camoswap.w a1, a2, c3 # CHECK: :[[@LINE]]:20: error: expected '(' or optional integer offset
camomin.w a1, a2, 1 # CHECK: :[[@LINE]]:21: error: expected '(' after optional integer offset
camomin.w a1, a2, 1(c3) # CHECK: :[[@LINE]]:19: error: optional integer offset must be 0
clr.w a4, c5 # CHECK: :[[@LINE]]:11: error: expected '(' or optional integer offset
clr.w a4, (a5) # CHECK: :[[@LINE]]:12: error: invalid operand for instruction

# Only .aq, .rl, and .aqrl suffixes are valid
camoxor.w.rlqa a2, a3, (c4) # CHECK: :[[@LINE]]:1: error: unrecognized instruction mnemonic
camoor.w.aq.rl a4, a5, (c6) # CHECK: :[[@LINE]]:1: error: unrecognized instruction mnemonic
camoor.w. a4, a5, (c6) # CHECK: :[[@LINE]]:1: error: unrecognized instruction mnemonic

# lr only takes two operands
clr.w s0, (cs1), s2 # CHECK: :[[@LINE]]:18: error: invalid operand for instruction

# Note: errors for use of RV64A instructions for RV32 are checked in
# rv64axcheri-cap-mode-valid.s
