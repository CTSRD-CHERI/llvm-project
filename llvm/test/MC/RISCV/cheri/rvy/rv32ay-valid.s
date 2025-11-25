# RUN: llvm-mc %s -triple=riscv32 -mattr=+a,+y -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+y -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+a,+y < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+a,+y -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+a,+y < %s \
# RUN:     | llvm-objdump -M no-aliases --mattr=+a,+y -d - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: lr.y ct0, (t1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x10]
lr.y ct0, (t1)
# CHECK-ASM-AND-OBJ: lr.y ct0, (t1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x10]
lr.c ct0, (t1)
# CHECK-ASM-AND-OBJ: lr.y.aq ct0, (t1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x14]
lr.y.aq ct0, (t1)
# CHECK-ASM-AND-OBJ: lr.y.aq ct0, (t1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x14]
lr.c.aq ct0, (t1)
# CHECK-ASM-AND-OBJ: lr.y.rl ct0, (t1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x12]
lr.y.rl ct0, (t1)
# CHECK-ASM-AND-OBJ: lr.y.rl ct0, (t1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x12]
lr.c.rl ct0, (t1)
# CHECK-ASM-AND-OBJ: lr.y.aqrl ct0, (t1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x16]
lr.y.aqrl ct0, (t1)
# CHECK-ASM-AND-OBJ: lr.y.aqrl ct0, (t1)
# CHECK-ASM: encoding: [0xaf,0x42,0x03,0x16]
lr.c.aqrl ct0, (t1)

# CHECK-ASM-AND-OBJ: sc.y t0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x18]
sc.y t0, ct1, (t2)
# CHECK-ASM-AND-OBJ: sc.y t0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x18]
sc.c t0, ct1, (t2)
# CHECK-ASM-AND-OBJ: sc.y.aq t0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x1c]
sc.y.aq t0, ct1, (t2)
# CHECK-ASM-AND-OBJ: sc.y.aq t0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x1c]
sc.c.aq t0, ct1, (t2)
# CHECK-ASM-AND-OBJ: sc.y.rl t0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x1a]
sc.y.rl t0, ct1, (t2)
# CHECK-ASM-AND-OBJ: sc.y.rl t0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x1a]
sc.c.rl t0, ct1, (t2)
# CHECK-ASM-AND-OBJ: sc.y.aqrl t0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x1e]
sc.y.aqrl t0, ct1, (t2)
# CHECK-ASM-AND-OBJ: sc.y.aqrl t0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x1e]
sc.c.aqrl t0, ct1, (t2)

# CHECK-ASM-AND-OBJ: amoswap.y ct0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x08]
amoswap.y ct0, ct1, (t2)
# CHECK-ASM-AND-OBJ: amoswap.y ct0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x08]
amoswap.c ct0, ct1, (t2)
# CHECK-ASM-AND-OBJ: amoswap.y.aq ct0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x0c]
amoswap.y.aq ct0, ct1, (t2)
# CHECK-ASM-AND-OBJ: amoswap.y.aq ct0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x0c]
amoswap.c.aq ct0, ct1, (t2)
# CHECK-ASM-AND-OBJ: amoswap.y.rl ct0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x0a]
amoswap.y.rl ct0, ct1, (t2)
# CHECK-ASM-AND-OBJ: amoswap.y.rl ct0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x0a]
amoswap.c.rl ct0, ct1, (t2)
# CHECK-ASM-AND-OBJ: amoswap.y.aqrl ct0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x0e]
amoswap.y.aqrl ct0, ct1, (t2)
# CHECK-ASM-AND-OBJ: amoswap.y.aqrl ct0, ct1, (t2)
# CHECK-ASM: encoding: [0xaf,0xc2,0x63,0x0e]
amoswap.c.aqrl ct0, ct1, (t2)
