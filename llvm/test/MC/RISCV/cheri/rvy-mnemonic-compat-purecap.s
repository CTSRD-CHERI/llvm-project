# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK,CHECK-32 %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK,CHECK-64 %s

## Check that RVY mnemonics are accepted and we can use X register names
# CHECK-32:      lc cra, 0(csp) # encoding: [0x83,0x30,0x01,0x00]
# CHECK-64:      lc cra, 0(csp) # encoding: [0x8f,0x20,0x01,0x00]
ly x1, 0(x2)
# CHECK-32-NEXT: sc cra, 0(csp) # encoding: [0x23,0x30,0x11,0x00]
# CHECK-64-NEXT: sc cra, 0(csp) # encoding: [0x23,0x40,0x11,0x00]
sy x1, 0(x2)
# CHECK-32-NEXT: lr.c cra, (csp) # encoding: [0xaf,0x30,0x01,0x10]
# CHECK-64-NEXT: lr.c cra, (csp) # encoding: [0xaf,0x40,0x01,0x10]
lr.y x1, (x2)
# CHECK-32-NEXT: lr.c.aq cra, (csp) # encoding: [0xaf,0x30,0x01,0x14]
# CHECK-64-NEXT: lr.c.aq cra, (csp) # encoding: [0xaf,0x40,0x01,0x14]
lr.y.aq x1, (x2)
# CHECK-32-NEXT: sc.c gp, cgp, (csp) # encoding: [0xaf,0x31,0x31,0x18]
# CHECK-64-NEXT: sc.c gp, cgp, (csp) # encoding: [0xaf,0x41,0x31,0x18]
sc.y x3, x3, (x2)
# CHECK-32-NEXT: amoswap.c cra, cgp, (csp) # encoding: [0xaf,0x30,0x31,0x08]
# CHECK-64-NEXT: amoswap.c cra, cgp, (csp) # encoding: [0xaf,0x40,0x31,0x08]
amoswap.y x1, x3, (x2)

## In purecap mode, standard integer memory access instructions (lb/lw) use capability
## base registers. We test here that unprefixed integer register names (x2) are correctly
## coerced to capability base registers (csp) by the parser, even though these are not new instructions.
# CHECK:      lb ra, 0(csp) # encoding: [0x83,0x00,0x01,0x00]
lb x1, 0(x2)
# CHECK-NEXT: lw ra, 0(csp) # encoding: [0x83,0x20,0x01,0x00]
lw x1, 0(x2)
# CHECK-NEXT: amoadd.w ra, gp, (csp) # encoding: [0xaf,0x20,0x31,0x00]
amoadd.w x1, x3, (x2)

## Same for jump instructions
# CHECK:      jalr cnull, 0(cra) # encoding: [0x67,0x80,0x00,0x00]
jr x1
# CHECK-NEXT: jalr cra, 0(cra) # encoding: [0xe7,0x80,0x00,0x00]
jalr x1, x1
# CHECK-NEXT: jal cra, 20 # encoding: [0xef,0x00,0x40,0x01]
jal x1, 20
