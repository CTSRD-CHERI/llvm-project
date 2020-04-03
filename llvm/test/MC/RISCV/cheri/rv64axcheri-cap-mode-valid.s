# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+a,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+a,+xcheri,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-OBJ,CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: clr.d t0, (ct1)
# CHECK-ASM: encoding: [0xaf,0x32,0x03,0x10]
clr.d t0, (ct1)
# CHECK-ASM-AND-OBJ: clr.d.aq t1, (ct2)
# CHECK-ASM: encoding: [0x2f,0xb3,0x03,0x14]
clr.d.aq t1, (ct2)
# CHECK-ASM-AND-OBJ: clr.d.rl t2, (ct3)
# CHECK-ASM: encoding: [0xaf,0x33,0x0e,0x12]
clr.d.rl t2, (ct3)
# CHECK-ASM-AND-OBJ: clr.d.aqrl t3, (ct4)
# CHECK-ASM: encoding: [0x2f,0xbe,0x0e,0x16]
clr.d.aqrl t3, (ct4)

# CHECK-ASM-AND-OBJ: clr.c ct0, (ct1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x10]
clr.c ct0, (ct1)
# CHECK-ASM-AND-OBJ: clr.c.aq ct1, (ct2)
# CHECK-ASM: encoding: [0x2f,0xc3,0x03,0x14]
clr.c.aq ct1, (ct2)
# CHECK-ASM-AND-OBJ: clr.c.rl ct2, (ct3)
# CHECK-ASM: encoding: [0xaf,0x43,0x0e,0x12]
clr.c.rl ct2, (ct3)
# CHECK-ASM-AND-OBJ: clr.c.aqrl ct3, (ct4)
# CHECK-ASM: encoding: [0x2f,0xce,0x0e,0x16]
clr.c.aqrl ct3, (ct4)

# CHECK-ASM-AND-OBJ: csc.d t6, t5, (ct4)
# CHECK-ASM: encoding: [0xaf,0xbf,0xee,0x19]
csc.d t6, t5, (ct4)
# CHECK-ASM-AND-OBJ: csc.d.aq t5, t4, (ct3)
# CHECK-ASM: encoding: [0x2f,0x3f,0xde,0x1d]
csc.d.aq t5, t4, (ct3)
# CHECK-ASM-AND-OBJ: csc.d.rl t4, t3, (ct2)
# CHECK-ASM: encoding: [0xaf,0xbe,0xc3,0x1b]
csc.d.rl t4, t3, (ct2)
# CHECK-ASM-AND-OBJ: csc.d.aqrl t3, t2, (ct1)
# CHECK-ASM: encoding: [0x2f,0x3e,0x73,0x1e]
csc.d.aqrl t3, t2, (ct1)

# CHECK-ASM-AND-OBJ: csc.c t6, ct5, (ct4)
# CHECK-ASM: encoding: [0xaf,0xcf,0xee,0x19]
csc.c t6, ct5, (ct4)
# CHECK-ASM-AND-OBJ: csc.c.aq t5, ct4, (ct3)
# CHECK-ASM: encoding: [0x2f,0x4f,0xde,0x1d]
csc.c.aq t5, ct4, (ct3)
# CHECK-ASM-AND-OBJ: csc.c.rl t4, ct3, (ct2)
# CHECK-ASM: encoding: [0xaf,0xce,0xc3,0x1b]
csc.c.rl t4, ct3, (ct2)
# CHECK-ASM-AND-OBJ: csc.c.aqrl t3, ct2, (ct1)
# CHECK-ASM: encoding: [0x2f,0x4e,0x73,0x1e]
csc.c.aqrl t3, ct2, (ct1)

# CHECK-ASM-AND-OBJ: camoswap.d a4, ra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x08]
camoswap.d a4, ra, (cs0)
# CHECK-ASM-AND-OBJ: camoadd.d a1, a2, (ca3)
# CHECK-ASM: encoding: [0xaf,0xb5,0xc6,0x00]
camoadd.d a1, a2, (ca3)
# CHECK-ASM-AND-OBJ: camoxor.d a2, a3, (ca4)
# CHECK-ASM: encoding: [0x2f,0x36,0xd7,0x20]
camoxor.d a2, a3, (ca4)
# CHECK-ASM-AND-OBJ: camoand.d a3, a4, (ca5)
# CHECK-ASM: encoding: [0xaf,0xb6,0xe7,0x60]
camoand.d a3, a4, (ca5)
# CHECK-ASM-AND-OBJ: camoor.d a4, a5, (ca6)
# CHECK-ASM: encoding: [0x2f,0x37,0xf8,0x40]
camoor.d a4, a5, (ca6)
# CHECK-ASM-AND-OBJ: camomin.d a5, a6, (ca7)
# CHECK-ASM: encoding: [0xaf,0xb7,0x08,0x81]
camomin.d a5, a6, (ca7)
# CHECK-ASM-AND-OBJ: camomax.d s7, s6, (cs5)
# CHECK-ASM: encoding: [0xaf,0xbb,0x6a,0xa1]
camomax.d s7, s6, (cs5)
# CHECK-ASM-AND-OBJ: camominu.d s6, s5, (cs4)
# CHECK-ASM: encoding: [0x2f,0x3b,0x5a,0xc1]
camominu.d s6, s5, (cs4)
# CHECK-ASM-AND-OBJ: camomaxu.d s5, s4, (cs3)
# CHECK-ASM: encoding: [0xaf,0xba,0x49,0xe1]
camomaxu.d s5, s4, (cs3)

# CHECK-ASM-AND-OBJ: camoswap.c ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x47,0x14,0x08]
camoswap.c ca4, cra, (cs0)

# CHECK-ASM-AND-OBJ: camoswap.d.aq a4, ra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x0c]
camoswap.d.aq a4, ra, (cs0)
# CHECK-ASM-AND-OBJ: camoadd.d.aq a1, a2, (ca3)
# CHECK-ASM: encoding: [0xaf,0xb5,0xc6,0x04]
camoadd.d.aq a1, a2, (ca3)
# CHECK-ASM-AND-OBJ: camoxor.d.aq a2, a3, (ca4)
# CHECK-ASM: encoding: [0x2f,0x36,0xd7,0x24]
camoxor.d.aq a2, a3, (ca4)
# CHECK-ASM-AND-OBJ: camoand.d.aq a3, a4, (ca5)
# CHECK-ASM: encoding: [0xaf,0xb6,0xe7,0x64]
camoand.d.aq a3, a4, (ca5)
# CHECK-ASM-AND-OBJ: camoor.d.aq a4, a5, (ca6)
# CHECK-ASM: encoding: [0x2f,0x37,0xf8,0x44]
camoor.d.aq a4, a5, (ca6)
# CHECK-ASM-AND-OBJ: camomin.d.aq a5, a6, (ca7)
# CHECK-ASM: encoding: [0xaf,0xb7,0x08,0x85]
camomin.d.aq a5, a6, (ca7)
# CHECK-ASM-AND-OBJ: camomax.d.aq s7, s6, (cs5)
# CHECK-ASM: encoding: [0xaf,0xbb,0x6a,0xa5]
camomax.d.aq s7, s6, (cs5)
# CHECK-ASM-AND-OBJ: camominu.d.aq s6, s5, (cs4)
# CHECK-ASM: encoding: [0x2f,0x3b,0x5a,0xc5]
camominu.d.aq s6, s5, (cs4)
# CHECK-ASM-AND-OBJ: camomaxu.d.aq s5, s4, (cs3)
# CHECK-ASM: encoding: [0xaf,0xba,0x49,0xe5]
camomaxu.d.aq s5, s4, (cs3)

# CHECK-ASM-AND-OBJ: camoswap.c.aq ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x47,0x14,0x0c]
camoswap.c.aq ca4, cra, (cs0)

# CHECK-ASM-AND-OBJ: camoswap.d.rl a4, ra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x0a]
camoswap.d.rl a4, ra, (cs0)
# CHECK-ASM-AND-OBJ: camoadd.d.rl a1, a2, (ca3)
# CHECK-ASM: encoding: [0xaf,0xb5,0xc6,0x02]
camoadd.d.rl a1, a2, (ca3)
# CHECK-ASM-AND-OBJ: camoxor.d.rl a2, a3, (ca4)
# CHECK-ASM: encoding: [0x2f,0x36,0xd7,0x22]
camoxor.d.rl a2, a3, (ca4)
# CHECK-ASM-AND-OBJ: camoand.d.rl a3, a4, (ca5)
# CHECK-ASM: encoding: [0xaf,0xb6,0xe7,0x62]
camoand.d.rl a3, a4, (ca5)
# CHECK-ASM-AND-OBJ: camoor.d.rl a4, a5, (ca6)
# CHECK-ASM: encoding: [0x2f,0x37,0xf8,0x42]
camoor.d.rl a4, a5, (ca6)
# CHECK-ASM-AND-OBJ: camomin.d.rl a5, a6, (ca7)
# CHECK-ASM: encoding: [0xaf,0xb7,0x08,0x83]
camomin.d.rl a5, a6, (ca7)
# CHECK-ASM-AND-OBJ: camomax.d.rl s7, s6, (cs5)
# CHECK-ASM: encoding: [0xaf,0xbb,0x6a,0xa3]
camomax.d.rl s7, s6, (cs5)
# CHECK-ASM-AND-OBJ: camominu.d.rl s6, s5, (cs4)
# CHECK-ASM: encoding: [0x2f,0x3b,0x5a,0xc3]
camominu.d.rl s6, s5, (cs4)
# CHECK-ASM-AND-OBJ: camomaxu.d.rl s5, s4, (cs3)
# CHECK-ASM: encoding: [0xaf,0xba,0x49,0xe3]
camomaxu.d.rl s5, s4, (cs3)

# CHECK-ASM-AND-OBJ: camoswap.c.rl ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x47,0x14,0x0a]
camoswap.c.rl ca4, cra, (cs0)

# CHECK-ASM-AND-OBJ: camoswap.d.aqrl a4, ra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x0e]
camoswap.d.aqrl a4, ra, (cs0)
# CHECK-ASM-AND-OBJ: camoadd.d.aqrl a1, a2, (ca3)
# CHECK-ASM: encoding: [0xaf,0xb5,0xc6,0x06]
camoadd.d.aqrl a1, a2, (ca3)
# CHECK-ASM-AND-OBJ: camoxor.d.aqrl a2, a3, (ca4)
# CHECK-ASM: encoding: [0x2f,0x36,0xd7,0x26]
camoxor.d.aqrl a2, a3, (ca4)
# CHECK-ASM-AND-OBJ: camoand.d.aqrl a3, a4, (ca5)
# CHECK-ASM: encoding: [0xaf,0xb6,0xe7,0x66]
camoand.d.aqrl a3, a4, (ca5)
# CHECK-ASM-AND-OBJ: camoor.d.aqrl a4, a5, (ca6)
# CHECK-ASM: encoding: [0x2f,0x37,0xf8,0x46]
camoor.d.aqrl a4, a5, (ca6)
# CHECK-ASM-AND-OBJ: camomin.d.aqrl a5, a6, (ca7)
# CHECK-ASM: encoding: [0xaf,0xb7,0x08,0x87]
camomin.d.aqrl a5, a6, (ca7)
# CHECK-ASM-AND-OBJ: camomax.d.aqrl s7, s6, (cs5)
# CHECK-ASM: encoding: [0xaf,0xbb,0x6a,0xa7]
camomax.d.aqrl s7, s6, (cs5)
# CHECK-ASM-AND-OBJ: camominu.d.aqrl s6, s5, (cs4)
# CHECK-ASM: encoding: [0x2f,0x3b,0x5a,0xc7]
camominu.d.aqrl s6, s5, (cs4)
# CHECK-ASM-AND-OBJ: camomaxu.d.aqrl s5, s4, (cs3)
# CHECK-ASM: encoding: [0xaf,0xba,0x49,0xe7]
camomaxu.d.aqrl s5, s4, (cs3)

# CHECK-ASM-AND-OBJ: camoswap.c.aqrl ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x47,0x14,0x0e]
camoswap.c.aqrl ca4, cra, (cs0)
