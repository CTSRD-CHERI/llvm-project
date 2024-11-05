# RUN: llvm-mc %s -triple=riscv32 -mattr=+h,+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+h,+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple riscv32 -mattr=+h,+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+h,+zcheripurecap,+cap-mode -M no-aliases -d - \
# RUN:     | FileCheck -check-prefix=CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+h,+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+h,+zcheripurecap,+cap-mode -M no-aliases -d - \
# RUN:     | FileCheck -check-prefix=CHECK-INST %s

# CHECK-INST: hfence.vvma zero, zero
# CHECK: encoding: [0x73,0x00,0x00,0x22]
hfence.vvma zero, zero

# CHECK-INST: hfence.vvma a0, a1
# CHECK: encoding: [0x73,0x00,0xb5,0x22]
hfence.vvma a0, a1

# CHECK-INST: hfence.gvma zero, zero
# CHECK: encoding: [0x73,0x00,0x00,0x62]
hfence.gvma zero, zero

# CHECK-INST: hfence.gvma a0, a1
# CHECK: encoding: [0x73,0x00,0xb5,0x62]
hfence.gvma a0, a1

# CHECK-INST: hlv.b ca0, (ca1)
# CHECK: encoding: [0x73,0xc5,0x05,0x60]
hlv.b   ca0, (ca1)

# CHECK-INST: hlv.bu ca0, (ca1)
# CHECK: encoding: [0x73,0xc5,0x15,0x60]
hlv.bu  ca0, (ca1)

# CHECK-INST: hlv.h ca1, (ca2)
# CHECK: encoding: [0xf3,0x45,0x06,0x64]
hlv.h   ca1, (ca2)

# CHECK-INST: hlv.hu ca1, (ca1)
# CHECK: encoding: [0xf3,0xc5,0x15,0x64]
hlv.hu  ca1, (ca1)

# CHECK-INST: hlvx.hu ca1, (ca2)
# CHECK: encoding: [0xf3,0x45,0x36,0x64]
hlvx.hu ca1, (ca2)

# CHECK-INST: hlv.w ca2, (ca2)
# CHECK: encoding: [0x73,0x46,0x06,0x68]
hlv.w   ca2, (ca2)

# CHECK-INST: hlvx.wu ca2, (ca3)
# CHECK: encoding: [0x73,0xc6,0x36,0x68]
hlvx.wu ca2, (ca3)

# CHECK-INST: hsv.b ca0, (ca1)
# CHECK: encoding: [0x73,0xc0,0xa5,0x62]
hsv.b   ca0, (ca1)

# CHECK-INST: hsv.h ca0, (ca1)
# CHECK: encoding: [0x73,0xc0,0xa5,0x66]
hsv.h   ca0, (ca1)

# CHECK-INST: hsv.w ca0, (ca1)
# CHECK: encoding: [0x73,0xc0,0xa5,0x6a]
hsv.w   ca0, (ca1)

# CHECK-INST: hlv.c ca0, (ca0)
# CHECK: encoding: [0x73,0x45,0x05,0x70]
hlv.c ca0, (ca0)

# CHECK-INST: hsv.c ca0, (ca0)
# CHECK: encoding: [0x73,0x40,0xa5,0x72]
hsv.c ca0, (ca0)

