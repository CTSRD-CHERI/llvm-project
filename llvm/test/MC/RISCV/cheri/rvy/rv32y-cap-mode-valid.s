# RUN: llvm-mc -triple=riscv32 -mattr=+y,+cap-mode -riscv-no-aliases -show-encoding < %s \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -triple=riscv64 -mattr=+y,+cap-mode -riscv-no-aliases -show-encoding < %s \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+y,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+y,+cap-mode -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+y,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+y,+cap-mode -d - \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: ly ca2, 17(ca0)
# CHECK-ASM-SAME: encoding: [0x0f,0x46,0x15,0x01]
ly ca2, 17(ca0)
# CHECK-ASM-AND-OBJ: ly ca2, 17(ca0)
# CHECK-ASM-SAME: encoding: [0x0f,0x46,0x15,0x01]
lc ca2, 17(ca0)
# CHECK-ASM-AND-OBJ: ly ca2, 17(ca0)
# CHECK-ASM-SAME: encoding: [0x0f,0x46,0x15,0x01]
clc ca2, 17(ca0)

# CHECK-ASM-AND-OBJ: ly ca2, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x0f,0x46,0x05,0x00]
ly ca2, (ca0)
# CHECK-ASM-AND-OBJ: ly ca2, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x0f,0x46,0x05,0x00]
lc ca2, (ca0)
# CHECK-ASM-AND-OBJ: ly ca2, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x0f,0x46,0x05,0x00]
clc ca2, (ca0)

# CHECK-ASM-AND-OBJ: sy ca5, 25(ca3)
# CHECK-ASM-SAME: encoding: [0xa3,0xcc,0xf6,0x00]
sy ca5, 25(ca3)
# CHECK-ASM-AND-OBJ: sy ca5, 25(ca3)
# CHECK-ASM-SAME: encoding: [0xa3,0xcc,0xf6,0x00]
sc ca5, 25(ca3)
# CHECK-ASM-AND-OBJ: sy ca5, 25(ca3)
# CHECK-ASM-SAME: encoding: [0xa3,0xcc,0xf6,0x00]
csc ca5, 25(ca3)

# CHECK-ASM-AND-OBJ: sy ca5, 0(ca3)
# CHECK-ASM-SAME: encoding: [0x23,0xc0,0xf6,0x00]
sy ca5, (ca3)
# CHECK-ASM-AND-OBJ: sy ca5, 0(ca3)
# CHECK-ASM-SAME: encoding: [0x23,0xc0,0xf6,0x00]
sc ca5, (ca3)
# CHECK-ASM-AND-OBJ: sy ca5, 0(ca3)
# CHECK-ASM-SAME: encoding: [0x23,0xc0,0xf6,0x00]
csc ca5, (ca3)
