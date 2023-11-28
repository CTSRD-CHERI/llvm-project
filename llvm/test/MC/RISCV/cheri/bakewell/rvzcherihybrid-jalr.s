# RUN: llvm-mc %s -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid,+zcheri-mode -riscv-no-aliases -show-encoding \
# RUN:  | FileCheck %s -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ

# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid,+zcheri-mode < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid,+zcheri-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc %s -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid,+zcheri-mode -riscv-no-aliases -show-encoding \
# RUN:  | FileCheck %s -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ

# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid,+zcheri-mode < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid,+zcheri-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

#### Check the non capmode instructions ###
.option nocapmode

# CHECK-ASM-AND-OBJ: jalr.mode ca0, 0(ca1)
# CHECK-ASM: encoding: [0x67,0x95,0x05,0x00]
jalr ca0, 0(ca1)

# CHECK-ASM-AND-OBJ: jalr.mode ca0, 0(ca1)
# CHECK-ASM: encoding: [0x67,0x95,0x05,0x00]
jalr ca0, ca1

# CHECK-ASM-AND-OBJ: jalr.mode cra, 0(ca1)
# CHECK-ASM: encoding: [0xe7,0x90,0x05,0x00]
jalr ca1

# CHECK-ASM-AND-OBJ: jalr.mode cnull, 0(ca1)
# CHECK-ASM: encoding: [0x67,0x90,0x05,0x00]
jr ca1

# CHECK-ASM-AND-OBJ: jalr.mode cnull, 0(cra)
# CHECK-ASM: encoding: [0x67,0x90,0x00,0x00]
ret.mode

# CHECK-ASM-AND-OBJ: jalr a0, 0(a0)
# CHECK-ASM: encoding: [0x67,0x05,0x05,0x00]
jalr a0, 0(a0)

# CHECK-ASM-AND-OBJ: jalr a0, 42(a0)
# CHECK-ASM: encoding: [0x67,0x05,0xa5,0x02]
jalr a0, 42(a0)

# CHECK-ASM-AND-OBJ: jalr a0, 0(a0)
# CHECK-ASM: encoding: [0x67,0x05,0x05,0x00]
jalr a0, a0

# CHECK-ASM-AND-OBJ: jalr ra, 0(a0)
# CHECK-ASM: encoding: [0xe7,0x00,0x05,0x00]
jalr a0 

# CHECK-ASM-AND-OBJ: jalr zero, 0(a0)
# CHECK-ASM: encoding: [0x67,0x00,0x05,0x00]
jr a0

# CHECK-ASM-AND-OBJ: jalr zero, 0(ra)
# CHECK-ASM: encoding: [0x67,0x80,0x00,0x00]
ret


#### Check the capmode instructions ###

.option capmode

# CHECK-ASM-AND-OBJ: jalr ca0, 0(ca1)
# CHECK-ASM: encoding: [0x67,0x85,0x05,0x00]
jalr ca0, 0(ca1)

# CHECK-ASM-AND-OBJ: jalr ca0, 42(ca0)
# CHECK-ASM: encoding: [0x67,0x05,0xa5,0x02]
jalr ca0, 42(ca0)

# CHECK-ASM-AND-OBJ: jalr ca0, 0(ca1)
# CHECK-ASM: encoding: [0x67,0x85,0x05,0x00]
jalr ca0, ca1

# CHECK-ASM-AND-OBJ: jalr cra, 0(ca1)
# CHECK-ASM: encoding: [0xe7,0x80,0x05,0x00]
jalr ca1

# CHECK-ASM-AND-OBJ: jalr cnull, 0(ca1)
# CHECK-ASM: encoding: [0x67,0x80,0x05,0x00]
jr ca1

# CHECK-ASM-AND-OBJ: jalr cnull, 0(cra)
# CHECK-ASM: encoding: [0x67,0x80,0x00,0x00]
ret

# CHECK-ASM-AND-OBJ: jalr.mode a0, 0(a0)
# CHECK-ASM: encoding: [0x67,0x15,0x05,0x00]
jalr a0, 0(a0)

# CHECK-ASM-AND-OBJ: jalr.mode a0, 0(a0)
# CHECK-ASM: encoding: [0x67,0x15,0x05,0x00]
jalr a0, a0

# CHECK-ASM-AND-OBJ: jalr.mode ra, 0(a0)
# CHECK-ASM: encoding: [0xe7,0x10,0x05,0x00]
jalr a0 

# CHECK-ASM-AND-OBJ: jalr.mode zero, 0(a0)
# CHECK-ASM: encoding: [0x67,0x10,0x05,0x00]
jr a0 

# CHECK-ASM-AND-OBJ: jalr.mode zero, 0(ra)
# CHECK-ASM: encoding: [0x67,0x90,0x00,0x00]
ret.mode
