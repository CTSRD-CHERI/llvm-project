# RUN: llvm-mc %s -triple=riscv32 -mattr=+zicboz,+zcheripurecap,+cap-mode -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+zicboz,+zcheripurecap,+cap-mode -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zicboz,+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zicboz,+zcheripurecap,+cap-mode -d -r - \
# RUN:     | FileCheck --check-prefix=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zicboz,+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zicboz,+zcheripurecap,+cap-mode -d -r - \
# RUN:     | FileCheck --check-prefix=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: cbo.zero (ct0)
# CHECK-ASM: encoding: [0x0f,0xa0,0x42,0x00]
cbo.zero (ct0)
# CHECK-ASM-AND-OBJ: cbo.zero (ct0)
# CHECK-ASM: encoding: [0x0f,0xa0,0x42,0x00]
cbo.zero 0(ct0)
