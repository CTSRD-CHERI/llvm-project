# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+a,+xcheri < %s \
# RUN:     | llvm-objdump --riscv-no-aliases --mattr=+a,+xcheri -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-OBJ,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+a,+xcheri < %s \
# RUN:     | llvm-objdump --riscv-no-aliases --mattr=+a,+xcheri -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-OBJ,CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: lr.b.ddc ra, (sp)
# CHECK-ASM: encoding: [0xdb,0x00,0x01,0xfb]
lr.b.ddc x1, (x2)
# CHECK-ASM-AND-OBJ: lr.h.ddc ra, (sp)
# CHECK-ASM: encoding: [0xdb,0x00,0x11,0xfb]
lr.h.ddc x1, (x2)
# CHECK-ASM-AND-OBJ: lr.w.ddc ra, (sp)
# CHECK-ASM: encoding: [0xdb,0x00,0x21,0xfb]
lr.w.ddc x1, (x2)

# CHECK-ASM-AND-OBJ: sc.b.ddc ra, (sp)
# CHECK-ASM: encoding: [0x5b,0x08,0x11,0xf8]
sc.b.ddc x1, (x2)
# CHECK-ASM-AND-OBJ: sc.h.ddc ra, (sp)
# CHECK-ASM: encoding: [0xdb,0x08,0x11,0xf8]
sc.h.ddc x1, (x2)
# CHECK-ASM-AND-OBJ: sc.w.ddc ra, (sp)
# CHECK-ASM: encoding: [0x5b,0x09,0x11,0xf8]
sc.w.ddc x1, (x2)

# CHECK-ASM-AND-OBJ: lr.b.cap ra, (csp)
# CHECK-ASM: encoding: [0xdb,0x00,0x81,0xfb]
lr.b.cap x1, (c2)
# CHECK-ASM-AND-OBJ: lr.h.cap ra, (csp)
# CHECK-ASM: encoding: [0xdb,0x00,0x91,0xfb]
lr.h.cap x1, (c2)
# CHECK-ASM-AND-OBJ: lr.w.cap ra, (csp)
# CHECK-ASM: encoding: [0xdb,0x00,0xa1,0xfb]
lr.w.cap x1, (c2)

# CHECK-ASM-AND-OBJ: sc.b.cap ra, (csp)
# CHECK-ASM: encoding: [0x5b,0x0c,0x11,0xf8]
sc.b.cap x1, (c2)
# CHECK-ASM-AND-OBJ: sc.h.cap ra, (csp)
# CHECK-ASM: encoding: [0xdb,0x0c,0x11,0xf8]
sc.h.cap x1, (c2)
# CHECK-ASM-AND-OBJ: sc.w.cap ra, (csp)
# CHECK-ASM: encoding: [0x5b,0x0d,0x11,0xf8]
sc.w.cap x1, (c2)
