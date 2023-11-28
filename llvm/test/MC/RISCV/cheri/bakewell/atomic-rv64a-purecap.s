# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+a,+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+a,+zcheripurecap,+zcherihybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# Tests instructions available in purecap + A, rv64.

.option capmode

# CHECK-ASM-AND-OBJ: sc.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x18]
sc.d a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: lr.d a1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0x05,0x10]
lr.d a1, 0(ca0)

# CHECK-ASM-AND-OBJ: amoswap.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x08]
amoswap.d a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amoadd.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x00]
amoadd.d a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amoxor.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x20]
amoxor.d a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amoand.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x60]
amoand.d a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amoor.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x40]
amoor.d a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amomin.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0x80]
amomin.d a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amomax.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0xa0]
amomax.d a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amominu.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0xc0]
amominu.d a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amomaxu.d a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x35,0xc5,0xe0]
amomaxu.d a1, a2, 0(ca0)
