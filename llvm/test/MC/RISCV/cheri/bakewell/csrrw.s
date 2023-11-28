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

# CHECK-ASM-AND-OBJ: csrrw ca0, dpcc, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x15,0x7b]
csrrw ca0, dpcc, ca0 

# CHECK-ASM-AND-OBJ: csrrw ca0, dscratch0c, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x25,0x7b]
csrrw ca0, dscratch0c, ca0

# CHECK-ASM-AND-OBJ: csrrw ca0, dscratch1c, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x35,0x7b]
csrrw ca0, dscratch1c, ca0

# CHECK-ASM-AND-OBJ: csrrw ca0, mtvecc, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x55,0x30]
csrrw ca0, mtvecc, ca0

# CHECK-ASM-AND-OBJ: csrrw ca0, mscratchc, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x05,0x34]
csrrw ca0, mscratchc, ca0

# CHECK-ASM-AND-OBJ: csrrw ca0, mepcc, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x15,0x34]
csrrw ca0, mepcc, ca0

# CHECK-ASM-AND-OBJ: csrrw ca0, stvecc, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x55,0x10]
csrrw ca0, stvecc, ca0

# CHECK-ASM-AND-OBJ: csrrw ca0, sscratchc, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x05,0x14]
csrrw ca0, sscratchc, ca0

# CHECK-ASM-AND-OBJ: csrrw ca0, sepcc, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x15,0x14]
csrrw ca0, sepcc, ca0

# CHECK-ASM-AND-OBJ: csrrw ca0, jvtc, ca0 
# CHECK-ASM: encoding: [0x73,0x15,0x75,0x01]
csrrw ca0, jvtc, ca0

# CHECK-ASM-AND-OBJ: csrrs ca0, mtvecc, zero
# CHECK-ASM: encoding: [0x73,0x25,0x50,0x30]
csrr ca0, mtvecc 

# CHECK-ASM-AND-OBJ: csrrw cnull, mtvecc, ca0 
# CHECK-ASM: encoding: [0x73,0x10,0x55,0x30]
csrw mtvecc, ca0

# CHECK-ASM-AND-OBJ: csrrs ca0, mtvecc, a0 
# CHECK-ASM: encoding: [0x73,0x25,0x55,0x30]
csrrs ca0, mtvecc, a0 

# CHECK-ASM-AND-OBJ: csrrc ca0, mtvecc, a0  
# CHECK-ASM: encoding: [0x73,0x35,0x55,0x30]
csrrc ca0, mtvecc, a0 

# CHECK-ASM-AND-OBJ: csrrwi ca0, mtvecc, 12 
# CHECK-ASM: encoding: [0x73,0x55,0x56,0x30]
csrrwi ca0, mtvecc, 12 

# CHECK-ASM-AND-OBJ: csrrsi ca0, mtvecc, 12
# CHECK-ASM: encoding: [0x73,0x65,0x56,0x30]
csrrsi ca0, mtvecc, 12

# CHECK-ASM-AND-OBJ: csrrci ca0, mtvecc, 12
# CHECK-ASM: encoding: [0x73,0x75,0x56,0x30]
csrrci ca0, mtvecc, 12
