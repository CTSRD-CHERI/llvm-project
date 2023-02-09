# RUN: not llvm-mc -triple riscv64 -mattr=+c,+xcheri,+cap-mode -filetype=null -riscv-no-aliases -show-encoding < %s 2>&1\
# RUN:     | FileCheck %s --check-prefixes=CHECK-RV64 --implicit-check-not="error:"
# RUN: not llvm-mc -triple riscv64 -mattr=+xcheri,+cap-mode -filetype=null -riscv-no-aliases -show-encoding < %s 2>&1 \
# RUN:     | FileCheck %s --check-prefixes=CHECK-RV64-NO-C --implicit-check-not="error:"
# RUN: not llvm-mc -triple riscv32 -mattr=+c,+xcheri,+cap-mode -filetype=null -riscv-no-aliases -show-encoding < %s 2>&1 \
# RUN:     | FileCheck %s --check-prefixes=CHECK-RV32 --implicit-check-not="error:"

## Invalid immediates
## FIXME: ideally this should report an immediate value error rather than only valid for RV64!
c.clccsp cra, 40(csp)
# CHECK-RV64: <stdin>:[[#@LINE-1]]:1: error: instruction requires the following: RV32I Base Instruction Set{{$}}
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-2]]:1: error: instruction requires the following: 'C' (Compressed Instructions), RV32I Base Instruction Set{{$}}

c.csccsp cra, 1024(csp)
# CHECK-RV32: <stdin>:[[#@LINE-1]]:15: error: immediate must be a multiple of 8 bytes in the range [0, 504]
# CHECK-RV64: <stdin>:[[#@LINE-2]]:15: error: immediate must be a multiple of 16 bytes in the range [0, 1008]
## FIXME: With missing C this should report "instruction requires the following"
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-4]]:15: error: invalid operand for instruction

## FIXME: ideally this should report an immediate value error rather than only valid for RV64!
c.clc ca2, 8(ca0)
# CHECK-RV64: <stdin>:[[#@LINE-1]]:1: error: instruction requires the following: RV32I Base Instruction Set{{$}}
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-2]]:1: error: instruction requires the following: 'C' (Compressed Instructions), RV32I Base Instruction Set{{$}}

## FIXME: ideally this should report an immediate value error rather than only valid for RV64!
c.csc ca5, 24(ca3)
# CHECK-RV64: <stdin>:[[#@LINE-1]]:1: error: instruction requires the following: RV32I Base Instruction Set{{$}}
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-2]]:1: error: instruction requires the following: 'C' (Compressed Instructions), RV32I Base Instruction Set{{$}}

c.cldcsp ra, 7(csp)
# CHECK-RV64: <stdin>:[[#@LINE-1]]:14: error: immediate must be a multiple of 8 bytes in the range [0, 504]
## FIXME: With missing C this should report "instruction requires the following"
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-3]]:14: error: invalid operand for instruction
# CHECK-RV32: <stdin>:[[#@LINE-4]]:14: error: invalid operand for instruction

c.csdcsp ra, 15(csp)
# CHECK-RV64: <stdin>:[[#@LINE-1]]:14: error: immediate must be a multiple of 8 bytes in the range [0, 504]
## FIXME: With missing C this should report "instruction requires the following"
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-3]]:14: error: invalid operand for instruction
# CHECK-RV32: <stdin>:[[#@LINE-4]]:14: error: invalid operand for instruction

c.cld a2, 2(ca0)
# CHECK-RV64: <stdin>:[[#@LINE-1]]:11: error: immediate must be a multiple of 8 bytes in the range [0, 248]
## FIXME: With missing C this should report "instruction requires the following"
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-3]]:11: error: invalid operand for instruction
# CHECK-RV32: <stdin>:[[#@LINE-4]]:11: error: invalid operand for instruction

c.csd a5, 4(ca3)
# CHECK-RV64: <stdin>:[[#@LINE-1]]:11: error: immediate must be a multiple of 8 bytes in the range [0, 248]
## FIXME: With missing C this should report "instruction requires the following"
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-3]]:11: error: invalid operand for instruction
# CHECK-RV32: <stdin>:[[#@LINE-4]]:11: error: invalid operand for instruction

## C.JAL is only defined for RV32C:
c.cjal 2046
# CHECK-RV64: <stdin>:[[#@LINE-1]]:1: error: instruction requires the following: RV32I Base Instruction Set{{$}}
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-2]]:1: error: instruction requires the following: 'C' (Compressed Instructions), RV32I Base Instruction Set{{$}}
