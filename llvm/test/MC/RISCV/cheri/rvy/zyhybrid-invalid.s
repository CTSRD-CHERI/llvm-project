# RUN: not llvm-mc -triple riscv32 -mattr=+y,+zyhybrid <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK
# RUN: not llvm-mc -triple riscv64 -mattr=+y,+zyhybrid <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK

cgetsealed  a0, ca0          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cgetflags   a0, ca0          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cunseal     ca0, ca0, ca0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
csetoffset  ca0, ca0, a0     # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ccleartag   ca0, ca0         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ccopytype   ca0, ca0, ca0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ccseal      ca0, ca0, ca0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cseal       ca0, ca0, ca0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cinvoke     ca0, ca0         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cclear      1, 0x42          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
fpclear     1, 0x42          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
crrl        a0, a0           # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cloadtags   a0, (ca0)        # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
jalr        ca0, 42(ca0)     # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: Capability Mode

ybndsiw     ca0, ca0, 33     # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
ybndsiw     ca0, ca0, 104    # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
ybndsiw     ca0, ca0, 512    # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
ybndsiw     ca0, ca0, sz     # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
cmv         a0 , a0          # CHECK: <stdin>:[[#@LINE]]:13: error: invalid operand for instruction
cbld        ca0, c0,  ca0    # CHECK: <stdin>:[[#@LINE]]:18: error: invalid operand for instruction
scss        a0 , c0,  ca0    # CHECK: <stdin>:[[#@LINE]]:18: error: invalid operand for instruction
gctype      ca0, ca0         # CHECK: <stdin>:[[#@LINE]]:13: error: invalid operand for instruction
gctype      a0, a0           # CHECK: <stdin>:[[#@LINE]]:17: error: invalid operand for instruction
