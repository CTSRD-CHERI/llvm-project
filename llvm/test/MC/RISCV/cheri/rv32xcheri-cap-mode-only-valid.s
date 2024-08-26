# RUN: llvm-mc %s -triple=riscv32 -mattr=+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+xcheri,+cap-mode -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# Capability load/store uses different encodings for RV32 vs RV64

# CHECK-ASM-AND-OBJ: clc ca2, 17(ca0)
# CHECK-ASM-SAME: encoding: [0x03,0x36,0x15,0x01]
clc ca2, 17(ca0)
# CHECK-ASM-AND-OBJ: csc ca5, 25(ca3)
# CHECK-ASM-SAME: encoding: [0xa3,0xbc,0xf6,0x00]
csc ca5, 25(ca3)

