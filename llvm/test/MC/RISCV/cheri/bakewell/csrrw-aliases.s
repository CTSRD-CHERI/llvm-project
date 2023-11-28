# RUN: llvm-mc %s -triple=riscv32 -mattr=+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc %s -triple=riscv64 -mattr=+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: csrrs   ca0, dscratch0c, zero
# CHECK-ASM: encoding: [0x73,0x25,0x20,0x7b]
csrr ca0, dscratch0c

# CHECK-ASM-AND-OBJ: csrrs   cnull, dscratch0c, zero
# CHECK-ASM: encoding: [0x73,0x20,0x20,0x7b]
csrr c0 , dscratch0c

# CHECK-ASM-AND-OBJ: csrrw   cnull, dscratch0c, ca0
# CHECK-ASM: encoding: [0x73,0x10,0x25,0x7b]
csrw dscratch0c, ca0

# CHECK-ASM-AND-OBJ: csrrw   cnull, dscratch0c, cnull
# CHECK-ASM: encoding: [0x73,0x10,0x20,0x7b]
csrw dscratch0c, c0

# CHECK-ASM-AND-OBJ: csrrs   cnull, dscratch0c, a0
# CHECK-ASM: encoding: [0x73,0x20,0x25,0x7b]
csrs dscratch0c, a0

# CHECK-ASM-AND-OBJ: csrrs   cnull, dscratch0c, zero
# CHECK-ASM: encoding: [0x73,0x20,0x20,0x7b]
csrs dscratch0c, x0

# CHECK-ASM-AND-OBJ: csrrc   cnull, dscratch0c, a0
# CHECK-ASM: encoding: [0x73,0x30,0x25,0x7b]
csrc dscratch0c, a0

# CHECK-ASM-AND-OBJ: csrrc   cnull, dscratch0c, zero
# CHECK-ASM: encoding: [0x73,0x30,0x20,0x7b]
csrc dscratch0c, x0

# CHECK-ASM-AND-OBJ: csrrwi  cnull, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x50,0x26,0x7b]
csrwi dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrwi  cnull, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x50,0x20,0x7b]
csrwi dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrsi  cnull, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x60,0x26,0x7b]
csrsi dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrsi  cnull, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x60,0x20,0x7b]
csrsi dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrci  cnull, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x70,0x26,0x7b]
csrci dscratch0c, 12 

# CHECK-ASM-AND-OBJ: csrrci  cnull, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x70,0x20,0x7b]
csrci dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrwi  cnull, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x50,0x26,0x7b]
csrw dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrwi  cnull, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x50,0x20,0x7b]
csrw dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrsi  cnull, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x60,0x26,0x7b]
csrs dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrsi  cnull, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x60,0x20,0x7b]
csrs dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrci  cnull, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x70,0x26,0x7b]
csrc dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrci  cnull, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x70,0x20,0x7b]
csrc dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrwi  ca0, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x55,0x26,0x7b]
csrrw ca0, dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrwi  ca0, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x55,0x20,0x7b]
csrrw ca0, dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrwi  cnull, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x50,0x26,0x7b]
csrrw c0 , dscratch0c, 12 

# CHECK-ASM-AND-OBJ: csrrwi  cnull, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x50,0x20,0x7b]
csrrw c0 , dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrsi  ca0, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x65,0x26,0x7b]
csrrs ca0, dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrsi  ca0, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x65,0x20,0x7b]
csrrs ca0, dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrsi  cnull, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x60,0x26,0x7b]
csrrs c0, dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrsi  cnull, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x60,0x20,0x7b]
csrrs c0, dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrci  ca0, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x75,0x26,0x7b]
csrrc ca0, dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrci  ca0, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x75,0x20,0x7b]
csrrc ca0, dscratch0c, 0

# CHECK-ASM-AND-OBJ: csrrci  cnull, dscratch0c, 12
# CHECK-ASM: encoding: [0x73,0x70,0x26,0x7b]
csrrc c0, dscratch0c, 12

# CHECK-ASM-AND-OBJ: csrrci  cnull, dscratch0c, 0
# CHECK-ASM: encoding: [0x73,0x70,0x20,0x7b]
csrrc c0, dscratch0c, 0
