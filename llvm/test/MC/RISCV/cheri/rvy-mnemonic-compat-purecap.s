# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-32 %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-64 %s

# CHECK-32:      lc cra, 0(csp) # encoding: [0x83,0x30,0x01,0x00]
# CHECK-64:      lc cra, 0(csp) # encoding: [0x8f,0x20,0x01,0x00]
ly c1, 0(c2)
# CHECK-32-NEXT: sc cra, 0(csp) # encoding: [0x23,0x30,0x11,0x00]
# CHECK-64-NEXT: sc cra, 0(csp) # encoding: [0x23,0x40,0x11,0x00]
sy c1, 0(c2)
# CHECK-32-NEXT: lr.c cra, (csp) # encoding: [0xaf,0x30,0x01,0x10]
# CHECK-64-NEXT: lr.c cra, (csp) # encoding: [0xaf,0x40,0x01,0x10]
lr.y c1, (c2)
# CHECK-32-NEXT: lr.c.aq cra, (csp) # encoding: [0xaf,0x30,0x01,0x14]
# CHECK-64-NEXT: lr.c.aq cra, (csp) # encoding: [0xaf,0x40,0x01,0x14]
lr.y.aq c1, (c2)
# CHECK-32-NEXT: sc.c gp, cgp, (csp) # encoding: [0xaf,0x31,0x31,0x18]
# CHECK-64-NEXT: sc.c gp, cgp, (csp) # encoding: [0xaf,0x41,0x31,0x18]
sc.y x3, c3, (c2)
# CHECK-32-NEXT: amoswap.c cra, cgp, (csp) # encoding: [0xaf,0x30,0x31,0x08]
# CHECK-64-NEXT: amoswap.c cra, cgp, (csp) # encoding: [0xaf,0x40,0x31,0x08]
amoswap.y c1, c3, (c2)
