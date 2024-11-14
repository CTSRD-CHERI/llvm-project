# RUN: not llvm-mc -triple=riscv32 -mattr=zcb,+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding %s 2>&1 \
# RUN:     | FileCheck -check-prefixes=CHECK-ERROR %s
# RUN: not llvm-mc -triple=riscv64 -mattr=zcb,+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding < %s 2>&1 \
# RUN:     | FileCheck -check-prefixes=CHECK-ERROR %s

# CHECK-ERROR: error: immediate must be an integer in the range [0, 3]
c.lbu a5, 10(ca4)

# CHECK-ERROR: error: immediate must be one of [0, 2]
c.lhu a5, 10(ca4)

# CHECK-ERROR: error: immediate must be one of [0, 2]
c.lh a5, 10(ca4)

# CHECK-ERROR: error: immediate must be an integer in the range [0, 3]
c.sb a5, 10(ca4)

# CHECK-ERROR: error: immediate must be one of [0, 2]
c.sh a5, 10(ca4)

# CHECK-ERROR: error: instruction requires the following: Integer Pointer Mode
c.lbu a5, 1(a4)

# CHECK-ERROR: error: instruction requires the following: Integer Pointer Mode
c.lhu a5, 0(a4)

# CHECK-ERROR: error: instruction requires the following: Integer Pointer Mode
c.lh a5, 0(a4)

# CHECK-ERROR: error: instruction requires the following: Integer Pointer Mode
c.sb a5, 1(a4)

# CHECK-ERROR: error: instruction requires the following: Integer Pointer Mode
c.sh a5, 0(a4)
