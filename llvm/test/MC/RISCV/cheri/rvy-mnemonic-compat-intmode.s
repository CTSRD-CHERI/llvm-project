# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK-32 %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK-64 %s

## Check that RVY mnemonics are accepted and we can use X register names
# CHECK-32: lc cra, 0(sp) # encoding: [0x83,0x30,0x01,0x00]
# CHECK-64: lc cra, 0(sp) # encoding: [0x8f,0x20,0x01,0x00]
ly x1, 0(x2)
# CHECK-32-NEXT: sc cra, 0(sp) # encoding: [0x23,0x30,0x11,0x00]
# CHECK-64-NEXT: sc cra, 0(sp) # encoding: [0x23,0x40,0x11,0x00]
sy x1, 0(x2)
# CHECK-32-NEXT: lr.c cra, (sp) # encoding: [0xaf,0x30,0x01,0x10]
# CHECK-64-NEXT: lr.c cra, (sp) # encoding: [0xaf,0x40,0x01,0x10]
lr.y x1, (x2)
# CHECK-32-NEXT: lr.c.aq cra, (sp) # encoding: [0xaf,0x30,0x01,0x14]
# CHECK-64-NEXT: lr.c.aq cra, (sp) # encoding: [0xaf,0x40,0x01,0x14]
lr.y.aq x1, (x2)
# CHECK-32-NEXT: sc.c gp, cgp, (sp) # encoding: [0xaf,0x31,0x31,0x18]
# CHECK-64-NEXT: sc.c gp, cgp, (sp) # encoding: [0xaf,0x41,0x31,0x18]
sc.y x3, x3, (x2)
# CHECK-32-NEXT: amoswap.c cra, cgp, (sp) # encoding: [0xaf,0x30,0x31,0x08]
# CHECK-64-NEXT: amoswap.c cra, cgp, (sp) # encoding: [0xaf,0x40,0x31,0x08]
amoswap.y x1, x3, (x2)

## Check that ABI register names work as expected
# CHECK-32:      lc cra, 0(sp) # encoding: [0x83,0x30,0x01,0x00]
# CHECK-64:      lc cra, 0(sp) # encoding: [0x8f,0x20,0x01,0x00]
ly ra, 0(sp)
# CHECK-32-NEXT: sc cra, 0(sp) # encoding: [0x23,0x30,0x11,0x00]
# CHECK-64-NEXT: sc cra, 0(sp) # encoding: [0x23,0x40,0x11,0x00]
sy ra, 0(sp)
