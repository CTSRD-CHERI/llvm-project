# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+y,+zyhybrid,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+a,+y,+zyhybrid,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+a,+y,+zyhybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+y,+zyhybrid,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+a,+y,+zyhybrid,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+a,+y,+zyhybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# Tests instructions available in purecap + A, rv32 and rv64.

# CHECK-ASM-AND-OBJ: lr.y ca1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0x05,0x10]
lr.y ca1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.y.aq ca1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0x05,0x14]
lr.y.aq ca1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.y.rl ca1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0x05,0x12]
lr.y.rl ca1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.y.aqrl ca1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0x05,0x16]
lr.y.aqrl ca1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.y ca1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0x05,0x10]
lr.c ca1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.y.aq ca1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0x05,0x14]
lr.c.aq ca1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.y.rl ca1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0x05,0x12]
lr.c.rl ca1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.y.aqrl ca1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0x05,0x16]
lr.c.aqrl ca1, 0(ca0)

# CHECK-ASM-AND-OBJ: amoswap.y ca1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x08]
amoswap.y ca1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: amoswap.y.aq ca1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x0c]
amoswap.y.aq ca1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: amoswap.y.rl ca1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x0a]
amoswap.y.rl ca1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: amoswap.y.aqrl ca1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x0e]
amoswap.y.aqrl ca1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: amoswap.y ca1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x08]
amoswap.c ca1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: amoswap.y.aq ca1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x0c]
amoswap.c.aq ca1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: amoswap.y.rl ca1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x0a]
amoswap.c.rl ca1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: amoswap.y.aqrl ca1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x0e]
amoswap.c.aqrl ca1, ca2, 0(ca0)

# CHECK-ASM-AND-OBJ: sc.y a1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x18]
sc.y a1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.y.aq a1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x1c]
sc.y.aq a1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.y.rl a1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x1a]
sc.y.rl a1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.y.aqrl a1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x1e]
sc.y.aqrl a1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.y a1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x18]
sc.c a1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.y.aq a1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x1c]
sc.c.aq a1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.y.rl a1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x1a]
sc.c.rl a1, ca2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.y.aqrl a1, ca2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x45,0xc5,0x1e]
sc.c.aqrl a1, ca2, 0(ca0)
