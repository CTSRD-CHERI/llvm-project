# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM32,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+a,+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+a,+zcheripurecap,+zcherihybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM64,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+a,+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+a,+zcheripurecap,+zcherihybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# Tests instructions available in purecap + A, rv32 and rv64.

.option capmode

# CHECK-ASM-AND-OBJ: sc.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0x18]
sc.w a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: sc.h a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x15,0xc5,0x18]
sc.h a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: sc.b a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x05,0xc5,0x18]
sc.b a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: sc.c a1, ca2, (ca0)
# CHECK-ASM32: # encoding: [0xaf,0x35,0xc5,0x18]
# CHECK-ASM64: # encoding: [0xaf,0x45,0xc5,0x18]
sc.c a1, ca2, 0(ca0)

# CHECK-ASM-AND-OBJ: lr.w a1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0x05,0x10]
lr.w a1, 0(ca0)

# CHECK-ASM-AND-OBJ: lr.h a1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x15,0x05,0x10]
lr.h a1, 0(ca0)

# CHECK-ASM-AND-OBJ: lr.b a1, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x05,0x05,0x10]
lr.b a1, 0(ca0)

# CHECK-ASM-AND-OBJ: lr.c ca1, (ca0)
# CHECK-ASM32: # encoding: [0xaf,0x35,0x05,0x10]
# CHECK-ASM64: # encoding: [0xaf,0x45,0x05,0x10]
lr.c ca1, 0(ca0)

# CHECK-ASM-AND-OBJ: amoswap.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0x08]
amoswap.w a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amoadd.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0x00]
amoadd.w a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amoxor.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0x20]
amoxor.w a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amoand.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0x60]
amoand.w a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amoor.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0x40]
amoor.w a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amomin.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0x80]
amomin.w a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amomax.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0xa0]
amomax.w a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amominu.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0xc0]
amominu.w a1, a2, 0(ca0)

# CHECK-ASM-AND-OBJ: amomaxu.w a1, a2, (ca0)
# CHECK-ASM: # encoding: [0xaf,0x25,0xc5,0xe0]
amomaxu.w a1, a2, 0(ca0)
