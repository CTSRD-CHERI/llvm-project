# RUN: llvm-mc -triple=riscv32 -mattr=+xcheri,+cap-mode -riscv-no-aliases -show-encoding < %s \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -triple=riscv64 -mattr=+xcheri,+cap-mode -riscv-no-aliases -show-encoding < %s \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+xcheri,+cap-mode -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ,CHECK-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+xcheri,+cap-mode -d - \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM-AND-OBJ,CHECK-OBJ %s

## Same test again without the "c" prefix on all lines
# RUN: sed -e 's/^c//' < %s > %t.s
# RUN: llvm-mc -triple=riscv32 -mattr=+xcheri,+cap-mode -riscv-no-aliases -show-encoding < %t.s \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -triple=riscv64 -mattr=+xcheri,+cap-mode -riscv-no-aliases -show-encoding < %t.s \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+xcheri,+cap-mode < %t.s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+xcheri,+cap-mode -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ,CHECK-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+xcheri,+cap-mode < %t.s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+xcheri,+cap-mode -d - \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM-AND-OBJ,CHECK-OBJ %s

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

# CHECK-ASM-AND-OBJ:  clb a2, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x03,0x06,0x05,0x00]
clb a2, (ca0)
# CHECK-ASM-AND-OBJ: csb a5, 0(ca3)
# CHECK-ASM-SAME: encoding: [0x23,0x80,0xf6,0x00]
csb a5, (ca3)

# CHECK-ASM-AND-OBJ: clh a2, 0(ca0)
# CHECK-ASM-SAME: encoding: [0x03,0x16,0x05,0x00]
clh a2, (ca0)
# CHECK-ASM-AND-OBJ: csh a5, 0(ca3)
# CHECK-ASM-SAME: encoding: [0x23,0x90,0xf6,0x00]
csh a5, (ca3)

# CHECK-ASM-AND-OBJ: clw ra, 0(csp)
# CHECK-ASM-SAME: encoding: [0x83,0x20,0x01,0x00]
clw ra, (csp)
# CHECK-ASM-AND-OBJ: csw ra, 0(csp)
# CHECK-ASM-SAME: encoding: [0x23,0x20,0x11,0x00]
csw ra, (csp)

# CHECK-ASM-AND-OBJ: clbu ra, 0(csp)
# CHECK-ASM-SAME: encoding: [0x83,0x40,0x01,0x00]
clbu ra, (csp)

# CHECK-ASM-AND-OBJ: clhu ra, 0(csp)
# CHECK-ASM-SAME: encoding: [0x83,0x50,0x01,0x00]
clhu ra, (csp)


# Jump instructions
# CHECK-ASM: cjal cnull, 16
# CHECK-OBJ: cjal cnull, 0x50 <.text+0x50>
# CHECK-ASM-SAME: encoding: [0x6f,0x00,0x00,0x01]
cj 16
# CHECK-ASM: cjal cra, 16
# CHECK-OBJ: cjal cra, 0x54 <.text+0x54>
# CHECK-ASM-SAME: encoding: [0xef,0x00,0x00,0x01]
cjal 16
# CHECK-ASM-AND-OBJ: cjalr cnull, 0(cgp)
# CHECK-ASM-SAME: encoding: [0x67,0x80,0x01,0x00]
cjr c3
# CHECK-ASM-AND-OBJ: cjalr cnull, 2(cgp)
# CHECK-ASM-SAME: encoding: [0x67,0x80,0x21,0x00]
cjr 2(c3)
# CHECK-ASM-AND-OBJ: cjalr cra, 0(cgp)
# CHECK-ASM-SAME: encoding: [0xe7,0x80,0x01,0x00]
cjalr c3
# CHECK-ASM-AND-OBJ:  cjalr cra, 2(cgp)
# CHECK-ASM-SAME:encoding: [0xe7,0x80,0x21,0x00]
cjalr 2(c3)
# CHECK-ASM-AND-OBJ: cjalr cra, 0(cgp)
# CHECK-ASM-SAME: encoding: [0xe7,0x80,0x01,0x00]
cjalr c1, c3
# CHECK-ASM-AND-OBJ: cjalr cnull, 0(cra)
# CHECK-ASM-SAME: encoding: [0x67,0x80,0x00,0x00]
cret
# CHECK-ASM-AND-OBJ: cjalr cnull, 16(cgp)
# CHECK-ASM-SAME: encoding: [0x67,0x80,0x01,0x01]
cjr c3, 16
# CHECK-ASM-AND-OBJ: cjalr cra, 16(cgp)
# CHECK-ASM-SAME: encoding: [0xe7,0x80,0x01,0x01]
cjalr c3, 16
# CHECK-ASM-AND-OBJ: cjalr cra, 16(cgp)
# CHECK-ASM-SAME: encoding: [0xe7,0x80,0x01,0x01]
cjalr c1, c3, 16
