
# With Bitmanip base extension:
# RUN: llvm-mc %s -triple=riscv64 -mattr=+zba,zcheripurecap,+cap-mode -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zba,zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zba,zcheripurecap,+cap-mode -d -r - \
# RUN:     | FileCheck --check-prefix=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: add.uw ct0, t1, ct2
# CHECK-ASM: encoding: [0xbb,0x02,0x73,0x08]
add.uw ct0, t1, ct2
# CHECK-ASM-AND-OBJ: sh1add.uw ct0, t1, ct2
# CHECK-ASM: encoding: [0xbb,0x22,0x73,0x20]
sh1add.uw ct0, t1, ct2
# CHECK-ASM-AND-OBJ: sh2add.uw ct0, t1, ct2
# CHECK-ASM: encoding: [0xbb,0x42,0x73,0x20]
sh2add.uw ct0, t1, ct2
# CHECK-ASM-AND-OBJ: sh3add.uw ct0, t1, ct2
# CHECK-ASM: encoding: [0xbb,0x62,0x73,0x20]
sh3add.uw ct0, t1, ct2
