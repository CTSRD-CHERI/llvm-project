# RUN: llvm-mc %s -triple=riscv32 -mattr=+c,+xcheri,+cap-mode,+f,+d -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK,CHECK-32 %s
# RUN: llvm-mc %s --defsym RV64=1 -triple=riscv64 -mattr=+c,+xcheri,+cap-mode,+f,+d -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK,CHECK-64 %s

## Check that compressed loads/stores accept GPR base registers in CapMode
# CHECK-32: c.lc ca0, 32(ca1) # encoding: [0x88,0x71]
# CHECK-64: c.lc ca0, 32(ca1) # encoding: [0x88,0x31]
c.ly a0, 32(a1)
# CHECK-32-NEXT: c.sc ca0, 32(ca1) # encoding: [0x88,0xf1]
# CHECK-64-NEXT: c.sc ca0, 32(ca1) # encoding: [0x88,0xb1]
c.sy a0, 32(a1)
# CHECK-32-NEXT: c.lcsp ca0, 32(csp) # encoding: [0x02,0x75]
# CHECK-64-NEXT: c.lcsp ca0, 32(csp) # encoding: [0x02,0x35]
c.lysp a0, 32(sp)
# CHECK-32-NEXT: c.scsp ca0, 32(csp) # encoding: [0x2a,0xf0]
# CHECK-64-NEXT: c.scsp ca0, 32(csp) # encoding: [0x2a,0xb0]
c.sysp a0, 32(sp)
#
## The normal loads/stores/jumps should also accept x registers
#
# CHECK: c.lw a2, 8(ca1) # encoding: [0x90,0x45]
c.lw a2, 8(a1)
# CHECK: c.sw a5, 8(ca1) # encoding: [0x9c,0xc5]
c.sw a5, 8(a1)
# CHECK: c.lwsp a0, 8(csp) # encoding: [0x22,0x45]
c.lwsp a0, 8(sp)
# CHECK: c.swsp a0, 8(csp) # encoding: [0x2a,0xc4]
c.swsp a0, 8(sp)
# CHECK-NEXT: c.jr ca1 # encoding: [0x82,0x85]
c.jr a1
# CHECK-NEXT: c.jalr ca1 # encoding: [0x82,0x95]
c.jalr a1
#
.ifdef RV64
# CHECK-64-NEXT: c.ld a0, 16(ca1) # encoding: [0x88,0x69]
c.ld a0, 16(a1)
# CHECK-64-NEXT: c.sd a0, 16(ca1) # encoding: [0x88,0xe9]
c.sd a0, 16(a1)
# CHECK-64-NEXT: c.ldsp a0, 16(csp) # encoding: [0x42,0x65]
c.ldsp a0, 16(sp)
# CHECK-64-NEXT: c.sdsp a0, 16(csp) # encoding: [0x2a,0xe8]
c.sdsp a0, 16(sp)
.else
# CHECK-32-NEXT: c.fld fa5, 8(ca1) # encoding: [0x9c,0x25]
c.fld fa5, 8(a1)
# CHECK-32-NEXT: c.fsd fa5, 8(ca1) # encoding: [0x9c,0xa5]
c.fsd fa5, 8(a1)
# CHECK-32-NEXT: c.fldsp fa5, 8(csp) # encoding: [0xa2,0x27]
c.fldsp fa5, 8(sp)
# CHECK-32-NEXT: c.fsdsp fa5, 8(csp) # encoding: [0x3e,0xa4]
c.fsdsp fa5, 8(sp)
.endif
