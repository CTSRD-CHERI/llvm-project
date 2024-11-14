# RUN: llvm-mc %s -triple=riscv32 -mattr=+m,+zbb,+zba,+zcb,+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+m,+zbb,+zba,+zcb,+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+m,+zbb,+zba,+zcb,+zcheripurecap,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+m,+zbb,+zba,+zcb,+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+m,+zbb,+zba,+zcheripurecap,+cap-mode,+zcb < %s \
# RUN:     | llvm-objdump --mattr=+m,+zbb,+zba,zcb,+zcheripurecap,+cap-mode -M no-aliases -d -r - \
# RUN:     | FileCheck --check-prefixes=CHECK-ASM-AND-OBJ %s
#
# RUN: not llvm-mc -triple riscv32 \
# RUN:     -riscv-no-aliases -show-encoding < %s 2>&1 \
# RUN:     | FileCheck -check-prefixes=CHECK-NO-EXT %s
# RUN: not llvm-mc -triple riscv64 \
# RUN:     -riscv-no-aliases -show-encoding < %s 2>&1 \
# RUN:     | FileCheck -check-prefixes=CHECK-NO-EXT %s

# CHECK-ASM-AND-OBJ: c.lbu a5, 2(ca4)
# CHECK-ASM: encoding: [0x3c,0x83]
# CHECK-NO-EXT: error: instruction requires the following: 'Zcb' (Compressed basic bit manipulation instructions), 'zcheripurecap' (CHERI aware Instructions), Capability Pointer Mode{{$}}
c.lbu a5, 2(ca4)

# CHECK-ASM-AND-OBJ: c.lhu a5, 2(ca4)
# CHECK-ASM: encoding: [0x3c,0x87]
# CHECK-NO-EXT: error: instruction requires the following: 'Zcb' (Compressed basic bit manipulation instructions), 'zcheripurecap' (CHERI aware Instructions), Capability Pointer Mode{{$}}
c.lhu a5, 2(ca4)

# CHECK-ASM-AND-OBJ: c.lh a5, 2(ca4)
# CHECK-ASM: encoding: [0x7c,0x87]
# CHECK-NO-EXT: error: instruction requires the following: 'Zcb' (Compressed basic bit manipulation instructions), 'zcheripurecap' (CHERI aware Instructions), Capability Pointer Mode{{$}}
c.lh a5, 2(ca4)

# CHECK-ASM-AND-OBJ: c.sb a5, 2(ca4)
# CHECK-ASM: encoding: [0x3c,0x8b]
# CHECK-NO-EXT: error: instruction requires the following: 'Zcb' (Compressed basic bit manipulation instructions), 'zcheripurecap' (CHERI aware Instructions), Capability Pointer Mode{{$}}
c.sb a5, 2(ca4)

# CHECK-ASM-AND-OBJ: c.sh a5, 2(ca4)
# CHECK-ASM: encoding: [0x3c,0x8f]
# CHECK-NO-EXT: error: instruction requires the following: 'Zcb' (Compressed basic bit manipulation instructions), 'zcheripurecap' (CHERI aware Instructions), Capability Pointer Mode{{$}}
c.sh a5, 2(ca4)
