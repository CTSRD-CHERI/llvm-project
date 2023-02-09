# RUN: not llvm-mc -triple riscv32 -mattr=+c,+xcheri,+xcheri-rvc,+cap-mode -filetype=null -riscv-no-aliases -show-encoding < %s 2>&1 \
# RUN:     | FileCheck %s --check-prefixes=CHECK,CHECK-RV32,CHECK-RV32-C --implicit-check-not="error:"
# RUN: not llvm-mc -triple riscv32 -mattr=+xcheri,+cap-mode -filetype=null -riscv-no-aliases -show-encoding < %s 2>&1 \
# RUN:     | FileCheck %s --check-prefixes=CHECK,CHECK-RV32,CHECK-RV32-NO-C --implicit-check-not="error:"
# RUN: not llvm-mc -triple riscv64 -mattr=+c,+xcheri,+xcheri-rvc,+cap-mode -filetype=null -riscv-no-aliases -show-encoding < %s 2>&1\
# RUN:     | FileCheck %s --check-prefixes=CHECK,CHECK-RV64-C --implicit-check-not="error:"
# RUN: not llvm-mc -triple riscv64 -mattr=+xcheri,+cap-mode -filetype=null -riscv-no-aliases -show-encoding < %s 2>&1 \
# RUN:     | FileCheck %s --check-prefixes=CHECK,CHECK-RV64-NO-C --implicit-check-not="error:"

## C.JAL is only defined for RV32C:
c.cjal 2046
# CHECK-RV64-C: <stdin>:[[#@LINE-1]]:1: error: instruction requires the following: RV32I Base Instruction Set{{$}}
# CHECK-RV32-NO-C: <stdin>:[[#@LINE-2]]:1: error: instruction requires the following: CHERI RVC Instructions, 'C' (Compressed Instructions){{$}}
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-3]]:1: error: instruction requires the following: CHERI RVC Instructions, 'C' (Compressed Instructions), RV32I Base Instruction Set{{$}}

## C.CSD needs RV64C:
c.csd a5, 124(ca3)
# CHECK-RV32: <stdin>:[[#@LINE-1]]:11: error: invalid operand for instruction
# CHECK-RV64-C: <stdin>:[[#@LINE-2]]:11: error: immediate must be a multiple of 8 bytes in the range [0, 248]
# TODO-RV64-NO-C: <stdin>:[[#@LINE-3]]:11: error: instruction requires the following: CHERI RVC Instructions, 'C' (Compressed Instructions){{$}}
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-4]]:11: error: invalid operand for instruction

## misaligned immediate:
c.csw a5, 1(ca3)
# CHECK-RV32-C: <stdin>:[[#@LINE-1]]:11: error: immediate must be a multiple of 4 bytes in the range [0, 124]
# TODO-RV32-NO-C: <stdin>:[[#@LINE-2]]:11: error: instruction requires the following: CHERI RVC Instructions, 'C' (Compressed Instructions){{$}}
# CHECK-RV32-NO-C: <stdin>:[[#@LINE-3]]:11: error: invalid operand for instruction
# CHECK-RV64-C: <stdin>:[[#@LINE-4]]:11: error: immediate must be a multiple of 4 bytes in the range [0, 124]
# TODO-RV64-NO-C: <stdin>:[[#@LINE-5]]:11: error: instruction requires the following: CHERI RVC Instructions, 'C' (Compressed Instructions){{$}}
# CHECK-RV64-NO-C: <stdin>:[[#@LINE-6]]:11: error: invalid operand for instruction

# Bad operands:
c.cjalr a1
# CHECK: <stdin>:[[#@LINE-1]]:9: error: invalid operand for instruction
c.cjr a1
# CHECK: <stdin>:[[#@LINE-1]]:7: error: invalid operand for instruction
c.csc a5, 16(ca3)
# CHECK: <stdin>:[[#@LINE-1]]:7: error: invalid operand for instruction
c.csc ca5, 16(a3)
# CHECK: <stdin>:[[#@LINE-1]]:15: error: invalid operand for instruction
