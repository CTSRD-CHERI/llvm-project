# RUN: llvm-mc %s -triple=riscv64 -mattr=+xcheri -show-encoding \
# RUN:     | FileCheck %s

# CHECK: cgetversion     gp, csp
# CHECK: encoding: [0xdb,0x01,0x31,0xff]
CGetVersion x3, c2
# CHECK: csetversion     cra, csp, gp
# CHECK: encoding: [0xdb,0x00,0x31,0x04]
CSetVersion c1, c2, x3
# CHECK: cloadversion    sp, cra
# CHECK: encoding: [0x5b,0x81,0x40,0xff]
CloadVersion x2, c1
# CHECK: cloadversions   sp, cra
# CHECK: encoding: [0x5b,0x81,0x50,0xff]
CloadVersions x2, c1
# CHECK: cstoreversion   ra, ctp
# CHECK: encoding: [0x5b,0x81,0x40,0xfc]
CStoreVersion x1, c4
# CHECK: camocdecversion ra, csp, gp
# CHECK: encoding: [0xdb,0x00,0x31,0x06]
CAmoCDecVersion x1, c2, x3


