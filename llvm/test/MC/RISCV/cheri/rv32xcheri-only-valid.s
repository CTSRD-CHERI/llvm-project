# RUN: llvm-mc %s -triple=riscv32 -mattr=+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+xcheri < %s \
# RUN:     | llvm-objdump --riscv-no-aliases --mattr=+xcheri -d - \
# RUN:     | FileCheck -check-prefix=CHECK-INST %s

# CHECK-INST: lc cra, 3(sp)
# CHECK: encoding: [0x83,0x30,0x31,0x00]
lc c1, 3(x2)

# CHECK-INST: sc cra, 3(sp)
# CHECK: encoding: [0xa3,0x31,0x11,0x00]
sc c1, 3(x2)

# CHECK-INST: lc.ddc cra, (sp)
# CHECK: encoding: [0xdb,0x00,0x31,0xfa]
lc.ddc c1, (x2)

# CHECK-INST: sc.ddc cra, (sp)
# CHECK: encoding: [0xdb,0x01,0x11,0xf8]
sc.ddc c1, (x2)

# CHECK-INST: lc.cap cra, (csp)
# CHECK: encoding: [0xdb,0x00,0xb1,0xfa]
lc.cap c1, (c2)

# CHECK-INST: sc.cap cra, (csp)
# CHECK: encoding: [0xdb,0x05,0x11,0xf8]
sc.cap c1, (c2)
