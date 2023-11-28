# RUN: llvm-mc -triple=riscv32 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding < %s\
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -triple=riscv64 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding < %s\
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+a,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+a,+xcheri,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+a,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+a,+xcheri,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

## Same test again without the "c" prefix on all lines
# RUN: sed -e 's/^c//' < %s > %t.s
# RUN: llvm-mc %t.s -triple=riscv32 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %t.s -triple=riscv64 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+a,+xcheri,+cap-mode < %t.s \
# RUN:     | llvm-objdump --mattr=+a,+xcheri,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+a,+xcheri,+cap-mode < %t.s \
# RUN:     | llvm-objdump --mattr=+a,+xcheri,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: lr.b t0, (ct1)
# CHECK-ASM: encoding: [0xaf,0x02,0x03,0x10]
clr.b t0, (ct1)
# CHECK-ASM-AND-OBJ: lr.b.aq t1, (ct2)
# CHECK-ASM: encoding: [0x2f,0x83,0x03,0x14]
clr.b.aq t1, (ct2)
# CHECK-ASM-AND-OBJ: lr.b.rl t2, (ct3)
# CHECK-ASM: encoding: [0xaf,0x03,0x0e,0x12]
clr.b.rl t2, (ct3)
# CHECK-ASM-AND-OBJ: lr.b.aqrl t3, (ct4)
# CHECK-ASM: encoding: [0x2f,0x8e,0x0e,0x16]
clr.b.aqrl t3, (ct4)

# CHECK-ASM-AND-OBJ: lr.h t0, (ct1)
# CHECK-ASM: encoding: [0xaf,0x12,0x03,0x10]
clr.h t0, (ct1)
# CHECK-ASM-AND-OBJ: lr.h.aq t1, (ct2)
# CHECK-ASM: encoding: [0x2f,0x93,0x03,0x14]
clr.h.aq t1, (ct2)
# CHECK-ASM-AND-OBJ: lr.h.rl t2, (ct3)
# CHECK-ASM: encoding: [0xaf,0x13,0x0e,0x12]
clr.h.rl t2, (ct3)
# CHECK-ASM-AND-OBJ: lr.h.aqrl t3, (ct4)
# CHECK-ASM: encoding: [0x2f,0x9e,0x0e,0x16]
clr.h.aqrl t3, (ct4)

# CHECK-ASM-AND-OBJ: lr.w t0, (ct1)
# CHECK-ASM: encoding: [0xaf,0x22,0x03,0x10]
clr.w t0, (ct1)
# CHECK-ASM-AND-OBJ: lr.w.aq t1, (ct2)
# CHECK-ASM: encoding: [0x2f,0xa3,0x03,0x14]
clr.w.aq t1, (ct2)
# CHECK-ASM-AND-OBJ: lr.w.rl t2, (ct3)
# CHECK-ASM: encoding: [0xaf,0x23,0x0e,0x12]
clr.w.rl t2, (ct3)
# CHECK-ASM-AND-OBJ: lr.w.aqrl t3, (ct4)
# CHECK-ASM: encoding: [0x2f,0xae,0x0e,0x16]
clr.w.aqrl t3, (ct4)

# CHECK-ASM-AND-OBJ: sc.b t6, t5, (ct4)
# CHECK-ASM: encoding: [0xaf,0x8f,0xee,0x19]
csc.b t6, t5, (ct4)
# CHECK-ASM-AND-OBJ: sc.b.aq t5, t4, (ct3)
# CHECK-ASM: encoding: [0x2f,0x0f,0xde,0x1d]
csc.b.aq t5, t4, (ct3)
# CHECK-ASM-AND-OBJ: sc.b.rl t4, t3, (ct2)
# CHECK-ASM: encoding: [0xaf,0x8e,0xc3,0x1b]
csc.b.rl t4, t3, (ct2)
# CHECK-ASM-AND-OBJ: sc.b.aqrl t3, t2, (ct1)
# CHECK-ASM: encoding: [0x2f,0x0e,0x73,0x1e]
csc.b.aqrl t3, t2, (ct1)

# CHECK-ASM-AND-OBJ: sc.h t6, t5, (ct4)
# CHECK-ASM: encoding: [0xaf,0x9f,0xee,0x19]
csc.h t6, t5, (ct4)
# CHECK-ASM-AND-OBJ: sc.h.aq t5, t4, (ct3)
# CHECK-ASM: encoding: [0x2f,0x1f,0xde,0x1d]
csc.h.aq t5, t4, (ct3)
# CHECK-ASM-AND-OBJ: sc.h.rl t4, t3, (ct2)
# CHECK-ASM: encoding: [0xaf,0x9e,0xc3,0x1b]
csc.h.rl t4, t3, (ct2)
# CHECK-ASM-AND-OBJ: sc.h.aqrl t3, t2, (ct1)
# CHECK-ASM: encoding: [0x2f,0x1e,0x73,0x1e]
csc.h.aqrl t3, t2, (ct1)

# CHECK-ASM-AND-OBJ: sc.w t6, t5, (ct4)
# CHECK-ASM: encoding: [0xaf,0xaf,0xee,0x19]
csc.w t6, t5, (ct4)
# CHECK-ASM-AND-OBJ: sc.w.aq t5, t4, (ct3)
# CHECK-ASM: encoding: [0x2f,0x2f,0xde,0x1d]
csc.w.aq t5, t4, (ct3)
# CHECK-ASM-AND-OBJ: sc.w.rl t4, t3, (ct2)
# CHECK-ASM: encoding: [0xaf,0xae,0xc3,0x1b]
csc.w.rl t4, t3, (ct2)
# CHECK-ASM-AND-OBJ: sc.w.aqrl t3, t2, (ct1)
# CHECK-ASM: encoding: [0x2f,0x2e,0x73,0x1e]
csc.w.aqrl t3, t2, (ct1)

# CHECK-ASM-AND-OBJ: amoswap.w a4, ra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x27,0x14,0x08]
camoswap.w a4, ra, (cs0)
# CHECK-ASM-AND-OBJ: amoadd.w a1, a2, (ca3)
# CHECK-ASM: encoding: [0xaf,0xa5,0xc6,0x00]
camoadd.w a1, a2, (ca3)
# CHECK-ASM-AND-OBJ: amoxor.w a2, a3, (ca4)
# CHECK-ASM: encoding: [0x2f,0x26,0xd7,0x20]
camoxor.w a2, a3, (ca4)
# CHECK-ASM-AND-OBJ: amoand.w a3, a4, (ca5)
# CHECK-ASM: encoding: [0xaf,0xa6,0xe7,0x60]
camoand.w a3, a4, (ca5)
# CHECK-ASM-AND-OBJ: amoor.w a4, a5, (ca6)
# CHECK-ASM: encoding: [0x2f,0x27,0xf8,0x40]
camoor.w a4, a5, (ca6)
# CHECK-ASM-AND-OBJ: amomin.w a5, a6, (ca7)
# CHECK-ASM: encoding: [0xaf,0xa7,0x08,0x81]
camomin.w a5, a6, (ca7)
# CHECK-ASM-AND-OBJ: amomax.w s7, s6, (cs5)
# CHECK-ASM: encoding: [0xaf,0xab,0x6a,0xa1]
camomax.w s7, s6, (cs5)
# CHECK-ASM-AND-OBJ: amominu.w s6, s5, (cs4)
# CHECK-ASM: encoding: [0x2f,0x2b,0x5a,0xc1]
camominu.w s6, s5, (cs4)
# CHECK-ASM-AND-OBJ: amomaxu.w s5, s4, (cs3)
# CHECK-ASM: encoding: [0xaf,0xaa,0x49,0xe1]
camomaxu.w s5, s4, (cs3)

# CHECK-ASM-AND-OBJ: amoswap.w.aq a4, ra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x27,0x14,0x0c]
camoswap.w.aq a4, ra, (cs0)
# CHECK-ASM-AND-OBJ: amoadd.w.aq a1, a2, (ca3)
# CHECK-ASM: encoding: [0xaf,0xa5,0xc6,0x04]
camoadd.w.aq a1, a2, (ca3)
# CHECK-ASM-AND-OBJ: amoxor.w.aq a2, a3, (ca4)
# CHECK-ASM: encoding: [0x2f,0x26,0xd7,0x24]
camoxor.w.aq a2, a3, (ca4)
# CHECK-ASM-AND-OBJ: amoand.w.aq a3, a4, (ca5)
# CHECK-ASM: encoding: [0xaf,0xa6,0xe7,0x64]
camoand.w.aq a3, a4, (ca5)
# CHECK-ASM-AND-OBJ: amoor.w.aq a4, a5, (ca6)
# CHECK-ASM: encoding: [0x2f,0x27,0xf8,0x44]
camoor.w.aq a4, a5, (ca6)
# CHECK-ASM-AND-OBJ: amomin.w.aq a5, a6, (ca7)
# CHECK-ASM: encoding: [0xaf,0xa7,0x08,0x85]
camomin.w.aq a5, a6, (ca7)
# CHECK-ASM-AND-OBJ: amomax.w.aq s7, s6, (cs5)
# CHECK-ASM: encoding: [0xaf,0xab,0x6a,0xa5]
camomax.w.aq s7, s6, (cs5)
# CHECK-ASM-AND-OBJ: amominu.w.aq s6, s5, (cs4)
# CHECK-ASM: encoding: [0x2f,0x2b,0x5a,0xc5]
camominu.w.aq s6, s5, (cs4)
# CHECK-ASM-AND-OBJ: amomaxu.w.aq s5, s4, (cs3)
# CHECK-ASM: encoding: [0xaf,0xaa,0x49,0xe5]
camomaxu.w.aq s5, s4, (cs3)

# CHECK-ASM-AND-OBJ: amoswap.w.rl a4, ra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x27,0x14,0x0a]
camoswap.w.rl a4, ra, (cs0)
# CHECK-ASM-AND-OBJ: amoadd.w.rl a1, a2, (ca3)
# CHECK-ASM: encoding: [0xaf,0xa5,0xc6,0x02]
camoadd.w.rl a1, a2, (ca3)
# CHECK-ASM-AND-OBJ: amoxor.w.rl a2, a3, (ca4)
# CHECK-ASM: encoding: [0x2f,0x26,0xd7,0x22]
camoxor.w.rl a2, a3, (ca4)
# CHECK-ASM-AND-OBJ: amoand.w.rl a3, a4, (ca5)
# CHECK-ASM: encoding: [0xaf,0xa6,0xe7,0x62]
camoand.w.rl a3, a4, (ca5)
# CHECK-ASM-AND-OBJ: amoor.w.rl a4, a5, (ca6)
# CHECK-ASM: encoding: [0x2f,0x27,0xf8,0x42]
camoor.w.rl a4, a5, (ca6)
# CHECK-ASM-AND-OBJ: amomin.w.rl a5, a6, (ca7)
# CHECK-ASM: encoding: [0xaf,0xa7,0x08,0x83]
camomin.w.rl a5, a6, (ca7)
# CHECK-ASM-AND-OBJ: amomax.w.rl s7, s6, (cs5)
# CHECK-ASM: encoding: [0xaf,0xab,0x6a,0xa3]
camomax.w.rl s7, s6, (cs5)
# CHECK-ASM-AND-OBJ: amominu.w.rl s6, s5, (cs4)
# CHECK-ASM: encoding: [0x2f,0x2b,0x5a,0xc3]
camominu.w.rl s6, s5, (cs4)
# CHECK-ASM-AND-OBJ: amomaxu.w.rl s5, s4, (cs3)
# CHECK-ASM: encoding: [0xaf,0xaa,0x49,0xe3]
camomaxu.w.rl s5, s4, (cs3)

# CHECK-ASM-AND-OBJ: amoswap.w.aqrl a4, ra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x27,0x14,0x0e]
camoswap.w.aqrl a4, ra, (cs0)
# CHECK-ASM-AND-OBJ: amoadd.w.aqrl a1, a2, (ca3)
# CHECK-ASM: encoding: [0xaf,0xa5,0xc6,0x06]
camoadd.w.aqrl a1, a2, (ca3)
# CHECK-ASM-AND-OBJ: amoxor.w.aqrl a2, a3, (ca4)
# CHECK-ASM: encoding: [0x2f,0x26,0xd7,0x26]
camoxor.w.aqrl a2, a3, (ca4)
# CHECK-ASM-AND-OBJ: amoand.w.aqrl a3, a4, (ca5)
# CHECK-ASM: encoding: [0xaf,0xa6,0xe7,0x66]
camoand.w.aqrl a3, a4, (ca5)
# CHECK-ASM-AND-OBJ: amoor.w.aqrl a4, a5, (ca6)
# CHECK-ASM: encoding: [0x2f,0x27,0xf8,0x46]
camoor.w.aqrl a4, a5, (ca6)
# CHECK-ASM-AND-OBJ: amomin.w.aqrl a5, a6, (ca7)
# CHECK-ASM: encoding: [0xaf,0xa7,0x08,0x87]
camomin.w.aqrl a5, a6, (ca7)
# CHECK-ASM-AND-OBJ: amomax.w.aqrl s7, s6, (cs5)
# CHECK-ASM: encoding: [0xaf,0xab,0x6a,0xa7]
camomax.w.aqrl s7, s6, (cs5)
# CHECK-ASM-AND-OBJ: amominu.w.aqrl s6, s5, (cs4)
# CHECK-ASM: encoding: [0x2f,0x2b,0x5a,0xc7]
camominu.w.aqrl s6, s5, (cs4)
# CHECK-ASM-AND-OBJ: amomaxu.w.aqrl s5, s4, (cs3)
# CHECK-ASM: encoding: [0xaf,0xaa,0x49,0xe7]
camomaxu.w.aqrl s5, s4, (cs3)
