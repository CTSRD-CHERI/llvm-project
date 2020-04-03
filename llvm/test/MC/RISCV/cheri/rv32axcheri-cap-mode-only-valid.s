# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+a,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+a,+xcheri,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-OBJ,CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: clr.c ct0, (ct1)
# CHECK-ASM: encoding: [0xaf,0x32,0x03,0x10]
clr.c ct0, (ct1)
# CHECK-ASM-AND-OBJ: clr.c.aq ct1, (ct2)
# CHECK-ASM: encoding: [0x2f,0xb3,0x03,0x14]
clr.c.aq ct1, (ct2)
# CHECK-ASM-AND-OBJ: clr.c.rl ct2, (ct3)
# CHECK-ASM: encoding: [0xaf,0x33,0x0e,0x12]
clr.c.rl ct2, (ct3)
# CHECK-ASM-AND-OBJ: clr.c.aqrl ct3, (ct4)
# CHECK-ASM: encoding: [0x2f,0xbe,0x0e,0x16]
clr.c.aqrl ct3, (ct4)

# CHECK-ASM-AND-OBJ: csc.c t6, ct5, (ct4)
# CHECK-ASM: encoding: [0xaf,0xbf,0xee,0x19]
csc.c t6, ct5, (ct4)
# CHECK-ASM-AND-OBJ: csc.c.aq t5, ct4, (ct3)
# CHECK-ASM: encoding: [0x2f,0x3f,0xde,0x1d]
csc.c.aq t5, ct4, (ct3)
# CHECK-ASM-AND-OBJ: csc.c.rl t4, ct3, (ct2)
# CHECK-ASM: encoding: [0xaf,0xbe,0xc3,0x1b]
csc.c.rl t4, ct3, (ct2)
# CHECK-ASM-AND-OBJ: csc.c.aqrl t3, ct2, (ct1)
# CHECK-ASM: encoding: [0x2f,0x3e,0x73,0x1e]
csc.c.aqrl t3, ct2, (ct1)

# CHECK-ASM-AND-OBJ: camoswap.c ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x08]
camoswap.c ca4, cra, (cs0)
# CHECK-ASM-AND-OBJ: camoswap.c.aq ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x0c]
camoswap.c.aq ca4, cra, (cs0)
# CHECK-ASM-AND-OBJ: camoswap.c.rl ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x0a]
camoswap.c.rl ca4, cra, (cs0)
# CHECK-ASM-AND-OBJ: camoswap.c.aqrl ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x0e]
camoswap.c.aqrl ca4, cra, (cs0)
