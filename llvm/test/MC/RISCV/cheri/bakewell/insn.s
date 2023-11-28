# RUN: llvm-mc %s -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM %s
# RUN: llvm-mc %s -triple riscv64 -mattr=+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-OBJ %s

.option capmode

target:

# CHECK-ASM: .insn r 51, 0, 6, ca0, ca1, a2
# CHECK-ASM: encoding: [0x33,0x85,0xc5,0x0c]
# CHECK-OBJ: cadd ca0, ca1, a2
.insn r  0x33,  0,  6, ca0, ca1, a2
# CHECK-ASM: .insn r 51, 0, 6, ca0, ca1, a2
# CHECK-ASM: encoding: [0x33,0x85,0xc5,0x0c]
# CHECK-OBJ: cadd ca0, ca1, a2
.insn r  OP,  0,  6, ca0, ca1, a2

# CHECK-ASM: .insn i 27, 2, ca0, ca1, 13
# CHECK-ASM: encoding: [0x1b,0xa5,0xd5,0x00]
# CHECK-OBJ: caddi ca0, ca1, 13
.insn i  0x1b,  2, ca0, ca1, 13
# CHECK-ASM: .insn i 27, 2, ca0, ca1, 13
# CHECK-ASM: encoding: [0x1b,0xa5,0xd5,0x00]
# CHECK-OBJ: caddi ca0, ca1, 13
.insn i  OP_IMM_32,  2, ca0, ca1, 13

# CHECK-ASM: .insn i 103, 0, ca0, 10(ca1)
# CHECK-ASM: encoding: [0x67,0x85,0xa5,0x00]
# CHECK-OBJ: jalr ca0, 10(ca1)
.insn i  0x67,  0, ca0, 10(ca1)
# CHECK-ASM: .insn i 103, 0, ca0, 10(ca1)
# CHECK-ASM: encoding: [0x67,0x85,0xa5,0x00]
# CHECK-OBJ: jalr ca0, 10(ca1)
.insn i  JALR,  0, ca0, 10(ca1)

# CHECK-ASM: .insn i 3, 0, a0, 4(ca1)
# CHECK-ASM: encoding: [0x03,0x85,0x45,0x00]
# CHECK-OBJ: lb a0, 4(ca1)
.insn i   0x3,  0, a0, 4(ca1)
# CHECK-ASM: .insn i 3, 0, a0, 4(ca1)
# CHECK-ASM: encoding: [0x03,0x85,0x45,0x00]
# CHECK-OBJ: lb a0, 4(ca1)
.insn i   LOAD,  0, a0, 4(ca1)

# CHECK-ASM: .insn s 35, 0, a0, 4(ca1)
# CHECK-ASM: encoding: [0x23,0x82,0xa5,0x00]
# CHECK-OBJ: sb a0, 4(ca1)
.insn s  0x23,  0, a0, 4(ca1)
# CHECK-ASM: .insn s 35, 0, a0, 4(ca1)
# CHECK-ASM: encoding: [0x23,0x82,0xa5,0x00]
# CHECK-OBJ: sb a0, 4(ca1)
.insn s  STORE,  0, a0, 4(ca1)

# CHECK-ASM: .insn u 23, ca0, 4095
# CHECK-ASM: encoding: [0x17,0xf5,0xff,0x00]
# CHECK-OBJ: auipc ca0, 4095
.insn u  0x17, ca0, 0xfff
# CHECK-ASM: .insn u 23, ca0, 4095
# CHECK-ASM: encoding: [0x17,0xf5,0xff,0x00]
# CHECK-OBJ: auipc ca0, 4095
.insn u  AUIPC, ca0, 0xfff

# CHECK-ASM: .insn j 111, ca0, target
# CHECK-ASM: encoding: [0x6f,0bAAAA0101,A,A]
# CHECK-OBJ: jal ca0, 0x0 <target>
.insn uj 0x6f, ca0, target
# CHECK-ASM: .insn j 111, ca0, target
# CHECK-ASM: encoding: [0x6f,0bAAAA0101,A,A]
# CHECK-OBJ: jal ca0, 0x0 <target>
.insn uj JAL, ca0, target

# CHECK-ASM: .insn j 111, ca0, target
# CHECK-ASM: encoding: [0x6f,0bAAAA0101,A,A]
# CHECK-OBJ: jal ca0, 0x0 <target>
.insn j  0x6f, ca0, target
# CHECK-ASM: .insn j 111, ca0, target
# CHECK-ASM: encoding: [0x6f,0bAAAA0101,A,A]
# CHECK-OBJ: jal ca0, 0x0 <target>
.insn j  JAL, ca0, target

# CHECK-ASM: .insn i 3, 5, t1, -2048(ct2)
# CHECK-ASM: encoding: [0x03,0xd3,0x03,0x80]
# CHECK-OBJ: lhu t1, -2048(ct2)
.insn i 0x3, 0x5, x6, %lo(2048)(c7)
# CHECK-ASM: .insn i 3, 5, t1, -2048(ct2)
# CHECK-ASM: encoding: [0x03,0xd3,0x03,0x80]
# CHECK-OBJ: lhu t1, -2048(ct2)
.insn i LOAD, 0x5, x6, %lo(2048)(c7)
