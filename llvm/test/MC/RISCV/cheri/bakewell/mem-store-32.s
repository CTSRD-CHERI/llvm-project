# RUN: llvm-mc %s -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

.option capmode

# CHECK-ASM-AND-OBJ: sw a0, 0(ca0)
# CHECK-ASM: encoding: [0x23,0x20,0xa5,0x00]
sw a0, 0(ca0)

# CHECK-ASM-AND-OBJ: sw a0, 0(ca0)
# CHECK-ASM: encoding: [0x23,0x20,0xa5,0x00]
sw a0, (ca0)

# CHECK-ASM-AND-OBJ: sh a0, 0(ca0)
# CHECK-ASM: encoding: [0x23,0x10,0xa5,0x00]
sh a0, 0(ca0)

# CHECK-ASM-AND-OBJ: sh a0, 0(ca0)
# CHECK-ASM: encoding: [0x23,0x10,0xa5,0x00]
sh a0, (ca0)

# CHECK-ASM-AND-OBJ: sb a0, 0(ca0)
# CHECK-ASM: encoding: [0x23,0x00,0xa5,0x00]
sb a0, 0(ca0)

# CHECK-ASM-AND-OBJ: sb a0, 0(ca0)
# CHECK-ASM: encoding: [0x23,0x00,0xa5,0x00]
sb a0, (ca0)

.option nocapmode

# CHECK-ASM-AND-OBJ: sw a0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x20,0xa5,0x00]
sw a0, 0(a0)

# CHECK-ASM-AND-OBJ: sw a0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x20,0xa5,0x00]
sw a0, (a0)

# CHECK-ASM-AND-OBJ: sh a0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x10,0xa5,0x00]
sh a0, 0(a0)

# CHECK-ASM-AND-OBJ: sh a0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x10,0xa5,0x00]
sh a0, (a0)

# CHECK-ASM-AND-OBJ: sb a0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x00,0xa5,0x00]
sb a0, 0(a0)

# CHECK-ASM-AND-OBJ: sb a0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x00,0xa5,0x00]
sb a0, (a0)
