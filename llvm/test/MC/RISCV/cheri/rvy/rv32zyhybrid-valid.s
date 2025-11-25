# RUN: llvm-mc %s -triple=riscv32 -mattr=+y,+zyhybrid,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+y,+zyhybrid,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+y,+zyhybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv32 -mattr=+y,+zyhybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+y,+zyhybrid < %s \
# RUN:     | llvm-objdump --mattr=+y,+zyhybrid -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
#
# RUN: llvm-mc %s -triple=riscv64 -mattr=+y,+zyhybrid,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+y,+zyhybrid,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+y,+zyhybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+y,+zyhybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+y,+zyhybrid < %s \
# RUN:     | llvm-objdump --mattr=+y,+zyhybrid -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: ymoder a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x35,0x10]
ymoder a0, ca0
# CHECK-ASM-AND-OBJ: ymoder a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x35,0x10]
gcmode a0, ca0

# CHECK-ASM-AND-OBJ: ymodew ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x75,0xa5,0x0c]
ymodew ca0, ca0, a0
# CHECK-ASM-AND-OBJ: ymodew ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x75,0xa5,0x0c]
scmode ca0, ca0, a0

# CHECK-ASM-AND-OBJ: ymodeswy
# CHECK-ASM: encoding: [0x33,0x10,0x00,0x12]
ymodeswy
# CHECK-ASM-AND-OBJ: ymodeswy
# CHECK-ASM: encoding: [0x33,0x10,0x00,0x12]
modesw.cap

# CHECK-ASM-AND-OBJ: ymodeswi
# CHECK-ASM: encoding: [0x33,0x10,0x00,0x14]
ymodeswi
# CHECK-ASM-AND-OBJ: ymodeswi
# CHECK-ASM: encoding: [0x33,0x10,0x00,0x14]
modesw.int
