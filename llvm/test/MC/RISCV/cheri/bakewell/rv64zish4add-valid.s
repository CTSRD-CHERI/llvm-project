# RUN: llvm-mc %s -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid,+zish4add -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid,+zish4add < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid,+zish4add -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

.option nocapmode

# CHECK-ASM-AND-OBJ: sh4add a0, a1, a2
# CHECK-ASM: encoding: [0x33,0xf5,0xc5,0x20]
sh4add a0, a1, a2

.option capmode

# CHECK-ASM-AND-OBJ: sh4add ca0, a1, ca2
# CHECK-ASM: encoding: [0x33,0xf5,0xc5,0x20]
sh4add ca0, a1, ca2

