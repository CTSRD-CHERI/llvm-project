# RUN: llvm-mc %s -triple=riscv32 -mattr=+zcheripurecap -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc %s -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc %s -triple=riscv64 -mattr=+zcheripurecap -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zcheripurecap < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc %s -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: addi a0, a1, 0
# CHECK-ASM: encoding: [0x13,0x85,0x05,0x00]
cgetaddr a0, ca1

