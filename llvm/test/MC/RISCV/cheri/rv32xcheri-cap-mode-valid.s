# RUN: llvm-mc %s -triple=riscv32 -mattr=+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+xcheri,+cap-mode -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+xcheri,+cap-mode -d - \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ:  clb a2, 17(ca0)
# CHECK-ASM-SAME: encoding: [0x03,0x06,0x15,0x01]
clb a2, 17(ca0)
# CHECK-ASM-AND-OBJ: csb a5, 25(ca3)
# CHECK-ASM-SAME: encoding: [0xa3,0x8c,0xf6,0x00]
csb a5, 25(ca3)

# CHECK-ASM-AND-OBJ: clh a2, 17(ca0)
# CHECK-ASM-SAME: encoding: [0x03,0x16,0x15,0x01]
clh a2, 17(ca0)
# CHECK-ASM-AND-OBJ: csh a5, 25(ca3)
# CHECK-ASM-SAME: encoding: [0xa3,0x9c,0xf6,0x00]
csh a5, 25(ca3)

# CHECK-ASM-AND-OBJ: clw ra, 17(csp)
# CHECK-ASM-SAME: encoding: [0x83,0x20,0x11,0x01]
clw ra, 17(csp)
# CHECK-ASM-AND-OBJ: csw ra, 25(csp)
# CHECK-ASM-SAME: encoding: [0xa3,0x2c,0x11,0x00]
csw ra, 25(csp)

# CHECK-ASM-AND-OBJ: clbu ra, 17(csp)
# CHECK-ASM-SAME: encoding: [0x83,0x40,0x11,0x01]
clbu ra, 17(csp)

# CHECK-ASM-AND-OBJ: clhu ra, 17(csp)
# CHECK-ASM-SAME: encoding: [0x83,0x50,0x11,0x01]
clhu ra, 17(csp)
