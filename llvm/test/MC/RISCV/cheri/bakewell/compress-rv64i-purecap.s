# RUN: llvm-mc -triple riscv64 -mattr=+c,+zcheripurecap,+cap-mode -show-encoding < %s \
# RUN:   | FileCheck -check-prefixes=CHECK-ALIAS %s
# RUN: llvm-mc -triple riscv64 -mattr=+c,+zcheripurecap,+cap-mode -show-encoding \
# RUN:   -riscv-no-aliases < %s | FileCheck -check-prefixes=CHECK-INST %s
# RUN: llvm-mc -triple riscv64 -mattr=+c,+zcheripurecap,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv64 --mattr=+c,+zcheripurecap,+cap-mode -d - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-ALIAS %s
# RUN: llvm-mc -triple riscv64 -mattr=+c,+zcheripurecap,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv64 --mattr=+c,+zcheripurecap,+cap-mode -d -M no-aliases - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-INST %s

# Tests instructions available in purecap + C rv64 and not available in rv32.

# CHECK-BYTES: e0 7f
# CHECK-ALIAS: ld s0, 248(ca5)
# CHECK-INST: c.cld s0, 248(ca5)
# CHECK: # encoding: [0xe0,0x7f]
ld s0, 248(ca5)

# CHECK-BYTES: a0 e3
# CHECK-ALIAS: sd s0, 64(ca5)
# CHECK-INST: c.csd s0, 64(ca5)
# CHECK: # encoding: [0xa0,0xe3]
sd s0, 64(ca5)

# CHECK-BYTES: 7d 22
# CHEACK-ALIAS: addiw tp, tp, 31
# CHECK-INST: c.addiw  tp, 31
# CHECK: # encoding: [0x7d,0x22]
addiw tp, tp, 31

# CHECK-BYTES: 1d 9c
# CHEACK-ALIAS: subw s0, s0, a5
# CHECK-INST: c.subw s0, a5
# CHECK: # encoding:  [0x1d,0x9c]
subw s0, s0, a5

# CHECK-BYTES: 3d 9c
# CHECK-ALIAS: addw s0, s0, a5
# CHECK-INST: c.addw s0, a5
# CHECK: # encoding: [0x3d,0x9c]
addw s0, s0, a5

# CHECK-BYTES: 3d 9c
# CHECK-ALIAS: addw s0, s0, a5
# CHECK-INST: c.addw s0, a5
# CHECK: # encoding: [0x3d,0x9c]
addw s0, a5, s0

# CHECK-BYTES: ee 70
# CHECK-ALIAS: ld ra, 248(csp)
# CHECK-INST: c.cldcsp ra, 248(csp)
# CHECK: # encoding:  [0xee,0x70]
ld ra, 248(csp)

# CHECK-BYTES: a2 e0
# CHECK-ALIAS: sd s0, 64(csp)
# CHECK-INST: c.csdcsp s0, 64(csp)
# CHECK: # encoding: [0xa2,0xe0]
sd s0, 64(csp)
