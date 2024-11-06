# RUN: llvm-mc %s -triple=riscv32 -mattr=+zicbom,+zcheripurecap,+cap-mode -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+zicbom,+zcheripurecap,+cap-mode -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zicbom,+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zicbom,+zcheripurecap,+cap-mode -d -r - \
# RUN:     | FileCheck --check-prefix=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zicbom,+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zicbom,+zcheripurecap,+cap-mode -d -r - \
# RUN:     | FileCheck --check-prefix=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: cbo.clean (ct0)
# CHECK-ASM: encoding: [0x0f,0xa0,0x12,0x00]
cbo.clean (ct0)
# CHECK-ASM-AND-OBJ: cbo.clean (ct0)
# CHECK-ASM: encoding: [0x0f,0xa0,0x12,0x00]
cbo.clean 0(ct0)

# CHECK-ASM-AND-OBJ: cbo.flush (ct1)
# CHECK-ASM: encoding: [0x0f,0x20,0x23,0x00]
cbo.flush (ct1)
# CHECK-ASM-AND-OBJ: cbo.flush (ct1)
# CHECK-ASM: encoding: [0x0f,0x20,0x23,0x00]
cbo.flush 0(ct1)

# CHECK-ASM-AND-OBJ: cbo.inval (ct2)
# CHECK-ASM: encoding: [0x0f,0xa0,0x03,0x00]
cbo.inval (ct2)
# CHECK-ASM-AND-OBJ: cbo.inval (ct2)
# CHECK-ASM: encoding: [0x0f,0xa0,0x03,0x00]
cbo.inval 0(ct2)
