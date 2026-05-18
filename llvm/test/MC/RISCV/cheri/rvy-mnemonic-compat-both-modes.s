# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK %s
# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck --check-prefixes=CHECK %s

# CHECK: cgetperm ra, csp # encoding: [0xdb,0x00,0x01,0xfe]
ypermr x1, x2
# CHECK-NEXT: cgettype ra, csp # encoding: [0xdb,0x00,0x11,0xfe]
ytyper x1, x2
# CHECK-NEXT: cgetbase ra, csp # encoding: [0xdb,0x00,0x21,0xfe]
ybaser x1, x2
# CHECK-NEXT: cgetlen ra, csp # encoding: [0xdb,0x00,0x31,0xfe]
ylenr x1, x2
# CHECK-NEXT: cgettag ra, csp # encoding: [0xdb,0x00,0x41,0xfe]
ytagr x1, x2
# CHECK-NEXT: cgethigh ra, csp # encoding: [0xdb,0x00,0x71,0xff]
yhir x1, x2
# CHECK-NEXT: csetaddr cra, csp, gp # encoding: [0xdb,0x00,0x31,0x20]
yaddrw x1, x2, x3
# CHECK-NEXT: csethigh cra, csp, gp # encoding: [0xdb,0x00,0x31,0x2c]
yhiw x1, x2, x3
# CHECK-NEXT: cincoffset cra, csp, gp # encoding: [0xdb,0x00,0x31,0x22]
yadd x1, x2, x3
# CHECK-NEXT: cincoffset cra, csp, -173 # encoding: [0xdb,0x10,0x31,0xf5]
yaddi x1, x2, -173
# CHECK-NEXT: cincoffset cra, csp, -173 # encoding: [0xdb,0x10,0x31,0xf5]
yadd x1, x2, -173
# CHECK-NEXT: csealentry cra, csp # encoding: [0xdb,0x00,0x11,0xff]
ysentry x1, x2
# CHECK-NEXT: cbuildcap cra, csp, cgp # encoding: [0xdb,0x00,0x31,0x3a]
ybld x1, x2, x3
# CHECK-NEXT: csetbounds cra, csp, gp # encoding: [0xdb,0x00,0x31,0x10]
ybndsrw x1, x2, x3
# CHECK-NEXT: csetboundsexact cra, csp, gp # encoding: [0xdb,0x00,0x31,0x12]
ybndsw x1, x2, x3
# CHECK-NEXT: csetbounds cra, csp, 3029 # encoding: [0xdb,0x20,0x51,0xbd]
ybndsw x1, x2, 0xbd5
# CHECK-NEXT: csetbounds cra, csp, 3029 # encoding: [0xdb,0x20,0x51,0xbd]
ybndswi x1, x2, 0xbd5
# CHECK-NEXT: cmove cra, csp # encoding: [0xdb,0x00,0xa1,0xfe]
ymv x1, x2
# CHECK-NEXT: crepresentablealignmentmask ra, sp # encoding: [0xdb,0x00,0x91,0xfe]
yamask x1, x2
# CHECK-NEXT: ctestsubset ra, csp, cgp # encoding: [0xdb,0x00,0x31,0x40]
yss x1, x2, x3
# CHECK-NEXT: csetequalexact ra, csp, cgp # encoding: [0xdb,0x00,0x31,0x42]
yeq x1, x2, x3
# CHECK-NEXT: xori ra, gp, -1 # encoding: [0x93,0xc0,0xf1,0xff]
# CHECK-NEXT: candperm cra, csp, ra # encoding: [0xdb,0x00,0x11,0x1a]
ypermc x1, x2, x3
# CHECK-NEXT: cgetflags ra, csp # encoding: [0xdb,0x00,0x71,0xfe]
# CHECK-NEXT: xori ra, ra, 1 # encoding: [0x93,0xc0,0x10,0x00]
ymoder x1, x2
# CHECK-NEXT: xori ra, gp, 1 # encoding: [0x93,0xc0,0x11,0x00]
# CHECK-NEXT: csetflags cra, csp, ra # encoding: [0xdb,0x00,0x11,0x1c]
ymodew x1, x2, x3
