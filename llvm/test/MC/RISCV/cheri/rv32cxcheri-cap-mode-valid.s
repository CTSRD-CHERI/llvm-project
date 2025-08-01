# RUN: llvm-mc %s -triple=riscv32 -mattr=+c,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+c,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+c,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+c,+xcheri,+cap-mode -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+c,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+c,+xcheri,+cap-mode -d - \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lwsp ra, 0(csp)
# CHECK-ASM-SAME: encoding: [0x82,0x40]
c.clwcsp ra, 0(csp)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lwsp ra, 0(csp)
# CHECK-ASM-SAME: encoding: [0x82,0x40]
c.lwsp ra, 0(csp)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.swsp ra, 252(csp)
# CHECK-ASM-SAME: encoding: [0x86,0xdf]
c.cswcsp ra, 252(csp)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.swsp ra, 252(csp)
# CHECK-ASM-SAME: encoding: [0x86,0xdf]
c.swsp ra, 252(csp)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lw a2, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x10,0x41]
c.clw a2, 0(ca0)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lw a2, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x10,0x41]
c.lw a2, 0(ca0)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.sw a5, 124(ca3)
# CHECK-ASM-SAME: encoding: [0xfc,0xde]
c.csw a5, 124(ca3)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.sw a5, 124(ca3)
# CHECK-ASM-SAME: encoding: [0xfc,0xde]
c.sw a5, 124(ca3)

# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.jr ca7
# CHECK-ASM-SAME: encoding: [0x82,0x88]
c.cjr ca7
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.jr ca7
# CHECK-ASM-SAME: encoding: [0x82,0x88]
c.jr ca7
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.jalr ca1
# CHECK-ASM-SAME: encoding: [0x82,0x95]
c.cjalr ca1
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.jalr ca1
# CHECK-ASM-SAME: encoding: [0x82,0x95]
c.jalr ca1

# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.cincoffset16csp csp, -512
# CHECK-ASM-SAME: encoding: [0x01,0x71]
c.cincoffset16csp csp, -512
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.cincoffset16csp csp, -512
# CHECK-ASM-SAME: encoding: [0x01,0x71]
c.addi16sp csp, -512
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.cincoffset16csp csp, 496
# CHECK-ASM-SAME: encoding: [0x7d,0x61]
c.cincoffset16csp csp, 496
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.cincoffset16csp csp, 496
# CHECK-ASM-SAME: encoding: [0x7d,0x61]
c.addi16sp csp, 496
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.cincoffset4cspn ca3, csp, 1020
# CHECK-ASM-SAME: encoding: [0xf4,0x1f]
c.cincoffset4cspn ca3, csp, 1020
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.cincoffset4cspn ca3, csp, 1020
# CHECK-ASM-SAME: encoding: [0xf4,0x1f]
c.addi4spn ca3, csp, 1020
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.cincoffset4cspn ca3, csp, 4
# CHECK-ASM-SAME: encoding: [0x54,0x00]
c.cincoffset4cspn ca3, csp, 4
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.cincoffset4cspn ca3, csp, 4
# CHECK-ASM-SAME: encoding: [0x54,0x00]
c.addi4spn ca3, csp, 4
