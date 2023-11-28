# RUN: not llvm-mc -triple riscv32 -mattr=+zcheripurecap,+cap-mode <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK
# RUN: not llvm-mc -triple riscv64 -mattr=+zcheripurecap,+cap-mode <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK 

cgettype    a0, ca0         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cgetsealed  a0, ca0         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cgetoffset  a0, ca0         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cgetflags   a0, ca0         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension  
cunseal     ca0, ca0, ca0   # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
scmode      ca0, ca0, a0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: The zcherihybrid Extension is Enabled
csetoffset  ca0, ca0, a0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ccleartag   ca0, ca0        # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ccopytype   ca0, ca0, ca0   # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ccseal      ca0, ca0, ca0   # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cseal       ca0, ca0, ca0   # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ctoptr      a0, ca0, ca0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cfromptr    ca0, ca0, a0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cinvoke     ca0, ca0        # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
modesw                      # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: The zcherihybrid Extension is Enabled
cclear      1, 0x42         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
fpclear     1, 0x42         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
crrl        a0, a0          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cloadtags   a0, (ca0)       # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
jalr        a0, 42(a0)      # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: Not Capability Mode
jalr.mode   ca0, 0(ca0)     # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: The zcherihybrid Extension is Enabled, Not Capability Mode
scbndsi     ca0, ca0, 33    # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
scbndsi     ca0, ca0, 104   # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
scbndsi     ca0, ca0, 512   # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
lc          ca0, 0(a0)      # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: Not Capability Mode
sc          ca0, 0(a0)      # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: Not Capability Mode
cmv         a0 , a0         # CHECK: <stdin>:[[#@LINE]]:13: error: invalid operand for instruction
cbld        ca0, c0,  ca0   # CHECK: <stdin>:[[#@LINE]]:18: error: invalid operand for instruction
scss        a0 , c0,  ca0   # CHECK: <stdin>:[[#@LINE]]:18: error: invalid operand for instruction
