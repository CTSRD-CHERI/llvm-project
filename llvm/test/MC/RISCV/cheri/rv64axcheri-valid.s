# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+a,+xcheri < %s \
# RUN:     | llvm-objdump --riscv-no-aliases --mattr=+a,+xcheri -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-OBJ,CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: lr.c ct0, (t1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x10]
lr.c ct0, (t1)
# CHECK-ASM-AND-OBJ: lr.c.aq ct1, (t2)
# CHECK-ASM: encoding: [0x2f,0xc3,0x03,0x14]
lr.c.aq ct1, (t2)
# CHECK-ASM-AND-OBJ: lr.c.rl ct2, (t3)
# CHECK-ASM: encoding: [0xaf,0x43,0x0e,0x12]
lr.c.rl ct2, (t3)
# CHECK-ASM-AND-OBJ: lr.c.aqrl ct3, (t4)
# CHECK-ASM: encoding: [0x2f,0xce,0x0e,0x16]
lr.c.aqrl ct3, (t4)

# CHECK-ASM-AND-OBJ: sc.c t6, ct5, (t4)
# CHECK-ASM: encoding: [0xaf,0xcf,0xee,0x19]
sc.c t6, ct5, (t4)
# CHECK-ASM-AND-OBJ: sc.c.aq t5, ct4, (t3)
# CHECK-ASM: encoding: [0x2f,0x4f,0xde,0x1d]
sc.c.aq t5, ct4, (t3)
# CHECK-ASM-AND-OBJ: sc.c.rl t4, ct3, (t2)
# CHECK-ASM: encoding: [0xaf,0xce,0xc3,0x1b]
sc.c.rl t4, ct3, (t2)
# CHECK-ASM-AND-OBJ: sc.c.aqrl t3, ct2, (t1)
# CHECK-ASM: encoding: [0x2f,0x4e,0x73,0x1e]
sc.c.aqrl t3, ct2, (t1)

# CHECK-ASM-AND-OBJ: amoswap.c ca4, cra, (s0)
# CHECK-ASM: encoding: [0x2f,0x47,0x14,0x08]
amoswap.c ca4, cra, (s0)
# CHECK-ASM-AND-OBJ: amoswap.c.aq ca4, cra, (s0)
# CHECK-ASM: encoding: [0x2f,0x47,0x14,0x0c]
amoswap.c.aq ca4, cra, (s0)
# CHECK-ASM-AND-OBJ: amoswap.c.rl ca4, cra, (s0)
# CHECK-ASM: encoding: [0x2f,0x47,0x14,0x0a]
amoswap.c.rl ca4, cra, (s0)
# CHECK-ASM-AND-OBJ: amoswap.c.aqrl ca4, cra, (s0)
# CHECK-ASM: encoding: [0x2f,0x47,0x14,0x0e]
amoswap.c.aqrl ca4, cra, (s0)

# CHECK-ASM-AND-OBJ: lr.d.ddc ra, (sp)
# CHECK-ASM: encoding: [0xdb,0x00,0x31,0xfb]
lr.d.ddc x1, (x2)
# CHECK-ASM-AND-OBJ: lr.c.ddc cra, (sp)
# CHECK-ASM: encoding: [0xdb,0x00,0x41,0xfb]
lr.c.ddc c1, (x2)

# CHECK-ASM-AND-OBJ: sc.d.ddc ra, (sp)
# CHECK-ASM: encoding: [0xdb,0x09,0x11,0xf8]
sc.d.ddc x1, (x2)
# CHECK-ASM-AND-OBJ: sc.c.ddc cra, (sp)
# CHECK-ASM: encoding: [0x5b,0x0a,0x11,0xf8]
sc.c.ddc c1, (x2)

# CHECK-ASM-AND-OBJ: lr.d.cap ra, (csp)
# CHECK-ASM: encoding: [0xdb,0x00,0xb1,0xfb]
lr.d.cap x1, (c2)
# CHECK-ASM-AND-OBJ: lr.c.cap cra, (csp)
# CHECK-ASM: encoding: [0xdb,0x00,0xc1,0xfb]
lr.c.cap c1, (c2)

# CHECK-ASM-AND-OBJ: sc.d.cap ra, (csp)
# CHECK-ASM: encoding: [0xdb,0x0d,0x11,0xf8]
sc.d.cap x1, (c2)
# CHECK-ASM-AND-OBJ: sc.c.cap cra, (csp)
# CHECK-ASM: encoding: [0x5b,0x0e,0x11,0xf8]
sc.c.cap c1, (c2)
