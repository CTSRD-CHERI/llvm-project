# RUN: llvm-mc -triple=riscv32 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding < %s \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+a,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+a,+xcheri,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

## Same test again without the "c" prefix on all lines
# RUN: sed -e 's/^c//' < %s > %t.s
# RUN: llvm-mc -triple=riscv32 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding < %t.s \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+a,+xcheri,+cap-mode < %t.s \
# RUN:     | llvm-objdump --mattr=+a,+xcheri,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: lr.c ct0, (ct1)
# CHECK-ASM: encoding: [0xaf,0x32,0x03,0x10]
clr.c ct0, (ct1)
# CHECK-ASM-AND-OBJ: lr.c.aq ct1, (ct2)
# CHECK-ASM: encoding: [0x2f,0xb3,0x03,0x14]
clr.c.aq ct1, (ct2)
# CHECK-ASM-AND-OBJ: lr.c.rl ct2, (ct3)
# CHECK-ASM: encoding: [0xaf,0x33,0x0e,0x12]
clr.c.rl ct2, (ct3)
# CHECK-ASM-AND-OBJ: lr.c.aqrl ct3, (ct4)
# CHECK-ASM: encoding: [0x2f,0xbe,0x0e,0x16]
clr.c.aqrl ct3, (ct4)

# CHECK-ASM-AND-OBJ: sc.c t6, ct5, (ct4)
# CHECK-ASM: encoding: [0xaf,0xbf,0xee,0x19]
csc.c t6, ct5, (ct4)
# CHECK-ASM-AND-OBJ: sc.c.aq t5, ct4, (ct3)
# CHECK-ASM: encoding: [0x2f,0x3f,0xde,0x1d]
csc.c.aq t5, ct4, (ct3)
# CHECK-ASM-AND-OBJ: sc.c.rl t4, ct3, (ct2)
# CHECK-ASM: encoding: [0xaf,0xbe,0xc3,0x1b]
csc.c.rl t4, ct3, (ct2)
# CHECK-ASM-AND-OBJ: sc.c.aqrl t3, ct2, (ct1)
# CHECK-ASM: encoding: [0x2f,0x3e,0x73,0x1e]
csc.c.aqrl t3, ct2, (ct1)

# CHECK-ASM-AND-OBJ: amoswap.c ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x08]
camoswap.c ca4, cra, (cs0)
# CHECK-ASM-AND-OBJ: amoswap.c.aq ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x0c]
camoswap.c.aq ca4, cra, (cs0)
# CHECK-ASM-AND-OBJ: amoswap.c.rl ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x0a]
camoswap.c.rl ca4, cra, (cs0)
# CHECK-ASM-AND-OBJ: amoswap.c.aqrl ca4, cra, (cs0)
# CHECK-ASM: encoding: [0x2f,0x37,0x14,0x0e]
camoswap.c.aqrl ca4, cra, (cs0)
