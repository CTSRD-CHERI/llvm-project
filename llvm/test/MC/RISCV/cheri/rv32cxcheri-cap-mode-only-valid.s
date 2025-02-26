# RUN: llvm-mc %s -triple=riscv32 -mattr=+c,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+c,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+c,+xcheri,+cap-mode -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-OBJ,CHECK-ASM-AND-OBJ %s

## Different encoding between RV64 and RV32 (see rv64cxcheri-cap-mode-only-valid.s)

# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lcsp cra, 40(csp)
# CHECK-ASM-SAME: encoding: [0xa2,0x70]
c.clccsp cra, 40(csp)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lcsp cra, 40(csp)
# CHECK-ASM-SAME: encoding: [0xa2,0x70]
c.lcsp cra, 40(csp)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.scsp cra, 256(csp)
# CHECK-ASM-SAME: encoding: [0x06,0xe2]
c.csccsp cra, 256(csp)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.scsp cra, 256(csp)
# CHECK-ASM-SAME: encoding: [0x06,0xe2]
c.scsp cra, 256(csp)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lc ca2, 16(ca0)
# CHECK-ASM-SAME: encoding: [0x10,0x69]
c.clc ca2, 16(ca0)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lc ca2, 16(ca0)
# CHECK-ASM-SAME: encoding: [0x10,0x69]
c.lc ca2, 16(ca0)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.sc ca5, 128(ca3)
# CHECK-ASM-SAME: encoding: [0xdc,0xe2]
c.csc ca5, 128(ca3)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.sc ca5, 128(ca3)
# CHECK-ASM-SAME: encoding: [0xdc,0xe2]
c.sc ca5, 128(ca3)

## C.JAL is only defined for RV32C:
# CHECK-OBJ: {{[[:<:]]}}c.jal 0x80e
# CHECK-ASM: {{[[:<:]]}}c.jal 2046
# CHECK-ASM-SAME: encoding: [0xfd,0x2f]
c.cjal 2046
# CHECK-OBJ: {{[[:<:]]}}c.jal 0x810
# CHECK-ASM: {{[[:<:]]}}c.jal 2046
# CHECK-ASM-SAME: encoding: [0xfd,0x2f]
c.jal 2046
