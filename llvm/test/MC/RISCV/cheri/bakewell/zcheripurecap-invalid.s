# RUN: not llvm-mc -triple riscv32 -mattr=+zcheripurecap,+cap-mode <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK
# RUN: not llvm-mc -triple riscv64 -mattr=+zcheripurecap,+cap-mode <%s 2>&1 \
# RUN:     | FileCheck %s -check-prefixes=CHECK

cgettype    a0, ca0          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cgetsealed  a0, ca0          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cgetoffset  a0, ca0          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cgetflags   a0, ca0          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cunseal     ca0, ca0, ca0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
scmode      ca0, ca0, a0     # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: 'zcherihybrid' (Backwards compatiblity for 'zcheripurecap' with RISCV)
csetoffset  ca0, ca0, a0     # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ccleartag   ca0, ca0         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ccopytype   ca0, ca0, ca0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
ccseal      ca0, ca0, ca0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cseal       ca0, ca0, ca0    # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cinvoke     ca0, ca0         # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
modesw.cap                   # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: 'zcherihybrid' (Backwards compatiblity for 'zcheripurecap' with RISCV)
modesw.int                   # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: 'zcherihybrid' (Backwards compatiblity for 'zcheripurecap' with RISCV)
gcmode      a0, ca0          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: 'zcherihybrid' (Backwards compatiblity for 'zcheripurecap' with RISCV)
cclear      1, 0x42          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
fpclear     1, 0x42          # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
crrl        a0, a0           # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
cloadtags   a0, (ca0)        # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: CHERI Extension
jalr        a0, 42(a0)       # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: Integer Pointer Mode
jalr.mode   ca0, 0(ca0)      # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: 'zcherihybrid' (Backwards compatiblity for 'zcheripurecap' with RISCV), Integer Pointer Mode
# With scbndsi we sadly don't get the warning for smaller values as the asm
# parser will try and parse CSetBounds and will succeed - warning instead about
# missing xcheri and not the out of range value. Once xcheri is removed along
# with its instructions we can reenable these tests with their original
# values. (33, 104, 512)
scbndsi     ca0, ca0, 0xffff # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
scbndsi     ca0, ca0, 0xffff # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
scbndsi     ca0, ca0, 0xffff # CHECK: <stdin>:[[#@LINE]]:23: error: immediate must be an integer in range [0, 31] or be a multiple of 16 in the range [0, 496]
lc          ca0, 0(a0)       # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: Integer Pointer Mode
sc          ca0, 0(a0)       # CHECK: <stdin>:[[#@LINE]]:1:  error: instruction requires the following: Integer Pointer Mode
cmv         a0 , a0          # CHECK: <stdin>:[[#@LINE]]:13: error: invalid operand for instruction
cbld        ca0, c0,  ca0    # CHECK: <stdin>:[[#@LINE]]:18: error: invalid operand for instruction
scss        a0 , c0,  ca0    # CHECK: <stdin>:[[#@LINE]]:18: error: invalid operand for instruction
gctype      ca0, ca0         # CHECK: <stdin>:[[#@LINE]]:13: error: invalid operand for instruction
gctype      a0, a0           # CHECK: <stdin>:[[#@LINE]]:17: error: invalid operand for instruction
