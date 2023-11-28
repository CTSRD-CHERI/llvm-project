# RUN: llvm-mc %s -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# RUN: llvm-mc %s -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

.option capmode 

# CHECK-ASM-AND-OBJ: csrrw ca0, mtvecc, ca0
# CHECK-ASM: encoding: [0x73,0x15,0x55,0x30]
csrrw ca0, mtvecc, ca0

## -- Should still be able to access standard CSRs
# CHECK-ASM-AND-OBJ: csrrw a0, mcause, a0
# CHECK-ASM: encoding: [0x73,0x15,0x25,0x34]
csrrw a0, mcause, a0

.option nocapmode 

## -- Notcapmode can only access XLEN wide CSR
# CHECK-ASM-AND-OBJ: csrrw a0, mcause, a0
# CHECK-ASM: encoding: [0x73,0x15,0x25,0x34]
csrrw a0, mcause, a0


## -- In nocapmode should still access XLEN wide aliases 

# CHECK-ASM-AND-OBJ: csrrw a0, mtvec, a0
# CHECK-ASM: encoding: [0x73,0x15,0x55,0x30]
csrrw a0, mtvec, a0

## -- In nocapmode can use integer constant for CSR

# CHECK-ASM-AND-OBJ: csrrw a0, 4095, a0
# CHECK-ASM: encoding: [0x73,0x15,0xf5,0xff]
csrrw a0, 4095, a0
