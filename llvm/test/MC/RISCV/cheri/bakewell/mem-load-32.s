# RUN: llvm-mc %s -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

.option capmode

# CHECK-ASM-AND-OBJ: lw a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x25,0x05,0x00]
lw a0, 0(ca0)

# CHECK-ASM-AND-OBJ: lw a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x25,0x05,0x00]
lw a0, (ca0)

# CHECK-ASM-AND-OBJ: lh a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x15,0x05,0x00]
lh a0, 0(ca0)

# CHECK-ASM-AND-OBJ: lh a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x15,0x05,0x00]
lh a0, (ca0)

# CHECK-ASM-AND-OBJ: lhu a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x55,0x05,0x00]
lhu a0, 0(ca0)

# CHECK-ASM-AND-OBJ: lhu a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x55,0x05,0x00]
lhu a0, (ca0)

# CHECK-ASM-AND-OBJ: lb a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x05,0x05,0x00]
lb a0, 0(ca0)

# CHECK-ASM-AND-OBJ: lb a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x05,0x05,0x00]
lb a0, (ca0)

# CHECK-ASM-AND-OBJ: lbu a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x45,0x05,0x00]
lbu a0, 0(ca0)

# CHECK-ASM-AND-OBJ: lbu a0, 0(ca0)
# CHECK-ASM: encoding: [0x03,0x45,0x05,0x00]
lbu a0, (ca0)

.option nocapmode

# CHECK-ASM-AND-OBJ: lw a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x25,0x05,0x00]
lw a0, 0(a0)

# CHECK-ASM-AND-OBJ: lw a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x25,0x05,0x00]
lw a0, (a0)

# CHECK-ASM-AND-OBJ: lh a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x15,0x05,0x00]
lh a0, 0(a0)

# CHECK-ASM-AND-OBJ: lh a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x15,0x05,0x00]
lh a0, (a0)

# CHECK-ASM-AND-OBJ: lhu a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x55,0x05,0x00]
lhu a0, 0(a0)

# CHECK-ASM-AND-OBJ: lhu a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x55,0x05,0x00]
lhu a0, (a0)

# CHECK-ASM-AND-OBJ: lb a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x05,0x05,0x00]
lb a0, 0(a0)

# CHECK-ASM-AND-OBJ: lb a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x05,0x05,0x00]
lb a0, (a0)

# CHECK-ASM-AND-OBJ: lbu a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x45,0x05,0x00]
lbu a0, 0(a0)

# CHECK-ASM-AND-OBJ: lbu a0, 0(a0)
# CHECK-ASM: encoding: [0x03,0x45,0x05,0x00]
lbu a0, (a0)
