# RUN: llvm-mc %s -triple=riscv64 -mattr=+xcheri -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+xcheri < %s \
# RUN:     | llvm-objdump --mattr=+xcheri --riscv-no-aliases -d - \
# RUN:     | FileCheck -check-prefix=CHECK-INST %s

# CHECK-INST: lc cra, 3(sp)
# CHECK: encoding: [0x8f,0x20,0x31,0x00]
lc c1, 3(x2)

# CHECK-INST: sc cra, 3(sp)
# CHECK: encoding: [0xa3,0x41,0x11,0x00]
sc c1, 3(x2)

# CHECK-INST: ld.ddc ra, (sp)
# CHECK: encoding: [0xdb,0x00,0x31,0xfa]
ld.ddc x1, (x2)
# CHECK-INST: lwu.ddc ra, (sp)
# CHECK: encoding: [0xdb,0x00,0x61,0xfa]
lwu.ddc x1, (x2)
# CHECK-INST: lc.ddc cra, (sp)
# CHECK: encoding: [0xdb,0x00,0x71,0xfb]
lc.ddc c1, (x2)

# CHECK-INST: sd.ddc ra, (sp)
# CHECK: encoding: [0xdb,0x01,0x11,0xf8]
sd.ddc x1, (x2)
# CHECK-INST: sc.ddc cra, (sp)
# CHECK: encoding: [0x5b,0x02,0x11,0xf8]
sc.ddc c1, (x2)

# CHECK-INST: ld.cap ra, (csp)
# CHECK: encoding: [0xdb,0x00,0xb1,0xfa]
ld.cap x1, (c2)
# CHECK-INST: lwu.cap ra, (csp)
# CHECK: encoding: [0xdb,0x00,0xe1,0xfa]
lwu.cap x1, (c2)
# CHECK-INST: lc.cap cra, (csp)
# CHECK: encoding: [0xdb,0x00,0xf1,0xfb]
lc.cap c1, (c2)

# CHECK-INST: sd.cap ra, (csp)
# CHECK: encoding: [0xdb,0x05,0x11,0xf8]
sd.cap x1, (c2)
# CHECK-INST: sc.cap cra, (csp)
# CHECK: encoding: [0x5b,0x06,0x11,0xf8]
sc.cap c1, (c2)
