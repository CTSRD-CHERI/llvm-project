# RUN: llvm-mc %s -triple=riscv64 -mattr=+c,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+c,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+c,+xcheri,+cap-mode -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

## Different encoding between RV64 and RV32 (see rv32cxcheri-cap-mode-only-valid.s)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lcsp cra, 32(csp)
# CHECK-ASM-SAME: encoding: [0x82,0x30]
c.clccsp cra, 32(csp)
# CHECK-ASM-AND-OBJ: {{[[:<:]]}}c.lcsp cra, 32(csp)
# CHECK-ASM-SAME: encoding: [0x82,0x30]
c.lcsp cra, 32(csp)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.scsp cra, 256(csp)
# CHECK-ASM-SAME: encoding: [0x06,0xa2]
c.csccsp cra, 256(csp)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.scsp cra, 256(csp)
# CHECK-ASM-SAME: encoding: [0x06,0xa2]
c.scsp cra, 256(csp)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.lc ca2, 16(ca0)
# CHECK-ASM-SAME: encoding: [0x10,0x29]
c.clc ca2, 16(ca0)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.lc ca2, 16(ca0)
# CHECK-ASM-SAME: encoding: [0x10,0x29]
c.lc ca2, 16(ca0)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.sc ca5, 128(ca3)
# CHECK-ASM-SAME: encoding: [0xdc,0xa2]
c.csc ca5, 128(ca3)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.sc ca5, 128(ca3)
# CHECK-ASM-SAME: encoding: [0xdc,0xa2]
c.sc ca5, 128(ca3)
## *D operations need RV64C:
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.ldsp ra, 40(csp)
# CHECK-ASM-SAME: encoding: [0xa2,0x70]
c.cldcsp ra, 40(csp)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.ldsp ra, 40(csp)
# CHECK-ASM-SAME: encoding: [0xa2,0x70]
c.ldsp ra, 40(csp)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.sdsp ra, 256(csp)
# CHECK-ASM-SAME: encoding: [0x06,0xe2]
c.sdsp ra, 256(csp)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.sdsp ra, 256(csp)
# CHECK-ASM-SAME: encoding: [0x06,0xe2]
c.sdsp ra, 256(csp)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.ld a2, 16(ca0)
# CHECK-ASM-SAME: encoding: [0x10,0x69]
c.cld a2, 16(ca0)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.ld a2, 16(ca0)
# CHECK-ASM-SAME: encoding: [0x10,0x69]
c.ld a2, 16(ca0)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.sd a5, 128(ca3)
# CHECK-ASM-SAME: encoding: [0xdc,0xe2]
c.csd a5, 128(ca3)
# CHECK-ASM-AND-OBJ-NEXT: {{[[:<:]]}}c.sd a5, 128(ca3)
# CHECK-ASM-SAME: encoding: [0xdc,0xe2]
c.sd a5, 128(ca3)
