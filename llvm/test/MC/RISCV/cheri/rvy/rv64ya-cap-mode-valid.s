# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+y,+zyhybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+a,+y,+zyhybrid < %s \
# RUN:     | llvm-objdump --mattr=+a,+y,+zyhybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# Tests instructions available in purecap + A, rv64.

.option capmode

# CHECK-ASM-AND-OBJ: amoswap.d a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x08]
amoswap.d a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoswap.d.aq a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x0c]
amoswap.d.aq a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoswap.d.rl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x0a]
amoswap.d.rl a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoswap.d.aqrl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x0e]
amoswap.d.aqrl a1, a2, 0(ca3)

# CHECK-ASM-AND-OBJ: amoadd.d a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x00]
amoadd.d a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoadd.d.aq a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x04]
amoadd.d.aq a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoadd.d.rl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x02]
amoadd.d.rl a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoadd.d.aqrl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x06]
amoadd.d.aqrl a1, a2, 0(ca3)

# CHECK-ASM-AND-OBJ: amoxor.d a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x20]
amoxor.d a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoxor.d.aq a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x24]
amoxor.d.aq a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoxor.d.rl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x22]
amoxor.d.rl a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoxor.d.aqrl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x26]
amoxor.d.aqrl a1, a2, 0(ca3)

# CHECK-ASM-AND-OBJ: amoadd.d a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x00]
amoadd.d a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoadd.d.aq a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x04]
amoadd.d.aq a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoadd.d.rl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x02]
amoadd.d.rl a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoadd.d.aqrl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x06]
amoadd.d.aqrl a1, a2, 0(ca3)

# CHECK-ASM-AND-OBJ: amoor.d a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x40]
amoor.d a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoor.d.aq a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x44]
amoor.d.aq a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoor.d.rl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x42]
amoor.d.rl a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoor.d.aqrl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x46]
amoor.d.aqrl a1, a2, 0(ca3)

# CHECK-ASM-AND-OBJ: amomin.d a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x80]
amomin.d a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amomin.d.aq a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x84]
amomin.d.aq a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amomin.d.rl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x82]
amomin.d.rl a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amomin.d.aqrl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0x86]
amomin.d.aqrl a1, a2, 0(ca3)

# CHECK-ASM-AND-OBJ: amomax.d a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xa0]
amomax.d a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amomax.d.aq a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xa4]
amomax.d.aq a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amomax.d.rl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xa2]
amomax.d.rl a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amomax.d.aqrl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xa6]
amomax.d.aqrl a1, a2, 0(ca3)

# CHECK-ASM-AND-OBJ: amominu.d a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xc0]
amominu.d a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amominu.d.aq a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xc4]
amominu.d.aq a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amominu.d.rl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xc2]
amominu.d.rl a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amominu.d.aqrl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xc6]
amominu.d.aqrl a1, a2, 0(ca3)

# CHECK-ASM-AND-OBJ: amomaxu.d a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xe0]
amomaxu.d a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amomaxu.d.aq a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xe4]
amomaxu.d.aq a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amomaxu.d.rl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xe2]
amomaxu.d.rl a1, a2, 0(ca3)
# CHECK-ASM-AND-OBJ: amomaxu.d.aqrl a1, a2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xb5,0xc6,0xe6]
amomaxu.d.aqrl a1, a2, 0(ca3)

# CHECK-ASM-AND-OBJ: lr.d a1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0x05,0x10]
lr.d a1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.d.aq a1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0x05,0x14]
lr.d.aq a1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.d.rl a1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0x05,0x12]
lr.d.rl a1, 0(ca0)
# CHECK-ASM-AND-OBJ: lr.d.aqrl a1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0x05,0x16]
lr.d.aqrl a1, 0(ca0)

# CHECK-ASM-AND-OBJ: sc.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x18]
sc.d a1, a2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.d.aq a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x1c]
sc.d.aq a1, a2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.d.rl a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x1a]
sc.d.rl a1, a2, 0(ca0)
# CHECK-ASM-AND-OBJ: sc.d.aqrl a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x1e]
sc.d.aqrl a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: lr.y ca1, (ca2)
# CHECK-ASM: # encoding: [0xaf,0x45,0x06,0x10]
lr.y ca1, 0(ca2)
# CHECK-ASM-AND-OBJ: lr.y.aq ca1, (ca2)
# CHECK-ASM: # encoding: [0xaf,0x45,0x06,0x14]
lr.y.aq ca1, 0(ca2)
# CHECK-ASM-AND-OBJ: lr.y.rl ca1, (ca2)
# CHECK-ASM: # encoding: [0xaf,0x45,0x06,0x12]
lr.y.rl ca1, 0(ca2)
# CHECK-ASM-AND-OBJ: lr.y.aqrl ca1, (ca2)
# CHECK-ASM: # encoding: [0xaf,0x45,0x06,0x16]
lr.y.aqrl ca1, 0(ca2)

# CHECK-ASM-AND-OBJ: amoswap.y ca1, ca2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xc5,0xc6,0x08]
amoswap.y ca1, ca2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoswap.y.aq ca1, ca2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xc5,0xc6,0x0c]
amoswap.y.aq ca1, ca2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoswap.y.rl ca1, ca2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xc5,0xc6,0x0a]
amoswap.y.rl ca1, ca2, 0(ca3)
# CHECK-ASM-AND-OBJ: amoswap.y.aqrl ca1, ca2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xc5,0xc6,0x0e]
amoswap.y.aqrl ca1, ca2, 0(ca3)

# CHECK-ASM-AND-OBJ: sc.y a1, ca2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xc5,0xc6,0x18]
sc.y a1, ca2, 0(ca3)
# CHECK-ASM-AND-OBJ: sc.y.aq a1, ca2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xc5,0xc6,0x1c]
sc.y.aq a1, ca2, 0(ca3)
# CHECK-ASM-AND-OBJ: sc.y.rl a1, ca2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xc5,0xc6,0x1a]
sc.y.rl a1, ca2, 0(ca3)
# CHECK-ASM-AND-OBJ: sc.y.aqrl a1, ca2, (ca3)
# CHECK-ASM: # encoding: [0xaf,0xc5,0xc6,0x1e]
sc.y.aqrl a1, ca2, 0(ca3)
