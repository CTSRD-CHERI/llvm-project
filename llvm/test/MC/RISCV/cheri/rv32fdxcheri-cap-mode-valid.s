# RUN: llvm-mc %s -triple=riscv32 -mattr=+xcheri,+cap-mode,+f,+d  -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+xcheri,+cap-mode,+f,+d < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+xcheri,+cap-mode,+f,+d  -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+xcheri,+cap-mode,+f,+d  -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+xcheri,+cap-mode,+f,+d < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+xcheri,+cap-mode,+f,+d  -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s


# CHECK-ASM-AND-OBJ: cfsd fa0, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x27,0x34,0xa5,0x02]
cfsd fa0, 40(ca0)
# CHECK-ASM-AND-OBJ: cfsd fa0, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x27,0x34,0xa5,0x02]
fsd fa0, 40(ca0)
# CHECK-ASM-AND-OBJ: cfld fa0, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x07,0x35,0x85,0x02]
cfld fa0, 40(ca0)
# CHECK-ASM-AND-OBJ: cfld fa0, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x07,0x35,0x85,0x02]
cfld fa0, 40(ca0)

# CHECK-ASM-AND-OBJ: cfsw ft1, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x27,0x24,0x15,0x02]
cfsw ft1, 40(ca0)
# CHECK-ASM-AND-OBJ: cfsw ft1, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x27,0x24,0x15,0x02]
fsw ft1, 40(ca0)
# CHECK-ASM-AND-OBJ: cflw ft1, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x87,0x20,0x85,0x02]
cflw ft1, 40(ca0)
# CHECK-ASM-AND-OBJ: cflw ft1, 40(ca0)
# CHECK-ASM-SAME: encoding: [0x87,0x20,0x85,0x02]
flw ft1, 40(ca0)

# CHECK-ASM-AND-OBJ: cfsd fa0, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x27,0x30,0xa5,0x00]
cfsd fa0, (ca0)
# CHECK-ASM-AND-OBJ: cfsd fa0, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x27,0x30,0xa5,0x00]
fsd fa0, (ca0)
# CHECK-ASM-AND-OBJ: cfld fa0, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x07,0x35,0x05,0x00]
cfld fa0, (ca0)
# CHECK-ASM-AND-OBJ: cfld fa0, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x07,0x35,0x05,0x00]
fld fa0, (ca0)

# CHECK-ASM-AND-OBJ: cfsw ft1, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x27,0x20,0x15,0x00]
cfsw ft1, (ca0)
# CHECK-ASM-AND-OBJ: cfsw ft1, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x27,0x20,0x15,0x00]
fsw ft1, (ca0)
# CHECK-ASM-AND-OBJ: cflw ft1, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x87,0x20,0x05,0x00]
cflw ft1, (ca0)
# CHECK-ASM-AND-OBJ: cflw ft1, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x87,0x20,0x05,0x00]
flw ft1, (ca0)
