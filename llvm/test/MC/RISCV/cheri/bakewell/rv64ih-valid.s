# RUN: llvm-mc %s -triple=riscv64 -mattr=+h,+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -filetype=obj -mattr=+h,+zcheripurecap,+cap-mode -triple riscv64 < %s \
# RUN:     | llvm-objdump --mattr=+h,+zcheripurecap,+cap-mode -M no-aliases -d - \
# RUN:     | FileCheck -check-prefix=CHECK-INST %s

# RUN: not llvm-mc -triple riscv32 -mattr=+h,+zcheripurecap,+cap-mode < %s 2>&1 \
# RUN:     | FileCheck -check-prefix=CHECK-RV32 %s

# CHECK-INST: hlv.wu ca0, (ca1)
# CHECK: encoding: [0x73,0xc5,0x15,0x68]
# CHECK-RV32: :[[@LINE+1]]:1: error: instruction requires the following: RV64I Base Instruction Set{{$}}
hlv.wu   ca0, (ca1)

# CHECK-INST: hlv.wu ca0, (ca1)
# CHECK: encoding: [0x73,0xc5,0x15,0x68]
# CHECK-RV32: :[[@LINE+1]]:1: error: instruction requires the following: RV64I Base Instruction Set{{$}}
hlv.wu   ca0, 0(ca1)

# CHECK-INST: hlv.d ca0, (ca1)
# CHECK: encoding: [0x73,0xc5,0x05,0x6c]
# CHECK-RV32: :[[@LINE+1]]:1: error: instruction requires the following: RV64I Base Instruction Set{{$}}
hlv.d  ca0, (ca1)

# CHECK-INST: hlv.d ca0, (ca1)
# CHECK: encoding: [0x73,0xc5,0x05,0x6c]
# CHECK-RV32: :[[@LINE+1]]:1: error: instruction requires the following: RV64I Base Instruction Set{{$}}
hlv.d  ca0, 0(ca1)

# CHECK-INST: hsv.d ca0, (ca1)
# CHECK: encoding: [0x73,0xc0,0xa5,0x6e]
# CHECK-RV32: :[[@LINE+1]]:1: error: instruction requires the following: RV64I Base Instruction Set{{$}}
hsv.d   ca0, (ca1)

# CHECK-INST: hsv.d ca0, (ca1)
# CHECK: encoding: [0x73,0xc0,0xa5,0x6e]
# CHECK-RV32: :[[@LINE+1]]:1: error: instruction requires the following: RV64I Base Instruction Set{{$}}
hsv.d   ca0, 0(ca1)
